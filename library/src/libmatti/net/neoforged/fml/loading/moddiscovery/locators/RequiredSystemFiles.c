#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/RequiredSystemFiles.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/util/jar/Manifest.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static boolean isNeoForgeManifest(Manifest manifest) { return "neoforge".equals(manifest.getMainAttributes().getValue("FML-System-Mods")); }
int LIBMATTI_FML_RequiredSystemFiles_IsNeoForgeManifest(const LIBMATTI_JU_Manifest *manifest)
{
    if (manifest == NULL) return 0;
    const char *value = LIBMATTI_JU_Manifest_GetMainValue(manifest, "FML-System-Mods");
    return value != NULL && strcmp(value, "neoforge") == 0;
}

// Java: private static boolean isNeoForgeManifest(Path path)
static int isNeoForgeManifestPath(const char *path)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(path, &length);
    if (bytes == NULL)
    {
        // Java: LOG.debug("Failed to read manifest at {}: {}", path, e);
        return 0;
    }

    LIBMATTI_JU_Manifest *manifest = LIBMATTI_JU_Manifest_Parse((const char *) bytes, length);
    free(bytes);
    int result = LIBMATTI_FML_RequiredSystemFiles_IsNeoForgeManifest(manifest);
    LIBMATTI_JU_Manifest_Free(manifest);
    return result;
}

// Java: private static JarContents openOrThrow(Path root)
static LIBMATTI_FML_JarContents *openOrThrow(const char *root)
{
    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(root);
    if (contents == NULL)
    {
        // Java: throw new ModLoadingException(error("fml.modloadingissue.corrupted_minecraft_jar").withAffectedPath(root).withCause(e));
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.corrupted_minecraft_jar", NULL, 0);
        issue = LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(issue, root);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", "fml.modloadingissue.corrupted_minecraft_jar");
        LIBMATTI_FML_ModLoader_AddLoadingIssue(issue);
    }
    return contents;
}

// Java: private static @Nullable JarContents findAndOpen(ILaunchContext context, ClassLoader loader, List<JarContents> alreadyOpened, String relativePath)
static LIBMATTI_FML_JarContents *findAndOpen(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                             LIBMATTI_JL_ClassLoader *loader,
                                             LIBMATTI_FML_JarContents ***alreadyOpened, size_t *alreadyOpenedCount,
                                             const char *relativePath)
{
    // Java: for (var contents : alreadyOpened) if (contents.containsFile(relativePath)) return contents;
    for (size_t i = 0; i < *alreadyOpenedCount; i++)
        if (LIBMATTI_FML_JarContents_ContainsFile((*alreadyOpened)[i], relativePath))
            return (*alreadyOpened)[i];

    size_t rootCount = 0;
    char **roots = LIBMATTI_FML_ClasspathResourceUtils_FindRoots(loader, relativePath, &rootCount);

    for (size_t i = 0; i < rootCount; i++)
    {
        // Java: if (!context.isLocated(path)) { var jar = openOrThrow(path); alreadyOpened.add(jar); return jar; }
        if (LIBMATTI_NEOFORGESPI_ILaunchContext_IsLocated(context, roots[i])) continue;

        LIBMATTI_FML_JarContents *jar = openOrThrow(roots[i]);
        if (jar == NULL) continue;

        *alreadyOpened = realloc(*alreadyOpened,
                                 sizeof(LIBMATTI_FML_JarContents *) * (*alreadyOpenedCount + 1));
        (*alreadyOpened)[(*alreadyOpenedCount)++] = jar;
        return jar;
    }

    return NULL;
}

// Java: private static JarContents findNeoForgeResources(List<JarContents> locatedRoots, ClassLoader loader)
static LIBMATTI_FML_JarContents *findNeoForgeResources(LIBMATTI_FML_JarContents **locatedRoots,
                                                       size_t locatedRootCount,
                                                       LIBMATTI_JL_ClassLoader *loader)
{
    // Java: for (var root : locatedRoots) if (isNeoForgeManifest(root.getManifest())) { LOG.debug("Found NeoForge MANIFEST.MF in {}", root); return root; }
    for (size_t i = 0; i < locatedRootCount; i++)
    {
        if (!LIBMATTI_FML_RequiredSystemFiles_IsNeoForgeManifest(
                LIBMATTI_FML_JarContents_GetManifest(locatedRoots[i])))
            continue;
        LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Found NeoForge MANIFEST.MF in {}",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(locatedRoots[i]));
        return locatedRoots[i];
    }

    // Java: var manifestRoots = ClasspathResourceUtils.findFileSystemRootsOfFileOnClasspath(loader, JarModsDotTomlModFileReader.MANIFEST);
    size_t rootCount = 0;
    char **manifestRoots = LIBMATTI_FML_ClasspathResourceUtils_FindRoots(loader, LIBMATTI_FML_MODS_TOML_MANIFEST,
                                                                         &rootCount);
    for (size_t i = 0; i < rootCount; i++)
    {
        // Java: if (!Files.isDirectory(manifestRoot)) continue;
        if (!LIBMATTI_JNF_Files_IsDirectory(manifestRoots[i])) continue;

        size_t length = strlen(manifestRoots[i]) + strlen(LIBMATTI_FML_MODS_TOML_MANIFEST) + 2;
        char *manifestPath = malloc(length);
        snprintf(manifestPath, length, "%s/%s", manifestRoots[i], LIBMATTI_FML_MODS_TOML_MANIFEST);
        int isNeoForge = isNeoForgeManifestPath(manifestPath);
        free(manifestPath);
        if (isNeoForge) return openOrThrow(manifestRoots[i]);
    }

    return NULL;
}

// Java: public static RequiredSystemFiles find(ILaunchContext context, ClassLoader loader)
LIBMATTI_FML_RequiredSystemFiles *LIBMATTI_FML_RequiredSystemFiles_Find(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                                        LIBMATTI_JL_ClassLoader *loader)
{
    LIBMATTI_FML_RequiredSystemFiles *result = calloc(1, sizeof(LIBMATTI_FML_RequiredSystemFiles));
    LIBMATTI_FML_JarContents **locatedRoots = NULL;
    size_t locatedRootCount = 0;

    result->commonClasses = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                        LIBMATTI_FML_REQUIRED_COMMON_CLASS);
    result->commonResources = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                          LIBMATTI_FML_REQUIRED_COMMON_RESOURCE_ROOT);
    result->clientClasses = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                       LIBMATTI_FML_REQUIRED_CLIENT_CLASS);
    result->clientResources = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                         LIBMATTI_FML_REQUIRED_CLIENT_RESOURCE_ROOT);
    result->neoForgeCommonClasses = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                                LIBMATTI_FML_REQUIRED_NEOFORGE_COMMON_CLASS);
    result->neoForgeClientClasses = findAndOpen(context, loader, &locatedRoots, &locatedRootCount,
                                                LIBMATTI_FML_REQUIRED_NEOFORGE_CLIENT_CLASS);
    result->neoForgeResources = findNeoForgeResources(locatedRoots, locatedRootCount, loader);

    // Java: catch (Exception e) { closeAll(locatedRoots); throw e; }
    // the opened roots are kept by the result; only the lookup array is released here
    free(locatedRoots);
    return result;
}

// Java: private static List<JarContents> uniqueAndNonNull(JarContents... contents)
static LIBMATTI_FML_JarContents **uniqueAndNonNull(size_t *count, LIBMATTI_FML_JarContents **contents,
                                                   size_t contentCount)
{
    LIBMATTI_FML_JarContents **result = malloc(sizeof(LIBMATTI_FML_JarContents *) * (contentCount > 0 ? contentCount : 1));
    *count = 0;
    for (size_t i = 0; i < contentCount; i++)
    {
        if (contents[i] == NULL) continue;
        int duplicate = 0;
        for (size_t j = 0; j < *count && !duplicate; j++)
            if (result[j] == contents[i])
                duplicate = 1;
        if (!duplicate) result[(*count)++] = contents[i];
    }
    return result;
}

// Java: public List<JarContents> getMinecraftJarComponents()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetMinecraftJarComponents(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count)
{
    LIBMATTI_FML_JarContents *contents[] = {files->commonResources, files->commonClasses, files->clientResources,
                                            files->clientClasses};
    return uniqueAndNonNull(count, contents, 4);
}

// Java: public List<JarContents> getNeoForgeJarComponents()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeJarComponents(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count)
{
    LIBMATTI_FML_JarContents *contents[] = {files->neoForgeResources, files->neoForgeCommonClasses,
                                            files->neoForgeClientClasses};
    return uniqueAndNonNull(count, contents, 3);
}

// Java: public boolean areNeoForgeAndMinecraftSeparate()
int LIBMATTI_FML_RequiredSystemFiles_AreNeoForgeAndMinecraftSeparate(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    size_t minecraftCount = 0;
    LIBMATTI_FML_JarContents **minecraft = LIBMATTI_FML_RequiredSystemFiles_GetMinecraftJarComponents(
        files, &minecraftCount);
    size_t neoForgeCount = 0;
    LIBMATTI_FML_JarContents **neoForge = LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeJarComponents(
        files, &neoForgeCount);

    // Java: Collections.disjoint(...)
    int disjoint = 1;
    for (size_t i = 0; i < minecraftCount && disjoint; i++)
        for (size_t j = 0; j < neoForgeCount; j++)
            if (minecraft[i] == neoForge[j])
            {
                disjoint = 0;
                break;
            }

    free(minecraft);
    free(neoForge);
    return disjoint;
}

// Java: public void checkForMissingMinecraftFiles(boolean clientRequired) - 0 when files are missing
int LIBMATTI_FML_RequiredSystemFiles_CheckForMissingMinecraftFiles(const LIBMATTI_FML_RequiredSystemFiles *files,
                                                                  int clientRequired)
{
    char *missingFiles[7];
    size_t missingCount = 0;

    if (files->commonClasses == NULL) missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_COMMON_CLASS;
    if (files->commonResources == NULL) missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_COMMON_RESOURCE_ROOT;
    if (files->neoForgeCommonClasses == NULL) missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_NEOFORGE_COMMON_CLASS;
    if (files->neoForgeResources == NULL) missingFiles[missingCount++] = "NeoForge MANIFEST.MF";
    if (clientRequired)
    {
        if (files->clientClasses == NULL) missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_CLIENT_CLASS;
        if (files->clientResources == NULL) missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_CLIENT_RESOURCE_ROOT;
        if (files->neoForgeClientClasses == NULL)
            missingFiles[missingCount++] = LIBMATTI_FML_REQUIRED_NEOFORGE_CLIENT_CLASS;
    }

    if (missingCount == 0) return 1;

    // Java: LOG.error("Couldn't find {} on classpath, while we did find other required files in: {}", missingFiles, foundRoots);
    char missingCountText[32];
    snprintf(missingCountText, sizeof(missingCountText), "%zu", missingCount);
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Couldn't find {} of the required files on the classpath",
                             missingCountText);
    return 0;
}

// Java: public List<JarContents> getAll()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetAll(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count)
{
    LIBMATTI_FML_JarContents *contents[] = {files->commonClasses, files->commonResources, files->clientClasses,
                                            files->clientResources, files->neoForgeCommonClasses,
                                            files->neoForgeClientClasses, files->neoForgeResources};
    LIBMATTI_FML_JarContents **result = malloc(sizeof(LIBMATTI_FML_JarContents *) * 7);
    *count = 0;
    for (size_t i = 0; i < 7; i++)
        if (contents[i] != NULL)
            result[(*count)++] = contents[i];
    return result;
}

// Java: public List<JarContents> getClassesRoots()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetClassesRoots(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count)
{
    LIBMATTI_FML_JarContents *contents[] = {files->commonClasses, files->clientClasses,
                                            files->neoForgeCommonClasses, files->neoForgeClientClasses};
    LIBMATTI_FML_JarContents **result = malloc(sizeof(LIBMATTI_FML_JarContents *) * 4);
    *count = 0;
    for (size_t i = 0; i < 4; i++)
        if (contents[i] != NULL)
            result[(*count)++] = contents[i];
    return result;
}

// Java: public boolean isEmpty()
int LIBMATTI_FML_RequiredSystemFiles_IsEmpty(const LIBMATTI_FML_RequiredSystemFiles *files)
{
    size_t count = 0;
    LIBMATTI_FML_JarContents **all = LIBMATTI_FML_RequiredSystemFiles_GetAll(files, &count);
    free(all);
    return count == 0;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetCommonClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->commonClasses;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetCommonResources(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->commonResources;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetClientClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->clientClasses;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetClientResources(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->clientResources;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeCommonClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->neoForgeCommonClasses;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeClientClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->neoForgeClientClasses;
}

LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeResources(
    const LIBMATTI_FML_RequiredSystemFiles *files)
{
    return files->neoForgeResources;
}

// Java: private static void closeAll(Iterable<JarContents> locatedRoots)
static void closeAll(LIBMATTI_FML_JarContents **roots, size_t rootCount)
{
    for (size_t i = 0; i < rootCount; i++)
        LIBMATTI_FML_JarContents_Close(roots[i]);
}

// Java: @Override public void close()
void LIBMATTI_FML_RequiredSystemFiles_Close(LIBMATTI_FML_RequiredSystemFiles *files)
{
    size_t count = 0;
    LIBMATTI_FML_JarContents **all = LIBMATTI_FML_RequiredSystemFiles_GetAll(files, &count);
    closeAll(all, count);
    free(all);
    // Java lets the GC collect the closed resources
    free(files);
}
