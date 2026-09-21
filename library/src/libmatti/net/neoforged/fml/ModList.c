// Port of net.neoforged.fml.ModList.

#include "libmatti/net/neoforged/fml/ModList.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/net/neoforged/fml/CrashReportCallables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Java: private static ModList INSTANCE;
static LIBMATTI_FML_ModList *INSTANCE = NULL;

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: mf.getModInfos().getFirst()
static LIBMATTI_NEOFORGESPI_IModInfo *first_mod_info(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    size_t count;
    LIBMATTI_NEOFORGESPI_IModInfo **mods = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(modFile, &count);
    // Java returns the impl's own list, so the array must not be freed here
    if (count == 0)
    {
        // Java: List.getFirst() throws NoSuchElementException
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "No mod infos in mod file {}",
                                 LIBMATTI_NEOFORGESPI_IModFile_GetFileName(modFile));
        return NULL;
    }

    return mods[0];
}

// Java: Comparator.comparing(modFile -> modFile.getModInfos().getFirst().getDisplayName(), String.CASE_INSENSITIVE_ORDER)
static int compare_display_names(const LIBMATTI_NEOFORGESPI_IModFile *a, const LIBMATTI_NEOFORGESPI_IModFile *b)
{
    LIBMATTI_NEOFORGESPI_IModInfo *modA = first_mod_info(a);
    LIBMATTI_NEOFORGESPI_IModInfo *modB = first_mod_info(b);
    if (modA == NULL || modB == NULL) return 0;

    return strcasecmp(LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(modA),
                      LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(modB));
}

// Java: modFiles.stream().map(IModFileInfo::getFile)
//         .sorted(<Comparator above>).map(function)
// Stream.sorted() is stable, hence the insertion sort.
static LIBMATTI_NEOFORGESPI_IModFile **sorted_mod_files(const LIBMATTI_FML_ModList *modList, size_t *count)
{
    size_t size = modList->modFileCount;
    LIBMATTI_NEOFORGESPI_IModFile **files = malloc(sizeof(*files) * (size + 1));
    for (size_t i = 0; i < size; i++)
        files[i] = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(modList->modFiles[i]);

    for (size_t i = 1; i < size; i++)
    {
        LIBMATTI_NEOFORGESPI_IModFile *current = files[i];
        size_t j = i;
        while (j > 0 && compare_display_names(files[j - 1], current) > 0)
        {
            files[j] = files[j - 1];
            j--;
        }
        files[j] = current;
    }

    *count = size;
    return files;
}

// Java: sortedList.indexOf(modInfo) - -1 when the mod info is not in the list
static int sorted_index(const LIBMATTI_FML_ModList *modList, const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    for (size_t i = 0; i < modList->sortedCount; i++)
        if (modList->sortedList[i] == modInfo) return (int)i;

    return -1;
}

// Java: String.format(Locale.ENGLISH, "%-50.50s|%-30.30s|%-30.30s|%-20.20s|Manifest: %s", ...)
char *LIBMATTI_FML_ModList_FileToLine(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    LIBMATTI_NEOFORGESPI_IModInfo *mainMod = first_mod_info(modFile);
    if (mainMod == NULL) return strdup("");

    const LIBMATTI_NEOFORGESPI_ArtifactVersion *version = LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(mainMod);

    // Java: ((ModFileInfo) mf.getModFileInfo()).getCodeSigningFingerprint().orElse("NOSIGNATURE")
    LIBMATTI_FML_ModFileInfo *modFileInfo =
        (LIBMATTI_FML_ModFileInfo *)LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(modFile);
    char *fingerprint = NULL;
    const char *manifest = LIBMATTI_FML_ModFileInfo_GetCodeSigningFingerprint(modFileInfo, &fingerprint)
                               ? fingerprint
                               : "NOSIGNATURE";

    size_t size = 50 + 30 + 30 + 20 + strlen(manifest) + 32;
    char *line = malloc(size);
    snprintf(line, size, "%-50.50s|%-30.30s|%-30.30s|%-20.20s|Manifest: %s",
             LIBMATTI_NEOFORGESPI_IModFile_GetFileName(modFile),
             LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(mainMod),
             LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mainMod),
             version != NULL && version->version != NULL ? version->version : "",
             manifest);

    free(fingerprint);
    return line;
}

// Java: CrashReportCallables.registerCrashCallable("Mod List", this::crashReport)
static char *crash_report_generator(void *userdata)
{
    return LIBMATTI_FML_ModList_CrashReport(userdata);
}

static void add_file_by_id(LIBMATTI_FML_ModList *modList, const char *modId,
                           LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    // Java: Collectors.toUnmodifiableMap rejects a duplicate key by throwing
    for (size_t i = 0; i < modList->fileByIdCount; i++)
    {
        if (strcmp(modList->fileById[i].modId, modId) != 0) continue;

        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Duplicate key {} in fileById", modId);
        return;
    }

    modList->fileById = realloc(modList->fileById, sizeof(*modList->fileById) * (modList->fileByIdCount + 1));
    modList->fileById[modList->fileByIdCount].modId = strdup(modId);
    modList->fileById[modList->fileByIdCount].modFileInfo = modFileInfo;
    modList->fileByIdCount++;
}

// Java: this.modFiles = modFiles.stream().map(ModFile::getModFileInfo).toList();
static void set_mod_files(LIBMATTI_FML_ModList *modList, LIBMATTI_FML_ModFile **modFiles, size_t count)
{
    modList->modFileCount = count;
    modList->modFiles = malloc(sizeof(*modList->modFiles) * (count + 1));
    for (size_t i = 0; i < count; i++)
        modList->modFiles[i] = LIBMATTI_FML_ModFile_GetModFileInfo(modFiles[i]);
}

// Java: this.sortedList = sortedList.stream().map(IModInfo.class::cast).toList();
static void set_sorted_list(LIBMATTI_FML_ModList *modList, LIBMATTI_FML_ModInfo **sortedList, size_t count)
{
    modList->sortedCount = count;
    modList->sortedList = malloc(sizeof(*modList->sortedList) * (count + 1));
    for (size_t i = 0; i < count; i++)
        modList->sortedList[i] = LIBMATTI_FML_ModInfo_AsModInfo(sortedList[i]);
}

// Java: this.fileById = this.modFiles.stream().map(IModFileInfo::getMods).flatMap(Collection::stream)
//         .map(ModInfo.class::cast).collect(Collectors.toUnmodifiableMap(ModInfo::getModId, ModInfo::getOwningFile));
static void build_file_by_id(LIBMATTI_FML_ModList *modList)
{
    for (size_t i = 0; i < modList->modFileCount; i++)
    {
        size_t count;
        LIBMATTI_NEOFORGESPI_IModInfo **mods =
            LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(modList->modFiles[i], &count);

        // Java returns the impl's own list, so the array must not be freed here
        for (size_t j = 0; j < count; j++)
            add_file_by_id(modList, LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mods[j]),
                           LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(mods[j]));
    }
}

// Java: public static ModList of(List<ModFile> modFiles, List<ModInfo> sortedList)
LIBMATTI_FML_ModList *LIBMATTI_FML_ModList_Of(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
                                             LIBMATTI_FML_ModInfo **sortedList, size_t sortedCount)
{
    LIBMATTI_FML_ModList *modList = calloc(1, sizeof(LIBMATTI_FML_ModList));

    set_mod_files(modList, modFiles, modFileCount);
    set_sorted_list(modList, sortedList, sortedCount);
    build_file_by_id(modList);

    LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplier("Mod List", crash_report_generator, modList);

    INSTANCE = modList;
    return INSTANCE;
}

// Java: public static ModList get()
LIBMATTI_FML_ModList *LIBMATTI_FML_ModList_Get(void)
{
    return INSTANCE;
}

// Java: @ApiStatus.Internal public static void clear() { INSTANCE = null; }
void LIBMATTI_FML_ModList_Clear(void)
{
    INSTANCE = NULL;
}

// Java: return "\n" + applyForEachModFileAlphabetical(this::fileToLine).collect(joining("\n\t\t", "\t\t", ""))
char *LIBMATTI_FML_ModList_CrashReport(LIBMATTI_FML_ModList *modList)
{
    size_t fileCount;
    LIBMATTI_NEOFORGESPI_IModFile **files = sorted_mod_files(modList, &fileCount);

    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_NewFromString("\n\t\t");
    for (size_t i = 0; i < fileCount; i++)
    {
        if (i > 0) LIBMATTI_JL_StringBuilder_Append(builder, "\n\t\t");

        char *line = LIBMATTI_FML_ModList_FileToLine(files[i]);
        LIBMATTI_JL_StringBuilder_Append(builder, line);
        free(line);
    }

    free(files);

    // Java returns a new String, so the caller owns the result
    char *report = strdup(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return report;
}

// Java: public List<IModFileInfo> getModFiles()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_FML_ModList_GetModFiles(const LIBMATTI_FML_ModList *modList,
                                                                    size_t *count)
{
    *count = modList->modFileCount;
    return modList->modFiles;
}

// Java: public IModFileInfo getModFileById(String modid)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModList_GetModFileById(const LIBMATTI_FML_ModList *modList,
                                                                      const char *modid)
{
    for (size_t i = 0; i < modList->fileByIdCount; i++)
        if (strcmp(modList->fileById[i].modId, modid) == 0) return modList->fileById[i].modFileInfo;

    return NULL;
}

// Java: void setLoadedMods(List<ModContainer> modContainers)
void LIBMATTI_FML_ModList_SetLoadedMods(LIBMATTI_FML_ModList *modList, LIBMATTI_FML_ModContainer **containers,
                                       size_t count)
{
    // Java: this.mods = modContainers;
    free(modList->mods);
    modList->modCount = count;
    modList->mods = malloc(sizeof(*modList->mods) * (count + 1));
    for (size_t i = 0; i < count; i++) modList->mods[i] = containers[i];

    // Java: sortedContainers = modContainers.stream()
    //         .sorted(Comparator.comparingInt(c -> sortedList.indexOf(c.getModInfo()))).toList();
    // Stream.sorted() is stable, hence the insertion sort.
    free(modList->sortedContainers);
    modList->sortedContainerCount = count;
    modList->sortedContainers = malloc(sizeof(*modList->sortedContainers) * (count + 1));
    for (size_t i = 0; i < count; i++)
    {
        size_t j = i;
        while (j > 0 &&
               sorted_index(modList, LIBMATTI_FML_ModContainer_GetModInfo(modList->sortedContainers[j - 1])) >
                   sorted_index(modList, LIBMATTI_FML_ModContainer_GetModInfo(containers[i])))
        {
            modList->sortedContainers[j] = modList->sortedContainers[j - 1];
            j--;
        }
        modList->sortedContainers[j] = containers[i];
    }

    // Java: indexedMods = modContainers.stream().collect(Collectors.toMap(ModContainer::getModId, Function.identity()));
    free(modList->indexedMods);
    modList->indexedMods = malloc(sizeof(*modList->indexedMods) * (count + 1));
    modList->indexedModCount = count;
    for (size_t i = 0; i < count; i++)
    {
        modList->indexedMods[i].modId = containers[i]->modId;
        modList->indexedMods[i].container = containers[i];
    }
}

// Java: public Optional<? extends ModContainer> getModContainerById(String modId)
LIBMATTI_FML_ModContainer *LIBMATTI_FML_ModList_GetModContainerById(const LIBMATTI_FML_ModList *modList,
                                                                   const char *modId)
{
    for (size_t i = 0; i < modList->indexedModCount; i++)
        if (strcmp(modList->indexedMods[i].modId, modId) == 0) return modList->indexedMods[i].container;

    return NULL;
}

// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_FML_ModList_GetMods(const LIBMATTI_FML_ModList *modList, size_t *count)
{
    *count = modList->sortedCount;
    return modList->sortedList;
}

// Java: public boolean isLoaded(String modTarget)
int LIBMATTI_FML_ModList_IsLoaded(const LIBMATTI_FML_ModList *modList, const char *modTarget)
{
    return LIBMATTI_FML_ModList_GetModContainerById(modList, modTarget) != NULL;
}

// Java: public int size()
int LIBMATTI_FML_ModList_Size(const LIBMATTI_FML_ModList *modList)
{
    return (int)modList->modCount;
}

// Java: public List<ModFileScanData> getAllScanData()
LIBMATTI_NEOFORGESPI_ModFileScanData **LIBMATTI_FML_ModList_GetAllScanData(LIBMATTI_FML_ModList *modList,
                                                                          size_t *count)
{
    if (modList->modFileScanData == NULL)
    {
        // Java: sortedList.stream().map(IModInfo::getOwningFile).filter(Objects::nonNull)
        //         .map(IModFileInfo::getFile).distinct().map(IModFile::getScanResult).collect(Collectors.toList())
        LIBMATTI_NEOFORGESPI_IModFile **files = malloc(sizeof(*files) * (modList->sortedCount + 1));
        size_t fileCount = 0;

        for (size_t i = 0; i < modList->sortedCount; i++)
        {
            LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile =
                LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(modList->sortedList[i]);
            if (owningFile == NULL) continue;

            LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile);

            int seen = 0;
            for (size_t j = 0; j < fileCount; j++)
                if (files[j] == file) seen = 1;
            if (!seen) files[fileCount++] = file;
        }

        modList->modFileScanData = malloc(sizeof(*modList->modFileScanData) * (fileCount + 1));
        modList->modFileScanDataCount = fileCount;
        for (size_t i = 0; i < fileCount; i++)
            modList->modFileScanData[i] = LIBMATTI_NEOFORGESPI_IModFile_GetScanResult(files[i]);

        free(files);
    }

    *count = modList->modFileScanDataCount;
    return modList->modFileScanData;
}

// Java: public void forEachModFile(Consumer<IModFile> fileConsumer)
void LIBMATTI_FML_ModList_ForEachModFile(const LIBMATTI_FML_ModList *modList,
                                         LIBMATTI_FML_ModList_ModFileConsumer fileConsumer, void *userdata)
{
    for (size_t i = 0; i < modList->modFileCount; i++)
        fileConsumer(LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(modList->modFiles[i]), userdata);
}

// Java: public <T> Stream<T> applyForEachModFile(Function<IModFile, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModFile(const LIBMATTI_FML_ModList *modList,
                                              LIBMATTI_FML_ModList_ModFileConsumer function, void *userdata)
{
    LIBMATTI_FML_ModList_ForEachModFile(modList, function, userdata);
}

// Java: public <T> Stream<T> applyForEachModFileAlphabetical(Function<IModFile, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModFileAlphabetical(const LIBMATTI_FML_ModList *modList,
                                                         LIBMATTI_FML_ModList_ModFileConsumer function,
                                                         void *userdata)
{
    size_t fileCount;
    LIBMATTI_NEOFORGESPI_IModFile **files = sorted_mod_files(modList, &fileCount);
    for (size_t i = 0; i < fileCount; i++)
        function(files[i], userdata);

    free(files);
}

// Java: public void forEachModContainer(BiConsumer<String, ModContainer> modContainerConsumer)
void LIBMATTI_FML_ModList_ForEachModContainer(const LIBMATTI_FML_ModList *modList,
                                              LIBMATTI_FML_ModList_ModContainerBiConsumer modContainerConsumer,
                                              void *userdata)
{
    for (size_t i = 0; i < modList->indexedModCount; i++)
        modContainerConsumer(modList->indexedMods[i].modId, modList->indexedMods[i].container, userdata);
}

// Java: public List<ModContainer> getSortedMods()
LIBMATTI_FML_ModContainer **LIBMATTI_FML_ModList_GetSortedMods(const LIBMATTI_FML_ModList *modList, size_t *count)
{
    *count = modList->sortedContainerCount;
    return modList->sortedContainers;
}

// Java: public void forEachModInOrder(Consumer<ModContainer> containerConsumer)
void LIBMATTI_FML_ModList_ForEachModInOrder(const LIBMATTI_FML_ModList *modList,
                                            LIBMATTI_FML_ModList_ModContainerConsumer containerConsumer,
                                            void *userdata)
{
    for (size_t i = 0; i < modList->sortedContainerCount; i++)
        containerConsumer(modList->sortedContainers[i], userdata);
}

// Java: public <T> Stream<T> applyForEachModContainer(Function<ModContainer, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModContainer(const LIBMATTI_FML_ModList *modList,
                                                  LIBMATTI_FML_ModList_ModContainerConsumer function, void *userdata)
{
    for (size_t i = 0; i < modList->indexedModCount; i++)
        function(modList->indexedMods[i].container, userdata);
}
