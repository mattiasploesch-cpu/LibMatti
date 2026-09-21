// Port of net.neoforged.fml.ModLoader.

#include "libmatti/net/neoforged/fml/ModLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/net/neoforged/bus/api/EventPriority.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoadingContext.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/FMLConstructModEvent.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/fml/loading/modscan/BackgroundScanHandler.h"
#include "libmatti/net/neoforged/fml/loading/progress/ProgressMeter.h"
#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"
#include "libmatti/net/neoforged/neoforgespi/IIssueReporting.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ForgeFeature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java drops a dispatched event after every mod has seen it; the port frees it. These event classes
// keep the Event base as their first member and own no heap field of their own.
static void free_event(LIBMATTI_BUS_Event *event)
{
    free(event);
}

// Java: private static final List<ModLoadingIssue> loadingIssues = new ArrayList<>();
static LIBMATTI_FML_ModLoadingIssue **loadingIssues = NULL;
static size_t loadingIssueCount = 0;
// Java: private static ModList modList;
static LIBMATTI_FML_ModList *modList = NULL;
// Java: throw new ModLoadingException(loadingIssues) - the port keeps what it would have thrown.
static LIBMATTI_JL_Throwable *lastException = NULL;

LIBMATTI_JL_Throwable *LIBMATTI_FML_ModLoader_GetLastException(void)
{
    return lastException;
}

// Java: public static void addLoadingIssue(ModLoadingIssue issue) { loadingIssues.add(issue); }
void LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue *issue)
{
    loadingIssues = realloc(loadingIssues, sizeof(void *) * (loadingIssueCount + 1));
    loadingIssues[loadingIssueCount++] = issue;
}

// Java: public static void addLoadingIssues(Collection<ModLoadingIssue> issues)
void LIBMATTI_FML_ModLoader_AddLoadingIssues(LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount)
{
    for (size_t i = 0; i < issueCount; i++)
        LIBMATTI_FML_ModLoader_AddLoadingIssue(issues[i]);
}

// Java: public static List<ModLoadingIssue> getLoadingIssues() { return List.copyOf(loadingIssues); }
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetLoadingIssues(size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **issues = malloc(sizeof(void *) * (loadingIssueCount > 0 ? loadingIssueCount : 1));
    for (size_t i = 0; i < loadingIssueCount; i++) issues[i] = loadingIssues[i];

    *count = loadingIssueCount;
    return issues;
}

// Java: public static List<ModLoadingIssue> getErrors()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetErrors(size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **errors = NULL;
    *count = 0;

    for (size_t i = 0; i < loadingIssueCount; i++)
    {
        if (loadingIssues[i]->severity != LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) continue;

        errors = realloc(errors, sizeof(void *) * (*count + 1));
        errors[(*count)++] = loadingIssues[i];
    }

    return errors;
}

// Java: public static List<ModLoadingIssue> getLoadingWarnings()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetWarnings(size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **warnings = NULL;
    *count = 0;

    for (size_t i = 0; i < loadingIssueCount; i++)
    {
        if (loadingIssues[i]->severity != LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING) continue;

        warnings = realloc(warnings, sizeof(void *) * (*count + 1));
        warnings[(*count)++] = loadingIssues[i];
    }

    return warnings;
}

// Java: public static boolean hasErrors()
int LIBMATTI_FML_ModLoader_HasErrors(void)
{
    for (size_t i = 0; i < loadingIssueCount; i++)
        if (loadingIssues[i]->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) return 1;

    return 0;
}

// Java: public static void clearLoadingIssues() { LOGGER.info("Clearing {} loading issues", loadingIssues.size()); loadingIssues.clear(); }
void LIBMATTI_FML_ModLoader_ClearLoadingIssues(void)
{
    char count[32];
    snprintf(count, sizeof(count), "%zu", loadingIssueCount);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Clearing {} loading issues", count);
    loadingIssueCount = 0;
}

// The port's "catch (ModLoadingException e)" channel: hand every pending issue to the
// caller (which owns them afterwards) and empty the list, so an issue registered while a
// reader ran is attributed to that reader instead of surfacing globally.
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_TakeLoadingIssues(size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **taken = loadingIssues;
    *count = loadingIssueCount;
    loadingIssues = NULL;
    loadingIssueCount = 0;
    return taken;
}

// Java: public static void clear() { LOGGER.info("Clearing ModLoader"); loadingIssues.clear(); modList = null; }
void LIBMATTI_FML_ModLoader_Clear(void)
{
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Clearing ModLoader");
    loadingIssueCount = 0;
    modList = NULL;
}

// Java: private static void cancelLoading(ModList modList)
static void cancelLoading(LIBMATTI_FML_ModList *cancelled)
{
    LIBMATTI_FML_StartupNotificationManager_ModLoaderMessage("ERROR DURING MOD LOADING");
    LIBMATTI_FML_ModList_SetLoadedMods(cancelled, NULL, 0);
}

// Java: private static void throwIfErrors(@Nullable ModList modList) - 1 when there was no error
static int throwIfErrors(LIBMATTI_FML_ModList *candidate)
{
    if (!LIBMATTI_FML_ModLoader_HasErrors()) return 1;

    size_t errorCount = 0;
    LIBMATTI_FML_ModLoadingIssue **loadingErrors = LIBMATTI_FML_ModLoader_GetErrors(&errorCount);
    for (size_t i = 0; i < errorCount; i++)
    {
        char *translation = LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(loadingErrors[i]);
        LIBMATTI_ML_Logger_Fatal(LOGGER(), &LIBMATTI_FML_Logging_CORE, "Error during pre-loading phase: {}",
                                 translation);
        free(translation);
    }
    free(loadingErrors);

    if (candidate != NULL) cancelLoading(candidate);

    // Java: throw new ModLoadingException(loadingIssues)
    lastException = &LIBMATTI_FML_ModLoadingException_NewWithIssues(loadingIssues, loadingIssueCount)->base;
    return 0;
}

// Java: ModLoader::addLoadingIssue - the IIssueReporting an IModLanguageLoader.validate gets
static void add_issue_adapter(void *self, LIBMATTI_FML_ModLoadingIssue *issue)
{
    (void) self;
    LIBMATTI_FML_ModLoader_AddLoadingIssue(issue);
}

// Java: private static ModContainer buildModContainerFromTOML(IModInfo modInfo, ModFileScanData scanData)
static LIBMATTI_FML_ModContainer *buildModContainerFromTOML(LIBMATTI_NEOFORGESPI_IModInfo *modInfo,
                                                           LIBMATTI_NEOFORGESPI_ModFileScanData *scanData)
{
    // Java: try { return modInfo.getLoader().loadMod(modInfo, scanData, FMLLoader.getCurrent().getGameLayer()); }
    //       catch (ModLoadingException mle) { loadingIssues.addAll(mle.getIssues()); return null; }
    // A C language loader cannot throw: it reports through ModLoader.addLoadingIssue and returns NULL.
    LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader =
        (LIBMATTI_NEOFORGESPI_IModLanguageLoader *) LIBMATTI_NEOFORGESPI_IModInfo_GetLoader(modInfo);

    return loader->loadMod(loader->self, modInfo, scanData,
                           LIBMATTI_FML_FMLLoader_GetGameLayer(LIBMATTI_FML_FMLLoader_GetCurrent()));
}

// Java: private static List<ModContainer> buildMods(IModFile modFile) - appends to the container list
static void buildMods(LIBMATTI_NEOFORGESPI_IModFile *modFile, LIBMATTI_FML_ModContainer ***containers,
                      size_t *containerCount)
{
    // Java: Map<IModLanguageLoader, Set<ModContainer>> byLoader = new IdentityHashMap<>();
    LIBMATTI_NEOFORGESPI_IModLanguageLoader **byLoader = NULL;
    LIBMATTI_FML_ModContainer ***byLoaderContainers = NULL;
    size_t *byLoaderCounts = NULL;
    size_t byLoaderCount = 0;

    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo = LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(modFile);
    size_t modInfoCount = 0;
    LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(modFileInfo, &modInfoCount);

    for (size_t i = 0; i < modInfoCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo *info = modInfos[i];
        LIBMATTI_FML_ModContainer *container =
            buildModContainerFromTOML(info, LIBMATTI_NEOFORGESPI_IModFile_GetScanResult(modFile));

        // Java: byLoader.computeIfAbsent(info.getLoader(), k -> new HashSet<>())
        LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader =
            (LIBMATTI_NEOFORGESPI_IModLanguageLoader *) LIBMATTI_NEOFORGESPI_IModInfo_GetLoader(info);

        size_t groupIndex = byLoaderCount;
        for (size_t k = 0; k < byLoaderCount; k++)
            if (byLoader[k] == loader) groupIndex = k;

        if (groupIndex == byLoaderCount)
        {
            byLoader = realloc(byLoader, sizeof(void *) * (byLoaderCount + 1));
            byLoaderContainers = realloc(byLoaderContainers, sizeof(void *) * (byLoaderCount + 1));
            byLoaderCounts = realloc(byLoaderCounts, sizeof(size_t) * (byLoaderCount + 1));
            byLoader[byLoaderCount] = loader;
            byLoaderContainers[byLoaderCount] = NULL;
            byLoaderCounts[byLoaderCount] = 0;
            byLoaderCount++;
        }

        if (container == NULL) continue;

        byLoaderContainers[groupIndex] =
            realloc(byLoaderContainers[groupIndex], sizeof(void *) * (byLoaderCounts[groupIndex] + 1));
        byLoaderContainers[groupIndex][byLoaderCounts[groupIndex]++] = container;

        // Java: the mapped stream drops the containers that failed to build
        *containers = realloc(*containers, sizeof(void *) * (*containerCount + 1));
        (*containers)[(*containerCount)++] = container;
    }

    // Java: byLoader.forEach((loader, loaded) -> loader.validate(modFile, loaded, ModLoader::addLoadingIssue));
    LIBMATTI_NEOFORGESPI_IIssueReporting reporter = {NULL, add_issue_adapter};
    for (size_t i = 0; i < byLoaderCount; i++)
    {
        if (byLoader[i]->validate == NULL) continue;

        byLoader[i]->validate(byLoader[i]->self, modFile, (void **) byLoaderContainers[i], byLoaderCounts[i],
                              &reporter);
    }

    for (size_t i = 0; i < byLoaderCount; i++) free(byLoaderContainers[i]);
    free(byLoaderContainers);
    free(byLoaderCounts);
    free(byLoader);
}

// Java: the workQueue::runTasks of constructMods' deferred queue
static void run_deferred_queue(void *userdata)
{
    LIBMATTI_FML_DeferredWorkQueue_RunTasks(userdata);
}

// Java: modContainer -> { modContainer.constructMod(); modContainer.acceptEvent(new FMLConstructModEvent(modContainer, workQueue)); }
static void construct_mod(LIBMATTI_FML_ModContainer *container, void *userdata)
{
    LIBMATTI_FML_ModContainer_ConstructMod(container);

    LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *event =
        LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_New(container, userdata);
    LIBMATTI_FML_ModContainer_AcceptEvent(container, &event->base.base.base);
    LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Free(event);
}

// Java: private static void constructMods(Executor syncExecutor, Executor parallelExecutor, Runnable periodicTask)
static void constructMods(void (*periodicTask)(void *userdata), void *userdata)
{
    LIBMATTI_FML_DeferredWorkQueue *workQueue = LIBMATTI_FML_DeferredWorkQueue_New("Mod Construction");
    LIBMATTI_FML_ModLoader_DispatchParallelTask("Mod Construction", periodicTask, userdata, construct_mod, workQueue);

    // Java: waitForTask("Mod Construction: Deferred Queue", periodicTask, CompletableFuture.runAsync(workQueue::runTasks, syncExecutor));
    LIBMATTI_FML_ModLoader_RunInitTask("Mod Construction: Deferred Queue", periodicTask, userdata, run_deferred_queue,
                                       workQueue);
    LIBMATTI_FML_DeferredWorkQueue_Free(workQueue);
}

// Java: public static void gatherAndInitializeMods(Executor syncExecutor, Executor parallelExecutor, Runnable periodicTask)
void LIBMATTI_FML_ModLoader_GatherAndInitializeMods(void (*periodicTask)(void *userdata), void *userdata)
{
    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrent();
    LIBMATTI_FML_LoadingModList *loadingModList =
        (LIBMATTI_FML_LoadingModList *) LIBMATTI_FML_FMLLoader_GetLoadingModList(loader);

    // Java: loadingIssues.addAll(loadingModList.getModLoadingIssues()); throwIfErrors(null);
    if (loadingModList != NULL)
    {
        size_t discoveryIssueCount = 0;
        LIBMATTI_FML_ModLoadingIssue **discoveryIssues =
            LIBMATTI_FML_LoadingModList_GetModLoadingIssues(loadingModList, &discoveryIssueCount);
        LIBMATTI_FML_ModLoader_AddLoadingIssues(discoveryIssues, discoveryIssueCount);
    }
    if (!throwIfErrors(NULL)) return;

    // Java: ForgeFeature.registerFeature("javaVersion", ForgeFeature.VersionFeatureTest.forVersionString(
    //         IModInfo.DependencySide.BOTH, System.getProperty("java.version")));
    // A JVM always has java.version; the port has no VM properties, so the feature is only registered
    // when the launcher set one (a NULL version is rejected by DefaultArtifactVersion, as in Java).
    const char *javaVersion = LIBMATTI_JL_System_GetProperty("java.version");
    if (javaVersion != NULL)
        LIBMATTI_NEOFORGESPI_ForgeFeature_RegisterFeature(
            "javaVersion", LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_ForVersionString(
                               LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH, javaVersion));

    // Java: FMLLoader.getCurrent().backgroundScanHandler.waitForScanToComplete(periodicTask);
    if (loader->backgroundScanHandler != NULL)
        LIBMATTI_FML_BackgroundScanHandler_WaitForScanToComplete(
            (LIBMATTI_FML_BackgroundScanHandler *) loader->backgroundScanHandler, periodicTask, userdata);

    // Java: ModList.of(loadingModList.getModFiles().stream().map(ModFileInfo::getFile).toList(), loadingModList.getMods())
    size_t modFileInfoCount = 0;
    LIBMATTI_FML_ModFileInfo **modFileInfos =
        LIBMATTI_FML_LoadingModList_GetModFiles(loadingModList, &modFileInfoCount);
    LIBMATTI_FML_ModFile **modFiles = calloc(modFileInfoCount > 0 ? modFileInfoCount : 1, sizeof(void *));
    for (size_t i = 0; i < modFileInfoCount; i++) modFiles[i] = LIBMATTI_FML_ModFileInfo_GetFile(modFileInfos[i]);

    size_t modCount = 0;
    LIBMATTI_FML_ModInfo **mods = LIBMATTI_FML_LoadingModList_GetMods(loadingModList, &modCount);

    LIBMATTI_FML_ModList *list = LIBMATTI_FML_ModList_Of(modFiles, modFileInfoCount, mods, modCount);
    if (!throwIfErrors(list))
    {
        free(modFiles);
        return;
    }

    // Java: for (var mod : loadingModList.getMods()) { for (var bound : mod.getForgeFeatures()) { ... } }
    for (size_t i = 0; i < modCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo *info = LIBMATTI_FML_ModInfo_AsModInfo(mods[i]);
        size_t boundCount = 0;
        void **bounds = LIBMATTI_NEOFORGESPI_IModInfo_GetForgeFeatures(info, &boundCount);

        for (size_t k = 0; k < boundCount; k++)
        {
            LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *bound = bounds[k];
            if (LIBMATTI_NEOFORGESPI_ForgeFeature_TestFeature(LIBMATTI_FML_FMLLoader_GetDist(loader), bound))
                continue;

            // Java passes the Bound record and the feature's value; the port has no record rendering.
            const char *args[] = {bound->featureName,
                                  (const char *) LIBMATTI_NEOFORGESPI_ForgeFeature_FeatureValue(bound)};
            LIBMATTI_FML_ModLoadingIssue *issue =
                LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.feature.missing", args, 2);

            LIBMATTI_NEOFORGESPI_IModFile *file =
                LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(info));
            LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(
                issue, mods[i], file, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file)));
            LIBMATTI_FML_ModLoadingIssue_Free(issue);
        }
        free(bounds);
    }
    if (!throwIfErrors(list))
    {
        free(modFiles);
        return;
    }

    // Java: var modContainers = loadingModList.getModFiles().stream().map(ModFileInfo::getFile)
    //         .map(ModLoader::buildMods).<ModContainer>mapMulti(Iterable::forEach).toList();
    LIBMATTI_FML_ModContainer **containers = NULL;
    size_t containerCount = 0;
    for (size_t i = 0; i < modFileInfoCount; i++)
        buildMods(LIBMATTI_FML_ModFile_AsModFile(LIBMATTI_FML_ModFileInfo_GetFile(modFileInfos[i])), &containers,
                  &containerCount);
    if (!throwIfErrors(list))
    {
        free(containers);
        free(modFiles);
        return;
    }

    // Java: modList.setLoadedMods(modContainers); ModLoader.modList = modList;
    modList = list;
    LIBMATTI_FML_ModList_SetLoadedMods(list, containers, containerCount);
    free(containers);
    free(modFiles);

    constructMods(periodicTask, userdata);
}

// Java: public static void runInitTask(String name, Executor syncExecutor, Runnable periodicTask, Runnable initTask)
void LIBMATTI_FML_ModLoader_RunInitTask(const char *name, void (*periodicTask)(void *userdata), void *userdata,
                                        void (*initTask)(void *userdata), void *taskUserdata)
{
    LIBMATTI_FML_ModLoader_WaitForTask(name, periodicTask, userdata, initTask, taskUserdata);
}

// Java: public static void waitForTask(String name, Runnable periodicTask, CompletableFuture<?> future)
void LIBMATTI_FML_ModLoader_WaitForTask(const char *name, void (*periodicTask)(void *userdata), void *userdata,
                                        void (*task)(void *userdata), void *taskUserdata)
{
    LIBMATTI_FML_ProgressMeter *progress = LIBMATTI_FML_StartupNotificationManager_AddProgressBar(name, 0);

    // Java: waitForFuture(name, periodicTask, future) - the port has no future, the task is the work.
    if (periodicTask != NULL) periodicTask(userdata);
    if (task != NULL) task(taskUserdata);

    LIBMATTI_FML_ProgressMeter_Complete(progress);
    LIBMATTI_FML_ProgressMeter_Free(progress);
}

// Java: public static void dispatchParallelTask(String name, Executor parallelExecutor, Runnable periodicTask,
//         Consumer<ModContainer> task)
void LIBMATTI_FML_ModLoader_DispatchParallelTask(const char *name, void (*periodicTask)(void *userdata),
                                                 void *userdata,
                                                 void (*task)(LIBMATTI_FML_ModContainer *container,
                                                              void *userdata),
                                                 void *taskUserdata)
{
    int steps = modList != NULL ? LIBMATTI_FML_ModList_Size(modList) : 0;
    LIBMATTI_FML_ProgressMeter *progress = LIBMATTI_FML_StartupNotificationManager_AddProgressBar(name, steps);

    // Java: periodicTask.run();
    if (periodicTask != NULL) periodicTask(userdata);

    size_t containerCount = 0;
    LIBMATTI_FML_ModContainer **containers =
        modList != NULL ? LIBMATTI_FML_ModList_GetSortedMods(modList, &containerCount) : NULL;

    // Java: the futures of a container's dependencies are awaited before its own task runs; a task whose
    //       dependency failed is skipped (DependentFutureFailedException).
    LIBMATTI_FML_LoadingModList *loadingModList =
        (LIBMATTI_FML_LoadingModList *) LIBMATTI_FML_FMLLoader_GetLoadingModList(LIBMATTI_FML_FMLLoader_GetCurrent());
    int *failed = calloc(containerCount > 0 ? containerCount : 1, sizeof(int));

    for (size_t i = 0; i < containerCount; i++)
    {
        LIBMATTI_FML_ModContainer *container = containers[i];

        int dependencyFailed = 0;
        if (loadingModList != NULL)
        {
            size_t dependencyCount = 0;
            LIBMATTI_FML_ModInfo **dependencies = LIBMATTI_FML_LoadingModList_GetDependencies(
                loadingModList, LIBMATTI_FML_ModInfo_AsConcrete(LIBMATTI_FML_ModContainer_GetModInfo(container)),
                &dependencyCount);

            for (size_t k = 0; k < dependencyCount; k++)
            {
                size_t index = containerCount;
                for (size_t j = 0; j < containerCount; j++)
                    if (LIBMATTI_FML_ModContainer_GetModInfo(containers[j]) ==
                        LIBMATTI_FML_ModInfo_AsModInfo(dependencies[k]))
                        index = j;

                if (index == containerCount)
                {
                    // Java: throw new IllegalStateException("Dependency future for mod %s which is a dependency of %s not found!")
                    LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                             "Dependency future for mod {} is missing",
                                             LIBMATTI_NEOFORGESPI_IModInfo_GetModId(
                                                 LIBMATTI_FML_ModInfo_AsModInfo(dependencies[k])));
                    continue;
                }

                if (failed[index]) dependencyFailed = 1;
            }
        }

        if (dependencyFailed)
        {
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                     "Skipping {} task for mod {} because a dependency threw an exception.", name,
                                     LIBMATTI_FML_ModContainer_GetModId(container));
            failed[i] = 1;
            LIBMATTI_FML_ProgressMeter_Increment(progress);
            continue;
        }

        // Java: ModLoadingContext.get().setActiveContainer(modContainer); task.accept(modContainer);
        //       finally { progress.increment(); ModLoadingContext.get().setActiveContainer(null); }
        LIBMATTI_FML_ModLoadingContext *context = LIBMATTI_FML_ModLoadingContext_Get();
        LIBMATTI_FML_ModLoadingContext_SetActiveContainer(context, container);
        if (task != NULL) task(container, taskUserdata);
        LIBMATTI_FML_ModLoadingContext_SetActiveContainer(context, NULL);
        LIBMATTI_FML_ProgressMeter_Increment(progress);
    }

    free(failed);
    // Java: getSortedMods() returns the list itself, not a copy - the array belongs to the ModList
    // and stays alive for the later dispatches; freeing it here would leave a dangling pointer.
    LIBMATTI_FML_ProgressMeter_Complete(progress);
    LIBMATTI_FML_ProgressMeter_Free(progress);
}

// Java: BiFunction<ModContainer, DeferredWorkQueue, ParallelDispatchEvent> - the port carries the queue
typedef struct
{
    LIBMATTI_FML_DeferredWorkQueue *workQueue;
    LIBMATTI_BUS_Event *(*eventConstructor)(LIBMATTI_FML_ModContainer *container,
                                            LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata);
    void *userdata;
} ParallelEventUserdata;

// Java: modContainer -> modContainer.acceptEvent(eventConstructor.apply(modContainer, workQueue))
static void dispatch_event(LIBMATTI_FML_ModContainer *container, void *userdata)
{
    ParallelEventUserdata *data = userdata;

    // Java builds one event per container and drops it again; the port frees it the same way.
    LIBMATTI_BUS_Event *event = data->eventConstructor(container, data->workQueue, data->userdata);
    LIBMATTI_FML_ModContainer_AcceptEvent(container, event);
    free_event(event);
}

// Java: public static void dispatchParallelEvent(String name, Executor syncExecutor, Executor parallelExecutor,
//         Runnable periodicTask, BiFunction<ModContainer, DeferredWorkQueue, ParallelDispatchEvent> eventConstructor)
void LIBMATTI_FML_ModLoader_DispatchParallelEvent(
    const char *name, void (*periodicTask)(void *userdata), void *userdata,
    LIBMATTI_BUS_Event *(*eventConstructor)(LIBMATTI_FML_ModContainer *container,
                                            LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata),
    void *constructorUserdata)
{
    LIBMATTI_FML_DeferredWorkQueue *workQueue = LIBMATTI_FML_DeferredWorkQueue_New(name);
    ParallelEventUserdata eventUserdata = {workQueue, eventConstructor, constructorUserdata};

    LIBMATTI_FML_ModLoader_DispatchParallelTask(name, periodicTask, userdata, dispatch_event, &eventUserdata);

    // Java: runInitTask(name + ": Deferred Queue", syncExecutor, periodicTask, workQueue::runTasks);
    char deferredName[256];
    snprintf(deferredName, sizeof(deferredName), "%s: Deferred Queue", name);
    LIBMATTI_FML_ModLoader_RunInitTask(deferredName, periodicTask, userdata, run_deferred_queue, workQueue);
    LIBMATTI_FML_DeferredWorkQueue_Free(workQueue);
}

// Java: ModList.get().forEachModInOrder(mc -> events.add(generator.apply(mc)))
typedef struct
{
    LIBMATTI_BUS_Event *(*generator)(LIBMATTI_FML_ModContainer *container, void *userdata);
    void *generatorUserdata;
    LIBMATTI_BUS_Event **events;
    size_t *eventCount;
} EventGeneratorUserdata;

static void generate_event(LIBMATTI_FML_ModContainer *container, void *userdata)
{
    EventGeneratorUserdata *data = userdata;
    data->events[(*data->eventCount)++] = data->generator(container, data->generatorUserdata);
}

// Java: public static <T extends Event & IModBusEvent> void runEventGenerator(Function<ModContainer, T> generator)
void LIBMATTI_FML_ModLoader_RunEventGenerator(
    LIBMATTI_BUS_Event *(*generator)(LIBMATTI_FML_ModContainer *container, void *userdata), void *userdata)
{
    if (LIBMATTI_FML_ModLoader_HasErrors())
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "Cowardly refusing to send event generator to a broken mod state");
        return;
    }

    // Java: List<ModContainer> modContainers = ModList.get().getSortedMods(); List<T> events = new ArrayList<>(size)
    size_t containerCount = 0;
    LIBMATTI_FML_ModContainer **containers =
        modList != NULL ? LIBMATTI_FML_ModList_GetSortedMods(modList, &containerCount) : NULL;
    LIBMATTI_BUS_Event **events = calloc(containerCount > 0 ? containerCount : 1, sizeof(void *));

    size_t eventCount = 0;
    EventGeneratorUserdata generatorUserdata = {generator, userdata, events, &eventCount};
    if (modList != NULL) LIBMATTI_FML_ModList_ForEachModInOrder(modList, generate_event, &generatorUserdata);

    // Java: for (EventPriority phase : EventPriority.values()) for (i) modContainers.get(i).acceptEvent(phase, events.get(i));
    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
        for (size_t i = 0; i < containerCount && i < eventCount; i++)
            LIBMATTI_FML_ModContainer_AcceptEventWithPhase(containers[i], (LIBMATTI_BUS_EventPriority) phase,
                                                          events[i]);

    // Java's generator result is a local, the port frees what the generator built for this call.
    for (size_t i = 0; i < eventCount; i++) free_event(events[i]);
    free(events);
    free(containers);
}

// Java: ModList.get().forEachModInOrder(mc -> mc.acceptEvent(phase, e))
typedef struct
{
    LIBMATTI_BUS_EventPriority phase;
    LIBMATTI_BUS_Event *event;
} PostEventUserdata;

static void post_event_to_container(LIBMATTI_FML_ModContainer *container, void *userdata)
{
    PostEventUserdata *data = userdata;
    LIBMATTI_FML_ModContainer_AcceptEventWithPhase(container, data->phase, data->event);
}

// Java: public static <T extends Event & IModBusEvent> void postEvent(T e)
void LIBMATTI_FML_ModLoader_PostEvent(LIBMATTI_BUS_Event *event)
{
    if (LIBMATTI_FML_ModLoader_HasErrors())
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cowardly refusing to send event {} to a broken mod state",
                                 LIBMATTI_JL_Class_GetName(event->clazz));
        return;
    }

    PostEventUserdata postUserdata = {LIBMATTI_BUS_EventPriority_HIGHEST, event};
    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
    {
        postUserdata.phase = (LIBMATTI_BUS_EventPriority) phase;
        if (modList != NULL) LIBMATTI_FML_ModList_ForEachModInOrder(modList, post_event_to_container, &postUserdata);
    }
}

// Java: public static <T extends Event & IModBusEvent> T postEventWithReturn(T e)
LIBMATTI_BUS_Event *LIBMATTI_FML_ModLoader_PostEventWithReturn(LIBMATTI_BUS_Event *event)
{
    LIBMATTI_FML_ModLoader_PostEvent(event);
    return event;
}

// Java: pre = (mc, e) -> ModLoadingContext.get().setActiveContainer(mc)
static void set_active_container(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata)
{
    (void) event;
    (void) userdata;
    LIBMATTI_FML_ModLoadingContext_SetActiveContainer(LIBMATTI_FML_ModLoadingContext_Get(), container);
}

// Java: post = (mc, e) -> ModLoadingContext.get().setActiveContainer(null)
static void clear_active_container(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata)
{
    (void) container;
    (void) event;
    (void) userdata;
    LIBMATTI_FML_ModLoadingContext_SetActiveContainer(LIBMATTI_FML_ModLoadingContext_Get(), NULL);
}

// Java: public static <T extends Event & IModBusEvent> void postEventWrapContainerInModOrder(T event)
void LIBMATTI_FML_ModLoader_PostEventWrapContainerInModOrder(LIBMATTI_BUS_Event *event)
{
    LIBMATTI_FML_ModLoader_PostEventWithWrapInModOrder(event, set_active_container, clear_active_container, NULL);
}

// Java: ModList.get().forEachModInOrder(mc -> { pre.accept(mc, e); mc.acceptEvent(phase, e); post.accept(mc, e); })
typedef struct
{
    LIBMATTI_BUS_EventPriority phase;
    LIBMATTI_BUS_Event *event;
    void (*pre)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata);
    void (*post)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata);
    void *userdata;
} WrappedPostEventUserdata;

static void wrapped_post_event_to_container(LIBMATTI_FML_ModContainer *container, void *userdata)
{
    WrappedPostEventUserdata *data = userdata;

    if (data->pre != NULL) data->pre(container, data->event, data->userdata);
    LIBMATTI_FML_ModContainer_AcceptEventWithPhase(container, data->phase, data->event);
    if (data->post != NULL) data->post(container, data->event, data->userdata);
}

// Java: public static <T extends Event & IModBusEvent> void postEventWithWrapInModOrder(T e, BiConsumer pre, BiConsumer post)
void LIBMATTI_FML_ModLoader_PostEventWithWrapInModOrder(
    LIBMATTI_BUS_Event *event,
    void (*pre)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata),
    void (*post)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata), void *userdata)
{
    if (LIBMATTI_FML_ModLoader_HasErrors())
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cowardly refusing to send event {} to a broken mod state",
                                 LIBMATTI_JL_Class_GetName(event->clazz));
        return;
    }

    WrappedPostEventUserdata wrapUserdata = {LIBMATTI_BUS_EventPriority_HIGHEST, event, pre, post, userdata};
    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
    {
        wrapUserdata.phase = (LIBMATTI_BUS_EventPriority) phase;
        if (modList != NULL)
            LIBMATTI_FML_ModList_ForEachModInOrder(modList, wrapped_post_event_to_container, &wrapUserdata);
    }
}
