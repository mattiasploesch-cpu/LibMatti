// Port of net.minecraft.client.Minecraft (the game skeleton). The constructor
// opens the GLFW window (Window.java + Window init in the constructor), run()
// is the while (!stopped) loop of Minecraft.run, runTick() carries the
// advanceTime/tick/render split, destroy() the shutdown path.

#include "libmatti/net/minecraft/client/Minecraft.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/matti/mixin/MixinHookTable.h"
#include "libmatti/matti_mixin.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureManager.h"
#include "libmatti/net/neoforged/fml/earlydisplay/EarlyFramebuffer.h"
#include "libmatti/net/neoforged/fml/earlydisplay/FontShader.h"
#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"
#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"
#include "libmatti/org/lwjgl/glfw/GLFW.h"
#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The theme resource root (see load_font below); every consumer defines it
// from CMake (client/src does the same for the selftest fixtures).
#ifndef MATTI_SOURCE_DIR
#define MATTI_SOURCE_DIR "."
#endif

// Java: private static final Logger LOGGER = LogUtils.getLogger();
#define LOG(...)                                                                                     \
    do                                                                                               \
    {                                                                                                \
        char msgbuf[512];                                                                            \
        snprintf(msgbuf, sizeof(msgbuf), __VA_ARGS__);                                               \
        LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, msgbuf);                   \
    } while (0)

struct LIBMATTI_MC_Minecraft
{
    // Java: private final GameConfig gameConfig values we keep
    LIBMATTI_MC_GameConfig config;
    char *launchVersion;
    char *versionType;

    // Java: private final Window window (the native GLFW handle)
    long window;

    // Java: private final DeltaTracker.Timer deltaTracker
    LIBMATTI_MC_DeltaTracker *deltaTracker;

    // Java: private volatile boolean running / public boolean stopped / pause / noRender
    int running;
    int stopped;
    int pause;
    int noRender;

    // Java: public int frames; private int fps (framerateLimitTracker keeps the
    // real one; the skeleton counts frames over one second)
    int frames;
    int fps;
    double fpsTimerStart;

    // Java: private long frameTimeNs
    long long frameTimeNs;

    // Java: private final TextureManager textureManager = new TextureManager(this.resourceManager)
    LIBMATTI_MC_TextureManager *textureManager;

    // The earlydisplay Monocraft font (Java: MaterializedTheme.fonts["gui"]) the
    // skeleton draws the title line with. NULL when the TTF is unavailable.
    LIBMATTI_FML_SimpleFont *font;

    // The font shader (Java: Theme.SHADER_FONT): screen-space quads tinted by
    // the GL_RED glyph texture.
    unsigned int fontProgram;
    int fontScreenSizeLocation;

    // Java: private final EarlyFramebuffer framebuffer = new EarlyFramebuffer(
    // LAYOUT_WIDTH, LAYOUT_HEIGHT) - the loading-screen layout is rendered into
    // this offscreen buffer and blitted (vertically flipped) onto the window.
    LIBMATTI_FML_EarlyFramebuffer *framebuffer;

    // The in-memory level the skeleton drives (game port content).
    void *level;
};

// Java: public static Minecraft getInstance()
static LIBMATTI_MC_Minecraft *instance = NULL;

LIBMATTI_MC_Minecraft *LIBMATTI_MC_Minecraft_GetInstance(void)
{
    return instance;
}

float LIBMATTI_MC_Minecraft_GetTickTargetMillis(float msPerTick)
{
    // Java: getTickTargetMillis returns the mspt override when set; the default
    // run keeps 50 ms.
    (void) msPerTick;
    return msPerTick;
}

// Java: private String createTitle() - "Minecraft*" + version name; the
// modification check and server suffix are game-port content.
static char *createTitle(const LIBMATTI_MC_Minecraft *minecraft)
{
    const char *modified = "";
    const char *version = LIBMATTI_MC_VERSION_STRING;
    size_t size = strlen("Minecraft") + strlen(modified) + 2 + strlen(version);
    char *title = malloc(size);
    snprintf(title, size, "Minecraft%s %s", modified, version);
    return title;
}

// Java (NeoForge Window.java.patch): private long takeOverWindow(
//       EarlyLoadingScreenController earlyLoadingScreen, String title) -
// the game takes over the FML early progress window instead of opening a
// second one; the size/pos come from the existing window.
static LIBMATTI_FML_SimpleFont *load_font(void);

static long take_over_window(LIBMATTI_FML_EarlyLoadingScreenController *earlyLoadingScreen, const char *title)
{
    // Java: long window = earlyLoadingScreen.takeOverGlfwWindow();
    long window = LIBMATTI_FML_EarlyLoadingScreenController_TakeOverGlfwWindow(earlyLoadingScreen);

    // Java: GLFW.glfwSetWindowTitle(window, title);
    LIBMATTI_GLFW_glfwSetWindowTitle(window, title);

    int x = 0, y = 0;
    LIBMATTI_GLFW_glfwGetWindowPos(window, &x, &y);

    int width = 0, height = 0;
    LIBMATTI_GLFW_glfwGetWindowSize(window, &width, &height);
    // Java: the window height and width can be 0 if minimized
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    (void) x;
    (void) y;
    return window;
}

LIBMATTI_MC_Minecraft *LIBMATTI_MC_Minecraft_New(const LIBMATTI_MC_GameConfig *config)
{
    LIBMATTI_MC_Minecraft *minecraft = calloc(1, sizeof(LIBMATTI_MC_Minecraft));
    // Java: private volatile boolean running = true (the field initializer).
    minecraft->running = 1;
    minecraft->config = *config;
    minecraft->launchVersion = strdup(config->game.launchVersion ? config->game.launchVersion : "");
    minecraft->versionType = strdup(config->game.versionType ? config->game.versionType : "release");

    // Java: this.deltaTracker = new DeltaTracker.Timer(20.0F, 0L, this::getTickTargetMillis);
    minecraft->deltaTracker = LIBMATTI_MC_DeltaTracker_Timer_New(20.0f, LIBMATTI_MC_Minecraft_GetTickTargetMillis);

    // ---- Window init (com.mojang.blaze3d.platform.Window) -----------------
    // Java: Window.init - glfwSetErrorCallback, checkGlfwError
    if (LIBMATTI_GLFW_glfwInit() != LIBMATTI_GLFW_TRUE)
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        fprintf(stderr, "[MINECRAFT] ERROR: GLFW init failed: %s\n",
                description ? description : "unknown error");
        free(minecraft->launchVersion);
        free(minecraft->versionType);
        LIBMATTI_MC_DeltaTracker_Free(minecraft->deltaTracker);
        free(minecraft);
        return NULL;
    }

    // Java: glfwDefaultWindowHints(); Window: client API, context version,
    // forward compat, debug context from the options; the skeleton keeps the
    // core-profile hints the game sets.
    LIBMATTI_GLFW_glfwDefaultWindowHints();
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CLIENT_API, LIBMATTI_GLFW_OPENGL_API);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MAJOR, 3);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MINOR, 2);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_PROFILE, LIBMATTI_GLFW_OPENGL_CORE_PROFILE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_FORWARD_COMPAT, LIBMATTI_GLFW_TRUE);

    const LIBMATTI_MC_DisplayData *display = &config->display;
    char *title = createTitle(minecraft);
    // Java (NeoForge Window ctor patch): var earlyLoadingScreen =
    //       EarlyLoadingScreenController.current(); if (earlyLoadingScreen != null)
    //       this.handle = takeOverWindow(earlyLoadingScreen, title); else glfwCreateWindow
    LIBMATTI_FML_EarlyLoadingScreenController *earlyLoadingScreen =
        LIBMATTI_FML_EarlyLoadingScreenController_Current();
    if (earlyLoadingScreen != NULL && earlyLoadingScreen->takeOverGlfwWindow != NULL)
    {
        minecraft->window = take_over_window(earlyLoadingScreen, title);
    }
    else
    {
        // Java: this.window = this.virtualScreen.newWindow(displaydata, ..., this.createTitle());
        minecraft->window = LIBMATTI_GLFW_glfwCreateWindow(display->width, display->height, title, 0, 0);
    }
    free(title);

    if (minecraft->window == 0)
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        fprintf(stderr, "[MINECRAFT] ERROR: Window creation failed: %s\n",
                description ? description : "unknown error");
        LIBMATTI_GLFW_glfwTerminate();
        free(minecraft->launchVersion);
        free(minecraft->versionType);
        LIBMATTI_MC_DeltaTracker_Free(minecraft->deltaTracker);
        free(minecraft);
        return NULL;
    }

    // Java: Window.init - the GL context + capabilities; RenderSystem.initRenderer
    // resolves the binding and stores the API description (GlDevice ctor path).
    LIBMATTI_GLFW_glfwMakeContextCurrent(minecraft->window);
    LIBMATTI_B3D_RenderSystem_InitRenderer(minecraft->window);
    LIBMATTI_B3D_RenderSystem_SetupDefaultState();
    LIBMATTI_GLFW_glfwSwapInterval(1); // Java: vsync on by default
    LIBMATTI_GLFW_glfwFocusWindow(minecraft->window);
    LIBMATTI_GLFW_glfwShowWindow(minecraft->window);

    // Java: fullscreen handling - Window.setMode(displaydata.isFullscreen);
    if (display->isFullscreen)
    {
        long monitor = LIBMATTI_GLFW_glfwGetPrimaryMonitor();
        int width = display->fullscreenWidthPresent ? display->fullscreenWidth : display->width;
        int height = display->fullscreenHeightPresent ? display->fullscreenHeight : display->height;
        int redBits = 0, greenBits = 0, blueBits = 0, refreshRate = 0;
        if (monitor != 0
            && LIBMATTI_GLFW_glfwGetVideoMode(monitor, &width, &height, &redBits, &greenBits, &blueBits,
                                              &refreshRate))
        {
            // The port keeps windowed mode when the video mode query fails; the
            // real setMode picks the display mode through glfwSetVideoMode.
            (void) width;
            (void) height;
        }
    }

    LOG("Backend library: LWJGL version (port), OpenGL context acquired");

    // Java (Minecraft ctor): this.textureManager = new TextureManager(this.resourceManager);
    // The ctor registers the missing texture (see TextureManager_New).
    minecraft->textureManager = LIBMATTI_MC_TextureManager_New(NULL);

    // The earlydisplay font + font shader (Java: MaterializedTheme loads the
    // theme fonts; RenderContext binds Theme.SHADER_FONT for text). Both are
    // optional: without them the skeleton still runs, just without the title.
    minecraft->font = load_font();
    if (minecraft->font != NULL)
    {
        minecraft->fontProgram = LIBMATTI_FML_FontShader_Compile(&minecraft->fontScreenSizeLocation);
        if (minecraft->fontProgram == 0)
            LOG("Font shader compile failed; title rendering disabled");
        // Java: LoadingScreenRenderer ctor - the fixed 854x480 layout buffer
        minecraft->framebuffer = LIBMATTI_FML_EarlyFramebuffer_New(854, 480);
    }

    instance = minecraft;
    return minecraft;
}

// Java: public void tick() - the per-tick body; level/entity/screens updates are
// the game port. The body is the mixable function the example mods hook
// (MATTI_MIXIN on "matticraft::demo::tick"): the dispatch runs the mod chain,
// the body prints once per tick like the demo target.
static void minecraft_demo_tick(void)
{
    printf("minecraft: tick body ran\n");
}

MATTI_MIXIN_TARGET("matticraft::demo::tick", minecraft_demo_tick)

static void tick(LIBMATTI_MC_Minecraft *minecraft)
{
    (void) minecraft;
    MattiMixinResult result = MATTI_MIXIN_PASS;
    void *args[1] = {NULL};
    // Java: the woven call site runs the mod chain, then the body unless cancelled.
    if (LIBMATTI_MIXIN_Invoke(LIBMATTI_MIXIN_HookTable_Default(), "matticraft::demo::tick", NULL, args, 1, &result))
        minecraft_demo_tick();
}

// Java: private void renderTitleLine(...) - the skeleton draws the game title
// with the earlydisplay Monocraft font (Java: MaterializedTheme fonts +
// RenderContext.renderTextWithShadow). White with the Java 0.25x shadow.
static void render_title(LIBMATTI_MC_Minecraft *minecraft, int width, int height)
{
    (void) width;
    (void) height;
    if (minecraft->font == NULL || minecraft->fontProgram == 0)
        return;

    char text[128];
    snprintf(text, sizeof(text), "Matticraft %s", minecraft->launchVersion);
    LIBMATTI_FML_SimpleFont_DisplayText texts[1] = {{text, 0xFFFFFFFFu}};

    // Java: RenderContext.renderText - bind the font texture + shader, draw
    // the batched glyph quads; GlStateManager routes the state (blend on for
    // the alpha-tested glyphs, like the font shader's premultiplied colour).
    LIBMATTI_B3D_GlStateManager_EnableBlend();
    LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(LIBMATTI_GL_GL_SRC_ALPHA,
                                                  LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA,
                                                  LIBMATTI_GL_GL_ONE, LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA);
    LIBMATTI_GL_glUseProgram(minecraft->fontProgram);
    // Java: LoadingScreenRenderer.renderToFramebuffer sets the shader uniforms
    // to the fixed layout size (LAYOUT_WIDTH, LAYOUT_HEIGHT), not the window.
    LIBMATTI_GL_glUniform2f(minecraft->fontScreenSizeLocation, 854.0f, 480.0f);
    LIBMATTI_B3D_GlStateManager_ActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
    LIBMATTI_B3D_GlStateManager_BindTexture((int) LIBMATTI_FML_SimpleFont_TextureId(minecraft->font));
    LIBMATTI_GL_glUniform1i(LIBMATTI_GL_glGetUniformLocation(minecraft->fontProgram, "tex"), 0);

    // Java: renderTextWithShadow(x + 2, y + 2, shadow) then (x, y, text)
    LIBMATTI_FML_SimpleFont_DisplayText shadow[1] = {{text, 0x40000000u}};
    LIBMATTI_FML_SimpleFont_DrawTexts(minecraft->font, 10.0f + 2.0f, 10.0f + 2.0f, shadow, 1);
    LIBMATTI_FML_SimpleFont_DrawTexts(minecraft->font, 10.0f, 10.0f, texts, 1);

    LIBMATTI_GL_glUseProgram(0);
    LIBMATTI_B3D_GlStateManager_DisableBlend();
}

// Java (Window.java Window ctor / MaterializedTheme): load the theme's "gui"
// font (the Monocraft.ttf the FML resources carry). The port reads it from the
// source tree resources (MATTI_SOURCE_DIR, the same mechanism the manifest
// uses); NULL leaves the title rendering off.
static LIBMATTI_FML_SimpleFont *load_font(void)
{
    const char *base = MATTI_SOURCE_DIR;
    const char *relative =
        "/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf";
    char path[1024];
    snprintf(path, sizeof(path), "%s%s", base, relative);

    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR: theme font not found: %s\n", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length <= 0)
    {
        fclose(file);
        return NULL;
    }
    unsigned char *data = malloc((size_t) length);
    if (fread(data, 1, (size_t) length, file) != (size_t) length)
    {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);

    LIBMATTI_FML_SimpleFont *font = LIBMATTI_FML_SimpleFont_New(data, (size_t) length);
    free(data);
    return font;
}

// Java: private void runTick(boolean renderLevelInMainMenu) - the loop body.
// Everything the game does inside (screens, packets, sounds) collapses into the
// three statements the loop structure owns: advanceTime, tick, render+swap.
static void runTick(LIBMATTI_MC_Minecraft *minecraft, int runGameTime)
{
    // Java: if (this.window.shouldClose()) this.stop();
    if (LIBMATTI_GLFW_glfwWindowShouldClose(minecraft->window))
        LIBMATTI_MC_Minecraft_Stop(minecraft);

    if (!minecraft->running) return;

    // Java: this.textureManager.tick();
    LIBMATTI_MC_TextureManager_Tick(minecraft->textureManager);

    // Java: int k = this.deltaTracker.advanceTime(Util.getMillis(), renderLevelInMainMenu);
    long long nowMs = LIBMATTI_JL_System_CurrentTimeMillis();
    int ticks = LIBMATTI_MC_DeltaTracker_AdvanceTime(minecraft->deltaTracker, nowMs, runGameTime);

    // Java: if (renderLevelInMainMenu) { ... for (l = 0; l < min(10, k); l++) tick(); }
    if (runGameTime)
    {
        int max = ticks < 10 ? ticks : 10;
        for (int l = 0; l < max; l++)
            tick(minecraft);
    }

    // Java: profilerfiller.push("gameRenderer"); gameRenderer.render(deltaTracker, ...);
    // The renderer is the game port; the skeleton renders the title line through
    // the LoadingScreenRenderer pipeline (renderToScreen): resize the layout FBO
    // to the window framebuffer, render the layout bottom-up into it, blit it
    // (vertically flipped, aspect-fitted) onto the window, then swap.
    if (!minecraft->noRender)
    {
        // Java: glfwMakeContextCurrent + the context check like LWJGL requires
        if (LIBMATTI_GLFW_glfwGetCurrentContext() == minecraft->window)
        {
            int width = 0, height = 0;
            LIBMATTI_GLFW_glfwGetFramebufferSize(minecraft->window, &width, &height);
            if (minecraft->framebuffer != NULL)
            {
                // Java: LoadingScreenRenderer.renderToScreen
                LIBMATTI_FML_EarlyFramebuffer_Resize(minecraft->framebuffer, width, height);

                // Java: renderToFramebuffer - fit the layout rectangle into the
                // FBO while maintaining the 854x480 aspect ratio
                LIBMATTI_FML_EarlyFramebuffer_Activate(minecraft->framebuffer);
                float desiredAspectRatio = 854.0f / 480.0f;
                float actualAspectRatio = (float) width / (float) height;
                if (minecraft->font != NULL && minecraft->fontProgram != 0)
                {
                    int offsetX, offsetY;
                    if (actualAspectRatio > desiredAspectRatio)
                    {
                        float actualWidth = desiredAspectRatio * (float) height;
                        offsetX = (int) ((float) width - actualWidth) / 2;
                        offsetY = 0;
                        LIBMATTI_B3D_GlStateManager_Viewport(offsetX, 0, (int) actualWidth, height);
                    }
                    else
                    {
                        float actualHeight = (float) width / desiredAspectRatio;
                        offsetX = 0;
                        offsetY = (int) ((float) height - actualHeight) / 2;
                        LIBMATTI_B3D_GlStateManager_Viewport(0, offsetY, width, (int) actualHeight);
                    }
                    (void) offsetX;
                    (void) offsetY;

                    // Java: clear to the theme screenBackground (#ef323d)
                    LIBMATTI_B3D_GlStateManager_ClearColor(0xef / 255.0f, 0x32 / 255.0f, 0x3d / 255.0f, 1.0f);
                    LIBMATTI_B3D_GlStateManager_Clear(LIBMATTI_GL_GL_COLOR_BUFFER_BIT | LIBMATTI_GL_GL_DEPTH_BUFFER_BIT);

                    // The skeleton's title line (Java: the theme's LabelElement
                    // renders the game title inside the layout).
                    render_title(minecraft, 854, 480);
                }
                LIBMATTI_FML_EarlyFramebuffer_Deactivate(minecraft->framebuffer);

                // Java: GlState.viewport(0, 0, w, h) before the blit
                LIBMATTI_B3D_GlStateManager_Viewport(0, 0, width, height);
                LIBMATTI_FML_EarlyFramebuffer_BlitToScreen(minecraft->framebuffer,
                                                           0xef / 255.0f, 0x32 / 255.0f, 0x3d / 255.0f,
                                                           width, height);

                // Debug: MATTI_FONT_DUMP=<path> writes the composited frame
                // (read straight from the default framebuffer after the blit)
                // once, for verifying what the window actually shows.
                if (getenv("MATTI_FONT_DUMP") != NULL)
                {
                    static int screenDumped = 0;
                    screenDumped++;
                    if (screenDumped == 3)
                    {
                        unsigned char *pix = malloc((size_t) (width * height * 3));
                        // byte-aligned rows (RGB of an odd width is not 4-aligned)
                        LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 1);
                        LIBMATTI_GL_glReadPixels(0, 0, width, height, LIBMATTI_GL_GL_RGB,
                                                 LIBMATTI_GL_GL_UNSIGNED_BYTE, pix);
                        LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 4);
                        FILE *out = fopen(getenv("MATTI_FONT_DUMP"), "wb");
                        if (out != NULL)
                        {
                            fprintf(out, "P6\n%d %d\n255\n", width, height);
                            // GL returns rows bottom-up; PPM is top-down - mirror
                            for (int row = height - 1; row >= 0; row--)
                                fwrite(pix + (size_t) row * width * 3, 1, (size_t) width * 3, out);
                            fclose(out);
                        }
                        free(pix);
                    }
                }
            }
            else
            {
                // No FBO (font unavailable): clear the window directly, like
                // RenderSystem does at frame start.
                LIBMATTI_B3D_GlStateManager_Viewport(0, 0, width, height);
                LIBMATTI_B3D_GlStateManager_ClearColor(0.2f, 0.3f, 0.8f, 1.0f);
                LIBMATTI_B3D_GlStateManager_Clear(LIBMATTI_GL_GL_COLOR_BUFFER_BIT | LIBMATTI_GL_GL_DEPTH_BUFFER_BIT);
            }
        }

        // Java: this.window.updateDisplay(...) - blitToScreen + swap
        LIBMATTI_GLFW_glfwSwapBuffers(minecraft->window);
    }

    // Java: profilerfiller.popPush("yield"); RenderSystem.flipFrame calls
    // pollEvents + swap; the loop splits them like Minecraft.runTick does.
    LIBMATTI_B3D_RenderSystem_PollEvents();

    minecraft->frames++;
    double now = LIBMATTI_GLFW_glfwGetTime();
    if (now - minecraft->fpsTimerStart >= 1.0)
    {
        minecraft->fps = minecraft->frames;
        minecraft->frames = 0;
        minecraft->fpsTimerStart = now;
    }
}

void LIBMATTI_MC_Minecraft_Run(LIBMATTI_MC_Minecraft *minecraft)
{
    // Java: this.gameThread = Thread.currentThread(); priority bump >4 cores.
    LOG("Running Minecraft (skeleton)");

    // Java: boolean flag = false; while (this.running) { runTick(!flag); flag = true; }
    int flag = 0;
    while (minecraft->running)
    {
        runTick(minecraft, !flag);
        flag = 1;
    }

    // Java: Minecraft.run falls out of the loop; destroy runs at the caller.
}

void LIBMATTI_MC_Minecraft_Stop(LIBMATTI_MC_Minecraft *minecraft)
{
    // Java: public void stop() { this.running = false; }
    minecraft->running = 0;
}

int LIBMATTI_MC_Minecraft_IsRunning(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->running;
}

void LIBMATTI_MC_Minecraft_Destroy(LIBMATTI_MC_Minecraft *minecraft)
{
    if (minecraft == NULL) return;

    // Java: public void destroy() { LOGGER.info("Stopping!"); ... }
    LOG("Stopping!");

    // Java: this.close() -> window.close() -> glfwDestroyWindow; then glfwTerminate
    // in RenderSystem/Window teardown.
    if (minecraft->window != 0)
    {
        LIBMATTI_GLFW_glfwDestroyWindow(minecraft->window);
        minecraft->window = 0;
    }
    LIBMATTI_GLFW_glfwTerminate();

    LIBMATTI_MC_DeltaTracker_Free(minecraft->deltaTracker);
    LIBMATTI_MC_TextureManager_Free(minecraft->textureManager);
    if (minecraft->fontProgram != 0)
    {
        LIBMATTI_GL_glDeleteProgram(minecraft->fontProgram);
        LIBMATTI_FML_EarlyFramebuffer_Free(minecraft->framebuffer);
    }
    LIBMATTI_FML_SimpleFont_Free(minecraft->font);
    free(minecraft->launchVersion);
    free(minecraft->versionType);
    free(minecraft);
    if (instance == minecraft)
        instance = NULL;
}

long LIBMATTI_MC_Minecraft_GetWindow(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->window;
}

LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_Minecraft_GetDeltaTracker(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->deltaTracker;
}

LIBMATTI_MC_TextureManager *LIBMATTI_MC_Minecraft_GetTextureManager(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->textureManager;
}

const char *LIBMATTI_MC_Minecraft_GetLaunchedVersion(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->launchVersion;
}

const char *LIBMATTI_MC_Minecraft_GetVersionType(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->versionType;
}

int LIBMATTI_MC_Minecraft_GetFps(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->fps;
}

int LIBMATTI_MC_Minecraft_GetFrames(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->frames;
}

int LIBMATTI_MC_Minecraft_NoRender(const LIBMATTI_MC_Minecraft *minecraft)
{
    return minecraft->noRender;
}

void LIBMATTI_MC_Minecraft_SetNoRender(LIBMATTI_MC_Minecraft *minecraft, int noRender)
{
    minecraft->noRender = noRender;
}
