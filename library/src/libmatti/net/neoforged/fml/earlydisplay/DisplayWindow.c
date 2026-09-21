// Port of net.neoforged.fml.earlydisplay.DisplayWindow.
//
// Java spins a scheduled renderer thread that draws the theme'd loading screen
// into the window. The C port owns the identical window lifecycle - init with
// the vanilla window hints, centered positioning, resize callback, hand-off to
// the game - and mirrors the renderer's progress through the
// StartupNotificationManager progress meters until the game takes over.

#include "libmatti/net/neoforged/fml/earlydisplay/DisplayWindow.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/fml/loading/progress/ProgressMeter.h"
#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"
#include "libmatti/org/lwjgl/glfw/GLFW.h"
#include "libmatti/org/lwjgl/util/tinyfd/TinyFileDialogs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: private static final Logger LOGGER = LoggerFactory.getLogger("EARLYDISPLAY");
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final String ERROR_URL = "https://links.neoforged.net/early-display-errors";
static const char *ERROR_URL = "https://links.neoforged.net/early-display-errors";

struct LIBMATTI_FML_DisplayWindow
{
    // Java: private final ProgressMeter mainProgress
    LIBMATTI_FML_ProgressMeter *mainProgress;

    int darkMode;
    int borderless;

    // Java: private long window - the GL ID of the window
    long window;
    int winWidth;
    int winHeight;
    char *assetsDir;
    char *assetIndex;

    int maximized;
    int closed;
    char *neoForgeVersion;
    char *minecraftVersion;
};

// Java: the window resize callback is an instance method; the port keeps the
// one instance the handler loaded (Java: ServiceLoader yields a single provider).
static LIBMATTI_FML_DisplayWindow *current_window = NULL;

void LIBMATTI_FML_DisplayWindow_SetCurrent(LIBMATTI_FML_DisplayWindow *window)
{
    current_window = window;
}

LIBMATTI_FML_DisplayWindow *LIBMATTI_FML_DisplayWindow_Current(void)
{
    return current_window;
}

// Java: /// Copied from SharedConstants.booleanProperty()
static int get_bool_property(const char *name)
{
    const char *value = LIBMATTI_JL_System_GetProperty(name);
    return value != NULL && (value[0] == '\0' || strcmp(value, "true") == 0 || strcmp(value, "TRUE") == 0);
}

// Java: private void crashElegantly(String errorDetails)
static void crash_elegantly(LIBMATTI_FML_DisplayWindow *window, const char *errorDetails)
{
    char msgBuilder[2000];
    snprintf(msgBuilder, sizeof(msgBuilder),
             "Failed to initialize the mod loading system and display.\n\n\nFailure details:\n%s\n\nIf you click "
             "yes, we will try and open %s in your default browser",
             errorDetails, ERROR_URL);
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "ERROR DISPLAY\n{}", msgBuilder);
    // Java: we show the display on a new dedicated thread; the port shows it inline
    int res = LIBMATTI_LWJGL_TINYFD_TinyFileDialogs_TinyfdMessageBox("Minecraft: NeoForge", msgBuilder, "yesno",
                                                                     "error", 1);
    if (res == 1)
    {
        // Java: Desktop.getDesktop().browse(URI.create(ERROR_URL)); - no desktop toolkit
        LIBMATTI_LWJGL_TINYFD_TinyFileDialogs_TinyfdMessageBox(
            "Minecraft: NeoForge", "Sadly, we couldn't open your browser.\nVisit https://links.neoforged.net/early-display-errors",
            "ok", "error", 1);
    }

    LIBMATTI_FML_DisplayWindow_Free(window);
    exit(1);
}

// Java: private static Optional<String> getLastGlfwError()
static int get_last_glfw_error(char *out, size_t outSize)
{
    const char *description = NULL;
    int error = LIBMATTI_GLFW_glfwGetError(&description);
    if (error != LIBMATTI_GLFW_NO_ERROR)
    {
        if (description != NULL)
            snprintf(out, outSize, "[0x%X] %s", error, description);
        else
            snprintf(out, outSize, "[0x%X]", error);
        return 1;
    }
    return 0;
}

// Java: private void setGlfwWindowIcon() - the theme icon loader does not exist yet,
// so the port keeps the window icon unset exactly on failure like Java logs it.
static void set_window_icon(LIBMATTI_FML_DisplayWindow *window)
{
    (void) window;
}

// Java: private void winResize(long window, int width, int height)
static void win_resize(long callbackWindow, int width, int height)
{
    // Java: if (window == this.window && width != 0 && height != 0)
    if (width != 0 && height != 0)
    {
        LIBMATTI_FML_DisplayWindow *window = LIBMATTI_FML_DisplayWindow_Current();
        if (window != NULL && callbackWindow == window->window)
        {
            window->winWidth = width;
            window->winHeight = height;
        }
    }
}

// Java: public void initWindow()
void LIBMATTI_FML_DisplayWindow_InitWindow(LIBMATTI_FML_DisplayWindow *window)
{
    // Java: boolean preferWayland = getBoolProperty("MC_DEBUG_ENABLED") && getBoolProperty("MC_DEBUG_PREFER_WAYLAND");
    int preferWayland = get_bool_property("MC_DEBUG_ENABLED") && get_bool_property("MC_DEBUG_PREFER_WAYLAND");
    if (LIBMATTI_GLFW_glfwPlatformSupported(LIBMATTI_GLFW_PLATFORM_WAYLAND) &&
        LIBMATTI_GLFW_glfwPlatformSupported(LIBMATTI_GLFW_PLATFORM_X11) && !preferWayland)
    {
        LIBMATTI_GLFW_glfwInitHint(LIBMATTI_GLFW_PLATFORM, LIBMATTI_GLFW_PLATFORM_X11);
    }

    // Java: Initialize GLFW with a time guard, in case something goes wrong
    long glfwInitBegin = (long) time(NULL) * 1000000000L;
    if (!LIBMATTI_GLFW_glfwInit())
    {
        crash_elegantly(window, "We are unable to initialize the graphics system.\nglfwInit failed.\n");
        return;
    }
    long glfwInitEnd = (long) time(NULL) * 1000000000L;

    if (glfwInitEnd - glfwInitBegin > 1000000000L)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "WARNING : glfwInit took {} seconds to start.",
                                 (double) (glfwInitEnd - glfwInitBegin) / 1.0e9);
    }

    // Java: Clear the Last Exception (#7285)
    char glfwError[128];
    if (get_last_glfw_error(glfwError, sizeof(glfwError)))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Suppressing Last GLFW error: {}", glfwError);
    }

    // Java: Set window hints for the new window we're gonna create.
    LIBMATTI_GLFW_glfwDefaultWindowHints();
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CLIENT_API, LIBMATTI_GLFW_OPENGL_API);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_CREATION_API, LIBMATTI_GLFW_NATIVE_CONTEXT_API);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MAJOR, 3);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MINOR, 3);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_PROFILE, LIBMATTI_GLFW_OPENGL_CORE_PROFILE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_FORWARD_COMPAT, LIBMATTI_GLFW_TRUE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_SOFT_FULLSCREEN, window->borderless ? LIBMATTI_GLFW_TRUE : LIBMATTI_GLFW_FALSE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_VISIBLE, LIBMATTI_GLFW_FALSE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_RESIZABLE, LIBMATTI_GLFW_TRUE);
    // Java: this emulates what we would get without early progress window
    const char *vanillaWindowTitle = "Minecraft*";
    LIBMATTI_GLFW_glfwWindowHintString(LIBMATTI_GLFW_X11_CLASS_NAME, vanillaWindowTitle);
    LIBMATTI_GLFW_glfwWindowHintString(LIBMATTI_GLFW_X11_INSTANCE_NAME, vanillaWindowTitle);

    long primaryMonitor = LIBMATTI_GLFW_glfwGetPrimaryMonitor();
    if (primaryMonitor == 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to find a primary monitor - this means LWJGL isn't working properly");
        crash_elegantly(window, "Failed to locate a primary monitor.\nglfwGetPrimaryMonitor failed.\n");
        return;
    }
    int vidWidth = 0, vidHeight = 0, r = 0, g = 0, b = 0, rr = 0;
    if (!LIBMATTI_GLFW_glfwGetVideoMode(primaryMonitor, &vidWidth, &vidHeight, &r, &g, &b, &rr))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to get the current display video mode.");
        crash_elegantly(window, "Failed to get current display resolution.\nglfwGetVideoMode failed.\n");
        return;
    }

    LIBMATTI_FML_DisplayWindow_SetCurrent(window);

    window->window = LIBMATTI_GLFW_glfwCreateWindow(window->winWidth, window->winHeight,
                                                    "Minecraft: NeoForge Loading...", 0L, 0L);
    char creationError[128] = "unknown error";
    get_last_glfw_error(creationError, sizeof(creationError));
    if (window->window == 0L)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to create window: {}", creationError);
        crash_elegantly(window, creationError);
        return;
    }

    int monitorX = 0, monitorY = 0;
    LIBMATTI_GLFW_glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);
    if (window->maximized)
    {
        LIBMATTI_GLFW_glfwMaximizeWindow(window->window);
    }

    LIBMATTI_GLFW_glfwGetWindowSize(window->window, &window->winWidth, &window->winHeight);

    LIBMATTI_GLFW_glfwSetWindowPos(window->window, (vidWidth - window->winWidth) / 2 + monitorX,
                                   (vidHeight - window->winHeight) / 2 + monitorY);

    set_window_icon(window);

    LIBMATTI_GLFW_glfwSetWindowSizeCallback(window->window, win_resize);

    // Java: Show the window
    LIBMATTI_GLFW_glfwShowWindow(window->window);
    if (get_last_glfw_error(glfwError, sizeof(glfwError)))
    {
        LIBMATTI_ML_Logger_Warn(LOGGER(), NULL, "Failed to show and position window: {}", glfwError);
    }
    LIBMATTI_GLFW_glfwPollEvents();
}

// Java: public DisplayWindow()
LIBMATTI_FML_DisplayWindow *LIBMATTI_FML_DisplayWindow_New(void)
{
    LIBMATTI_FML_DisplayWindow *window = calloc(1, sizeof(LIBMATTI_FML_DisplayWindow));
    // Java: mainProgress = StartupNotificationManager.addProgressBar("", 0);
    window->mainProgress = LIBMATTI_FML_StartupNotificationManager_AddProgressBar("", 0);
    window->borderless = 1;
    return window;
}

void LIBMATTI_FML_DisplayWindow_Free(LIBMATTI_FML_DisplayWindow *window)
{
    if (window == NULL) return;
    LIBMATTI_FML_ProgressMeter_Free(window->mainProgress);
    free(window->assetsDir);
    free(window->assetIndex);
    free(window->neoForgeVersion);
    free(window->minecraftVersion);
    free(window);
}

// Java: @Override public String name()
static const char *display_window_name(void *self)
{
    (void) self;
    return "fmlearlywindow";
}

// Java: @Override public void initialize(ProgramArgs arguments)
static void display_window_initialize(void *self, LIBMATTI_FML_ProgramArgs *arguments)
{
    LIBMATTI_FML_DisplayWindow *window = self;

    // Java: the OptionParser reads width/height/earlywindow.maximized/assetsDir/assetIndex
    size_t count = 0;
    char **args = LIBMATTI_FML_ProgramArgs_GetArguments(arguments, &count);
    window->winWidth = LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_WIDTH);
    window->winHeight = LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_HEIGHT);
    window->maximized = LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_MAXIMIZED);
    for (size_t i = 0; i + 1 < count; i++)
    {
        if (strcmp(args[i], "--width") == 0)
            window->winWidth = atoi(args[i + 1]);
        else if (strcmp(args[i], "--height") == 0)
            window->winHeight = atoi(args[i + 1]);
        else if (strcmp(args[i], "--earlywindow.maximized") == 0)
            window->maximized = 1;
        else if (strcmp(args[i], "--assetsDir") == 0)
        {
            free(window->assetsDir);
            window->assetsDir = strdup(args[i + 1]);
        }
        else if (strcmp(args[i], "--assetIndex") == 0)
        {
            free(window->assetIndex);
            window->assetIndex = strdup(args[i + 1]);
        }
    }
    LIBMATTI_FML_FMLConfig_UpdateConfigInt(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_WIDTH, window->winWidth);
    LIBMATTI_FML_FMLConfig_UpdateConfigInt(LIBMATTI_FML_FMLConfig_EARLY_WINDOW_HEIGHT, window->winHeight);

    // Java: read darkMode/borderless from options.txt in the game dir
    int darkMode = 0;
    int borderless = 1;
    char *optionsPath = LIBMATTI_JNF_Path_Resolve(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_GAMEDIR), "options.txt");
    char *optionsContent = LIBMATTI_JNF_Files_ReadString(optionsPath);
    if (optionsContent != NULL)
    {
        char *line = optionsContent;
        while (line != NULL && line[0] != '\0')
        {
            char *next = strchr(line, '\n');
            if (next != NULL) *next = '\0';
            if (strncmp(line, "darkMojangStudiosBackground:", strlen("darkMojangStudiosBackground:")) == 0)
            {
                char *lower = strdup(line);
                for (char *p = lower; *p != '\0'; p++)
                    if (*p >= 'A' && *p <= 'Z') *p = (char) (*p - 'A' + 'a');
                darkMode = strstr(lower, "true") != NULL && strlen(lower) > strlen("darkMojangStudiosBackground:") &&
                           strcmp(lower + strlen(lower) - 4, "true") == 0;
                free(lower);
            }
            else if (strncmp(line, "exclusiveFullscreen:", strlen("exclusiveFullscreen:")) == 0)
            {
                char *lower = strdup(line);
                for (char *p = lower; *p != '\0'; p++)
                    if (*p >= 'A' && *p <= 'Z') *p = (char) (*p - 'A' + 'a');
                borderless = !(strlen(lower) > strlen("exclusiveFullscreen:") &&
                               strcmp(lower + strlen(lower) - 4, "false") == 0);
                free(lower);
            }
            line = next != NULL ? next + 1 : NULL;
        }
        free(optionsContent);
    }
    free(optionsPath);
    window->darkMode = get_bool_property("fml.earlyWindowDarkMode") || darkMode;
    window->borderless = borderless;

    // Java: this.maximized = parsed.has(maximizedopt) || FMLConfig.getBoolConfigValue(...);

    LIBMATTI_FML_DisplayWindow_InitWindow(window);

    // Java: rendererFuture = renderScheduler.schedule(() -> new LoadingScreenRenderer(...), 1, MILLISECONDS);
    // The renderer port does not exist yet; the progress meters carry the same state.

    // Java: updateProgress("Initializing Game Graphics");
    LIBMATTI_FML_ProgressMeter_SetLabel(window->mainProgress, "Initializing Game Graphics");
}

// Java: @Override public void setMinecraftVersion(String version)
static void display_window_set_minecraft_version(void *self, const char *version)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    free(window->minecraftVersion);
    // Java: version may be null when the game version is unknown; the port mirrors that
    window->minecraftVersion = version != NULL ? strdup(version) : NULL;
}

// Java: @Override public void setNeoForgeVersion(String version)
static void display_window_set_neoforge_version(void *self, const char *version)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    if (version == NULL || window->neoForgeVersion == NULL ||
        strcmp(window->neoForgeVersion, version) != 0)
    {
        free(window->neoForgeVersion);
        window->neoForgeVersion = version != NULL ? strdup(version) : NULL;
        if (version != NULL)
        {
            LIBMATTI_FML_StartupNotificationManager_Consumer consumer =
                LIBMATTI_FML_StartupNotificationManager_ModLoaderConsumer();
            if (consumer != NULL)
            {
                char message[128];
                snprintf(message, sizeof(message), "Starting NeoForge %s", version);
                consumer(message);
            }
        }
    }
}

// Java: public long takeOverGlfwWindow()
long LIBMATTI_FML_DisplayWindow_TakeOverGlfwWindow(LIBMATTI_FML_DisplayWindow *window)
{
    // Java: wait for the renderer, stop automatic rendering, complete the progress bar
    LIBMATTI_FML_ProgressMeter_Complete(window->mainProgress);

    LIBMATTI_GLFW_glfwMakeContextCurrent(window->window);
    // Java: Set the title to what the game wants
    LIBMATTI_GLFW_glfwSwapInterval(0);
    // Java: Clean up our hooks
    LIBMATTI_GLFW_glfwSetWindowSizeCallback(window->window, NULL);
    return window->window;
}

// Java: public void renderToFramebuffer()
void LIBMATTI_FML_DisplayWindow_RenderToFramebuffer(LIBMATTI_FML_DisplayWindow *window)
{
    // Java: rendererFuture.resultNow().renderToFramebuffer(); - renderer port pending
    (void) window;
}

// Java: public int getFramebufferTextureId()
int LIBMATTI_FML_DisplayWindow_GetFramebufferTextureId(LIBMATTI_FML_DisplayWindow *window)
{
    // Java: rendererFuture.resultNow().getFramebufferTextureId(); - renderer port pending
    (void) window;
    return 0;
}

// Java: @Override public void periodicTick()
static void display_window_periodic_tick(void *self)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    LIBMATTI_GLFW_glfwPollEvents();
    // Java: if (!closed) { repaintTick.run(); } - the renderer port carries the repaint
}

// Java: @Override public void updateProgress(String label)
static void display_window_update_progress(void *self, const char *label)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    LIBMATTI_FML_ProgressMeter_SetLabel(window->mainProgress, label);
}

// Java: @Override public void completeProgress()
static void display_window_complete_progress(void *self)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    LIBMATTI_FML_ProgressMeter_Complete(window->mainProgress);
}

// Java: @Override public void crash(String message)
static void display_window_crash(void *self, const char *message)
{
    crash_elegantly(self, message);
}

// Java: @Override public void displayFatalErrorAndExit(...)
static void display_window_display_fatal_error_and_exit(void *self, LIBMATTI_FML_ModLoadingIssue **issues,
                                                        size_t issueCount, const char *modsFolder,
                                                        const char *logFile, const char *crashReportFile)
{
    LIBMATTI_FML_DisplayWindow *window = self;
    long windowId = LIBMATTI_FML_DisplayWindow_TakeOverGlfwWindow(window);
    LIBMATTI_FML_DisplayWindow_Close(window);
    // Java: ErrorDisplay.fatal(windowId, assetsDir, assetIndex, issues, modsFolder, logFile, crashReportFile);
    // The ErrorDisplay port does not exist; the fatal details go to the crash dialog instead.
    (void) windowId;
    (void) issues;
    (void) issueCount;
    (void) modsFolder;
    (void) logFile;
    (void) crashReportFile;
    // Java: ErrorDisplay terminates the game
    LIBMATTI_FML_DisplayWindow_Free(window);
    exit(1);
}

// Java: public void close()
void LIBMATTI_FML_DisplayWindow_Close(LIBMATTI_FML_DisplayWindow *window)
{
    if (!window->closed)
    {
        window->closed = 1;
        // Java: renderScheduler.shutdown(); rendererFuture.get().close();
        if (window->window != 0)
        {
            LIBMATTI_GLFW_glfwDestroyWindow(window->window);
            window->window = 0;
        }
    }
}

// Java: the ServiceLoader-visible vtable of DisplayWindow
LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *LIBMATTI_FML_DisplayWindow_AsProvider(
    LIBMATTI_FML_DisplayWindow *window)
{
    LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider = calloc(1, sizeof(*provider));
    provider->controller.self = window;
    provider->controller.takeOverGlfwWindow = (long (*)(void *)) LIBMATTI_FML_DisplayWindow_TakeOverGlfwWindow;
    provider->controller.periodicTick = display_window_periodic_tick;
    provider->controller.updateProgress = display_window_update_progress;
    provider->controller.completeProgress = display_window_complete_progress;
    provider->name = display_window_name;
    provider->initialize = display_window_initialize;
    provider->setMinecraftVersion = display_window_set_minecraft_version;
    provider->setNeoForgeVersion = display_window_set_neoforge_version;
    provider->crash = display_window_crash;
    provider->displayFatalErrorAndExit = display_window_display_fatal_error_and_exit;
    return provider;
}
