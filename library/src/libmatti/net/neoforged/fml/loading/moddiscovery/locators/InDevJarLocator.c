#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/InDevJarLocator.h"

#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdlib.h>
#include <string.h>

// Java: @Override public String toString() { return "indevjar"; }
char *LIBMATTI_FML_InDevJarLocator_ToString(void *self)
{
    (void) self;
    return strdup("indevjar");
}

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_InDevJarLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                 LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    (void) self;
    (void) context;

    // Java: for (var path : ClasspathResourceUtils.findFileSystemRootsOfFileOnClasspath(JarModsDotTomlModFileReader.MODS_TOML))
    size_t modsTomlRootCount = 0;
    char **modsTomlRoots = LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootsOfFileOnClasspath(
        LIBMATTI_FML_MODS_TOML, &modsTomlRootCount);
    for (size_t i = 0; i < modsTomlRootCount; i++)
    {
        // Java: if (Files.isRegularFile(path)) pipeline.addPath(path, DEFAULT, WARN_ON_KNOWN_INCOMPATIBILITY);
        if (!LIBMATTI_JNF_Files_IsRegularFile(modsTomlRoots[i])) continue;

        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
        LIBMATTI_NEOFORGESPI_IModFile *added = NULL;
        LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
            pipeline, modsTomlRoots[i], &attributes,
            LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ON_KNOWN_INCOMPATIBILITY, &added);
    }

    // Java: for (var path : ClasspathResourceUtils.findFileSystemRootsOfFileOnClasspath(JarFile.MANIFEST_NAME))
    size_t manifestRootCount = 0;
    char **manifestRoots = LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootsOfFileOnClasspath(
        LIBMATTI_FML_MODS_TOML_MANIFEST, &manifestRootCount);
    for (size_t i = 0; i < manifestRootCount; i++)
    {
        if (!LIBMATTI_JNF_Files_IsRegularFile(manifestRoots[i])) continue;

        // Java: try (var jarFile = new JarFile(path.toFile(), false)) { manifest = jarFile.getManifest(); }
        //       catch (IOException e) { pipeline.addIssue(...invalidzip...); continue; }
        LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(manifestRoots[i]);
        if (contents == NULL)
        {
            LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.brokenfile.invalidzip", NULL, 0);
            LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(&pipeline->issueReporting,
                                                         LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(
                                                             issue, manifestRoots[i]));
            continue;
        }

        // Java: var modType = manifest.getMainAttributes().getValue(ModFile.TYPE); if (modType != null) pipeline.addPath(...);
        LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(contents);
        const char *modType = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, LIBMATTI_FML_MODFILE_TYPE)
                                               : NULL;
        if (modType != NULL)
        {
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
                LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
            LIBMATTI_NEOFORGESPI_IModFile *added = NULL;
            LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
                pipeline, manifestRoots[i], &attributes,
                LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ON_KNOWN_INCOMPATIBILITY, &added);
        }

        LIBMATTI_FML_JarContents_Close(contents);
    }
}

// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_InDevJarLocator_GetPriority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_HIGHEST_SYSTEM_PRIORITY;
}

// Java: public InDevJarLocator()
LIBMATTI_FML_InDevJarLocator *LIBMATTI_FML_InDevJarLocator_New(void)
{
    LIBMATTI_FML_InDevJarLocator *locator = calloc(1, sizeof(LIBMATTI_FML_InDevJarLocator));
    locator->locator.ordered.self = locator;
    locator->locator.ordered.getPriority = LIBMATTI_FML_InDevJarLocator_GetPriority;
    locator->locator.findCandidates = LIBMATTI_FML_InDevJarLocator_FindCandidates;
    locator->locator.toString = LIBMATTI_FML_InDevJarLocator_ToString;
    return locator;
}
