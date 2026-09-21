#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModDiscoverer.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/java/util/zip/Zip.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"
#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/UniqueModListBuilder.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/IncompatibleModReason.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/fml/util/ServiceLoaderUtil.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: the ServiceLoader.get() step of ServiceLoaderUtil - the host registered the instances
static void *instantiate(const LIBMATTI_JU_ServiceLoader_Provider *provider)
{
    return provider->provider;
}

// Java: ((IOrderedProvider) service).getPriority() - all three service interfaces extend
// IOrderedProvider, which the port places first in their structs
static int orderedPriority(void *service)
{
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_GetPriority(service);
}

// Java: log4j formats the {} arguments; the port's logger takes strings only
static const char *formatNumber(long long value)
{
    static char buffers[8][32];
    static int slot = 0;
    char *buffer = buffers[slot];
    slot = (slot + 1) % 8;
    snprintf(buffer, sizeof(buffers[0]), "%lld", value);
    return buffer;
}

// ---------------------------------------------------------------------------
// Java: the shared growable lists (Java: List<ModFile> loadedFiles / List<ModLoadingIssue> issues)
// ---------------------------------------------------------------------------

typedef struct
{
    LIBMATTI_FML_ModFile **items;
    size_t count;
} ModFileList;

typedef struct
{
    LIBMATTI_FML_ModLoadingIssue **items;
    size_t count;
} IssueList;

// Java: private class DiscoveryPipeline implements IDiscoveryPipeline
typedef struct
{
    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline pipeline;

    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes defaultAttributes;
    ModFileList *loadedFiles;
    IssueList *issues;
    LIBMATTI_NEOFORGESPI_IModFileReader **modFileReaders;
    size_t modFileReaderCount;
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext;

    int successCount;
    int errorCount;
    int warningCount;
    int skipCount;
} DiscoveryPipeline;

// Java: private static void closeJarContents(JarContents jarContents)
static void closeJarContents(LIBMATTI_FML_JarContents *jarContents)
{
    LIBMATTI_FML_JarContents_Close(jarContents);
}

// Java: private static boolean causeChainContains(Throwable e, Class<?> exceptionClass)
//      -> java.util.zip.ZipException
static int causeChainContainsZipException(void *cause)
{
    // Java: while (e != null) { if (exceptionClass.isInstance(e)) return true; e = e.getCause(); }
    const LIBMATTI_JL_Throwable *throwable = cause;
    while (throwable != NULL)
    {
        if (LIBMATTI_JU_ZipException_IsInstance(throwable))
            return 1;
        throwable = LIBMATTI_JL_Throwable_GetCause(throwable);
    }
    return 0;
}

// Java: public boolean addModFile(IModFile mf)
static int pipeline_addModFile(void *self, LIBMATTI_NEOFORGESPI_IModFile *mf)
{
    DiscoveryPipeline *pipeline = self;

    // Java: Objects.requireNonNull(mf, "mf"); if (!(mf instanceof ModFile modFile)) { ... return false; }
    if (mf == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "mf");
        return 0;
    }
    LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFile_AsConcrete(mf);
    if (modFile == NULL)
    {
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.technical_error", NULL, 0);
        LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(&pipeline->pipeline.issueReporting, issue);
        return 0;
    }

    // Java: modFile.setDiscoveryAttributes(defaultAttributes.merge(mf.getDiscoveryAttributes()));
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Merge(&pipeline->defaultAttributes,
                                                              LIBMATTI_FML_ModFile_GetDiscoveryAttributes(modFile));
    LIBMATTI_FML_ModFile_SetDiscoveryAttributes(modFile, &attributes);

    // Java: LOGGER.info(LogMarkers.SCAN, "Found {} file \"{}\" {}", mf.getType().name().toLowerCase(ROOT), mf.getFileName(), discoveryAttributes);
    char *attributesText = LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_ToString(&attributes);
    LIBMATTI_ML_Logger_Info(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Found {} file \"{}\" {}",
                            LIBMATTI_NEOFORGESPI_IModFile_GetType(mf) == LIBMATTI_NEOFORGESPI_IModFile_Type_MOD
                                ? "mod"
                                : (LIBMATTI_NEOFORGESPI_IModFile_GetType(mf) ==
                                           LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY
                                       ? "library"
                                       : "gamelibrary"),
                            LIBMATTI_NEOFORGESPI_IModFile_GetFileName(mf), attributesText);
    free(attributesText);

    pipeline->loadedFiles->items = realloc(pipeline->loadedFiles->items,
                                           sizeof(LIBMATTI_FML_ModFile *) * (pipeline->loadedFiles->count + 1));
    pipeline->loadedFiles->items[pipeline->loadedFiles->count++] = modFile;
    pipeline->successCount++;
    return 1;
}

// Java: public void addIssue(ModLoadingIssue issue)
static void pipeline_addIssue(void *self, LIBMATTI_FML_ModLoadingIssue *issue)
{
    DiscoveryPipeline *pipeline = self;
    pipeline->issues->items = realloc(pipeline->issues->items,
                                     sizeof(LIBMATTI_FML_ModLoadingIssue *) * (pipeline->issues->count + 1));
    pipeline->issues->items[pipeline->issues->count++] = issue;

    // Java: switch (issue.severity()) { case WARNING -> warningCount++; case ERROR -> errorCount++; }
    if (issue->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING) pipeline->warningCount++;
    if (issue->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) pipeline->errorCount++;
}

// Java: public Optional<IModFile> addJarContent(JarContents jarContents, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting reporting)
static int pipeline_addJarContent(void *self, LIBMATTI_FML_JarContents *jarContents,
                                  const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
                                  LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting,
                                  LIBMATTI_NEOFORGESPI_IModFile **out)
{
    DiscoveryPipeline *pipeline = self;
    *out = NULL;

    // Java: attributes = defaultAttributes.merge(attributes);
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes merged =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Merge(&pipeline->defaultAttributes, attributes);

    LIBMATTI_FML_ModLoadingIssue **incompatibilityIssues = NULL;
    size_t incompatibilityIssueCount = 0;

    // Java: for (var reader : modFileReaders) { try { var provided = reader.read(jarContents, attributes); ... } }
    for (size_t i = 0; i < pipeline->modFileReaderCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModFile *provided = LIBMATTI_NEOFORGESPI_IModFileReader_Read(
            pipeline->modFileReaders[i], jarContents, &merged);
        if (provided != NULL)
        {
            if (pipeline_addModFile(self, provided))
            {
                *out = provided;
                return 1;
            }

            // Java: the reader might have returned something other than a ModFile
            LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFile_AsConcrete(provided);
            if (modFile != NULL)
                LIBMATTI_FML_ModFile_Close(modFile);
            else
                closeJarContents(jarContents);
            for (size_t j = 0; j < incompatibilityIssueCount; j++)
                pipeline_addIssue(self, incompatibilityIssues[j]);
            free(incompatibilityIssues);
            return 0;
        }

        // Java: catch (ModLoadingException e) { incompatibilityIssues.addAll(e.getIssues()); }
        // The reader channel: a reader that "threw" registered its issues with ModLoader
        // (ModLoadingException_New registers them), so the take hands them over to the
        // incompatibility stash exactly like the Java catch clause.
        size_t readerIssueCount = 0;
        LIBMATTI_FML_ModLoadingIssue **readerIssues = LIBMATTI_FML_ModLoader_TakeLoadingIssues(&readerIssueCount);
        if (readerIssueCount > 0)
        {
            incompatibilityIssues = realloc(incompatibilityIssues,
                                            sizeof(LIBMATTI_FML_ModLoadingIssue *) *
                                                (incompatibilityIssueCount + readerIssueCount));
            for (size_t j = 0; j < readerIssueCount; j++)
                incompatibilityIssues[incompatibilityIssueCount++] = readerIssues[j];
        }
        free(readerIssues);
    }

    // Java: if (reporting != IncompatibleFileReporting.IGNORE) { ... }
    if (reporting != LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_IGNORE)
    {
        LIBMATTI_FML_IncompatibleModReason reason;
        if (LIBMATTI_FML_IncompatibleModReason_Detect(jarContents, &reason))
        {
            char *reasonKey = LIBMATTI_FML_IncompatibleModReason_GetReason(reason);
            const char *args[] = {NULL};
            incompatibilityIssues = realloc(incompatibilityIssues,
                                            sizeof(LIBMATTI_FML_ModLoadingIssue *) * (incompatibilityIssueCount + 1));
            incompatibilityIssues[incompatibilityIssueCount++] = LIBMATTI_FML_ModLoadingIssue_Error(reasonKey, args, 0);
            free(reasonKey);
        }
        else if (reporting != LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ON_KNOWN_INCOMPATIBILITY &&
                 incompatibilityIssueCount == 0)
        {
            incompatibilityIssues = realloc(incompatibilityIssues,
                                            sizeof(LIBMATTI_FML_ModLoadingIssue *) * (incompatibilityIssueCount + 1));
            incompatibilityIssues[incompatibilityIssueCount++] = LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.brokenfile.unknown", NULL, 0);
        }

        for (size_t i = 0; i < incompatibilityIssueCount; i++)
        {
            // Java: issue = issue.withSeverity(reporting.getIssueSeverity());
            LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_WithSeverity(
                incompatibilityIssues[i], LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetIssueSeverity(reporting));
            LIBMATTI_FML_ModLoadingIssue *withPath = LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(
                issue, LIBMATTI_FML_JarContents_GetPrimaryPath(jarContents));

            // Java: LOGGER.atLevel(reporting.getLogLevel()).addMarker(SCAN).log("Skipping jar. {}", FMLTranslations.translateIssueEnglish(issue));
            char *translated = LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(withPath);
            LIBMATTI_ML_Logger_Log(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                   LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetLogLevel(reporting),
                                   "Skipping jar. {}", translated);
            free(translated);

            pipeline_addIssue(self, withPath);
        }
    }

    free(incompatibilityIssues);

    // Java: closeJarContents(jarContents); return Optional.empty();
    closeJarContents(jarContents);
    return 0;
}

// Java: public Optional<IModFile> addPath(List<Path> groupedPaths, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting reporting)
static int pipeline_addPath(void *self, const char **groupedPaths, size_t groupedPathCount,
                            const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
                            LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting,
                            LIBMATTI_NEOFORGESPI_IModFile **out)
{
    DiscoveryPipeline *pipeline = self;
    *out = NULL;

    // Java: var primaryPath = groupedPaths.getFirst();
    const char *primaryPath = groupedPaths[0];

    // Java: if (!launchContext.addLocated(primaryPath)) { LOGGER.debug(...); skipCount++; return Optional.empty(); }
    if (!LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(pipeline->launchContext, primaryPath))
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Skipping {} because it was already located earlier", primaryPath);
        pipeline->skipCount++;
        return 0;
    }

    // Java: jarContents = JarContents.ofPaths(groupedPaths);
    LIBMATTI_FML_JarContents *jarContents = LIBMATTI_FML_JarContents_OfPaths(groupedPaths, groupedPathCount);
    if (jarContents == NULL)
    {
        void *cause = NULL;
        LIBMATTI_FML_ModLoadingIssue *issue;
        if (causeChainContainsZipException(cause))
            issue = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.brokenfile.invalidzip", NULL, 0);
        else
            issue = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.brokenfile", NULL, 0);
        pipeline_addIssue(self, LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(issue, primaryPath));
        return 0;
    }

    return pipeline_addJarContent(self, jarContents, attributes, reporting, out);
}

// Java: public @Nullable IModFile readModFile(JarContents jarContents, ModFileDiscoveryAttributes attributes)
static LIBMATTI_NEOFORGESPI_IModFile *pipeline_readModFile(void *self, LIBMATTI_FML_JarContents *jarContents,
                                                           const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    DiscoveryPipeline *pipeline = self;
    for (size_t i = 0; i < pipeline->modFileReaderCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModFile *provided = LIBMATTI_NEOFORGESPI_IModFileReader_Read(
            pipeline->modFileReaders[i], jarContents, attributes);
        if (provided != NULL) return provided;
    }

    // Java: throw new RuntimeException("No mod reader felt responsible for " + jarContents.getPrimaryPath());
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "No mod reader felt responsible for {}",
                             LIBMATTI_FML_JarContents_GetPrimaryPath(jarContents));
    return NULL;
}

static void pipeline_init(DiscoveryPipeline *pipeline)
{
    pipeline->pipeline.issueReporting.self = pipeline;
    pipeline->pipeline.issueReporting.addIssue = pipeline_addIssue;
    pipeline->pipeline.addPath = pipeline_addPath;
    pipeline->pipeline.addJarContent = pipeline_addJarContent;
    pipeline->pipeline.addModFile = pipeline_addModFile;
    pipeline->pipeline.readModFile = pipeline_readModFile;
}

// Java: public ModDiscoverer(ILaunchContext launchContext)
LIBMATTI_FML_ModDiscoverer *LIBMATTI_FML_ModDiscoverer_New(LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext)
{
    return LIBMATTI_FML_ModDiscoverer_NewWithLocators(launchContext, NULL, 0);
}

// Java: public ModDiscoverer(ILaunchContext launchContext, Collection<IModFileCandidateLocator> additionalModFileLocators)
LIBMATTI_FML_ModDiscoverer *LIBMATTI_FML_ModDiscoverer_NewWithLocators(
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext,
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator **additionalModFileLocators, size_t additionalCount)
{
    LIBMATTI_FML_ModDiscoverer *discoverer = calloc(1, sizeof(LIBMATTI_FML_ModDiscoverer));
    discoverer->launchContext = launchContext;

    // Java: modFileLocators = ServiceLoaderUtil.loadEarlyServices(launchContext, IModFileCandidateLocator.class, additionalModFileLocators);
    LIBMATTI_FML_ServiceLoaderUtil_Hooks locatorHooks = {0};
    locatorHooks.applyPriority = 1;
    locatorHooks.getPriority = orderedPriority;
    locatorHooks.instantiate = instantiate;
    LIBMATTI_FML_Service *additionalLocators = calloc(additionalCount > 0 ? additionalCount : 1,
                                                      sizeof(LIBMATTI_FML_Service));
    for (size_t i = 0; i < additionalCount; i++)
        additionalLocators[i].service = additionalModFileLocators[i];

    LIBMATTI_FML_ServiceList *locators = LIBMATTI_FML_ServiceLoaderUtil_LoadEarlyServices(
        launchContext, "net.neoforged.neoforgespi.locating.IModFileCandidateLocator", additionalLocators,
        additionalCount, &locatorHooks);
    discoverer->modFileLocatorCount = locators->count;
    discoverer->modFileLocators = malloc(sizeof(LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *) *
                                         (locators->count > 0 ? locators->count : 1));
    for (size_t i = 0; i < locators->count; i++)
        discoverer->modFileLocators[i] = locators->services[i].service;
    free(additionalLocators);
    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(locators);

    // Java: modFileReaders = ServiceLoaderUtil.loadEarlyServices(launchContext, IModFileReader.class, List.of());
    LIBMATTI_FML_ServiceLoaderUtil_Hooks readerHooks = {0};
    readerHooks.applyPriority = 1;
    readerHooks.getPriority = orderedPriority;
    readerHooks.instantiate = instantiate;
    LIBMATTI_FML_ServiceList *readers = LIBMATTI_FML_ServiceLoaderUtil_LoadEarlyServices(
        launchContext, "net.neoforged.neoforgespi.locating.IModFileReader", NULL, 0, &readerHooks);
    discoverer->modFileReaderCount = readers->count;
    discoverer->modFileReaders = malloc(sizeof(LIBMATTI_NEOFORGESPI_IModFileReader *) *
                                        (readers->count > 0 ? readers->count : 1));
    for (size_t i = 0; i < readers->count; i++)
        discoverer->modFileReaders[i] = readers->services[i].service;
    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(readers);

    // Java: dependencyLocators = ServiceLoaderUtil.loadEarlyServices(launchContext, IDependencyLocator.class, List.of());
    LIBMATTI_FML_ServiceLoaderUtil_Hooks dependencyHooks = {0};
    dependencyHooks.applyPriority = 1;
    dependencyHooks.getPriority = orderedPriority;
    dependencyHooks.instantiate = instantiate;
    LIBMATTI_FML_ServiceList *dependencies = LIBMATTI_FML_ServiceLoaderUtil_LoadEarlyServices(
        launchContext, "net.neoforged.neoforgespi.locating.IDependencyLocator", NULL, 0, &dependencyHooks);
    discoverer->dependencyLocatorCount = dependencies->count;
    discoverer->dependencyLocators = malloc(sizeof(LIBMATTI_NEOFORGESPI_IDependencyLocator *) *
                                            (dependencies->count > 0 ? dependencies->count : 1));
    for (size_t i = 0; i < dependencies->count; i++)
        discoverer->dependencyLocators[i] = dependencies->services[i].service;
    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(dependencies);

    return discoverer;
}

// Java has no destructor; the C port releases what the constructor allocated.
void LIBMATTI_FML_ModDiscoverer_Free(LIBMATTI_FML_ModDiscoverer *discoverer)
{
    if (discoverer == NULL) return;
    free(discoverer->modFileLocators);
    free(discoverer->dependencyLocators);
    free(discoverer->modFileReaders);
    free(discoverer);
}

// Java has no destructor; the port releases the Result lists once the caller is done with them.
void LIBMATTI_FML_ModDiscoverer_Result_Free(LIBMATTI_FML_ModDiscoverer_Result *result)
{
    if (result == NULL) return;
    free(result->modFiles);
    free(result->discoveryIssues);
    free(result);
}

// Java: private String fileToLine(IModFile mf)
static char *fileToLine(LIBMATTI_NEOFORGESPI_IModFile *mf)
{
    // Java: String.format(Locale.ENGLISH, "%s %s (%s)", name, version, modId)
    size_t modInfoCount = 0;
    LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(mf, &modInfoCount);
    if (modInfoCount == 0) return strdup("");

    LIBMATTI_NEOFORGESPI_IModInfo *mainMod = modInfos[0];
    const char *displayName = LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(mainMod);
    const char *version = LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(mainMod)->version;
    const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mainMod);

    size_t length = strlen(displayName) + strlen(version) + strlen(modId) + 5;
    char *line = malloc(length);
    snprintf(line, length, "%s %s (%s)", displayName, version, modId);
    return line;
}

// Java: private String logReport(Collection<List<ModFile>> modFiles)
char *LIBMATTI_FML_ModDiscoverer_LogReport(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount)
{
    // Java: .filter(modFile -> !modFile.getModInfos().isEmpty()).sorted(comparing(displayName, CASE_INSENSITIVE_ORDER)).map(this::fileToLine).collect(joining("\n\t\t", "\t\t", ""))
    LIBMATTI_NEOFORGESPI_IModFile **withMods = malloc(sizeof(LIBMATTI_NEOFORGESPI_IModFile *) *
                                                      (modFileCount > 0 ? modFileCount : 1));
    size_t withModCount = 0;
    for (size_t i = 0; i < modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(LIBMATTI_FML_ModFile_AsModFile(modFiles[i]), &modInfoCount);
        if (modInfoCount > 0) withMods[withModCount++] = LIBMATTI_FML_ModFile_AsModFile(modFiles[i]);
    }

    for (size_t i = 0; i + 1 < withModCount; i++)
        for (size_t j = i + 1; j < withModCount; j++)
        {
            size_t leftCount = 0;
            size_t rightCount = 0;
            LIBMATTI_NEOFORGESPI_IModInfo **left = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(withMods[i], &leftCount);
            LIBMATTI_NEOFORGESPI_IModInfo **right = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(withMods[j], &rightCount);
            if (strcasecmp(LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(left[0]),
                           LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(right[0])) > 0)
            {
                LIBMATTI_NEOFORGESPI_IModFile *swap = withMods[i];
                withMods[i] = withMods[j];
                withMods[j] = swap;
            }
        }

    char *report = strdup("\t\t");
    size_t length = 2;
    for (size_t i = 0; i < withModCount; i++)
    {
        char *line = fileToLine(withMods[i]);
        size_t lineLength = strlen(line);
        report = realloc(report, length + lineLength + 4);
        if (i > 0)
        {
            memcpy(report + length, "\n\t\t", 3);
            length += 3;
        }
        memcpy(report + length, line, lineLength + 1);
        length += lineLength;
        free(line);
    }

    free(withMods);
    return report;
}

// Java: public Result discoverMods(List<ModFile> additionalDependencySources)
LIBMATTI_FML_ModDiscoverer_Result *LIBMATTI_FML_ModDiscoverer_DiscoverMods(
    LIBMATTI_FML_ModDiscoverer *discoverer, LIBMATTI_FML_ModFile **additionalDependencySources, size_t count)
{
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                             "Scanning for mods and other resources to load. We know {} ways to find mods",
                             formatNumber((long long) discoverer->modFileLocatorCount));

    ModFileList loadedFiles = {0};
    IssueList discoveryIssues = {0};
    int successfullyLoadedMods = 1;
    LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress("Discovering mod files");

    // Java: for (var locator : modFileLocators) { ... locator.findCandidates(launchContext, pipeline); ... }
    for (size_t i = 0; i < discoverer->modFileLocatorCount; i++)
    {
        char *locatorName = LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ToString(
            discoverer->modFileLocators[i]);
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Trying locator {}", locatorName);

        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes defaultAttributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithLocator(
                &(LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes){0}, discoverer->modFileLocators[i]);

        DiscoveryPipeline pipeline = {0};
        pipeline_init(&pipeline);
        pipeline.defaultAttributes = defaultAttributes;
        pipeline.loadedFiles = &loadedFiles;
        pipeline.issues = &discoveryIssues;
        pipeline.modFileReaders = discoverer->modFileReaders;
        pipeline.modFileReaderCount = discoverer->modFileReaderCount;
        pipeline.launchContext = discoverer->launchContext;

        // Java: catch (ModLoadingException e) { discoveryIssues.addAll(e.getIssues()); }
        //       catch (Exception e) { discoveryIssues.add(ModLoadingIssue.error("fml.modloadingissue.technical_error", locator + " failed").withCause(e)); }
        LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_FindCandidates(discoverer->modFileLocators[i],
                                                                    discoverer->launchContext, &pipeline.pipeline);

        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Locator {} found {} mods, {} warnings, {} errors and skipped {} candidates",
                                 locatorName, formatNumber(pipeline.successCount),
                                 formatNumber(pipeline.warningCount), formatNumber(pipeline.errorCount),
                                 formatNumber(pipeline.skipCount));
        free(locatorName);
    }

    // Java: First processing run of the mod list.
    LIBMATTI_FML_ModFile **modFilesMap = NULL;
    size_t modFilesMapCount = 0;
    int hasModFilesMap = 0;
    {
        LIBMATTI_FML_UniqueModListBuilder *uniqueListBuilder =
            LIBMATTI_FML_UniqueModListBuilder_New(loadedFiles.items, loadedFiles.count);
        LIBMATTI_FML_ModLoadingIssue **uniqueIssues = NULL;
        size_t uniqueIssueCount = 0;
        LIBMATTI_FML_UniqueModListData *uniqueModsData =
            LIBMATTI_FML_UniqueModListBuilder_BuildUniqueList(uniqueListBuilder, &uniqueIssues, &uniqueIssueCount);
        if (uniqueModsData != NULL)
        {
            for (size_t i = 0; i < uniqueModsData->discardedFileCount; i++)
                LIBMATTI_FML_ModFile_Close(uniqueModsData->discardedFiles[i]);

            modFilesMap = uniqueModsData->modFiles;
            modFilesMapCount = uniqueModsData->modFileCount;
            hasModFilesMap = 1;
            free(loadedFiles.items);
            loadedFiles.items = uniqueModsData->modFiles;
            loadedFiles.count = uniqueModsData->modFileCount;

            // the mod file array is handed to the caller; release the rest of the builder data
            uniqueModsData->modFiles = NULL;
        }
        else
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                     "Failed to build unique mod list after mod discovery.");
            discoveryIssues.items = realloc(discoveryIssues.items,
                                            sizeof(LIBMATTI_FML_ModLoadingIssue *) *
                                            (discoveryIssues.count + uniqueIssueCount));
            memcpy(discoveryIssues.items + discoveryIssues.count, uniqueIssues,
                   sizeof(LIBMATTI_FML_ModLoadingIssue *) * uniqueIssueCount);
            discoveryIssues.count += uniqueIssueCount;
            successfullyLoadedMods = 0;
            for (size_t i = 0; i < loadedFiles.count; i++)
                LIBMATTI_FML_ModFile_Close(loadedFiles.items[i]);
            free(loadedFiles.items);
            loadedFiles.items = NULL;
            loadedFiles.count = 0;
        }

        free(uniqueIssues);
        LIBMATTI_FML_UniqueModListData_Free(uniqueModsData);
        free(uniqueListBuilder);
    }

    if (successfullyLoadedMods)
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Successfully Loaded {} mods. Attempting to load dependencies...",
                                 formatNumber((long long) loadedFiles.count));

        // Java: List<IModFile> dependencySources = new ArrayList<>(loadedFiles); dependencySources.addAll(additionalDependencySources);
        LIBMATTI_NEOFORGESPI_IModFile **dependencySources = malloc(
            sizeof(LIBMATTI_NEOFORGESPI_IModFile *) * (loadedFiles.count + count > 0
                                                          ? loadedFiles.count + count
                                                          : 1));
        LIBMATTI_NEOFORGESPI_IModFile **dependencySourcesView = dependencySources;
        size_t dependencySourceCount = 0;
        for (size_t i = 0; i < loadedFiles.count; i++)
            dependencySourcesView[dependencySourceCount++] = LIBMATTI_FML_ModFile_AsModFile(loadedFiles.items[i]);
        for (size_t i = 0; i < count; i++)
            dependencySourcesView[dependencySourceCount++] = LIBMATTI_FML_ModFile_AsModFile(additionalDependencySources[i]);

        for (size_t i = 0; i < discoverer->dependencyLocatorCount; i++)
        {
            // Java: LOGGER.debug(LogMarkers.SCAN, "Trying locator {}", locator) - Object.toString() is "class@hash"
            char locatorName[64];
            snprintf(locatorName, sizeof(locatorName), "dependencylocator@%zx",
                     (size_t) (uintptr_t) discoverer->dependencyLocators[i]->ordered.self);
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Trying locator {}", locatorName);

            DiscoveryPipeline pipeline = {0};
            pipeline_init(&pipeline);
            pipeline.defaultAttributes = LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithDependencyLocator(
                &(LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes){0}, discoverer->dependencyLocators[i]);
            pipeline.loadedFiles = &loadedFiles;
            pipeline.issues = &discoveryIssues;
            pipeline.modFileReaders = discoverer->modFileReaders;
            pipeline.modFileReaderCount = discoverer->modFileReaderCount;
            pipeline.launchContext = discoverer->launchContext;

            LIBMATTI_NEOFORGESPI_IDependencyLocator_ScanMods(discoverer->dependencyLocators[i],
                                                             dependencySourcesView, dependencySourceCount,
                                                             &pipeline.pipeline);
        }
        free(dependencySources);

        // Java: Second processing run of the mod list.
        LIBMATTI_FML_UniqueModListBuilder *uniqueListBuilder =
            LIBMATTI_FML_UniqueModListBuilder_New(loadedFiles.items, loadedFiles.count);
        LIBMATTI_FML_ModLoadingIssue **uniqueIssues = NULL;
        size_t uniqueIssueCount = 0;
        LIBMATTI_FML_UniqueModListData *uniqueModsData =
            LIBMATTI_FML_UniqueModListBuilder_BuildUniqueList(uniqueListBuilder, &uniqueIssues, &uniqueIssueCount);
        if (uniqueModsData != NULL)
        {
            for (size_t i = 0; i < uniqueModsData->discardedFileCount; i++)
                LIBMATTI_FML_ModFile_Close(uniqueModsData->discardedFiles[i]);
            modFilesMap = uniqueModsData->modFiles;
            modFilesMapCount = uniqueModsData->modFileCount;
            hasModFilesMap = 1;
            free(loadedFiles.items);
            loadedFiles.items = uniqueModsData->modFiles;
            loadedFiles.count = uniqueModsData->modFileCount;

            uniqueModsData->modFiles = NULL;
        }
        else
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                     "Failed to build unique mod list after dependency discovery.");
            discoveryIssues.items = realloc(discoveryIssues.items,
                                            sizeof(LIBMATTI_FML_ModLoadingIssue *) *
                                            (discoveryIssues.count + uniqueIssueCount));
            memcpy(discoveryIssues.items + discoveryIssues.count, uniqueIssues,
                   sizeof(LIBMATTI_FML_ModLoadingIssue *) * uniqueIssueCount);
            discoveryIssues.count += uniqueIssueCount;
            modFilesMap = loadedFiles.items;
            modFilesMapCount = loadedFiles.count;
            hasModFilesMap = 1;
        }

        free(uniqueIssues);
        LIBMATTI_FML_UniqueModListData_Free(uniqueModsData);
        free(uniqueListBuilder);
    }
    else
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Mod Discovery failed. Skipping dependency discovery.");
    }

    // Java: LOGGER.info("\n     Mod List:\n\t\tName Version (Mod Id)\n\n{}", logReport(modFilesMap.values()));
    if (hasModFilesMap)
    {
        char *report = LIBMATTI_FML_ModDiscoverer_LogReport(modFilesMap, modFilesMapCount);
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "\n     Mod List:\n\t\tName Version (Mod Id)\n\n{}", report);
        free(report);
    }

    LIBMATTI_FML_ModDiscoverer_Result *result = calloc(1, sizeof(LIBMATTI_FML_ModDiscoverer_Result));
    result->modFiles = modFilesMap;
    result->modFileCount = modFilesMapCount;
    result->discoveryIssues = discoveryIssues.items;
    result->discoveryIssueCount = discoveryIssues.count;
    return result;
}
