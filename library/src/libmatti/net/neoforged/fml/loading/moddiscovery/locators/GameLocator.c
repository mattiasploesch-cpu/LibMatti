#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/GameLocator.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModJarMetadata.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/MinecraftModInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/NeoForgeDevDistCleaner.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/RequiredSystemFiles.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: public static final String[] NEOFORGE_SPECIFIC_PATH_PREFIXES
static const char *const NEOFORGE_SPECIFIC_PATH_PREFIXES[] = {
    "net/neoforged/neoforge/", "META-INF/services/", LIBMATTI_FML_MODS_TOML,
};
#define NEOFORGE_PREFIX_COUNT 3

static int endsWith(const char *value, const char *suffix)
{
    size_t valueLength = strlen(value);
    size_t suffixLength = strlen(suffix);
    return suffixLength <= valueLength && strcmp(value + valueLength - suffixLength, suffix) == 0;
}

static int startsWith(const char *value, const char *prefix)
{
    return strncmp(value, prefix, strlen(prefix)) == 0;
}

// Java: JarContents.PathFilter nfJarFilter
static int neoforgeJarFilter(const char *relativePath, void *userdata)
{
    (void) userdata;
    if (!endsWith(relativePath, ".class")) return 1;

    for (size_t i = 0; i < NEOFORGE_PREFIX_COUNT; i++)
        if (startsWith(relativePath, NEOFORGE_SPECIFIC_PATH_PREFIXES[i]))
            return 1;
    return 0;
}

// Java: JarContents.PathFilter mcClassesFilter
static int minecraftClassesFilter(const char *relativePath, void *userdata)
{
    (void) userdata;
    if (!endsWith(relativePath, ".class")) return 0;

    for (size_t i = 0; i < NEOFORGE_PREFIX_COUNT; i++)
        if (startsWith(relativePath, NEOFORGE_SPECIFIC_PATH_PREFIXES[i]))
            return 0;
    return 1;
}

// Java: JarContents.PathFilter.and(pathFilter, getMaskedResourceFilter(container, requiredDist))
typedef struct
{
    int excludeClasses;
    char **maskedResources;
    size_t maskedResourceCount;
} MaskedResourceFilter;

static int maskedResourceFilter(const char *relativePath, void *userdata)
{
    MaskedResourceFilter *filter = userdata;

    // Java: pathFilter = relativePath -> !relativePath.endsWith(".class")
    if (filter->excludeClasses && endsWith(relativePath, ".class")) return 0;

    for (size_t i = 0; i < filter->maskedResourceCount; i++)
    {
        if (strcmp(filter->maskedResources[i], relativePath) != 0) continue;
        LIBMATTI_ML_Logger_Debug(LOGGER(), NULL,
                                 "Masking access to {} since it's from a different Minecraft distribution.",
                                 relativePath);
        return 0;
    }
    return 1;
}

static int containsJarContents(LIBMATTI_FML_JarContents **contents, size_t count, LIBMATTI_FML_JarContents *value)
{
    for (size_t i = 0; i < count; i++)
        if (contents[i] == value)
            return 1;
    return 0;
}

// Java: private static JarContents.@Nullable PathFilter getMaskedResourceFilter(JarContents jar, Dist requiredDist)
static MaskedResourceFilter *getMaskedResourceFilter(LIBMATTI_FML_JarContents *jar, LIBMATTI_DIST_Dist requiredDist)
{
    // Java: NeoForgeDevDistCleaner.getMaskedFiles(jar, requiredDist).filter(path -> !path.endsWith(".class")).collect(toSet())
    size_t maskedCount = 0;
    char **masked = LIBMATTI_FML_NeoForgeDevDistCleaner_GetMaskedFiles(jar, requiredDist, &maskedCount);

    MaskedResourceFilter *filter = calloc(1, sizeof(MaskedResourceFilter));
    for (size_t i = 0; i < maskedCount; i++)
    {
        if (endsWith(masked[i], ".class")) continue;
        filter->maskedResources = realloc(filter->maskedResources,
                                          sizeof(char *) * (filter->maskedResourceCount + 1));
        filter->maskedResources[filter->maskedResourceCount++] = masked[i];
    }

    if (filter->maskedResourceCount == 0)
    {
        free(filter);
        return NULL;
    }
    return filter;
}

// Java: private static JarContents.FilteredPath buildFilteredMinecraftResourcesFilteredPath(JarContents container, Dist requiredDist, RequiredSystemFiles systemFiles)
static LIBMATTI_FML_FilteredPath buildFilteredMinecraftResourcesFilteredPath(
    LIBMATTI_FML_JarContents *container, LIBMATTI_DIST_Dist requiredDist,
    LIBMATTI_FML_RequiredSystemFiles *systemFiles)
{
    LIBMATTI_FML_FilteredPath filteredPath = {0};
    filteredPath.path = LIBMATTI_FML_JarContents_GetPrimaryPath(container);

    // Java: if (systemFiles.getClassesRoots().contains(container)) pathFilter = relativePath -> !relativePath.endsWith(".class");
    size_t classRootCount = 0;
    LIBMATTI_FML_JarContents **classRoots = LIBMATTI_FML_RequiredSystemFiles_GetClassesRoots(systemFiles,
                                                                                            &classRootCount);
    int isClassRoot = containsJarContents(classRoots, classRootCount, container);
    free(classRoots);

    MaskedResourceFilter *masked = getMaskedResourceFilter(container, requiredDist);
    if (isClassRoot || masked != NULL)
    {
        // Java: JarContents.PathFilter.and(pathFilter, getMaskedResourceFilter(container, requiredDist))
        if (masked == NULL) masked = calloc(1, sizeof(MaskedResourceFilter));
        masked->excludeClasses = isClassRoot;
        filteredPath.filter = maskedResourceFilter;
        filteredPath.filterUserdata = masked;
    }

    return filteredPath;
}

// Java: private static List<JarContents.FilteredPath> getMinecraftResourcesRoots(ILaunchContext context, RequiredSystemFiles systemFiles)
static LIBMATTI_FML_FilteredPath *getMinecraftResourcesRoots(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, LIBMATTI_FML_RequiredSystemFiles *systemFiles, size_t *count)
{
    LIBMATTI_FML_JarContents *commonResources = LIBMATTI_FML_RequiredSystemFiles_GetCommonResources(systemFiles);
    LIBMATTI_FML_JarContents *clientResources = LIBMATTI_FML_RequiredSystemFiles_GetClientResources(systemFiles);

    LIBMATTI_FML_FilteredPath *result = malloc(sizeof(LIBMATTI_FML_FilteredPath) * 2);
    *count = 0;
    result[(*count)++] = buildFilteredMinecraftResourcesFilteredPath(
        commonResources, LIBMATTI_NEOFORGESPI_ILaunchContext_GetRequiredDistribution(context), systemFiles);

    if (clientResources != NULL && clientResources != commonResources)
        result[(*count)++] = buildFilteredMinecraftResourcesFilteredPath(
            clientResources, LIBMATTI_NEOFORGESPI_ILaunchContext_GetRequiredDistribution(context), systemFiles);

    return result;
}

// Java: private static void addContentRoot(List<JarContents.FilteredPath> roots, JarContents jarContents, JarContents.PathFilter filter)
static void addContentRoot(LIBMATTI_FML_FilteredPath **roots, size_t *rootCount, LIBMATTI_FML_JarContents *jarContents,
                           LIBMATTI_FML_JarContents_PathFilter filter, void *filterUserdata)
{
    if (jarContents == NULL) return;

    for (size_t i = 0; i < *rootCount; i++)
        if (strcmp((*roots)[i].path, LIBMATTI_FML_JarContents_GetPrimaryPath(jarContents)) == 0 &&
            (*roots)[i].filter == filter)
            return;

    *roots = realloc(*roots, sizeof(LIBMATTI_FML_FilteredPath) * (*rootCount + 1));
    (*roots)[*rootCount].path = LIBMATTI_FML_JarContents_GetPrimaryPath(jarContents);
    (*roots)[*rootCount].filter = filter;
    (*roots)[*rootCount].filterUserdata = filterUserdata;
    (*rootCount)++;
}

// Java: private static JarContents getCombinedMinecraftJar(ILaunchContext context, RequiredSystemFiles systemFiles)
static LIBMATTI_FML_JarContents *getCombinedMinecraftJar(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                         LIBMATTI_FML_RequiredSystemFiles *systemFiles)
{
    // Java: if (systemFiles.getCommonResources() == systemFiles.getNeoForgeResources()) throw new IllegalStateException(...)
    if (LIBMATTI_FML_RequiredSystemFiles_GetCommonResources(systemFiles) ==
        LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeResources(systemFiles))
        return NULL;

    size_t rootCount = 0;
    LIBMATTI_FML_FilteredPath *roots = getMinecraftResourcesRoots(context, systemFiles, &rootCount);

    addContentRoot(&roots, &rootCount, LIBMATTI_FML_RequiredSystemFiles_GetCommonClasses(systemFiles),
                   minecraftClassesFilter, NULL);
    addContentRoot(&roots, &rootCount, LIBMATTI_FML_RequiredSystemFiles_GetClientClasses(systemFiles),
                   minecraftClassesFilter, NULL);

    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfFilteredPaths(roots, rootCount);
    free(roots);
    return contents;
}

// Java: private static String detectMinecraftVersion(JarContents mcJarContents)
char *LIBMATTI_FML_GameLocator_DetectMinecraftVersion(LIBMATTI_FML_JarContents *mcJarContents)
{
    // Java: try (var in = mcJarContents.openFile("version.json")) { ... new Gson().fromJson(reader, JsonObject.class)
    //       .getAsJsonPrimitive("id").getAsString(); }
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_FML_JarContents_OpenFile(mcJarContents, "version.json", &length);
    if (bytes == NULL)
    {
        // Java: LOG.error("Minecraft version.json not found in {}.", mcJarContents); throw new ModLoadingException(...)
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Minecraft version.json not found in {}.",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(mcJarContents));
        LIBMATTI_FML_ModLoader_AddLoadingIssue(
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.corrupted_minecraft_jar", NULL, 0));
        return NULL;
    }

    // Java: var versionElement = new Gson().fromJson(reader, JsonObject.class);
    LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
    LIBMATTI_GSON_JsonElement *versionElement = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) bytes, length);
    free(bytes);
    LIBMATTI_GSON_Gson_Free(gson);
    if (versionElement == NULL)
    {
        // Java: catch (IOException e) { LOG.error("Failed to read Minecraft version.json from {}"); throw new ModLoadingException(...withCause(e)); }
        LIBMATTI_JL_Throwable *cause = LIBMATTI_JL_Throwable_NewNamed("java.io.IOException",
                                                                      "Failed to parse version.json");
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to read Minecraft version.json from {}",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(mcJarContents));
        LIBMATTI_FML_ModLoadingIssue *issue =
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.corrupted_minecraft_jar", NULL, 0);
        LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_WithCause(issue, cause));
        return NULL;
    }

    // Java: var idPrimitive = versionElement.getAsJsonPrimitive("id"); if (idPrimitive == null) { ... }
    LIBMATTI_GSON_JsonElement *idPrimitive = LIBMATTI_GSON_JsonElement_GetMember(versionElement, "id");
    if (idPrimitive == NULL || !LIBMATTI_GSON_JsonElement_IsJsonPrimitive(idPrimitive))
    {
        // Java: LOG.error("Minecraft version.json found in {} is missing 'id' field. Available fields are: {}", ...);
        char *availableFields = LIBMATTI_GSON_JsonElement_ToString(versionElement);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "Minecraft version.json found in {} is missing 'id' field. Available fields are: {}",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(mcJarContents), availableFields);
        free(availableFields);
        LIBMATTI_GSON_JsonElement_Free(versionElement);
        LIBMATTI_FML_ModLoader_AddLoadingIssue(
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.corrupted_minecraft_jar", NULL, 0));
        return NULL;
    }

    // Java: minecraftVersion = idPrimitive.getAsString();
    char *minecraftVersion = LIBMATTI_GSON_JsonElement_GetAsString(idPrimitive);
    LIBMATTI_GSON_JsonElement_Free(versionElement);
    return minecraftVersion;
}

// Java: private static void handleMergedMinecraftAndNeoForgeJar(ILaunchContext context, IDiscoveryPipeline pipeline, RequiredSystemFiles systemFiles)
static void handleMergedMinecraftAndNeoForgeJar(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline,
                                                LIBMATTI_FML_RequiredSystemFiles *systemFiles)
{
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL,
                            "Detected a joined NeoForge and Minecraft configuration. Applying filtering...");

    LIBMATTI_FML_JarContents *mcJarContents = getCombinedMinecraftJar(context, systemFiles);
    LIBMATTI_NEOFORGESPI_IModFile *minecraftModFile;

    // Java: if (mcJarContents.containsFile("META-INF/neoforged.mods.toml")) pipeline.addJarContent(...)
    if (LIBMATTI_FML_JarContents_ContainsFile(mcJarContents, LIBMATTI_FML_MODS_TOML))
    {
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
        LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddJarContent(
            pipeline, mcJarContents, &attributes, LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_IGNORE,
            &minecraftModFile);
        if (minecraftModFile == NULL)
        {
            // Java: throw new ModLoadingException(error("fml.modloadingissue.corrupted_minecraft_jar"));
            LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.corrupted_minecraft_jar", NULL, 0));
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", "fml.modloadingissue.corrupted_minecraft_jar");
            return;
        }
    }
    else
    {
        // Java: var minecraftVersion = detectMinecraftVersion(mcJarContents); var mcJarMetadata = new ModJarMetadata(); minecraftModFile = IModFile.create(mcJarContents, mcJarMetadata, new MinecraftModInfo(minecraftVersion)::buildMinecraftModInfo);
        char *minecraftVersion = LIBMATTI_FML_GameLocator_DetectMinecraftVersion(mcJarContents);
        LIBMATTI_FML_MinecraftModInfo *minecraftModInfo = LIBMATTI_FML_MinecraftModInfo_New(
            minecraftVersion != NULL ? minecraftVersion : "");
        LIBMATTI_FML_ModJarMetadata *mcJarMetadata = LIBMATTI_FML_ModJarMetadata_New();
        minecraftModFile = LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadata(
            mcJarContents, LIBMATTI_FML_ModJarMetadata_AsJarModuleInfo(mcJarMetadata),
            LIBMATTI_FML_MinecraftModInfo_BuildMinecraftModInfo, minecraftModInfo);
        LIBMATTI_FML_ModJarMetadata_SetModFile(mcJarMetadata, minecraftModFile);
        if (!LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddModFile(pipeline, minecraftModFile))
        {
            // Java: throw new ModLoadingException(error("fml.modloadingissue.corrupted_minecraft_jar"));
            LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.corrupted_minecraft_jar", NULL, 0));
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", "fml.modloadingissue.corrupted_minecraft_jar");
            return;
        }
    }

    // Java: if (!minecraftModFile.getId().equals("minecraft")) throw new ModLoadingException(...)
    if (strcmp(LIBMATTI_NEOFORGESPI_IModFile_GetId(minecraftModFile), "minecraft") != 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "The mod id for the Minecraft jar is not 'minecraft': {}",
                                 LIBMATTI_NEOFORGESPI_IModFile_GetId(minecraftModFile));
        LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.corrupted_minecraft_jar", NULL, 0));
        return;
    }

    // Java: JarContents nfJarContents = JarContents.ofFilteredPaths(nfJarRoots);
    size_t neoForgeRootCount = 0;
    LIBMATTI_FML_JarContents **neoForgeRoots = LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeJarComponents(
        systemFiles, &neoForgeRootCount);
    LIBMATTI_FML_FilteredPath *neoForgeFilteredPaths = malloc(sizeof(LIBMATTI_FML_FilteredPath) *
                                                              (neoForgeRootCount > 0 ? neoForgeRootCount : 1));
    for (size_t i = 0; i < neoForgeRootCount; i++)
    {
        neoForgeFilteredPaths[i].path = LIBMATTI_FML_JarContents_GetPrimaryPath(neoForgeRoots[i]);
        neoForgeFilteredPaths[i].filter = neoforgeJarFilter;
        neoForgeFilteredPaths[i].filterUserdata = NULL;
    }
    free(neoForgeRoots);

    LIBMATTI_FML_JarContents *nfJarContents = LIBMATTI_FML_JarContents_OfFilteredPaths(neoForgeFilteredPaths,
                                                                                        neoForgeRootCount);
    free(neoForgeFilteredPaths);
    if (nfJarContents == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to construct filtered NeoForge jar");
        return;
    }

    // Java: var modFile = JarModsDotTomlModFileReader.createModFile(nfJarContents, ModFileDiscoveryAttributes.DEFAULT);
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
    LIBMATTI_NEOFORGESPI_IModFile *modFile = LIBMATTI_FML_JarModsDotTomlModFileReader_CreateModFile(nfJarContents,
                                                                                                    &attributes);
    if (modFile == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to construct NeoForge mod file");
        return;
    }
    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddModFile(pipeline, modFile);

    // Java: systemFiles.getAll().stream().map(JarContents::getPrimaryPath).forEach(context::addLocated);
    size_t allCount = 0;
    LIBMATTI_FML_JarContents **all = LIBMATTI_FML_RequiredSystemFiles_GetAll(systemFiles, &allCount);
    for (size_t i = 0; i < allCount; i++)
        LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(context, LIBMATTI_FML_JarContents_GetPrimaryPath(all[i]));
    free(all);
}

// Java: private static void locateProductionMinecraft(ILaunchContext context, IDiscoveryPipeline pipeline)
// The C port deliberately ignores this Java-only path: it never consumes the launcher's Minecraft and
// NeoForge jars (the "libraryDirectory" system property and its Maven coordinates), because the game
// content is provided by the ported game itself.
static void locateProductionMinecraft(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                      LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    (void) context;
    (void) pipeline;
}

// Java: private void preventLoadingOfObfuscatedClientJar(ILaunchContext context, ClassLoader ourCl)
static void preventLoadingOfObfuscatedClientJar(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                               LIBMATTI_JL_ClassLoader *ourCl)
{
    // Java: var resources = ourCl.getResources(RequiredSystemFiles.COMMON_CLASS);
    size_t resourceCount = 0;
    LIBMATTI_JN_URL **resources = LIBMATTI_JL_ClassLoader_FindResources(ourCl,
                                                                       LIBMATTI_FML_REQUIRED_COMMON_CLASS,
                                                                       &resourceCount);
    for (size_t i = 0; i < resourceCount; i++)
    {
        char *jarPath = LIBMATTI_FML_ClasspathResourceUtils_FindJarPathFor(
            LIBMATTI_FML_REQUIRED_COMMON_CLASS, "minecraft jar", resources[i]);
        if (jarPath == NULL) continue;

        // Java: try (var zip = new ZipFile(jarPath.toFile())) { ... }
        LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(jarPath);
        if (contents == NULL)
        {
            free(jarPath);
            continue;
        }

        // Java: if (zip.getEntry(RequiredSystemFiles.COMMON_RESOURCE_ROOT) == null) continue;
        if (!LIBMATTI_FML_JarContents_ContainsFile(contents, LIBMATTI_FML_REQUIRED_COMMON_RESOURCE_ROOT))
        {
            LIBMATTI_FML_JarContents_Close(contents);
            free(jarPath);
            continue;
        }

        int hasClientClasses = LIBMATTI_FML_JarContents_ContainsFile(contents, LIBMATTI_FML_REQUIRED_CLIENT_CLASS);
        int hasClientResources = LIBMATTI_FML_JarContents_ContainsFile(contents,
                                                                       LIBMATTI_FML_REQUIRED_CLIENT_RESOURCE_ROOT);
        // Java: if (hasClientClasses != hasClientResources) continue;
        if (hasClientClasses != hasClientResources)
        {
            LIBMATTI_FML_JarContents_Close(contents);
            free(jarPath);
            continue;
        }

        // Java: if (zip.getEntry("META-INF/neoforge.mods.toml") != null) continue;
        if (LIBMATTI_FML_JarContents_ContainsFile(contents, LIBMATTI_FML_MODS_TOML))
        {
            LIBMATTI_FML_JarContents_Close(contents);
            free(jarPath);
            continue;
        }

        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Marking unmodified client jar as claimed to prevent loading: {}",
                                jarPath);
        LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(context, jarPath);
        LIBMATTI_FML_JarContents_Close(contents);
        free(jarPath);
        return;
    }
}

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_GameLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                             LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    (void) self;

    // Java: var ourCl = Thread.currentThread().getContextClassLoader();
    LIBMATTI_JL_ClassLoader *ourCl = LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread());

    // Java: preventLoadingOfObfuscatedClientJar(context, ourCl);
    preventLoadingOfObfuscatedClientJar(context, ourCl);

    // Java: try (var systemFiles = RequiredSystemFiles.find(context, ourCl)) { if (!systemFiles.isEmpty()) { ... return; } else LOG.info(...); }
    LIBMATTI_FML_RequiredSystemFiles *systemFiles = LIBMATTI_FML_RequiredSystemFiles_Find(context, ourCl);
    if (!LIBMATTI_FML_RequiredSystemFiles_IsEmpty(systemFiles))
    {
        // Java: systemFiles.checkForMissingMinecraftFiles(context.getRequiredDistribution() == Dist.CLIENT);
        LIBMATTI_FML_RequiredSystemFiles_CheckForMissingMinecraftFiles(
            systemFiles, LIBMATTI_NEOFORGESPI_ILaunchContext_GetRequiredDistribution(context) == LIBMATTI_DIST_CLIENT);

        handleMergedMinecraftAndNeoForgeJar(context, pipeline, systemFiles);
        LIBMATTI_FML_RequiredSystemFiles_Close(systemFiles);
        return;
    }

    LIBMATTI_FML_RequiredSystemFiles_Close(systemFiles);

    // Java: LOGGER.info("Failed to find common Minecraft classes and resources on the classpath. Assuming we're launching production.");
    //       locateProductionMinecraft(context, pipeline);
    // The C port has no launcher libraries to fall back on, so the production lookup is not performed.
    locateProductionMinecraft(context, pipeline);
}

// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_GameLocator_GetPriority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_HIGHEST_SYSTEM_PRIORITY;
}

// Java: @Override public String toString() { return "game locator"; }
char *LIBMATTI_FML_GameLocator_ToString(void *self)
{
    (void) self;
    return strdup("game locator");
}

// Java: public GameLocator()
LIBMATTI_FML_GameLocator *LIBMATTI_FML_GameLocator_New(void)
{
    LIBMATTI_FML_GameLocator *locator = calloc(1, sizeof(LIBMATTI_FML_GameLocator));
    locator->locator.ordered.self = locator;
    locator->locator.ordered.getPriority = LIBMATTI_FML_GameLocator_GetPriority;
    locator->locator.findCandidates = LIBMATTI_FML_GameLocator_FindCandidates;
    locator->locator.toString = LIBMATTI_FML_GameLocator_ToString;
    return locator;
}
