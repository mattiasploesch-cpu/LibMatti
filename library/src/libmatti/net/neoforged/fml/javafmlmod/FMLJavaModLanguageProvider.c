// Port of net.neoforged.fml.javafmlmod.FMLJavaModLanguageProvider.

#include "libmatti/net/neoforged/fml/javafmlmod/FMLJavaModLanguageProvider.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/javafmlmod/AutomaticEventSubscriber.h"
#include "libmatti/net/neoforged/fml/javafmlmod/FMLModContainer.h"
#include "libmatti/net/neoforged/fml/loading/BuiltInLanguageLoader.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER / Type MOD_TYPE = Type.getType(Mod.class)
#define MOD_TYPE "net.neoforged.fml.common.Mod"

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

static LIBMATTI_NEOFORGESPI_IModLanguageLoader languageLoader;

// Java: public String name() { return NAME; }
static const char *provider_name(void *self)
{
    (void)self;
    return LIBMATTI_FML_FMLJavaModLanguageProvider_NAME;
}

// Java: public String version() (BuiltInLanguageLoader)
static const char *provider_version(void *self)
{
    (void)self;
    return LIBMATTI_FML_BuiltInLanguageLoader_Version();
}

// Java: data.annotationData().get(key)
static const char *annotation_value(const LIBMATTI_NEOFORGESPI_AnnotationData *data, const char *key)
{
    for (size_t i = 0; i < data->annotationDataCount; i++)
        if (strcmp(data->annotationDataKeys[i], key) == 0) return data->annotationDataValues[i];

    return NULL;
}

// Java: private static List<String> getDepends(ModFileScanData.AnnotationData data)
// Java's annotation value is a List<String>; the port's annotation values are strings, so the array is
// carried as the same delimited form AutomaticEventSubscriber.getSides reads.
size_t LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(const char *data, const char **depends, size_t capacity)
{
    // Java: return depends != null ? (List<String>) depends : Collections.emptyList();
    if (data == NULL) return 0;

    size_t count = 0;
    const char *cursor = data;
    while (*cursor != '\0')
    {
        while (*cursor == ',' || *cursor == ';' || *cursor == ' ') cursor++;
        if (*cursor == '\0') break;

        const char *end = cursor;
        while (*end != '\0' && *end != ',' && *end != ';') end++;

        size_t length = (size_t)(end - cursor);
        while (length > 0 && cursor[length - 1] == ' ') length--;

        if (count < capacity && length > 0)
        {
            char *value = malloc(length + 1);
            memcpy(value, cursor, length);
            value[length] = '\0';
            depends[count] = value;
        }
        count++;

        cursor = end;
    }

    return count;
}

// Java: FMLLoader.getCurrent().getLoadingModList().getModFileById(otherMod) != null
static int depends_are_loaded(const char *data, LIBMATTI_FML_ModList *modList)
{
    size_t count = LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(data, NULL, 0);
    if (count == 0) return 1;

    const char **depends = calloc(count, sizeof(char *));
    LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(data, depends, count);

    int loaded = 1;
    for (size_t i = 0; i < count; i++)
    {
        if (LIBMATTI_FML_ModList_GetModFileById(modList, depends[i]) == NULL)
        {
            loaded = 0;
            break;
        }
    }

    for (size_t i = 0; i < count; i++)
        free((void *)depends[i]);
    free(depends);
    return loaded;
}

// Java: public ModContainer loadMod(IModInfo info, ModFileScanData modFileScanResults, ModuleLayer layer)
static void *provider_load_mod(void *self, void *infoData, void *modFileScanResults, LIBMATTI_JL_ModuleLayer *layer)
{
    (void)self;
    (void)layer;

    LIBMATTI_NEOFORGESPI_IModInfo *info = infoData;
    LIBMATTI_NEOFORGESPI_ModFileScanData *scanData = modFileScanResults;

    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrent();
    LIBMATTI_DIST_Dist dist = LIBMATTI_FML_FMLLoader_GetDist(loader);
    LIBMATTI_FML_ModList *modList = LIBMATTI_FML_FMLLoader_GetLoadingModList(loader);
    const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(info);

    // Java: scanData.getAnnotatedBy(Mod.class, ElementType.TYPE).filter(...)
    LIBMATTI_NEOFORGESPI_AnnotationData **annotations = NULL;
    size_t annotationCount = 0;
    if (scanData != NULL)
        annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
            scanData, MOD_TYPE, LIBMATTI_NEOFORGESPI_ElementType_TYPE, &annotationCount);

    // Java: the filters keep the annotation count stable, so the port keeps them in place
    LIBMATTI_NEOFORGESPI_AnnotationData **candidates = calloc(annotationCount > 0 ? annotationCount : 1,
                                                             sizeof(void *));
    size_t candidateCount = 0;
    for (size_t i = 0; i < annotationCount; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[i];

        const char *value = annotation_value(data, "value");
        if (value == NULL || strcmp(value, modId) != 0) continue;

        LIBMATTI_DIST_Dist sides[2];
        size_t sideCount = LIBMATTI_FML_AutomaticEventSubscriber_GetSides(annotation_value(data, "dist"), sides, 2);
        int hasSide = 0;
        for (size_t j = 0; j < sideCount; j++)
            if (sides[j] == dist) hasSide = 1;
        if (!hasSide) continue;

        if (!depends_are_loaded(annotation_value(data, "depends"), modList)) continue;

        candidates[candidateCount++] = data;
    }

    // Java: .sorted(comparingInt(ad -> getDepends(ad).size()).thenComparingInt(ad -> -getSides(...).size()))
    for (size_t i = 1; i < candidateCount; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData *current = candidates[i];
        size_t currentDepends = LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(
            annotation_value(current, "depends"), NULL, 0);

        size_t j = i;
        while (j > 0)
        {
            size_t otherDepends = LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(
                annotation_value(candidates[j - 1], "depends"), NULL, 0);
            int cmp = currentDepends < otherDepends ? -1 : (currentDepends > otherDepends ? 1 : 0);
            if (cmp == 0)
            {
                LIBMATTI_DIST_Dist currentSides[2];
                LIBMATTI_DIST_Dist otherSides[2];
                size_t currentSideCount = LIBMATTI_FML_AutomaticEventSubscriber_GetSides(
                    annotation_value(current, "dist"), currentSides, 2);
                size_t otherSideCount = LIBMATTI_FML_AutomaticEventSubscriber_GetSides(
                    annotation_value(candidates[j - 1], "dist"), otherSides, 2);
                cmp = currentSideCount > otherSideCount ? -1 : (currentSideCount < otherSideCount ? 1 : 0);
            }

            if (cmp >= 0) break;

            candidates[j] = candidates[j - 1];
            j--;
        }
        candidates[j] = current;
    }

    // Java: .map(ad -> ad.clazz().getClassName()).toList()
    const char **modClasses = calloc(candidateCount > 0 ? candidateCount : 1, sizeof(char *));
    for (size_t i = 0; i < candidateCount; i++)
    {
        char *className = LIBMATTI_ASM_Type_GetClassName(candidates[i]->clazz);
        modClasses[i] = className;
    }

    // Java: return new FMLModContainer(info, modClasses, modFileScanResults, layer);
    // gameLayer.findModule(info.getOwningFile().getFile().getId())
    LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(
        LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(info));
    const char *gameLayer = file != NULL ? LIBMATTI_NEOFORGESPI_IModFile_GetId(file) : NULL;

    void *container = LIBMATTI_FML_FMLModContainer_New(info, modClasses, candidateCount, scanData, gameLayer);

    for (size_t i = 0; i < candidateCount; i++)
        free((void *) modClasses[i]);
    free(modClasses);
    free(candidates);
    free(annotations);
    return container;
}

// Java: @Override public void validate(IModFile file, Collection<ModContainer> loadedContainers, IIssueReporting reporter)
static void provider_validate(void *self, void *fileData, void **loadedContainers, size_t loadedContainerCount,
                              LIBMATTI_NEOFORGESPI_IIssueReporting *reporter)
{
    (void)loadedContainers;
    (void)loadedContainerCount;
    (void)self;

    LIBMATTI_NEOFORGESPI_IModFile *file = fileData;

    // Java: Set<String> modIds = the @Mod ids of the infos whose loader is this loader
    size_t modInfoCount = 0;
    LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(file, &modInfoCount);

    char **modIds = calloc(modInfoCount > 0 ? modInfoCount : 1, sizeof(char *));
    size_t modIdCount = 0;
    for (size_t i = 0; i < modInfoCount; i++)
    {
        if (LIBMATTI_NEOFORGESPI_IModInfo_GetLoader(modInfos[i]) != (void *)self) continue;
        modIds[modIdCount++] = (char *)LIBMATTI_NEOFORGESPI_IModInfo_GetModId(modInfos[i]);
    }

    size_t annotationCount = 0;
    LIBMATTI_NEOFORGESPI_AnnotationData **annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        LIBMATTI_NEOFORGESPI_IModFile_GetScanResult(file), MOD_TYPE, LIBMATTI_NEOFORGESPI_ElementType_TYPE,
        &annotationCount);

    for (size_t i = 0; i < annotationCount; i++)
    {
        const char *modId = annotation_value(annotations[i], "value");

        int known = 0;
        for (size_t j = 0; j < modIdCount; j++)
            if (strcmp(modIds[j], modId) == 0) known = 1;
        if (known) continue;

        // Java: ModLoadingIssue.error("fml.modloadingissue.javafml.dangling_entrypoint", modId, entrypointClass, file.getFilePath()).withAffectedModFile(file)
        char *entrypointClass = LIBMATTI_ASM_Type_GetClassName(annotations[i]->clazz);
        const char *args[] = {modId, entrypointClass, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file)};
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.javafml.dangling_entrypoint", args, 3);
        LIBMATTI_FML_ModLoadingIssue *withFile = LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(
            issue, file, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file));
        LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(reporter, withFile);
        free(entrypointClass);
    }

    free(modIds);
    free(annotations);
}

// Java: the instance used as an IModLanguageLoader
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_FMLJavaModLanguageProvider_AsLanguageLoader(void)
{
    // Java: modInfo.getLoader() == this - the port compares against the language loader itself
    languageLoader.self = &languageLoader;
    languageLoader.name = provider_name;
    languageLoader.version = provider_version;
    languageLoader.loadMod = provider_load_mod;
    languageLoader.validate = provider_validate;
    return &languageLoader;
}
