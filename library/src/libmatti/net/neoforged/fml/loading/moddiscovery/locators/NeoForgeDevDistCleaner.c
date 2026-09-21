#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/NeoForgeDevDistCleaner.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/util/jar/Manifest.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public static boolean supportsDistCleaning(JarContents minecraftModFile)
int LIBMATTI_FML_NeoForgeDevDistCleaner_SupportsDistCleaning(LIBMATTI_FML_JarContents *minecraftModFile)
{
    // Java: return minecraftModFile.getManifest().getMainAttributes().containsKey(NAME_DISTS);
    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(minecraftModFile);
    return manifest != NULL && LIBMATTI_JU_Manifest_GetMainValue(manifest, LIBMATTI_FML_DIST_CLEANER_NAME_DISTS) != NULL;
}

// Java: private static String distName(Dist currentDist) - the switch in getMaskedFiles
static const char *distName(LIBMATTI_DIST_Dist dist)
{
    return dist == LIBMATTI_DIST_CLIENT ? "client" : "server";
}

// Java: public static Stream<String> getMaskedFiles(JarContents minecraftJar, Dist currentDist)
char **LIBMATTI_FML_NeoForgeDevDistCleaner_GetMaskedFiles(LIBMATTI_FML_JarContents *minecraftJar,
                                                          LIBMATTI_DIST_Dist currentDist, size_t *count)
{
    *count = 0;

    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(minecraftJar);
    const char *dists = manifest != NULL
                            ? LIBMATTI_JU_Manifest_GetMainValue(manifest, LIBMATTI_FML_DIST_CLEANER_NAME_DISTS)
                            : NULL;
    if (dists == NULL)
    {
        // Java: throw new ModLoadingException(error("fml.modloadingissue.neodev_missing_dists_attribute", NAME_DISTS));
        LIBMATTI_FML_ModLoadingIssue *issue =
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.neodev_missing_dists_attribute",
                                               (const char *[]){LIBMATTI_FML_DIST_CLEANER_NAME_DISTS}, 1);
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL, "{}",
                                 "fml.modloadingissue.neodev_missing_dists_attribute");
        LIBMATTI_FML_ModLoader_AddLoadingIssue(issue);
        return NULL;
    }

    // Java: dists.split("\\s+") - the tokens, compared against the current dist
    const char *dist = distName(currentDist);
    size_t tokenCount = 0;
    int allMatch = 1;
    int hasMatch = 0;
    for (const char *token = dists; token != NULL && *token != '\0';)
    {
        while (*token == ' ' || *token == '\t' || *token == '\n') token++;
        if (*token == '\0') break;

        size_t tokenLength = strcspn(token, " \t\n");
        tokenCount++;
        if (tokenLength == strlen(dist) && strncmp(token, dist, tokenLength) == 0)
            hasMatch = 1;
        else
            allMatch = 0;
        token += tokenLength;
    }

    // Java: if (dists.split("\\s+").allMatch(s -> s.equals(dist))) return Stream.empty();
    if (tokenCount > 0 && allMatch) return NULL;
    // Java: if (dists.split("\\s+").noneMatch(s -> s.equals(dist))) throw new ModLoadingException(ModLoadingIssue.error("fml.modloadingissue.neodev_missing_appropriate_dist", dist, NAME_DISTS));
    if (!hasMatch)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "The jar has no marker for the {} distribution", dist);
        return NULL;
    }

    // Java: return manifest.getEntries().entrySet().stream().filter(entry -> { var fileDist = entry.getValue().getValue(NAME_DIST); return fileDist != null && !fileDist.equals(dist); }).map(Map.Entry::getKey);
    char **maskedFiles = NULL;
    for (size_t i = 0; i < manifest->sectionCount; i++)
    {
        const char *fileDist = LIBMATTI_JU_Attributes_GetValue(&manifest->sections[i].attributes,
                                                               LIBMATTI_FML_DIST_CLEANER_NAME_DIST);
        if (fileDist == NULL || strcmp(fileDist, dist) == 0) continue;

        maskedFiles = realloc(maskedFiles, sizeof(char *) * (*count + 1));
        maskedFiles[(*count)++] = strdup(manifest->sections[i].name);
    }

    return maskedFiles;
}

// Java: public ProcessorName name() { return ClassProcessorIds.DIST_CLEANER; }
static LIBMATTI_NEOFORGESPI_ProcessorName *name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_DIST_CLEANER();
}

// Java: public Set<ProcessorName> runsBefore() { return Set.of(ClassProcessorIds.COMPUTING_FRAMES); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **runs_before(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                                             size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[1];
    (void) self;
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
    *count = 1;
    return names;
}

// Java: public Set<ProcessorName> runsAfter() { return Set.of(); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **runs_after(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                                            size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: public ComputeFlags processClass(TransformationContext context) { return ComputeFlags.NO_REWRITE; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    (void) self;
    (void) context;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

// Java: private static <T, X extends Throwable> void throwUnchecked(T throwable) throws X
// The C port has no unchecked-throw; the ClassNotFoundException is reported through the log instead.

// Java: public boolean handlesClass(SelectionContext context)
static int handles_class(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                         const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    LIBMATTI_FML_NeoForgeDevDistCleaner *cleaner = (LIBMATTI_FML_NeoForgeDevDistCleaner *) self;

    // Java: if (maskedClasses.contains(context.type().getClassName())) { ... }
    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int masked = 0;
    for (size_t i = 0; i < cleaner->maskedClassCount; i++)
    {
        if (strcmp(cleaner->maskedClasses[i], className) == 0)
        {
            masked = 1;
            break;
        }
    }

    if (masked)
    {
        // Java: String message = String.format("Attempted to load class %s which is not present on the %s", context.type().getClassName(), switch (dist) { ... });
        const char *distText = cleaner->dist == LIBMATTI_DIST_CLIENT ? "client" : "dedicated server";
        size_t length = strlen(className) + strlen(distText) + 64;
        char *message = malloc(length);
        snprintf(message, length, "Attempted to load class %s which is not present on the %s", className, distText);

        // Java: LOGGER.error(DISTXFORM, message); throwUnchecked(new ClassNotFoundException(message));
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL, "{}", message);
        free(message);
    }

    free(className);
    return masked;
}

// Java: public NeoForgeDevDistCleaner(JarContents minecraftModFile, Dist requestedDist)
LIBMATTI_FML_NeoForgeDevDistCleaner *LIBMATTI_FML_NeoForgeDevDistCleaner_New(
    LIBMATTI_FML_JarContents *minecraftModFile, LIBMATTI_DIST_Dist requestedDist)
{
    LIBMATTI_FML_NeoForgeDevDistCleaner *cleaner = calloc(1, sizeof(LIBMATTI_FML_NeoForgeDevDistCleaner));
    cleaner->dist = requestedDist;

    // Java: this.maskedClasses = getMaskedFiles(minecraftModFile, requestedDist).map(path -> path.endsWith(".class") ? path.substring(0, path.length() - ".class".length()).replace('/', '.') : null).filter(Objects::nonNull).collect(Collectors.toSet());
    size_t maskedFileCount = 0;
    char **maskedFiles = LIBMATTI_FML_NeoForgeDevDistCleaner_GetMaskedFiles(minecraftModFile, requestedDist,
                                                                          &maskedFileCount);
    for (size_t i = 0; i < maskedFileCount; i++)
    {
        // Classes are kept, but set to be filtered out at runtime; resources are removed entirely.
        size_t length = strlen(maskedFiles[i]);
        if (length < 6 || strcmp(maskedFiles[i] + length - 6, ".class") != 0)
        {
            free(maskedFiles[i]);
            continue;
        }

        char *className = strdup(maskedFiles[i]);
        className[length - 6] = '\0';
        for (char *c = className; *c != '\0'; c++)
            if (*c == '/') *c = '.';
        free(maskedFiles[i]);

        cleaner->maskedClasses = realloc(cleaner->maskedClasses,
                                         sizeof(char *) * (cleaner->maskedClassCount + 1));
        cleaner->maskedClasses[cleaner->maskedClassCount++] = className;
    }
    free(maskedFiles);

    cleaner->processor.name = name;
    cleaner->processor.runsBefore = runs_before;
    cleaner->processor.runsAfter = runs_after;
    cleaner->processor.processClass = process_class;
    cleaner->processor.handlesClass = handles_class;
    return cleaner;
}

void LIBMATTI_FML_NeoForgeDevDistCleaner_Free(LIBMATTI_FML_NeoForgeDevDistCleaner *cleaner)
{
    if (cleaner == NULL) return;
    for (size_t i = 0; i < cleaner->maskedClassCount; i++) free(cleaner->maskedClasses[i]);
    free(cleaner->maskedClasses);
    free(cleaner);
}
