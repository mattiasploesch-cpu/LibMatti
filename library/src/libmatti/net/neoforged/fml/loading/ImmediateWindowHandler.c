// Port of net.neoforged.fml.loading.ImmediateWindowHandler.
//
// Java discovers the provider through the ServiceLoader; the C port registers
// the built-in fmlearlywindow provider (DisplayWindow) directly, since the C
// port has no META-INF/services to read.

#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/earlydisplay/DisplayWindow.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/neoforgespi/earlywindow/GraphicsBootstrapper.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogManager.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: static ImmediateWindowProvider provider
static LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider = NULL;
// Java: the provider is owned by the handler; the port keeps the DisplayWindow instance
static LIBMATTI_FML_DisplayWindow *displayWindow = NULL;

void *LIBMATTI_FML_ImmediateWindowHandler_GetProvider(void)
{
    return provider;
}

// Java: public static void load(ILaunchContext context, boolean headless, ProgramArgs arguments)
void LIBMATTI_FML_ImmediateWindowHandler_Load(LIBMATTI_NEOFORGESPI_ILaunchContext *context, int headless,
                                              LIBMATTI_FML_ProgramArgs *arguments)
{
    // Java: ServiceLoaderUtil.loadEarlyServices(context, GraphicsBootstrapper.class, List.of()).forEach(bootstrap -> {
    //           LOGGER.info("Running graphics bootstrap plugin {}", bootstrap.name());
    //           bootstrap.bootstrap(arguments.getArguments()); });
    (void) context;

    if (headless)
    {
        // Java: provider = null; LOGGER.info("Not loading early display in headless mode.");
        provider = NULL;
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Not loading early display in headless mode.");
        return;
    }

    if (!LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_CONTROL))
    {
        // Java: provider = null; LOGGER.info("ImmediateWindowProvider not loading because splash screen is disabled");
        provider = NULL;
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "ImmediateWindowProvider not loading because splash screen is disabled");
    }
    else
    {
        // Java: var providername = FMLConfig.getConfigValue(FMLConfig.ConfigValue.EARLY_WINDOW_PROVIDER);
        const char *providername = LIBMATTI_FML_FMLConfig_GetConfigValue(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_PROVIDER);
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Loading ImmediateWindowProvider {}", providername);

        // Java: var maybeProvider = ServiceLoaderUtil.loadEarlyServices(context, ImmediateWindowProvider.class, List.of())
        //           .stream().filter(p -> Objects.equals(p.name(), providername)).findFirst();
        // The C port has no ServiceLoader; the built-in fmlearlywindow provider is the candidate.
        displayWindow = LIBMATTI_FML_DisplayWindow_New();
        LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *candidate =
            LIBMATTI_FML_DisplayWindow_AsProvider(displayWindow);
        if (strcmp(candidate->name(candidate->controller.self), providername) == 0)
        {
            provider = candidate;
        }
        else
        {
            LIBMATTI_FML_DisplayWindow_Free(displayWindow);
            displayWindow = NULL;
            free(candidate);
        }

        if (provider == NULL)
        {
            // Java: LOGGER.info("Failed to find ImmediateWindowProvider {}, disabling", providername);
            LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Failed to find ImmediateWindowProvider {}, disabling", providername);
        }
        else
        {
            // Java: try { provider.initialize(arguments); } catch (Exception e) { LOGGER.error(...); provider = null; }
            provider->initialize(provider->controller.self, arguments);
        }
    }

    // Java: if (provider != null) { FMLConfig.updateConfig(FMLConfig.ConfigValue.EARLY_WINDOW_PROVIDER, provider.name()); }
    if (provider != NULL)
    {
        LIBMATTI_FML_FMLConfig_UpdateConfigString(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_PROVIDER,
                                                  provider->name(provider->controller.self));
    }
}

// Java: public static void setNeoForgeVersion(String version)
void LIBMATTI_FML_ImmediateWindowHandler_SetNeoForgeVersion(const char *version)
{
    if (provider == NULL) return;

    // Java: provider.setNeoForgeVersion(version);
    provider->setNeoForgeVersion(provider->controller.self, version);
}

// Java: public static void setMinecraftVersion(String version)
void LIBMATTI_FML_ImmediateWindowHandler_SetMinecraftVersion(const char *version)
{
    if (provider == NULL) return;

    // Java: provider.setMinecraftVersion(version);
    provider->setMinecraftVersion(provider->controller.self, version);
}

// Java: public static void renderTick()
void LIBMATTI_FML_ImmediateWindowHandler_RenderTick(void)
{
    if (provider == NULL) return;

    // Java: provider.periodicTick();
    provider->controller.periodicTick(provider->controller.self);
}

// Java: public static void updateProgress(String message)
void LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress(const char *message)
{
    if (provider == NULL) return;

    // Java: provider.updateProgress(message);
    provider->controller.updateProgress(provider->controller.self, message);
}

// Java: public static void crash(String message)
void LIBMATTI_FML_ImmediateWindowHandler_Crash(const char *message)
{
    if (provider == NULL) return;

    // Java: provider.crash(message);
    provider->crash(provider->controller.self, message);
}

// Java: public static void displayFatalErrorAndExit(List<ModLoadingIssue> issues, Path modsFolder, Path logFile, Path crashReportFile)
void LIBMATTI_FML_ImmediateWindowHandler_DisplayFatalErrorAndExit(LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount,
                                                                  const char *modsFolder, const char *logFile,
                                                                  const char *crashReportFile)
{
    if (provider == NULL) return;

    // Java: provider.displayFatalErrorAndExit(issues, modsFolder, logFile, crashReportFile);
    provider->displayFatalErrorAndExit(provider->controller.self, issues, issueCount, modsFolder, logFile,
                                       crashReportFile);
}
