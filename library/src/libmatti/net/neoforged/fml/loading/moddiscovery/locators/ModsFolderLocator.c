#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/ModsFolderLocator.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/StringUtils.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: the interface default getPriority()
static int default_priority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_DEFAULT_PRIORITY;
}

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_ModsFolderLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                   LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    LIBMATTI_FML_ModsFolderLocator *locator = self;
    (void) context;

    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Scanning mods dir {} for mods",
                             locator->modFolder);

    // Java: Files.list(modFolder).filter(p -> StringUtils.toLowerCase(p.getFileName().toString()).endsWith(SUFFIX)).sorted(comparing(...)).toList();
    size_t entryCount = 0;
    char **entries = LIBMATTI_JNF_Files_List(locator->modFolder, &entryCount);
    char **directoryContent = malloc(sizeof(char *) * (entryCount > 0 ? entryCount : 1));
    size_t contentCount = 0;
    for (size_t i = 0; i < entryCount; i++)
    {
        const char *fileName = strrchr(entries[i], '/');
        fileName = fileName != NULL ? fileName + 1 : entries[i];
        size_t fileNameLength = strlen(fileName);
        size_t suffixLength = strlen(LIBMATTI_FML_MODS_FOLDER_SUFFIX);
        if (fileNameLength < suffixLength ||
            strcasecmp(fileName + fileNameLength - suffixLength, LIBMATTI_FML_MODS_FOLDER_SUFFIX) != 0)
        {
            free(entries[i]);
            continue;
        }

        // Java: .sorted(Comparator.comparing(path -> StringUtils.toLowerCase(path.getFileName().toString())))
        size_t index = contentCount;
        while (index > 0 && strcasecmp(directoryContent[index - 1], entries[i]) > 0)
        {
            directoryContent[index] = directoryContent[index - 1];
            index--;
        }
        directoryContent[index] = entries[i];
        contentCount++;
    }
    free(entries);

    for (size_t i = 0; i < contentCount; i++)
    {
        // Java: if (!Files.isRegularFile(file)) { pipeline.addIssue(ModLoadingIssue.warning("fml.modloadingissue.brokenfile.unknown").withAffectedPath(file)); continue; }
        if (!LIBMATTI_JNF_Files_IsRegularFile(directoryContent[i]))
        {
            LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Warning(
                "fml.modloadingissue.brokenfile.unknown", NULL, 0);
            LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(&pipeline->issueReporting,
                                                         LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(
                                                             issue, directoryContent[i]));
            continue;
        }

        // Java: pipeline.addPath(file, ModFileDiscoveryAttributes.DEFAULT, IncompatibleFileReporting.WARN_ALWAYS);
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
        LIBMATTI_NEOFORGESPI_IModFile *added = NULL;
        LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
            pipeline, directoryContent[i], &attributes,
            LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ALWAYS, &added);
    }

    for (size_t i = 0; i < contentCount; i++) free(directoryContent[i]);
    free(directoryContent);
}

// Java: public ModsFolderLocator()
LIBMATTI_FML_ModsFolderLocator *LIBMATTI_FML_ModsFolderLocator_New(void)
{
    // Java: this(FMLPaths.MODSDIR.get());
    return LIBMATTI_FML_ModsFolderLocator_NewWithFolder(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_MODSDIR),
                                                        "mods folder");
}

// Java: public ModsFolderLocator(Path modFolder, String name)
LIBMATTI_FML_ModsFolderLocator *LIBMATTI_FML_ModsFolderLocator_NewWithFolder(const char *modFolder,
                                                                             const char *name)
{
    LIBMATTI_FML_ModsFolderLocator *locator = calloc(1, sizeof(LIBMATTI_FML_ModsFolderLocator));
    locator->modFolder = modFolder;
    locator->customName = name;

    locator->locator.ordered.self = locator;
    locator->locator.ordered.getPriority = default_priority;
    locator->locator.findCandidates = LIBMATTI_FML_ModsFolderLocator_FindCandidates;
    locator->locator.toString = LIBMATTI_FML_ModsFolderLocator_ToString;
    return locator;
}

// Java: @Override public String toString() { return "{" + customName + " locator at " + this.modFolder + "}"; }
char *LIBMATTI_FML_ModsFolderLocator_ToString(void *self)
{
    const LIBMATTI_FML_ModsFolderLocator *locator = self;
    size_t length = strlen(locator->customName) + strlen(locator->modFolder) + strlen("{} locator at ") + 1;
    char *result = malloc(length);
    snprintf(result, length, "{%s locator at %s}", locator->customName, locator->modFolder);
    return result;
}
