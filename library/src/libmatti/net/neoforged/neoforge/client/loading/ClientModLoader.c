// Port of net.neoforged.neoforge.client.loading.ClientModLoader.

#include "libmatti/net/neoforged/neoforge/client/loading/ClientModLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"
#include "libmatti/net/neoforged/neoforge/internal/CommonModLoader.h"
#include "libmatti/net/neoforged/neoforge/client/network/registration/ClientNetworkRegistry.h"
#include "libmatti/net/neoforged/neoforge/logging/CrashReportExtender.h"
#include "libmatti/net/neoforged/neoforge/resource/ResourcePackLoader.h"
#include "libmatti/net/neoforged/neoforge/server/LanguageHook.h"

#include <stdio.h>
#include <stdlib.h>

// Java: private static final Logger LOGGER = LogManager.getLogger();
#define LOGGER() LIBMATTI_ML_LogManager_GetLogger()

// Java: private static boolean loading; private static boolean loadingComplete;
static int loading = 0;
static int loadingComplete = 0;
// Java: @Nullable private static ModLoadingException error;
static int hasError = 0;

// Java: catchLoadingException wraps a run() that throws ModLoadingException; the port's ModLoader
// records loading issues instead of throwing, so the error state comes from ModLoader.hasErrors().
static void catch_loading_exception(void (*run)(void *userdata), void *userdata)
{
    // Java: if (loadingComplete) return; if (ModLoader.hasErrors()) return;
    if (loadingComplete) return;
    if (LIBMATTI_FML_ModLoader_HasErrors())
    {
        hasError = 1;
        return;
    }

    run(userdata);
    if (LIBMATTI_FML_ModLoader_HasErrors()) hasError = 1;
}

static void run_load(void *userdata)
{
    (void) userdata;
    // Java: load(syncExecutor, parallelExecutor) - CommonModLoader's protected load
    LIBMATTI_NEOFORGE_CommonModLoader_Load();
}

static void run_finish_and_network(void *userdata)
{
    (void) userdata;
    // Java: finish(syncExecutor, parallelExecutor); ModLoader.runInitTask("Client network registry lock",
    //       syncExecutor, periodicTask, ClientNetworkRegistry::setup)
    LIBMATTI_NEOFORGE_CommonModLoader_Finish();
    LIBMATTI_NEOFORGE_ClientNetworkRegistry_Setup();
}

// Java: public static void begin()
void LIBMATTI_NEOFORGE_ClientModLoader_Begin(void)
{
    // Java: Runtime.getRuntime().addShutdownHook(new Thread(LogManager::shutdown)) - the port's
    // logger writes to stdout/stderr and needs no shutdown hook.

    // Java: var earlyLoadingScreen = EarlyLoadingScreenController.current();
    LIBMATTI_FML_EarlyLoadingScreenController *earlyLoadingScreen =
        LIBMATTI_FML_EarlyLoadingScreenController_Current();
    if (earlyLoadingScreen != NULL)
        LIBMATTI_FML_EarlyLoadingScreenController_UpdateProgress(earlyLoadingScreen, "Loading mods");

    loading = 1;

    // Java: LanguageHook.loadBuiltinLanguages();
    LIBMATTI_NEOFORGE_LanguageHook_LoadBuiltinLanguages();

    // Java: Runnable periodicTick = earlyLoadingScreen != null ? earlyLoadingScreen::periodicTick : () -> {};
    void (*periodicTick)(void *userdata) = NULL;
    void *tickUserdata = NULL;
    if (earlyLoadingScreen != NULL)
    {
        periodicTick = (void (*)(void *userdata)) LIBMATTI_FML_EarlyLoadingScreenController_PeriodicTick;
        tickUserdata = earlyLoadingScreen;
    }

    // Java: try { begin(periodicTick, false); } catch (ModLoadingException e) { error = e; }
    LIBMATTI_NEOFORGE_CommonModLoader_Begin(periodicTick, tickUserdata, 0);
    if (LIBMATTI_FML_ModLoader_HasErrors()) hasError = 1;
}

// Java: public static void finish(final PackRepository defaultResourcePacks, final ReloadableResourceManager mcResourceManager)
void LIBMATTI_NEOFORGE_ClientModLoader_Finish(void *defaultResourcePacks, void *mcResourceManager)
{
    (void) mcResourceManager;
    // Java: if (error == null) { ResourcePackLoader.populatePackRepository(defaultResourcePacks, PackType.CLIENT_RESOURCES, false); DataPackConfig.DEFAULT.addModPacks(...); }
    if (hasError) return;
    LIBMATTI_NEOFORGE_ResourcePackLoader_PopulatePackRepository(defaultResourcePacks, 0 /* PackType.CLIENT_RESOURCES */, 0);
}

// Java: private static void finishModLoading(Executor syncExecutor, Executor parallelExecutor)
static void finish_mod_loading(void)
{
    // Java: catchLoadingException(() -> { finish(...); ModLoader.runInitTask("Client network registry lock", ...); })
    catch_loading_exception(run_finish_and_network, NULL);
    loading = 0;
    loadingComplete = 1;
}

// Java: startModLoading - the onResourceReload first stage
static void start_mod_loading(void)
{
    catch_loading_exception(run_load, NULL);
}

// Java: public static CompletableFuture<Void> onResourceReload(PreparableReloadListener.SharedState sharedState,
//         Executor asyncExecutor, PreparationBarrier stage, Executor syncExecutor)
// Java's future chain runs startModLoading, waits on the barrier, then finishModLoading; the port
// runs the stages in order on the caller's thread.
void LIBMATTI_NEOFORGE_ClientModLoader_OnResourceReload(void)
{
    start_mod_loading();
    // Java: .thenCompose(stage::wait) - the barrier hands the background stage over to the sync
    // thread; the port's stages run on the caller's thread, so the wait is the sequencing point
    // between startModLoading and finishModLoading (PreparationBarrier is the game port's type).
    finish_mod_loading();
}

// Java: public static Runnable completeModLoading(Runnable initialScreensTask)
void (*LIBMATTI_NEOFORGE_ClientModLoader_CompleteModLoading(void (*initialScreensTask)(void *userdata)))(
    void *userdata)
{
    // Java: List<ModLoadingIssue> warnings = ModLoader.getLoadingIssues();
    size_t issueCount = 0;
    LIBMATTI_FML_ModLoadingIssue **issues = LIBMATTI_FML_ModLoader_GetLoadingIssues(&issueCount);

    // Java: showWarnings = NeoForgeClientConfig.INSTANCE.showLoadWarnings.get(), catch -> assume true
    int showWarnings = 1;

    if (hasError)
    {
        // Java: LanguageHook.loadBuiltinLanguages();
        LIBMATTI_NEOFORGE_LanguageHook_LoadBuiltinLanguages();
        // Java: dumpLocation = CrashReportExtender.dumpModLoadingCrashReport(LOGGER, error.getIssues(),
        //       Minecraft.getInstance().gameDirectory)
        char *dumpLocation = LIBMATTI_NEOFORGE_CrashReportExtender_DumpModLoadingCrashReport(LOGGER(), issues,
                                                                                             issueCount, ".");
        // Java: return () -> Minecraft.setScreen(new LoadingErrorScreen(error.getIssues(), dumpedLocation, () -> {}))
        // - the screen is the game port's part; the report path is reported instead
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Client mod loading failed; the game shows the error screen");
        free(dumpLocation);
        for (size_t i = 0; i < issueCount; i++)
            LIBMATTI_FML_ModLoadingIssue_Free(issues[i]);
        free(issues);
        return NULL;
    }

    if (issueCount > 0)
    {
        if (showWarnings)
        {
            // Java: return () -> Minecraft.setScreen(new LoadingErrorScreen(warnings, null, initialScreensTask))
            // The port has no screen yet, so the initial task runs and the warnings were already logged.
        }
        else
        {
            // Java: LOGGER.warn(LOADING, "Mods loaded with {} warning(s)", warnings.size()); per warning: key [args]
            LIBMATTI_ML_Logger_Warn(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Mods loaded with {} warning(s)",
                                    issueCount == 1 ? "1" : "many");
        }
    }

    for (size_t i = 0; i < issueCount; i++)
        LIBMATTI_FML_ModLoadingIssue_Free(issues[i]);
    free(issues);

    // Java: return initialScreensTask - the runnable the caller passed runs unchanged
    return initialScreensTask;
}

// Java: static boolean isLoading()
int LIBMATTI_NEOFORGE_ClientModLoader_IsLoading(void)
{
    return loading;
}
