// Port of net.minecraft.client.Minecraft (the game skeleton). The constructor
// opens the GLFW window (Window.java + Window init in the constructor), run()
// is the while (!stopped) loop of Minecraft.run, runTick() carries the
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
// advanceTime/tick/render split, destroy() the shutdown path.

#include "libmatti/net/minecraft/client/Minecraft.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/matti/mixin/MixinHookTable.h"
#include "libmatti/matti_mixin.h"
#include "libmatti/net/minecraft/client/Camera.h"
#include "libmatti/net/minecraft/client/renderer/culling/Frustum.h"
#include "libmatti/net/minecraft/client/renderer/SkyRenderer.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureManager.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/neoforged/fml/earlydisplay/EarlyFramebuffer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionRenderDispatcher.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionShader.h"
#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"
#include "libmatti/net/minecraft/client/renderer/block/BlockRenderDispatcher.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockModels.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockTextures.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaCelestialTextures.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/org/joml/Matrix4f.h"
#include "libmatti/net/neoforged/fml/earlydisplay/FontShader.h"
#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"
#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"
#include "libmatti/org/lwjgl/glfw/GLFW.h"
#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libmatti/net/minecraft/client/EmbeddedFont.h"

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

    // Java: this.levelRenderer = new LevelRenderer - the section dispatcher the
    // chunk meshes go through (the P4.1 port).
    LIBMATTI_MC_SectionRenderDispatcher *sectionDispatcher;

    // Java: this.modelManager - the baked block models (the P4.2 port); the
    // section compiler resolves the block model quads through it.
    LIBMATTI_MC_ModelManager *modelManager;

    // Java: this.blockRenderer = new BlockRenderDispatcher(...) - the P4.4
    // facade (BlockColors + the AO ModelBlockRenderer) the section compiler's
    // renderBatched path runs through.
    LIBMATTI_MC_BlockRenderDispatcher *blockRenderer;

    // Java: private final Camera camera (GameRenderer owns it in Java; the
    // skeleton keeps it on the Minecraft struct) + the culling frustum the
    // level renderer feeds per frame (the P4.3 port).
    LIBMATTI_MC_Camera camera;
    LIBMATTI_MC_Frustum frustum;
    int frustumInitialized;

    // Java: private SkyRenderer skyRenderer (the P4.5 port) + the CELESTIALS
    // atlas the sun/moon quads ride.
    LIBMATTI_MC_SkyRenderer *skyRenderer;
    LIBMATTI_MC_TextureAtlas *celestialsAtlas;
};

// Java: GameRenderer.renderLevel - "Matrix4f matrix4f1 = new Matrix4f()
// .rotation(quaternionf)" with quaternionf = camera.rotation().conjugate():
// the view ROTATION, and the camera position enters through the PoseStack
// translate(-camera) the level path applies. The port folds that translation
// into the view's translation column: the struct is JOML column-major
// (mCR = column C, row R), so the GL translation slots are m30/m31/m32 and
// the per-column constants are -dot(row, camPos) for the right/up rows and
// +dot(forward, camPos) for the -forward row (V * p = R^T * (p - camPos)).
// The rows are right/up/-forward with right = -left (the camera's left is
// screen-left); that 3x3 IS the inverse camera rotation.
void LIBMATTI_MC_GameRenderer_BuildRotationMatrix(const LIBMATTI_MC_Camera *camera, LIBMATTI_JOML_Matrix4f *view)
{
    const LIBMATTI_JOML_Vector3f *f = LIBMATTI_MC_Camera_ForwardVector(camera);
    const LIBMATTI_JOML_Vector3f *u = LIBMATTI_MC_Camera_UpVector(camera);
    const LIBMATTI_JOML_Vector3f *l = LIBMATTI_MC_Camera_LeftVector(camera);
    float rx = -l->x, ry = -l->y, rz = -l->z;
    float cx = (float) camera->x, cy = (float) camera->y, cz = (float) camera->z;
    // Column-major view: element (column, row) = m<column><row>.
    view->m00 = rx; view->m01 = u->x; view->m02 = -f->x; view->m03 = 0.0f;
    view->m10 = ry; view->m11 = u->y; view->m12 = -f->y; view->m13 = 0.0f;
    view->m20 = rz; view->m21 = u->z; view->m22 = -f->z; view->m23 = 0.0f;
    // The translation column (GL: elements [0][3], [1][3], [2][3]).
    view->m30 = -(rx * cx + ry * cy + rz * cz);
    view->m31 = -(u->x * cx + u->y * cy + u->z * cz);
    view->m32 = (f->x * cx + f->y * cy + f->z * cz);
    view->m33 = 1.0f;
}

// The full view for the terrain shader: the rotation above plus the negated
// rotation*translation column (the world->camera shift the Java path does
// through the PoseStack translate(-camera) instead).

// Java: the ModelManager model lookup the compiler consults per block - the
// block's registry key path maps onto the model id ("block/<path>").
static const LIBMATTI_MC_QuadCollection *model_for_block(void *userdata, const LIBMATTI_MC_Block *block)
{
    LIBMATTI_MC_ModelManager *manager = (LIBMATTI_MC_ModelManager *) userdata;
    LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey(block);
    if (key == NULL)
        return NULL;
    // Java: ResourceKey.location() - the identifier directly (no ToString parse).
    char modelId[128];
    snprintf(modelId, sizeof(modelId), "block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
    const LIBMATTI_MC_QuadCollection *model = LIBMATTI_MC_ModelManager_GetModel(manager, modelId);
    return model;
}

// Java: the sprite-rect resolver - the block's model sprite rect for the face
// UVs (the atlas texture "block/<path>").
static void sprite_rect_for_block(void *userdata, const LIBMATTI_MC_Block *block, float uv[4])
{
    LIBMATTI_MC_ModelManager *manager = (LIBMATTI_MC_ModelManager *) userdata;
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_ModelManager_GetAtlas(manager);
    if (atlas == NULL)
        return; // the default full-sprite rect stays.
    LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey(block);
    if (key == NULL)
        return;
    // Java: ResourceKey.location() - the identifier directly (no ToString parse).
    char textureId[128];
    snprintf(textureId, sizeof(textureId), "block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
    LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, textureId, uv);
    return;
}

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

    // Java: the Camera field initializer (GameRenderer's camera).
    LIBMATTI_MC_Camera_Init(&minecraft->camera);
    minecraft->frustumInitialized = 0;

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
    // Java: vsync on by default. On a headless runner the GLX swap with
    // interval 1 never returns (xvfb delivers no vblank), so CI gets interval 0
    // and the loop ticks like on a real display.
    LIBMATTI_GLFW_glfwSwapInterval(getenv("CI") != NULL ? 0 : 1);
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

    // Java: this.level = new ClientLevel(...) + levelRenderer.setLevel - the
    // skeleton's demo level: bootstrap the vanilla blocks, one in-memory level
    // and a flat 16x16 stone platform at y=64 the section compiler meshes.
    {
        LIBMATTI_MC_Bootstrap_BootStrap();
        LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
        LIBMATTI_MC_Block *stone = LIBMATTI_MC_VanillaBlocks_GetByName("STONE");
        LIBMATTI_MC_Block *dirt = LIBMATTI_MC_VanillaBlocks_GetByName("DIRT");
        if (stone != NULL && dirt != NULL)
        {
            for (int x = 0; x < 16; x++)
            {
                for (int z = 0; z < 16; z++)
                {
                    LIBMATTI_MC_BlockPos ground = {{x, 64, z}};
                    LIBMATTI_MC_Level_SetBlock(level, &ground, LIBMATTI_MC_Block_DefaultBlockState(stone),
                                               LIBMATTI_MC_Level_UPDATE_CLIENTS);
                    LIBMATTI_MC_BlockPos fill = {{x, 63, z}};
                    LIBMATTI_MC_Level_SetBlock(level, &fill, LIBMATTI_MC_Block_DefaultBlockState(dirt),
                                               LIBMATTI_MC_Level_UPDATE_CLIENTS);
                }
            }
            // Two stone towers to make the geometry visible from the camera.
            for (int y = 65; y < 70; y++)
            {
                LIBMATTI_MC_BlockPos a = {{4, y, 4}};
                LIBMATTI_MC_Level_SetBlock(level, &a, LIBMATTI_MC_Block_DefaultBlockState(stone),
                                           LIBMATTI_MC_Level_UPDATE_CLIENTS);
                LIBMATTI_MC_BlockPos b = {{11, y, 11}};
                LIBMATTI_MC_Level_SetBlock(level, &b, LIBMATTI_MC_Block_DefaultBlockState(stone),
                                           LIBMATTI_MC_Level_UPDATE_CLIENTS);
            }
            // A second platform four sections east (x 64..79, section 4,4,0):
            // the frustum-culling proof - it draws when the camera faces east
            // and is culled otherwise, exactly like Java's
            // cullingFrustum.isVisible(sectionAABB) gate.
            for (int x = 64; x < 80; x++)
            {
                for (int z = 0; z < 16; z++)
                {
                    LIBMATTI_MC_BlockPos ground = {{x, 64, z}};
                    LIBMATTI_MC_Level_SetBlock(level, &ground, LIBMATTI_MC_Block_DefaultBlockState(stone),
                                               LIBMATTI_MC_Level_UPDATE_CLIENTS);
                    LIBMATTI_MC_BlockPos fill = {{x, 63, z}};
                    LIBMATTI_MC_Level_SetBlock(level, &fill, LIBMATTI_MC_Block_DefaultBlockState(dirt),
                                               LIBMATTI_MC_Level_UPDATE_CLIENTS);
                }
            }
        }

        minecraft->level = level;
        minecraft->sectionDispatcher = LIBMATTI_MC_SectionRenderDispatcher_New();

        // Java: the bootstrap order - MODEL_ATLAS (the block textures stitch
        // first) then ModelBakery (the models bake against it). The atlas is
        // procedural in the port (VanillaBlockTextures), the models embedded
        // (VanillaModels) - headless-safe, no resource pack on disk.
        LIBMATTI_MC_TextureAtlas *blockAtlas = LIBMATTI_MC_VanillaBlockTextures_Bootstrap(1024);
        minecraft->modelManager = LIBMATTI_MC_ModelManager_New(blockAtlas);
        if (LIBMATTI_MC_VanillaBlockModels_Bootstrap(minecraft->modelManager))
        {
            LIBMATTI_MC_SectionRenderDispatcher_SetModelResolver(
                minecraft->sectionDispatcher,
                (const LIBMATTI_MC_QuadCollection * (*)(void *, const LIBMATTI_MC_Block *))
                    model_for_block,
                minecraft->modelManager);
            // Java: the sprite-rect resolver rides on the same atlas - the
            // compiler maps the block's model sprite for the face UVs.
            LIBMATTI_MC_SectionRenderDispatcher_SetSpriteResolver(
                minecraft->sectionDispatcher, sprite_rect_for_block, minecraft->modelManager);
        }

        // Java: this.blockRenderer = new BlockRenderDispatcher(blockModelShaper,
        // materials, blockColors) - created with the model manager so the
        // compiler's renderBatched path carries AO + tint.
        minecraft->blockRenderer = LIBMATTI_MC_BlockRenderDispatcher_New();
        minecraft->sectionDispatcher->compiler->blockRenderer = minecraft->blockRenderer;
        minecraft->blockRenderer->modelForBlock = model_for_block;
        minecraft->blockRenderer->modelUserdata = minecraft->modelManager;

        LIBMATTI_MC_SectionRenderDispatcher_CreateSection(minecraft->sectionDispatcher, 0, 4, 0);
        LIBMATTI_MC_SectionRenderDispatcher_CreateSection(minecraft->sectionDispatcher, 4, 4, 0);

        // Java: the CELESTIALS atlas load + this.skyRenderer = new SkyRenderer(
        // textureManager, atlasManager) - the sun/moon quads ride the atlas,
        // the stars/sunrise/discs build once.
        minecraft->celestialsAtlas = LIBMATTI_MC_VanillaCelestialTextures_Bootstrap(1024);
        minecraft->skyRenderer = LIBMATTI_MC_SkyRenderer_New();
        LIBMATTI_MC_SkyRenderer_SetAtlas(minecraft->skyRenderer, minecraft->celestialsAtlas);
        LOG("SkyRenderer ready (atlas=%p)", (void *) minecraft->celestialsAtlas);
    }

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
    MattiMixinResult result = MATTI_MIXIN_PASS;
    void *args[1] = {NULL};
    // Java: the woven call site runs the mod chain, then the body unless cancelled.
    if (LIBMATTI_MIXIN_Invoke(LIBMATTI_MIXIN_HookTable_Default(), "matticraft::demo::tick", NULL, args, 1, &result))
        minecraft_demo_tick();

    // Java: LevelRenderer.renderLevel's compile pass - every dirty section is
    // rebuilt before the frame draws (the port compiles synchronously).
    if (minecraft->sectionDispatcher != NULL && minecraft->level != NULL)
    {
        LIBMATTI_MC_SectionRenderDispatcher_CompileDirty(minecraft->sectionDispatcher,
                                                         (LIBMATTI_MC_Level *) minecraft->level,
                                                         (long long) (LIBMATTI_GLFW_glfwGetTime() * 1000.0));
    }
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

// Java (Window.java Window ctor / MaterializedTheme): load the theme's
// "gui" font (the Monocraft.ttf the FML resources carry). The binary embeds
// the font (release packages do not ship the vendor source tree); the repo
// checkout stays as the dev-build fallback. No environment or user input
// reaches fopen - the paths are compile-time constants. NULL leaves the
// title rendering off.
static LIBMATTI_FML_SimpleFont *load_font(void)
{
    unsigned char *data = NULL;
    size_t length = 0;

    // 1) the repo checkout (dev builds, the same path the manifest uses).
    const char *base = MATTI_SOURCE_DIR;
    const char *relative =
        "/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf";

    char path[1024];
    snprintf(path, sizeof(path), "%s%s", base, relative);

    FILE *file = fopen(path, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (size > 0)
        {
            data = malloc((size_t) size);
            if (fread(data, 1, (size_t) size, file) != (size_t) size)
            {
                free(data);
                data = NULL;
            }
            else
            {
                length = (size_t) size;
            }
        }
        fclose(file);
    }

    // 2) the embedded copy - always present.
    if (data == NULL)
    {
        length = LIBMATTI_MC_EmbeddedFont_Monocraft_Size;
        data = malloc(length);
        if (data != NULL)
            memcpy(data, LIBMATTI_MC_EmbeddedFont_Monocraft, length);
    }

    if (data == NULL)
    {
        fprintf(stderr, "ERROR: theme font not available (repo path and embedded copy failed)\n");
        return NULL;
    }

    LIBMATTI_FML_SimpleFont *font = LIBMATTI_FML_SimpleFont_New(data, length);
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

                    // Java: LevelRenderer.renderLevel - the demo level's section
                    // meshes draw behind the loading layout (perspective from a
                    // fixed camera above the platform).
                    if (minecraft->sectionDispatcher != NULL)
                    {
                        static unsigned int terrainProgram = 0;
                        if (terrainProgram == 0)
                            terrainProgram = LIBMATTI_MC_SectionShader_Compile(NULL, NULL, NULL);
                        if (terrainProgram != 0)
                        {
                            // Java: the render pass resets the raster state every
                            // frame (RenderSystem.layeredGlState).
                            LIBMATTI_B3D_GlStateManager_Viewport(0, 0, width, height);
                            LIBMATTI_GL_glColorMask(1, 1, 1, 1);
                            LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_SCISSOR_TEST);
                            LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_STENCIL_TEST);
                            LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_CULL_FACE);
                            LIBMATTI_B3D_GlStateManager_DisableBlend();
                            LIBMATTI_B3D_GlStateManager_DisableColorLogicOp();
                            LIBMATTI_B3D_GlStateManager_DisableScissorTest();
                            LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_POLYGON_OFFSET_FILL);
                            LIBMATTI_B3D_GlStateManager_EnableDepthTest();
                            LIBMATTI_B3D_GlStateManager_DepthFunc(LIBMATTI_GL_GL_LEQUAL);
                            LIBMATTI_B3D_GlStateManager_DepthMask(1);

                            // Java: yaw 0 faces +Z (south), negative pitch looks
                            // UP - so the skeleton camera looks north-down at the
                            // spawn platform with yaw 180, pitch +50.
                            // MATTI_CAM_YAW/MATTI_CAM_PITCH sweep for the cull check.
                            float camYaw = 180.0f, camPitch = 50.0f;
                            if (getenv("MATTI_CAM_YAW") != NULL)
                                camYaw = (float) atof(getenv("MATTI_CAM_YAW"));
                            if (getenv("MATTI_CAM_PITCH") != NULL)
                                camPitch = (float) atof(getenv("MATTI_CAM_PITCH"));
                            LIBMATTI_MC_Camera_SetRotation(&minecraft->camera, camYaw, camPitch);
                            LIBMATTI_MC_Camera_SetPosition(&minecraft->camera, 8.0, 88.0, 40.0);

                            LIBMATTI_JOML_Matrix4f proj, view, mvpM;
                            const float aspect = (float) width / (float) height;
                            LIBMATTI_JOML_Matrix4f_SetPerspective(&proj, 1.2217f, aspect, 0.05f, 1000.0f);
                            LIBMATTI_MC_GameRenderer_BuildRotationMatrix(&minecraft->camera, &view);
                            // The clip matrix: projection·view (the projection
                            // applies last - the GL convention the frustum's
                            // plane extraction and the shader both need).
                            LIBMATTI_JOML_Matrix4f_Mul(&proj, &view, &mvpM);

                            // Java: this.cullingFrustum = new Frustum(proj, view);
                            // cullingFrustum.prepare(camera.getPosition()); the
                            // section culler consumes the same matrix as the shader.
                            LIBMATTI_MC_Frustum_Init(&minecraft->frustum, &proj, &view);
                            LIBMATTI_MC_Frustum_Prepare(&minecraft->frustum,
                                                        minecraft->camera.x, minecraft->camera.y, minecraft->camera.z);

                            // Java: LevelRenderer.renderLevel's sky pass (the
                            // renderSky section) - the disc, sunrise/sunset,
                            // sun/moon/stars and the dark disc draw through
                            // the SkyRenderer before the sections.
                            if (minecraft->skyRenderer != NULL)
                            {
                                // Java: ClientLevel - timeOfDay = dayTime % 24000
                                // over the DAY timeline period; the angles follow
                                // the 1.21.11 keys (sun/star 0 -> 360, moon 180 ->
                                // 540 over the day, sunrise color at the edges).
                                long dayTime = minecraft->level != NULL
                                                   ? LIBMATTI_MC_Level_GetDayTime((LIBMATTI_MC_Level *) minecraft->level)
                                                   : 0;
                                float timeOfDay = (float) (dayTime % 24000L);
                                float dayFraction = timeOfDay / 24000.0f;
                                // Java: the angle keys ease around noon (6000);
                                // the linear stand-in keeps the same key geometry.
                                float sunAngle = dayFraction * (float) (2.0 * M_PI);
                                float moonAngle = sunAngle + (float) M_PI;
                                float starAngle = sunAngle;
                                LIBMATTI_MC_MoonPhase phase = LIBMATTI_MC_MoonPhase_ByIndex(
                                    (int) (((dayTime % (24000L * LIBMATTI_MC_MoonPhase_COUNT)) / 24000L)));
                                // Java: the rainBrightness multiplier (1 - rain
                                // level) - no weather yet, so 1.
                                float rainBrightness = 1.0f;
                                // Java: Timelines.DAY's STAR_BRIGHTNESS keys - 0
                                // by day, 0.5 over the night plateau.
                                float starBrightness = 0.0f;
                                if (dayFraction > 13228.0f / 24000.0f || dayFraction < 92.0f / 24000.0f)
                                    starBrightness = 0.5f;

                                // Java: the sky color (the biome SKY_COLOR through
                                // the DAY timeline's night multiplier; the port
                                // carries the overworld day curve inline).
                                float skyR = 0.466f, skyG = 0.709f, skyB = 0.996f;
                                if (dayFraction > 13670.0f / 24000.0f && dayFraction < 22330.0f / 24000.0f)
                                {
                                    skyR = 0.028f;
                                    skyG = 0.028f;
                                    skyB = 0.088f;
                                }

                                LIBMATTI_B3D_GlStateManager_EnableBlend();
                                LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(LIBMATTI_GL_GL_SRC_ALPHA,
                                                                              LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA,
                                                                              LIBMATTI_GL_GL_ONE,
                                                                              LIBMATTI_GL_GL_ZERO);
                                LIBMATTI_B3D_GlStateManager_DepthMask(0);

                                if (getenv("MATTI_STATE_DEBUG") != NULL)
                                {
                                    fprintf(stderr, "[STATE] pre-sky depthMask=%d\n",
                                            LIBMATTI_GL_glGetInteger(0x0B72));
                                }

                                LIBMATTI_MC_SkyRenderer_DrawSkyDisc(minecraft->skyRenderer, &view, &proj, skyR,
                                                                    skyG, skyB, 1.0f);

                                // Java: renderSunriseAndSunset - the alpha comes
                                // from the SUNRISE_SUNSET_COLOR track's edges.
                                float sunriseAlpha = 0.0f;
                                float edge = dayFraction < 0.5f
                                                 ? dayFraction - 71.0f / 24000.0f
                                                 : 1.0f - (dayFraction - 21807.0f / 24000.0f) * (24000.0f / 2193.0f);
                                (void) edge;
                                if ((dayFraction > 71.0f / 24000.0f && dayFraction < 730.0f / 24000.0f)
                                    || (dayFraction > 21807.0f / 24000.0f && dayFraction < 23757.0f / 24000.0f))
                                    sunriseAlpha = 0.6f;
                                if (sunriseAlpha > 0.0f)
                                    LIBMATTI_MC_SkyRenderer_DrawSunriseSunset(
                                        minecraft->skyRenderer, &view, &proj, sunAngle, sunriseAlpha);

                                LIBMATTI_MC_SkyRenderer_RenderSunMoonAndStars(
                                    minecraft->skyRenderer, &view, &proj, sunAngle, moonAngle, starAngle, phase,
                                    rainBrightness, starBrightness);

                                // Java: shouldRenderDarkDisc - the eye below the
                                // horizon; the fixed skeleton camera stays above.

                                LIBMATTI_B3D_GlStateManager_DepthMask(1);
                                LIBMATTI_B3D_GlStateManager_DisableBlend();
                            }

                            float mvp[16];
                            memcpy(mvp, &mvpM, sizeof(mvp));
                            float origin[3] = {0.0f, 0.0f, 0.0f};
                            LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(
                                minecraft->sectionDispatcher, LIBMATTI_MC_ChunkSectionLayer_SOLID,
                                terrainProgram, mvp, origin, &minecraft->frustum);
                        }
                    }

                    // The skeleton's title line (Java: the theme's LabelElement
                    // renders the game title inside the layout).
                    if (getenv("MATTI_NO_TITLE") == NULL)
                        render_title(minecraft, 854, 480);

                    // The MATTI_SCREENSHOT debug hook: reads the layout FBO's
                    // back buffer into a PPM once (the renderer verification).
                    // The FBO carries the window framebuffer's size (Resize
                    // runs with width/height), the hook reads exactly that.
                    if (getenv("MATTI_SCREENSHOT") != NULL)
                    {
                        static int shotFrame = 0;
                        shotFrame++;
                        int targetFrame = 3;
                        if (getenv("MATTI_SCREENSHOT_FRAME") != NULL)
                            targetFrame = atoi(getenv("MATTI_SCREENSHOT_FRAME"));
                        if (shotFrame < targetFrame)
                            return;
                        int fbw = 0, fbh = 0;
                        LIBMATTI_GLFW_glfwGetFramebufferSize(minecraft->window, &fbw, &fbh);
                        // Default: read the layout FBO itself (the same buffer
                        // BlitToScreen reads); MATTI_SCREENSHOT_WINDOW=1 reads
                        // the window's default framebuffer instead. The layout
                        // FBO's handle is the GlStateManager's bound WRITE target
                        // right after this frame's draws.
                        if (getenv("MATTI_SCREENSHOT_WINDOW") == NULL)
                            LIBMATTI_B3D_GlStateManager_BindFramebuffer(36008,
                                                                        (unsigned int) LIBMATTI_B3D_GlStateManager_GetFrameBuffer(36009));
                        fprintf(stderr, "[SHOT] fb=%dx%d readFbo=%d writeFbo=%d sample=(%d,%d)\n",
                                fbw, fbh,
                                LIBMATTI_B3D_GlStateManager_GetFrameBuffer(36008),
                                LIBMATTI_B3D_GlStateManager_GetFrameBuffer(36009), fbw / 2, fbh / 2);
                        unsigned char *pixels = malloc((size_t) fbw * fbh * 3);
                        if (pixels != NULL)
                        {
                            LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 1);
                            LIBMATTI_GL_glReadPixels(0, 0, fbw, fbh, LIBMATTI_GL_GL_RGB,
                                                     LIBMATTI_GL_GL_UNSIGNED_BYTE, pixels);
                            LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 4);
                            int outFd = open(getenv("MATTI_SCREENSHOT"),
                                             O_WRONLY | O_CREAT | O_TRUNC,
                                             S_IRUSR | S_IWUSR);
                            FILE *out = NULL;
                            if (outFd >= 0)
                                out = fdopen(outFd, "wb");
                            if (out == NULL)
                            {
                                if (outFd >= 0)
                                    close(outFd);
                            }
                            else
                            {
                                fprintf(out, "P6\n%d %d\n255\n", fbw, fbh);
                                for (int y = fbh - 1; y >= 0; y--)
                                    fwrite(pixels + (size_t) y * fbw * 3, 1, (size_t) fbw * 3, out);
                                fclose(out);
                            }
                            free(pixels);
                            LIBMATTI_MC_Minecraft_Stop(minecraft);
                        }
                    }
                }
                LIBMATTI_FML_EarlyFramebuffer_Deactivate(minecraft->framebuffer);

                // Java: GlState.viewport(0, 0, w, h) before the blit
                LIBMATTI_B3D_GlStateManager_Viewport(0, 0, width, height);
                LIBMATTI_FML_EarlyFramebuffer_BlitToScreen(minecraft->framebuffer,
                                                           0xef / 255.0f, 0x32 / 255.0f, 0x3d / 255.0f,
                                                           width, height);

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

    // Java: this.close() -> levelRenderer.close() -> the section dispatcher's
    // sections and compiled meshes free with the level.
    if (minecraft->blockRenderer != NULL)
    {
        LIBMATTI_MC_BlockRenderDispatcher_Free(minecraft->blockRenderer);
        minecraft->blockRenderer = NULL;
    }
    if (minecraft->sectionDispatcher != NULL)
    {
        LIBMATTI_MC_SectionRenderDispatcher_Free(minecraft->sectionDispatcher);
        minecraft->sectionDispatcher = NULL;
    }

    // Java: the ModelManager closes with the game (the atlas stays with the
    // TextureManager path; the port owns it here).
    if (minecraft->modelManager != NULL)
    {
        LIBMATTI_MC_ModelManager_Free(minecraft->modelManager);
        minecraft->modelManager = NULL;
    }

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
