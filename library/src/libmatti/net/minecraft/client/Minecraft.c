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
#include "libmatti/net/minecraft/client/KeyMapping.h"
#include "libmatti/net/minecraft/client/player/LocalPlayer.h"
#include "libmatti/net/minecraft/client/renderer/culling/Frustum.h"
#include "libmatti/net/minecraft/client/renderer/SkyRenderer.h"
#include "libmatti/net/minecraft/client/renderer/CloudRenderer.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureManager.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/neoforged/fml/earlydisplay/EarlyFramebuffer.h"
#include "libmatti/net/minecraft/client/gui/GuiRenderer.h"
#include "libmatti/net/minecraft/client/gui/GuiLayout.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionRenderDispatcher.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionShader.h"
#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaAssetLoader.h"
#include "libmatti/net/minecraft/client/renderer/block/BlockRenderDispatcher.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockModels.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockTextures.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaCelestialTextures.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/SectionPos.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/ClipContext.h"
#include "libmatti/net/minecraft/world/phys/BlockHitResult.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"
#include "libmatti/net/minecraft/world/item/Item.h"
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

// Java: Inventory.getSelectionSize() - the hotbar carries 9 slots.
#define HOTBAR_SIZE 9

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
    // Java: private final CloudRenderer cloudRenderer (LevelRenderer)
    LIBMATTI_MC_CloudRenderer *cloudRenderer;
    LIBMATTI_MC_TextureAtlas *celestialsAtlas;

    // Java: public LocalPlayer player (the P5.2 port) - the camera rides its
    // entity position/rotation; the skeleton keeps the mouse-look state here
    // (Java: MouseHandler + the smooth-camera fields)
    LIBMATTI_MC_LocalPlayer *localPlayer;
    int mouseLookInitialized;
    int mouseLookEnabled;
    double lastCursorX;
    double lastCursorY;
    // Java: public HitResult hitResult - the crosshair pick (the P5.4 port); the
    // mouse-button edge state rides beside it (Java: the GameSettings attack/use
    // KeyMappings + MouseHandler's event feeding).
    LIBMATTI_MC_BlockHitResult hitResult;
    int attackDown;
    int useDown;

    // Java: this.gui = new Gui(this) - the HUD the frame draws after the world
    // (the P5.5 port: the batcher renders, the hotbar data lives here).
    LIBMATTI_MC_GuiRenderer *guiRenderer;
    // Java: Inventory.selectedSlot + the 9 hotbar ItemStacks (the port builds
    // them once from the vanilla block items - the creative palette).
    int hotbarSelected;
    LIBMATTI_MC_ItemStack *hotbarItems[HOTBAR_SIZE];
    // Java: Gui.tick - the toolHighlightTimer (10s fade after a hotbar switch)
    // + lastToolHighlight (the name it renders).
    int toolHighlightTimer;
    const LIBMATTI_MC_ItemStack *toolHighlight;
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
    // The registry keys the baked models by the namespaced ModelResourceLocation
    // ("minecraft:block/<path>", the VanillaModels convention).
    char modelId[128];
    snprintf(modelId, sizeof(modelId), "minecraft:block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
    const LIBMATTI_MC_QuadCollection *model = LIBMATTI_MC_ModelManager_GetModel(manager, modelId);
    return model;
}

// Java: the sprite-rect resolver - the block's model sprite rect for the face
// UVs (the atlas texture "block/<path>"). An unknown texture falls back to
// the missing-no sprite (Java: getSprite -> the missingSprite checkerboard),
// never the full-atlas rect (that smears the whole page over the face - the
// "weird triangles" the demo rendered before).
static void sprite_rect_for_block(void *userdata, const LIBMATTI_MC_Block *block, float uv[4])
{
    LIBMATTI_MC_ModelManager *manager = (LIBMATTI_MC_ModelManager *) userdata;
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_ModelManager_GetAtlas(manager);
    if (atlas == NULL)
        return; // the default full-sprite rect stays (no atlas at all).
    LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey(block);
    if (key == NULL)
        return;
    // Java: ResourceKey.location() - the identifier directly (no ToString parse).
    char textureId[128];
    snprintf(textureId, sizeof(textureId), "block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
    if (LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, textureId, uv))
        return;
    // Java: the missingSprite's rect (the checkerboard fallback).
    LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "missingno", uv);
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
    // The skeleton spawn: first-person on the platform (eye at feet 65 +
    // 1.62 eye height). Like the old skeleton camera: yaw 180 faces north
    // (-Z), the spawn eye at z=24 looks straight at the platform (z 0..16).
    LIBMATTI_MC_Camera_Init(&minecraft->camera);
    LIBMATTI_MC_Camera_SetRotation(&minecraft->camera, 180.0f, 20.0f);
    LIBMATTI_MC_Camera_SetPosition(&minecraft->camera, 8.0, 65.0 + 1.62, 24.0);
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
        // first) then ModelBakery (the models bake against it). The assets
        // ride the embedded pack (the gzip blob in the executable): the loader
        // runs the SpriteSourceList -> SpriteLoader -> ModelBakery pipeline
        // over the RAM-only pack, driven entirely by the pack's JSON + PNGs.
        // A missing blob (or a failed stitch) falls back to the procedural
        // atlas + the embedded cube models so the demo keeps rendering.
        LIBMATTI_MC_TextureAtlas *blockAtlas = NULL;
        if (LIBMATTI_MC_VanillaAssetLoader_Load())
        {
            minecraft->modelManager = LIBMATTI_MC_VanillaAssetLoader_GetModelManager();
            blockAtlas = LIBMATTI_MC_ModelManager_GetAtlas(minecraft->modelManager);
            fprintf(stderr, "[ASSETS] embedded pack active (%d entries)\n",
                    (int) LIBMATTI_MC_VanillaAssetLoader_PackEntryCount());
        }
        else
        {
            blockAtlas = LIBMATTI_MC_VanillaBlockTextures_Bootstrap(1024);
            minecraft->modelManager = LIBMATTI_MC_ModelManager_New(blockAtlas);
            LIBMATTI_MC_VanillaBlockModels_Bootstrap(minecraft->modelManager);
            fprintf(stderr, "[ASSETS] procedural fallback active\n");
        }
        LIBMATTI_MC_SectionRenderDispatcher_SetModelResolver(
            minecraft->sectionDispatcher,
            (const LIBMATTI_MC_QuadCollection * (*)(void *, const LIBMATTI_MC_Block *))
                model_for_block,
            minecraft->modelManager);
        // Java: the sprite-rect resolver rides on the same atlas - the
        // compiler maps the block's model sprite for the face UVs.
        LIBMATTI_MC_SectionRenderDispatcher_SetSpriteResolver(
            minecraft->sectionDispatcher, sprite_rect_for_block, minecraft->modelManager);

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
        // Java: this.cloudRenderer = new CloudRenderer() (the reload listener
        // applies the texture cells once).
        minecraft->cloudRenderer = LIBMATTI_MC_CloudRenderer_New();
        LOG("CloudRenderer ready (%d cells)",
            minecraft->cloudRenderer != NULL ? 1 : 0);
    }

    // Java: this.options = new Options(this, ...) - the Options constructor
    // creates the vanilla KeyMappings (key.forward/key.left/...) BEFORE the
    // LocalPlayer/KeyboardInput read them through the Options fields. Without
    // this the accessors return NULL and every IsDown() answers false - the
    // move vector stays (0, 0) and the player never walks (the mouse-look does
    // not touch the mappings, which is why turning still worked).
    LIBMATTI_MC_KeyMapping_CreateVanillaMappings();

    // Java: this.player = new LocalPlayer(this, this.level, ...) - the session
    // profile name rides the GameConfig user. The spawn rides the platform
    // centre (the 16x16 slab spans x/z 0..15 at y 64, top face 65) - since the
    // P5.3 physics the player collides, so an off-platform spawn falls into
    // the void; yaw 180 faces north over the slab, pitch 20 looks slightly down
    minecraft->localPlayer = LIBMATTI_MC_LocalPlayer_New(minecraft->level,
                                                         config->user.name ? config->user.name : "Player", NULL);
    if (minecraft->localPlayer != NULL)
    {
        LIBMATTI_MC_Entity *entity = &minecraft->localPlayer->player.base.base;
        LIBMATTI_MC_Entity_SetPos(entity, 8.0, 65.0, 8.0);
        LIBMATTI_MC_Entity_SetRot(entity, 180.0f, 20.0f);
        LIBMATTI_MC_Level_AddEntity(minecraft->level, entity);
    }

    // Java: this.hitResult = BlockHitResult.createMiss(...) - the shared miss
    // the renderer reads until the first pick lands.
    minecraft->hitResult = LIBMATTI_MC_BlockHitResult_DefaultMiss();
    minecraft->attackDown = 0;
    minecraft->useDown = 0;

    // Java: this.gui = new Gui(this) - the HUD renderer compiles with the GL
    // context up (the constructor opened the window); a NULL renderer leaves
    // the HUD off like the font path gates the title line.
    minecraft->guiRenderer = LIBMATTI_MC_GuiRenderer_New();
    if (minecraft->guiRenderer == NULL)
        fprintf(stderr, "[GUI] renderer unavailable - HUD stays off\n");

    // Java: the creative inventory's hotbar defaults - the port builds the 9
    // stacks once from the vanilla block items (the palette the right click
    // places from).
    static const char *const hotbarBlocks[HOTBAR_SIZE] = {
        "STONE", "DIRT", "COBBLESTONE", "OAK_PLANKS", "GLASS",
        "BRICKS", "SAND", "GRAVEL", "OAK_LOG"};
    for (int slot = 0; slot < HOTBAR_SIZE; slot++)
    {
        LIBMATTI_MC_Item *item = LIBMATTI_MC_VanillaItems_GetByName(hotbarBlocks[slot]);
        minecraft->hotbarItems[slot] = item != NULL ? LIBMATTI_MC_ItemStack_NewWithCount(item, 64) : NULL;
    }
    minecraft->hotbarSelected = 0;
    minecraft->toolHighlightTimer = 0;
    minecraft->toolHighlight = NULL;

    return minecraft;
}

// Java: public void tick() - the per-tick body; level/entity/screens updates are
// the game port. The body is the mixable function the example mods hook
// (MATTI_MIXIN on "matticraft::demo::tick"): the dispatch runs the mod chain,
// the body prints once per tick like the demo target.
static void minecraft_demo_tick(void)
{
    // printf("minecraft: tick body ran\n");
}

MATTI_MIXIN_TARGET("matticraft::demo::tick", minecraft_demo_tick)

static void apply_walk(LIBMATTI_MC_Minecraft *minecraft);
static void handle_block_interaction(LIBMATTI_MC_Minecraft *minecraft);
static void handle_hotbar_keys(LIBMATTI_MC_Minecraft *minecraft);
static void dirty_sections_around(LIBMATTI_MC_Minecraft *minecraft, const LIBMATTI_MC_BlockPos *pos);

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

    // Java: this.tick() runs the player through LocalPlayer.tick -> aiStep ->
    // travel: the per-TICK input poll + walk/gravity impulse (20 Hz, not the
    // per-frame rate - the tick loop above repeats this body for every
    // accumulated tick).
    apply_walk(minecraft);

    // Java: startUseItem/continueAttack ride the tick loop (the multi/hold
    // semantics run at tick rate) - the block interaction acts on the pick
    // the renderer refreshed this frame.
    handle_block_interaction(minecraft);

    // Java: the hotbar keys ride the same poll (Inventory.selectedSlot).
    handle_hotbar_keys(minecraft);

    // Java: Gui.tick - the 10s name fade decays per tick (the timer only runs
    // while the HUD shows it).
    if (minecraft->toolHighlightTimer > 0)
        minecraft->toolHighlightTimer--;
}

// Java: private void renderTitleLine(...) - the skeleton draws the game title
// with the earlydisplay Monocraft font (Java: MaterializedTheme fonts +
// RenderContext.renderTextWithShadow). White with the Java 0.25x shadow.
// MATTI_SHOT_AFTER: the phase-gated screenshot (clear|sky|terrain) - the
// readback runs right after the named phase and stops, so the pixel histogram
// names the pass that paints (or erases) the content.
static void shot_after(LIBMATTI_MC_Minecraft *minecraft, const char *phase)
{
    const char *want = getenv("MATTI_SHOT_AFTER");
    if (want == NULL || strcmp(want, phase) != 0)
        return;
    static int done = 0;
    if (done)
        return;
    done = 1;
    int fbw = 0, fbh = 0;
    LIBMATTI_GLFW_glfwGetFramebufferSize(minecraft->window, &fbw, &fbh);
    unsigned char *pixels = malloc((size_t) fbw * fbh * 3);
    if (pixels == NULL)
        return;
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 1);
    LIBMATTI_GL_glReadPixels(0, 0, fbw, fbh, LIBMATTI_GL_GL_RGB, LIBMATTI_GL_GL_UNSIGNED_BYTE, pixels);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 4);
    char path[128];
    snprintf(path, sizeof(path), "/tmp/phase_%s.ppm", phase);
    FILE *out = fopen(path, "wb");
    if (out != NULL)
    {
        fprintf(out, "P6\n%d %d\n255\n", fbw, fbh);
        for (int y = fbh - 1; y >= 0; y--)
            fwrite(pixels + (size_t) y * fbw * 3, 1, (size_t) fbw * 3, out);
        fclose(out);
    }
    free(pixels);
    fprintf(stderr, "[PHASE] wrote %s\n", path);
    LIBMATTI_MC_Minecraft_Stop(minecraft);
}

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

// Java: Gui.render + Gui.renderSelectedItemName - the HUD layer over the world
// pass: the crosshair, the hotbar sprite, the 9 item cells (the real block
// textures through the block atlas) and the selected name line (the
// toolHighlightTimer fade). Java computes scaledWidth = fbWidth / guiScale and
// blits every element at guiScale pixels per layout pixel - the port runs the
// layout math over width/2 x height/2 (the guiScale-2 space the 1080p window
// drives) and scales every rect x2 into the framebuffer pixels.
static void render_hud(LIBMATTI_MC_Minecraft *minecraft, int width, int height)
{
    if (minecraft->guiRenderer == NULL || LIBMATTI_MC_GuiRenderer_Program(minecraft->guiRenderer) == 0)
        return;

    // Java: Window.getGuiScale - the layout space the Gui math runs over and
    // the scale the blits multiply with (the port pins guiScale 2).
    const int guiWidth = width / 2, guiHeight = height / 2;
    const float scale = 2.0f;
    if (guiWidth < 1 || guiHeight < 1)
        return;

    // Java: RenderSystem.setShaderTexture - the quads sample the block atlas
    // for the item cells and the white 1x1 for the tint-only widgets.
    unsigned int atlasTexture = 0;
    if (minecraft->modelManager != NULL)
    {
        const LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_ModelManager_GetAtlas(minecraft->modelManager);
        if (atlas != NULL)
            atlasTexture = atlas->base.texture; // 0 until the lazy world upload.
    }

    // Java: the crosshair (15x15, centred) tints over the world (the normal
    // blend, the GUI_TEXTURED path).
    LIBMATTI_B3D_GlStateManager_EnableBlend();
    LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(LIBMATTI_GL_GL_SRC_ALPHA,
                                                  LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA,
                                                  LIBMATTI_GL_GL_ONE, LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA);

    int x = 0, y = 0, w = 0, h = 0;
    LIBMATTI_MC_GuiLayout_CrosshairRect(guiWidth, guiHeight, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(minecraft->guiRenderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xB0FFFFFFu);

    // The white-texture group flushes first (the widgets sample the 1x1);
    // the atlas group rides a second flush (one texture per draw).
    LIBMATTI_MC_GuiRenderer_SetTexture(minecraft->guiRenderer, 0);

    // Java: the hotbar (182x22 at w/2-91, h-22) - the dark bar with the 9
    // slot cells (the widget texture's insets the flat fallback shades in).
    LIBMATTI_MC_GuiLayout_HotbarRect(guiWidth, guiHeight, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(minecraft->guiRenderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xA0202020u);
    // Java: the widget's slot insets - the cell shading separates the 9 slots
    // (the sprite carries the borders; the port shades one cell quad each).
    for (int slot = 0; slot < HOTBAR_SIZE; slot++)
    {
        LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
        LIBMATTI_MC_GuiRenderer_BlitQuad(minecraft->guiRenderer, (float) (x - 1) * scale, (float) (y - 1) * scale,
                                         (float) (w + 2) * scale, (float) (h + 2) * scale,
                                         0.0f, 0.0f, 1.0f, 1.0f, 0x50000000u);
    }
    LIBMATTI_MC_GuiRenderer_Flush(minecraft->guiRenderer, (float) guiWidth * scale, (float) guiHeight * scale);

    // Java: renderSlot - the 16x16 item quad per cell with the item's model
    // sprite (the atlas texture the terrain renders through).
    if (atlasTexture != 0)
        LIBMATTI_MC_GuiRenderer_SetTexture(minecraft->guiRenderer, atlasTexture);
    for (int slot = 0; slot < HOTBAR_SIZE; slot++)
    {
        const LIBMATTI_MC_ItemStack *stack = minecraft->hotbarItems[slot];
        if (stack == NULL)
            continue;
        LIBMATTI_MC_Item *item = LIBMATTI_MC_ItemStack_GetItem(stack);
        if (item == NULL || item->block == NULL)
            continue;

        float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_ModelManager_GetAtlas(minecraft->modelManager);
        LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey((const LIBMATTI_MC_Block *) item->block);
        if (atlas == NULL || key == NULL)
            continue;
        char textureId[128];
        snprintf(textureId, sizeof(textureId), "block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
        if (!LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, textureId, uv))
        {
            // Java: the missingSprite fallback (the checkerboard).
            if (!LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "missingno", uv))
                continue;
        }

        LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
        // Java: the item sprite fills the cell (the 16x16 sprite at the
        // scaled slot rect).
        LIBMATTI_MC_GuiRenderer_BlitQuad(minecraft->guiRenderer, (float) x * scale, (float) y * scale,
                                         (float) w * scale, (float) h * scale,
                                         uv[0], uv[1], uv[2], uv[3], 0xFFFFFFFFu);
    }
    LIBMATTI_MC_GuiRenderer_Flush(minecraft->guiRenderer, (float) guiWidth * scale, (float) guiHeight * scale);
    LIBMATTI_MC_GuiRenderer_SetTexture(minecraft->guiRenderer, 0);

    // Java: the 24x23 selection frame over the selected slot (AFTER the items
    // - the white frame overlaps the cell and the item like the vanilla path).
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(guiWidth, guiHeight, minecraft->hotbarSelected, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(minecraft->guiRenderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xE0FFFFFFu);
    LIBMATTI_MC_GuiRenderer_Flush(minecraft->guiRenderer, (float) guiWidth * scale, (float) guiHeight * scale);

    // Java: renderSelectedItemName - the name line (hover name of the selected
    // stack) centred over the hotbar, fading with the toolHighlightTimer.
    if (minecraft->toolHighlightTimer > 0 && minecraft->font != NULL && minecraft->fontProgram != 0)
    {
        const LIBMATTI_MC_ItemStack *stack = minecraft->toolHighlight;
        if (stack != NULL)
        {
            LIBMATTI_MC_Item *item = LIBMATTI_MC_ItemStack_GetItem(stack);
            const char *name = item != NULL ? LIBMATTI_MC_Item_GetDescriptionId(item) : NULL;
            if (name != NULL)
            {
                int textWidth = LIBMATTI_FML_SimpleFont_StringWidth(minecraft->font, name);
                int nameY = 0;
                int nameX = LIBMATTI_MC_GuiLayout_SelectedItemNameRect(guiWidth, guiHeight, textWidth, &nameY);
                int alpha = minecraft->toolHighlightTimer * 256 / 10;
                if (alpha > 255)
                    alpha = 255;
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%s", name);
                LIBMATTI_FML_SimpleFont_DisplayText shadow[1] = {{buffer, 0x40000000u | ((unsigned) alpha)}};
                LIBMATTI_FML_SimpleFont_DisplayText text[1] = {{buffer, 0x00FFFFFFu | ((unsigned) alpha << 24)}};
                LIBMATTI_GL_glUseProgram(minecraft->fontProgram);
                LIBMATTI_GL_glUniform2f(minecraft->fontScreenSizeLocation, 854.0f, 480.0f);
                LIBMATTI_B3D_GlStateManager_ActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
                LIBMATTI_B3D_GlStateManager_BindTexture((int) LIBMATTI_FML_SimpleFont_TextureId(minecraft->font));
                LIBMATTI_GL_glUniform1i(LIBMATTI_GL_glGetUniformLocation(minecraft->fontProgram, "tex"), 0);
                // Java: the guiScale scales the name line with the widgets -
                // the font shader keeps the layout normalization, the
                // positions ride the scale factor (doubled like the blits).
                LIBMATTI_FML_SimpleFont_DrawTexts(minecraft->font, (float) (nameX + 2) * scale,
                                                  (float) (nameY + 2) * scale, shadow, 1);
                LIBMATTI_FML_SimpleFont_DrawTexts(minecraft->font, (float) nameX * scale, (float) nameY * scale,
                                                  text, 1);
                LIBMATTI_GL_glUseProgram(0);
            }
        }
    }

    // Java: the GUI quads flush after the text (the batcher uploads the
    // accumulated vertices and draws; the blend state stays the caller's).
    LIBMATTI_MC_GuiRenderer_Flush(minecraft->guiRenderer, (float) guiWidth * scale, (float) guiHeight * scale);
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

// Java: MouseHandler.onMove - the mouse-look. The cursor is captured
// (GLFW_CURSOR_DISABLED) and its per-frame delta turns the LOCAL PLAYER's
// entity rotation through Entity.turn (0.15 sensitivity, the +-90 pitch
// clamp) - the camera follows the entity. The enabled flag absorbs the first
// frame: the fresh grab reports the jump to the window centre as one huge
// delta that must not turn the camera.
static void updateMouseLook(LIBMATTI_MC_Minecraft *minecraft)
{
    if (minecraft->window == 0 || minecraft->localPlayer == NULL) return;

    if (!minecraft->mouseLookInitialized)
    {
        minecraft->mouseLookInitialized = 1;
        LIBMATTI_GLFW_glfwSetInputMode(minecraft->window, LIBMATTI_GLFW_CURSOR,
                                       LIBMATTI_GLFW_CURSOR_DISABLED);
    }
    if (LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_ESCAPE))
    {
        if (minecraft->mouseLookEnabled)
            LIBMATTI_MC_Minecraft_Stop(minecraft);
    }

    double cx = 0.0, cy = 0.0;
    LIBMATTI_GLFW_glfwGetCursorPos(minecraft->window, &cx, &cy);
    if (minecraft->mouseLookEnabled)
    {
        float dx = (float) (cx - minecraft->lastCursorX);
        float dy = (float) (cy - minecraft->lastCursorY);
        // Java: MouseHandler.turnPlayer -> entity.turn(dx, dy) - the raw cursor
        // delta passes through unmodified (MouseHandler.java:392); Java's turn
        // ADDS the pitch (moving the mouse up lowers xRot and the camera
        // SetRotation's -xRot raises the view). The old -dy inverted the axis.
        LIBMATTI_MC_Entity_Turn(&minecraft->localPlayer->player.base.base, (double) dx, (double) dy);
    }
    minecraft->lastCursorX = cx;
    minecraft->lastCursorY = cy;
    minecraft->mouseLookEnabled = 1;
}

// Java: LocalPlayer.aiStep -> the travel impulse - the input move vector turns
// into the walk acceleration in the entity's yaw plane (Java: xxa * cos(yawRad)
// - zza * sin(yawRad) over the movedRelative basis); the port folds the
// LivingEntity friction into a flat per-frame walk speed scaled by the abilities
// walking speed. The gravity rides Java's LivingEntity.aiStep default (-0.08
// per tick, * 0.98 the drag), the jump the vanilla +0.42 impulse.
static void apply_walk(LIBMATTI_MC_Minecraft *minecraft)
{
    if (minecraft->window == 0 || minecraft->localPlayer == NULL) return;
    LIBMATTI_MC_Entity *entity = &minecraft->localPlayer->player.base.base;

    // Java: KeyMapping.set(key, glfwGetKey(window, key) == GLFW_PRESS) - the
    // skeleton polls the movement keys before the input tick (the real client
    // feeds the same pair through the key callbacks)
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_W) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 83,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_S) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 65,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_A) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 68,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_D) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 32,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_SPACE) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 340,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_LEFT_SHIFT) == LIBMATTI_GLFW_PRESS);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 341,
                               LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_LEFT_CONTROL) == LIBMATTI_GLFW_PRESS);

    // Java: LocalPlayer.tick -> input.tick() - the Input record + move vector
    LIBMATTI_MC_LocalPlayer_TickInput(minecraft->localPlayer);
    const LIBMATTI_MC_Input *presses = LIBMATTI_MC_LocalPlayer_GetKeyPresses(minecraft->localPlayer);

    // Java: the jump - onGround && keyJump.isDown() -> jumpFromGround() (+0.42
    // the vanilla impulse, sprint adds the horizontal boost)
    if (presses->jump && LIBMATTI_MC_Entity_OnGround(entity))
    {
        LIBMATTI_MC_Vec3 jump = {entity->dx, 0.42, entity->dz};
        LIBMATTI_MC_Entity_SetDeltaMovement(entity, &jump);
    }

    // the walk: the move vector (x = strafe, +1 = LEFT like Java's input
    // convention, y = forward impulse) in the player entity's yaw plane, onto
    // the delta movement (the friction keeps the speed bounded between frames)
    LIBMATTI_MC_Vec2 move = LIBMATTI_MC_LocalPlayer_GetMoveVector(minecraft->localPlayer);
    float yawRad = entity->yRot * ((float) M_PI / 180.0f);
    // forward = (-sin yaw, cos yaw) like the camera basis; the strafe rides the
    // LEFT vector (Java: getInputVector rotates xxa = +1 into the left)
    float fwdX = -(float) sin(yawRad), fwdZ = (float) cos(yawRad);
    float rightX = -(float) cos(yawRad), rightZ = -(float) sin(yawRad);
    // Java: the travel speed - 0.1 (the walk speed) * the move vector length;
    // the sprint rides the 1.3 multiplier
    float speed = 0.1f * (float) LIBMATTI_MC_Mth_Length(move.x, move.y);
    if (presses->sprint)
        speed *= 1.3f;
    float accelX = (fwdX * move.y - rightX * move.x) * speed;
    float accelZ = (fwdZ * move.y - rightZ * move.x) * speed;
    // Java: LivingEntity.travel - the friction is the move-through block's
    // slipperiness (0.6 default) * the entity inertia 0.91 = 0.546 ON THE
    // GROUND, the raw 0.91 in the air; the steady-state walk speed is
    // accel/(1 - friction) = 0.1/0.454 = 0.22 blocks/tick (4.4 m/s, vanilla).
    // The old flat 0.91 ran the physics per FRAME and 14x too fast.
    float friction = LIBMATTI_MC_Entity_OnGround(entity) ? 0.546f : 0.91f;
    LIBMATTI_MC_Vec3 next = {entity->dx * friction + accelX,
                             entity->dy * 0.98 - 0.08,
                             entity->dz * friction + accelZ};
    LIBMATTI_MC_Entity_SetDeltaMovement(entity, &next);

    // Java: this.move(MoverType.SELF, this.getDeltaMovement()) - the collide
    // path clips the motion against the level's blocks (the P5.3 port)
    LIBMATTI_MC_Entity_Move(entity, LIBMATTI_MC_MoverType_SELF, &next);

    // MATTI_DEBUG_POS - the per-second position/onGround trace (the input/
    // physics smoke runs grep it to prove the walk actually moves the player).
    static int debugPos = -1;
    if (debugPos < 0)
        debugPos = getenv("MATTI_DEBUG_POS") != NULL;
    if (debugPos)
    {
        static int posFrame = 0;
        if (posFrame++ % 20 == 0)
            fprintf(stderr, "[POS] x=%.2f y=%.2f z=%.2f onGround=%d move=(%.2f,%.2f) fwdMap=%d rawW=%d\n",
                    entity->x, entity->y, entity->z,
                    LIBMATTI_MC_Entity_OnGround(entity), move.x, move.y,
                    LIBMATTI_MC_KeyMapping_Forward() != NULL,
                    LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_W));
    }
}

// MATTI_DEBUG_PICK - the crosshair trace (the pick smoke greps the hit block +
// face to prove the ray reaches the aimed block).
static void debug_pick(LIBMATTI_MC_Minecraft *minecraft)
{
    static int debugPick = -1;
    if (debugPick < 0)
        debugPick = getenv("MATTI_DEBUG_PICK") != NULL;
    if (!debugPick)
        return;
    static int pickFrame = 0;
    if (pickFrame++ % 20 != 0)
        return;
    const LIBMATTI_MC_BlockHitResult *hit = &minecraft->hitResult;
    if (hit->type == LIBMATTI_MC_HitResult_BLOCK)
        fprintf(stderr, "[PICK] block=(%d,%d,%d) face=%s loc=(%.2f,%.2f,%.2f)\n",
                hit->blockPos.base.x, hit->blockPos.base.y, hit->blockPos.base.z,
                LIBMATTI_MC_Direction_GetName(hit->direction),
                hit->location.x, hit->location.y, hit->location.z);
    else
        fprintf(stderr, "[PICK] miss\n");
}

// Java: the attack/use mouse edge handling (MouseHandler feeds the events into
// the KeyMappings, Minecraft.startUseItem / continueAttack act on them) - the
// port polls the buttons and acts on the press edges.
static void handle_block_interaction(LIBMATTI_MC_Minecraft *minecraft)
{
    if (minecraft->window == 0 || minecraft->localPlayer == NULL || minecraft->level == NULL)
    {
        minecraft->attackDown = 0;
        minecraft->useDown = 0;
        return;
    }

    const LIBMATTI_MC_BlockHitResult *hit = &minecraft->hitResult;
    LIBMATTI_MC_Level *level = (LIBMATTI_MC_Level *) minecraft->level;

    // Java: continueAttack - left click breaks the aimed block (the creative
    // instant-break path; hold-repeat rides the same poll per tick)
    int attacking = LIBMATTI_GLFW_glfwGetMouseButton(minecraft->window, LIBMATTI_GLFW_MOUSE_BUTTON_1) == LIBMATTI_GLFW_PRESS;
    if (attacking && !minecraft->attackDown)
    {
        if (hit->type == LIBMATTI_MC_HitResult_BLOCK)
        {
            if (LIBMATTI_MC_Level_DestroyBlock(level, &hit->blockPos, false))
                dirty_sections_around(minecraft, &hit->blockPos);
        }
    }
    minecraft->attackDown = attacking;

    // Java: startUseItem - right click places against the hit face (the
    // creative block-in-hand path: the block next to the entry face; BlockItem
    // canPlace rejects the hit cell itself, so the inside hit skips like here).
    int using = LIBMATTI_GLFW_glfwGetMouseButton(minecraft->window, LIBMATTI_GLFW_MOUSE_BUTTON_2) == LIBMATTI_GLFW_PRESS;
    if (using && !minecraft->useDown)
    {
        if (hit->type == LIBMATTI_MC_HitResult_BLOCK && !hit->inside)
        {
            // Java: BlockItem.place - the selected hotbar stack's block (the
            // creative palette) places against the hit face.
            LIBMATTI_MC_Direction face = hit->direction;
            LIBMATTI_MC_BlockPos placePos = {{hit->blockPos.base.x + LIBMATTI_MC_Direction_GetStepX(face),
                                              hit->blockPos.base.y + LIBMATTI_MC_Direction_GetStepY(face),
                                              hit->blockPos.base.z + LIBMATTI_MC_Direction_GetStepZ(face)}};
            // Java: BlockItem.canPlace -> level().noCollision(this, context)
            // - the placement rejects the cell when the PLAYER's bounding box
            // intersects it (no blocks inside the player, no walking inside a
            // placed block).
            LIBMATTI_MC_Entity *player = &minecraft->localPlayer->player.base.base;
            const LIBMATTI_MC_AABB *playerBox = LIBMATTI_MC_Entity_GetBoundingBox(player);
            LIBMATTI_MC_AABB cellBox = {
                (double) placePos.base.x, (double) placePos.base.y, (double) placePos.base.z,
                (double) placePos.base.x + 1.0, (double) placePos.base.y + 1.0,
                (double) placePos.base.z + 1.0};
            if (playerBox != NULL
                && playerBox->minX < cellBox.maxX && playerBox->maxX > cellBox.minX
                && playerBox->minY < cellBox.maxY && playerBox->maxY > cellBox.minY
                && playerBox->minZ < cellBox.maxZ && playerBox->maxZ > cellBox.minZ)
            {
                minecraft->useDown = using;
                return; // Java: the canPlace rejection - no placement, no face skip
            }
            LIBMATTI_MC_ItemStack *stack = minecraft->hotbarItems[minecraft->hotbarSelected];
            LIBMATTI_MC_Item *item = stack != NULL ? LIBMATTI_MC_ItemStack_GetItem(stack) : NULL;
            // Java: BlockItem.block - the port exposes the field directly (no
            // getter the Item.h surface carries).
            LIBMATTI_MC_Block *block = item != NULL ? (LIBMATTI_MC_Block *) item->block : NULL;
            LIBMATTI_MC_BlockState *placeState = block != NULL
                                                     ? LIBMATTI_MC_Block_DefaultBlockState(block)
                                                     : LIBMATTI_MC_Block_DefaultBlockState(
                                                           LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
            if (LIBMATTI_MC_Level_SetBlock(level, &placePos, placeState, LIBMATTI_MC_Level_UPDATE_CLIENTS))
            {
                dirty_sections_around(minecraft, &placePos);
                static int debugPlace = -1;
                if (debugPlace < 0)
                    debugPlace = getenv("MATTI_DEBUG_PICK") != NULL;
                if (debugPlace)
                {
                    // the placed block name proves the hotbar selection rides
                    // the placement (the hotbar-key smoke greps it).
                    const char *placed = item != NULL ? LIBMATTI_MC_Item_GetDescriptionId(item) : "?";
                    fprintf(stderr, "[PLACE] pos=(%d,%d,%d) item=%s slot=%d\n",
                            placePos.base.x, placePos.base.y, placePos.base.z,
                            placed != NULL ? placed : "?", minecraft->hotbarSelected);
                }
            }
        }
    }
    minecraft->useDown = using;
}

// Java: KeyboardInput + KeyMapping hotbar keys - keys 1..9 select the hotbar
// slot directly (Inventory.selectedSlot = index); the selection rides the same
// per-tick poll the block interaction runs on.
static void handle_hotbar_keys(LIBMATTI_MC_Minecraft *minecraft)
{
    if (minecraft->window == 0)
        return;
    // GLFW_KEY_1..KEY_9 are the consecutive keycodes 49..57.
    for (int slot = 0; slot < HOTBAR_SIZE; slot++)
    {
        if (LIBMATTI_GLFW_glfwGetKey(minecraft->window, LIBMATTI_GLFW_KEY_1 + slot) == LIBMATTI_GLFW_PRESS)
        {
            if (minecraft->hotbarSelected != slot)
            {
                minecraft->hotbarSelected = slot;
                // Java: Gui.tick - the 10s (200 tick) highlight fade restarts
                // on every selection change.
                minecraft->toolHighlightTimer = 200;
                minecraft->toolHighlight = minecraft->hotbarItems[slot];
            }
        }
    }
}

// Java: the sections re-mesh when a block changes inside them (LevelRenderer
// blockChanged -> setSectionDirty). The port walks the registered sections and
// flags the ones overlapping the position's 3x3x3 block neighbourhood (Java's
// markAndRebuildBlocks spans the faces the change can bleed into).
static void dirty_sections_around(LIBMATTI_MC_Minecraft *minecraft, const LIBMATTI_MC_BlockPos *pos)
{
    if (minecraft->sectionDispatcher == NULL)
        return;
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher = minecraft->sectionDispatcher;
    int minX = pos->base.x - 1, minY = pos->base.y - 1, minZ = pos->base.z - 1;
    int maxX = pos->base.x + 1, maxY = pos->base.y + 1, maxZ = pos->base.z + 1;
    for (int i = 0; i < dispatcher->sectionCount; i++)
    {
        LIBMATTI_MC_RenderSection *section = dispatcher->sections[i];
        const LIBMATTI_MC_Vec3i *origin = &section->sectionPos->base;
        int sx = LIBMATTI_MC_Vec3i_GetX(origin) * LIBMATTI_MC_SectionPos_SECTION_SIZE;
        int sy = LIBMATTI_MC_Vec3i_GetY(origin) * LIBMATTI_MC_SectionPos_SECTION_SIZE;
        int sz = LIBMATTI_MC_Vec3i_GetZ(origin) * LIBMATTI_MC_SectionPos_SECTION_SIZE;
        if (maxX < sx || minX >= sx + LIBMATTI_MC_SectionPos_SECTION_SIZE
            || maxY < sy || minY >= sy + LIBMATTI_MC_SectionPos_SECTION_SIZE
            || maxZ < sz || minZ >= sz + LIBMATTI_MC_SectionPos_SECTION_SIZE)
            continue;
        LIBMATTI_MC_RenderSection_SetDirty(section, 1);
    }
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

    // Java: MouseHandler - the mouse-look input runs every frame, before
    // the renderer picks the camera up. The walk/gravity impulses ride the
    // per-tick loop below (apply_walk inside tick), not the frame rate.
    updateMouseLook(minecraft);

    // Java: Camera.setup(BlockGetter, Entity, ...) - the camera rides the local
    // player's entity: eye position + rotation (the renderer reads it below)
    if (minecraft->localPlayer != NULL)
    {
        LIBMATTI_MC_Entity *entity = &minecraft->localPlayer->player.base.base;
        LIBMATTI_MC_Camera_SetRotation(&minecraft->camera, entity->yRot, entity->xRot);
        LIBMATTI_MC_Camera_SetPosition(&minecraft->camera, entity->x, entity->y + LIBMATTI_MC_Entity_GetEyeHeight(entity), entity->z);

        // Java: GameRenderer.pick - the crosshair ray from the eye along the
        // view vector over the CREATIVE reach (4.5 blocks). The context rides
        // the COLLIDER block mode (the outline shape equals the cube here) and
        // the NONE fluid mode (the port has no fluids).
        if (minecraft->level != NULL)
        {
            LIBMATTI_MC_Vec3 eye = {entity->x, entity->y + LIBMATTI_MC_Entity_GetEyeHeight(entity), entity->z};
            LIBMATTI_MC_Vec3 end = {eye.x + minecraft->camera.forwards.x * 4.5,
                                    eye.y + minecraft->camera.forwards.y * 4.5,
                                    eye.z + minecraft->camera.forwards.z * 4.5};
            LIBMATTI_MC_ClipContext context = LIBMATTI_MC_ClipContext_New(
                &eye, &end, LIBMATTI_MC_ClipContext_Block_COLLIDER,
                LIBMATTI_MC_ClipContext_Fluid_NONE, NULL, NULL);
            minecraft->hitResult = LIBMATTI_MC_Level_Clip((LIBMATTI_MC_Level *) minecraft->level, &context);
            debug_pick(minecraft);
        }
    }

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

                    // Java: the world pass clears to the sky fog color (the
                    // theme red stays the 2D layout's background).
                    LIBMATTI_B3D_GlStateManager_ClearColor(0.47f, 0.65f, 1.0f, 1.0f);
                    LIBMATTI_B3D_GlStateManager_Clear(LIBMATTI_GL_GL_COLOR_BUFFER_BIT | LIBMATTI_GL_GL_DEPTH_BUFFER_BIT);
                    shot_after(minecraft, "clear");

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

                            // Java: the GameRenderer picks up the camera the
                            // player controls - MouseLook wrote yRot/xRot and
                            // WASD walked the eye. The env overrides stay for
                            // the deterministic screenshot/cull sweeps.
                            float camYaw = minecraft->camera.yRot;
                            float camPitch = minecraft->camera.xRot;
                            if (getenv("MATTI_CAM_YAW") != NULL)
                                camYaw = (float) atof(getenv("MATTI_CAM_YAW"));
                            if (getenv("MATTI_CAM_PITCH") != NULL)
                                camPitch = (float) atof(getenv("MATTI_CAM_PITCH"));
                            LIBMATTI_MC_Camera_SetRotation(&minecraft->camera, camYaw, camPitch);

                            LIBMATTI_JOML_Matrix4f proj, view, mvpM;
                            const float aspect = (float) width / (float) height;
                            LIBMATTI_JOML_Matrix4f_SetPerspective(&proj, 1.2217f, aspect, 0.05f, 1000.0f);
                            // The earlydisplay FBO is blitted vertically flipped
                            // onto the window (the y-down GUI elements need it).
                            // The 3D world passes render y-up, so they draw with
                            // the Y-flipped projection: after the blit flip the
                            // world lands upright AND the GUI stays consistent.
                            proj.m11 = -proj.m11;
                            LIBMATTI_MC_GameRenderer_BuildRotationMatrix(&minecraft->camera, &view);
                            // The clip matrix: projection·view (the projection
                            // applies last - the GL convention the frustum's
                            // plane extraction and the shader both need).
                            LIBMATTI_JOML_Matrix4f_Mul(&proj, &view, &mvpM);

                            // Java: this.cullingFrustum = new Frustum(proj, view)
                            // with the ROTATION-ONLY modelview - prepare() shifts
                            // the box tests by the camera position instead. The
                            // full view (with the translation) rides the shader.
                            LIBMATTI_JOML_Matrix4f viewRotOnly;
                            memcpy(&viewRotOnly, &view, sizeof(viewRotOnly));
                            viewRotOnly.m30 = 0.0f;
                            viewRotOnly.m31 = 0.0f;
                            viewRotOnly.m32 = 0.0f;
                            LIBMATTI_MC_Frustum_Init(&minecraft->frustum, &proj, &viewRotOnly);
                            LIBMATTI_MC_Frustum_Prepare(&minecraft->frustum,
                                                        minecraft->camera.x, minecraft->camera.y, minecraft->camera.z);

                            // Java: LevelRenderer.renderLevel's sky pass (the
                            // renderSky section) - the disc, sunrise/sunset,
                            // sun/moon/stars and the dark disc draw through
                            // the SkyRenderer before the sections.
                            long dayTime = minecraft->level != NULL
                                               ? LIBMATTI_MC_Level_GetDayTime((LIBMATTI_MC_Level *) minecraft->level)
                                               : 0;
                            float timeOfDay = (float) (dayTime % 24000L);
                            float dayFraction = timeOfDay / 24000.0f;
                            if (minecraft->skyRenderer != NULL && getenv("MATTI_NO_SKY") == NULL)
                            {
                                // Java: ClientLevel - timeOfDay = dayTime % 24000
                                // over the DAY timeline period; the angles follow
                                // the 1.21.11 keys (sun/star 0 -> 360, moon 180 ->
                                // 540 over the day, sunrise color at the edges).
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

                                // Java: the sky pass renders with the
                                // rotation-only model view - the disc/celestials
                                // coordinates are CAMERA-RELATIVE (the disc at
                                // y=16 is 16 above the EYE, not above world 0).
                                // The full view (with the translation) would put
                                // the disc 72 blocks below the camera at (8,88).
                                LIBMATTI_MC_SkyRenderer_DrawSkyDisc(minecraft->skyRenderer, &viewRotOnly, &proj, skyR,
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
                                        minecraft->skyRenderer, &viewRotOnly, &proj, sunAngle, sunriseAlpha);

                                LIBMATTI_MC_SkyRenderer_RenderSunMoonAndStars(
                                    minecraft->skyRenderer, &viewRotOnly, &proj, sunAngle, moonAngle, starAngle, phase,
                                    rainBrightness, starBrightness);

                                // Java: shouldRenderDarkDisc - the eye below the
                                // horizon; the fixed skeleton camera stays above.

                                LIBMATTI_B3D_GlStateManager_DepthMask(1);
                                LIBMATTI_B3D_GlStateManager_DisableBlend();
                                shot_after(minecraft, "sky");
                            }

                            float mvp[16];
                            memcpy(mvp, &mvpM, sizeof(mvp));
                            float origin[3] = {0.0f, 0.0f, 0.0f};
                            // Java: renderLevel binds the atlas texture before the
                            // section draws (ShaderInstance.SAMPLER_SOURCE -> the
                            // TextureManager's block atlas on Sampler0). The unit
                            // is set explicitly - the sky pass leaves unit 1 bound.
                            if (minecraft->modelManager != NULL)
                            {
                                LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_ModelManager_GetAtlas(minecraft->modelManager);
                                // The procedural atlas uploads lazily - the GL
                                // context does not exist at bootstrap time. The
                                // first terrain draw pushes the 32x16 RGBA page.
                                if (atlas != NULL && atlas->base.texture == 0 && atlas->base.pixels != NULL)
                                {
                                    atlas->base.texture = LIBMATTI_B3D_GlStateManager_GenTexture();
                                    atlas->base.textureView = atlas->base.texture;
                                    // Raw-bind on unit 0 BEFORE the upload: the
                                    // GlStateManager cache may believe another texture
                                    // is bound and skip the bind inside WriteToTexture,
                                    // uploading into a foreign object.
                                    LIBMATTI_GL_glActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
                                    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, atlas->base.texture);
                                    LIBMATTI_MC_Texture_WriteToTexture(atlas->base.texture, atlas->base.pixels);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
                                    LIBMATTI_GL_glGenerateMipmap(LIBMATTI_GL_GL_TEXTURE_2D);
                                    fprintf(stderr, "[TERRAINTEX] uploaded atlas=%u page=%dx%d\n",
                                            atlas->base.texture, atlas->base.pixels->width, atlas->base.pixels->height);
                                }
                                if (atlas != NULL && atlas->base.texture != 0)
                                {
                                    // All raw: any GlStateManager cache state may
                                    // be stale (the sky pass and FBO binds go around
                                    // it), so force unit 0 + the atlas + sane filters
                                    // right before the section draws.
                                    LIBMATTI_GL_glActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
                                    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, atlas->base.texture);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
                                    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
                                    int samplerLocation = LIBMATTI_GL_glGetUniformLocation(terrainProgram, "Sampler0");
                                    if (samplerLocation >= 0)
                                    {
                                        // glUniform1i requires an active program -
                                        // RenderLayer's glUseProgram comes later, so
                                        // activate ours here or the call is a no-op.
                                        LIBMATTI_GL_glUseProgram(terrainProgram);
                                        LIBMATTI_GL_glUniform1i(samplerLocation, 0);
                                    }
                                }
                                else
                                {
                                    fprintf(stderr, "[TERRAINTEX] NO ATLAS (atlas=%p tex=%u)\n",
                                            (const void *) atlas, atlas != NULL ? atlas->base.texture : 0);
                                }
                            }
                            LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(
                                minecraft->sectionDispatcher, LIBMATTI_MC_ChunkSectionLayer_SOLID,
                                terrainProgram, mvp, origin, &minecraft->frustum);

                            // Java: the CUTOUT_TERRAIN pass - the non-occluding
                            // blocks (glass, leaves) render after the SOLID pass
                            // with the alpha-tested fragment discard.
                            LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(
                                minecraft->sectionDispatcher, LIBMATTI_MC_ChunkSectionLayer_CUTOUT,
                                terrainProgram, mvp, origin, &minecraft->frustum);

                            // Java: addCloudsPass - after the main (terrain)
                            // pass, before weather. The cloud color is the
                            // CLOUD_COLOR attribute (the overworld curve:
                            // white by day, dimmed at dusk/night), the cloud
                            // height is the vanilla 192.
                            if (minecraft->cloudRenderer != NULL && getenv("MATTI_NO_CLOUDS") == NULL)
                            {
                                float dim = 1.0f;
                                if (dayFraction > 13670.0f / 24000.0f && dayFraction < 22330.0f / 24000.0f)
                                    dim = 0.35f;
                                LIBMATTI_B3D_GlStateManager_EnableBlend();
                                LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(LIBMATTI_GL_GL_SRC_ALPHA,
                                                                              LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA,
                                                                              LIBMATTI_GL_GL_ONE,
                                                                              LIBMATTI_GL_GL_ZERO);
                                LIBMATTI_B3D_GlStateManager_DepthMask(0);
                                LIBMATTI_MC_CloudRenderer_Render(minecraft->cloudRenderer,
                                                                 LIBMATTI_MC_CloudStatus_FANCY,
                                                                 1.0f, 1.0f, 1.0f, dim, 192.0f,
                                                                 minecraft->camera.x, minecraft->camera.y,
                                                                 minecraft->camera.z,
                                                                 LIBMATTI_MC_Level_GetGameTime(
                                                                     (LIBMATTI_MC_Level *) minecraft->level),
                                                                 0.0f, &view, &proj, 12);
                                LIBMATTI_B3D_GlStateManager_DepthMask(1);
                                LIBMATTI_B3D_GlStateManager_DisableBlend();
                            }
                            shot_after(minecraft, "terrain");
                        }
                    }                    // The skeleton's title line (Java: the theme's LabelElement
                    // renders the game title inside the layout).

                    if (getenv("MATTI_NO_TITLE") == NULL)
                        render_title(minecraft, 854, 480);

                    // Java: Gui.render - the HUD layer (the crosshair + hotbar
                    // + name line) over the world pass, gated like the title
                    // line (MATTI_NO_HUD keeps the deterministic screenshots
                    // clean).
                    if (getenv("MATTI_NO_HUD") == NULL)
                    {
                        render_hud(minecraft, width, height);
                        shot_after(minecraft, "hud");
                    }

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

    // Java's run(): boolean flag = false; while (running) { runTick(!flag) } -
    // the flag only flips inside the OOM catch, so EVERY frame runs
    // runTick(true) (renderLevelInMainMenu is the first-frame special case;
    // the old loop-tail flip shape no longer exists in 1.21.11).
    int flag = 0;
    while (minecraft->running)
    {
        runTick(minecraft, !flag);
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

    // Java: this.player = null - the local player leaves the level list and
    // frees (the port's level entities are borrowed pointers)
    if (minecraft->localPlayer != NULL)
    {
        if (minecraft->level != NULL)
            LIBMATTI_MC_Level_RemoveEntity(minecraft->level, &minecraft->localPlayer->player.base.base);
        LIBMATTI_MC_LocalPlayer_Free(minecraft->localPlayer);
        minecraft->localPlayer = NULL;
    }
    // Java: Options - the static KeyMapping table releases
    LIBMATTI_MC_KeyMapping_ReleaseAll();

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

    // Java: this.gui = null - the HUD batcher closes with the game.
    if (minecraft->guiRenderer != NULL)
    {
        LIBMATTI_MC_GuiRenderer_Free(minecraft->guiRenderer);
        minecraft->guiRenderer = NULL;
    }
    for (int slot = 0; slot < HOTBAR_SIZE; slot++)
    {
        if (minecraft->hotbarItems[slot] != NULL)
        {
            LIBMATTI_MC_ItemStack_Free(minecraft->hotbarItems[slot]);
            minecraft->hotbarItems[slot] = NULL;
        }
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
