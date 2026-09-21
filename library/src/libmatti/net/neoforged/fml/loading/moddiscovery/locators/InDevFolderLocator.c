#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/InDevFolderLocator.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private void loadFromSystemProperty()
// "a/b/;c/d/;" -> "modid%%c:\\fish\\pepper;modid%%c:\\fish2\\pepper2\\;modid2%%c:\\fishy\\bums;modid2%%c:\\hmm"
void LIBMATTI_FML_InDevFolderLocator_LoadFromSystemProperty(LIBMATTI_FML_InDevFolderLocator *locator)
{
    // Java: Optional.ofNullable(System.getenv("MOD_CLASSES")).orElse(System.getProperty("fml.modFolders", ""));
    const char *modFolders = LIBMATTI_JL_System_Getenv("MOD_CLASSES");
    if (modFolders == NULL) modFolders = LIBMATTI_JL_System_GetProperty("fml.modFolders");
    if (modFolders == NULL || modFolders[0] == '\0')
        return;

    LIBMATTI_ML_Logger_Info(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Got mod coordinates {} from env", modFolders);

    // Java: groupedEntries = Arrays.stream(modFolders.split(File.pathSeparator))
    //         .collect(groupingBy(inp -> { var splitIdx = inp.indexOf("%%"); return splitIdx != -1 ? inp.substring(0, splitIdx) : "defaultmodid"; },
    //                             mapping(inp -> new File(inp.substring(splitIdx + 2)), toList())));
    char *copy = strdup(modFolders);
    for (char *token = strtok(copy, ":"); token != NULL; token = strtok(NULL, ":"))
    {
        const char *separator = strstr(token, "%%");
        const char *groupId = "defaultmodid";
        char *groupIdCopy = NULL;
        const char *file = token;
        if (separator != NULL)
        {
            groupIdCopy = strndup(token, (size_t) (separator - token));
            groupId = groupIdCopy;
            file = separator + 2;
        }

        // Java: virtualJarMemberIndex.put(file, virtualJar)
        size_t index = locator->memberCount;
        for (size_t i = 0; i < locator->memberCount; i++)
            if (strcmp(locator->memberValues[i]->name, groupId) == 0)
                index = i;
        if (index == locator->memberCount)
        {
            locator->memberCount++;
            locator->memberValues = realloc(locator->memberValues,
                                            sizeof(LIBMATTI_FML_VirtualJarManifestEntry *) * locator->memberCount);
            locator->memberValues[index] = calloc(1, sizeof(LIBMATTI_FML_VirtualJarManifestEntry));
            locator->memberValues[index]->name = strdup(groupId);
        }

        LIBMATTI_FML_VirtualJarManifestEntry *entry = locator->memberValues[index];
        entry->files = realloc(entry->files, sizeof(char *) * (entry->fileCount + 1));
        entry->files[entry->fileCount++] = strdup(file);

        locator->memberKeys = realloc(locator->memberKeys, sizeof(char *) * locator->memberCount);
        locator->memberKeys[index] = strdup(file);

        free(groupIdCopy);
    }
    free(copy);
}

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_InDevFolderLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    LIBMATTI_FML_InDevFolderLocator *locator = self;
    LIBMATTI_FML_InDevFolderLocator_LoadFromSystemProperty(locator);

    // Java: for (var entry : new HashSet<>(virtualJarMemberIndex.values()))
    for (size_t i = 0; i < locator->memberCount; i++)
    {
        LIBMATTI_FML_VirtualJarManifestEntry *entry = locator->memberValues[i];

        // Java: if (paths.stream().noneMatch(context::isLocated))
        int located = 0;
        for (size_t j = 0; j < entry->fileCount && !located; j++)
            if (LIBMATTI_NEOFORGESPI_ILaunchContext_IsLocated(context, entry->files[j]))
                located = 1;
        if (located) continue;

        // Java: pipeline.addJarContent(JarContents.ofPaths(paths), DEFAULT, IncompatibleFileReporting.ERROR);
        LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPaths((const char **) entry->files,
                                                                             entry->fileCount);
        if (contents == NULL)
        {
            LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.brokenfile.invalidzip", NULL, 0);
            LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(&pipeline->issueReporting,
                                                         LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(
                                                             issue, entry->files[0]));
            continue;
        }

        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
        LIBMATTI_NEOFORGESPI_IModFile *added = NULL;
        LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddJarContent(
            pipeline, contents, &attributes, LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_ERROR, &added);
    }
}

// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_InDevFolderLocator_GetPriority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_HIGHEST_SYSTEM_PRIORITY;
}

// Java: @Override public String toString() { return "indevfolder"; }
char *LIBMATTI_FML_InDevFolderLocator_ToString(void *self)
{
    (void) self;
    return strdup("indevfolder");
}

// Java: public InDevFolderLocator()
LIBMATTI_FML_InDevFolderLocator *LIBMATTI_FML_InDevFolderLocator_New(void)
{
    LIBMATTI_FML_InDevFolderLocator *locator = calloc(1, sizeof(LIBMATTI_FML_InDevFolderLocator));
    locator->locator.ordered.self = locator;
    locator->locator.ordered.getPriority = LIBMATTI_FML_InDevFolderLocator_GetPriority;
    locator->locator.findCandidates = LIBMATTI_FML_InDevFolderLocator_FindCandidates;
    locator->locator.toString = LIBMATTI_FML_InDevFolderLocator_ToString;
    return locator;
}
