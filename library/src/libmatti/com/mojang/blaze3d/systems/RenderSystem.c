// Port of com.mojang.blaze3d.systems.RenderSystem - the static render API.
// The 1.21.11 original routes through a GpuDevice; the port keeps the static
// surface (thread assertions, poll/flip, FPS limit, model-view stack, shared
// index buffers) and talks to GL through the GlStateManager cache layer.

#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/org/lwjgl/glfw/GLFW.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdlib.h>
#include <string.h>

// Java: private static Thread renderThread
static void *renderThread = NULL;
// Java: private static double lastDrawTime = Double.MIN_VALUE;
static double lastDrawTime = 2.2250738585072014E-308;
// Java: private static String apiDescription = "Unknown";
static char *apiDescription = NULL;
// Java: private static final AtomicLong pollEventsWaitStart, AtomicBoolean pollingEvents
static long long pollEventsWaitStart = 0;
static int pollingEvents = 0;
// Java: projectionType / savedProjectionType
static LIBMATTI_B3D_ProjectionType projectionType = LIBMATTI_B3D_ProjectionType_PERSPECTIVE;
static LIBMATTI_B3D_ProjectionType savedProjectionType = LIBMATTI_B3D_ProjectionType_PERSPECTIVE;
// Java: private static final Matrix4fStack modelViewStack = new Matrix4fStack(16);
// A Matrix4fStack is a Matrix4f plus a position stack of 16 float slots.
#define MODELVIEW_STACK_DEPTH 16
static LIBMATTI_JOML_Matrix4f modelViewStack;
static LIBMATTI_JOML_Matrix4f modelViewStackEntries[MODELVIEW_STACK_DEPTH];
static int modelViewStackPosition = 0;
// Java: projectionMatrixBuffer / savedProjectionMatrixBuffer hold GpuBufferSlice
// references; the port keeps the float matrices directly.
static float projectionMatrix[16];
static int projectionMatrixSet = 0;
static float savedProjectionMatrix[16];
// Java: shaderFog / shaderLightDirections
static void *shaderFog = NULL;
static void *shaderLightDirections = NULL;
// Java: private static final ArrayListDeque<GpuAsyncTask> PENDING_FENCES
typedef struct GpuAsyncTask
{
    void (*callback)(void *);
    void *userdata;
    struct GpuAsyncTask *next;
} GpuAsyncTask;
static GpuAsyncTask *pendingTasksHead = NULL;
static GpuAsyncTask *pendingTasksTail = NULL;

// Java: RenderSystem.AutoStorageIndexBuffer
typedef struct AutoStorageIndexBuffer
{
    int vertexStride;
    int indexStride;
    // Java: IndexGenerator - fills the buffer for one index quad/segment base.
    int kind;  // 0 = sequential, 1 = quad, 2 = lines
    unsigned int buffer;
    LIBMATTI_B3D_VertexFormat_IndexType type;
    int indexCount;
} AutoStorageIndexBuffer;

static AutoStorageIndexBuffer sharedSequential = {1, 1, 0, 0, LIBMATTI_B3D_IndexType_SHORT, 0};
static AutoStorageIndexBuffer sharedSequentialQuad = {4, 6, 1, 0, LIBMATTI_B3D_IndexType_SHORT, 0};
static AutoStorageIndexBuffer sharedSequentialLines = {4, 6, 2, 0, LIBMATTI_B3D_IndexType_SHORT, 0};

static LIBMATTI_ML_Logger *logger(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

void LIBMATTI_B3D_RenderSystem_InitRenderThread(void)
{
    // Java: if (renderThread != null) throw new IllegalStateException(...)
    if (renderThread != NULL)
    {
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Could not initialize render thread");
        return;
    }
    renderThread = (void *) (size_t) 1;  // Port: single render thread marker.
}

int LIBMATTI_B3D_RenderSystem_IsOnRenderThread(void)
{
    return renderThread != NULL;
}

void LIBMATTI_B3D_RenderSystem_AssertOnRenderThread(void)
{
    // Java: assertOnRenderThread throws "Rendersystem called from wrong thread";
    // the port logs once per call site state and continues (the game loop is
    // single threaded in the current skeleton).
    if (!LIBMATTI_B3D_RenderSystem_IsOnRenderThread())
    {
        static int warned = 0;
        if (!warned)
        {
            warned = 1;
            LIBMATTI_ML_Logger_Error(logger(), NULL, "Rendersystem called from wrong thread");
        }
    }
}

void LIBMATTI_B3D_RenderSystem_PollEvents(void)
{
    // Java: pollEventsWaitStart.set(Util.getMillis()); pollingEvents.set(true);
    // Java: Util.getMillis() - the port's time base is System.currentTimeMillis.
    pollEventsWaitStart = LIBMATTI_JL_System_CurrentTimeMillis();
    pollingEvents = 1;
    LIBMATTI_GLFW_glfwPollEvents();
    pollingEvents = 0;
}

int LIBMATTI_B3D_RenderSystem_IsFrozenAtPollEvents(void)
{
    return pollingEvents && LIBMATTI_JL_System_CurrentTimeMillis() - pollEventsWaitStart > 200L;
}

void LIBMATTI_B3D_RenderSystem_FlipFrame(long window)
{
    LIBMATTI_B3D_RenderSystem_PollEvents();
    // Java: Tesselator.getInstance().clear();
    // Java: GLFW.glfwSwapBuffers(p_424398_.handle());
    LIBMATTI_GLFW_glfwSwapBuffers(window);
    // Java: dynamicUniforms.reset(); Minecraft.getInstance().levelRenderer.endFrame();
    // - nothing registered in the skeleton yet.
    LIBMATTI_B3D_RenderSystem_PollEvents();
}

void LIBMATTI_B3D_RenderSystem_LimitDisplayFPS(int fps)
{
    // Java: double d0 = lastDrawTime + 1.0 / fps;
    //       for (d1 = glfwGetTime(); d1 < d0; d1 = glfwGetTime())
    //           glfwWaitEventsTimeout(d0 - d1);
    //       lastDrawTime = d1;
    double target = lastDrawTime + 1.0 / fps;
    double now;
    for (now = LIBMATTI_GLFW_glfwGetTime(); now < target; now = LIBMATTI_GLFW_glfwGetTime())
        LIBMATTI_GLFW_glfwWaitEventsTimeout(target - now);
    lastDrawTime = now;
}

void LIBMATTI_B3D_RenderSystem_SetShaderFog(void *value)
{
    shaderFog = value;
}

void *LIBMATTI_B3D_RenderSystem_GetShaderFog(void)
{
    return shaderFog;
}

void LIBMATTI_B3D_RenderSystem_SetShaderLights(void *value)
{
    shaderLightDirections = value;
}

void *LIBMATTI_B3D_RenderSystem_GetShaderLights(void)
{
    return shaderLightDirections;
}

void LIBMATTI_B3D_RenderSystem_EnableScissorForRenderTypeDraws(int x, int y, int width, int height)
{
    // Java: scissorStateForRenderTypeDraws.enable(...); the ScissorState tracks
    // a stack of rectangles. The port enables the scissor test directly through
    // the state manager (one level, matching what RenderType draws need).
    LIBMATTI_B3D_GlStateManager_EnableScissorTest();
    // Java: ScissorState.enable applies y-flip against the window height in the
    // caller; the raw box is forwarded.
    LIBMATTI_B3D_GlStateManager_ScissorBox(x, y, width, height);
}

void LIBMATTI_B3D_RenderSystem_DisableScissorForRenderTypeDraws(void)
{
    LIBMATTI_B3D_GlStateManager_DisableScissorTest();
}

const char *LIBMATTI_B3D_RenderSystem_GetBackendDescription(void)
{
    // Java: String.format("LWJGL version %s", GLX._getLWJGLVersion());
    return "LWJGL version 3.3.3+build.1 (port)";
}

const char *LIBMATTI_B3D_RenderSystem_GetApiDescription(void)
{
    return apiDescription != NULL ? apiDescription : "Unknown";
}

void LIBMATTI_B3D_RenderSystem_InitRenderer(long window)
{
    (void) window;
    // Java: DEVICE = new GlDevice(handle, ...); apiDescription = getDevice()
    //       .getImplementationInformation();
    // The port resolves the GL entry points eagerly for the current context.
    LIBMATTI_GL_Load();
    free(apiDescription);
    const char *vendor = LIBMATTI_GL_glGetString(LIBMATTI_GL_GL_VENDOR);
    const char *version = LIBMATTI_GL_glGetString(LIBMATTI_GL_GL_VERSION);
    const char *renderer = LIBMATTI_GL_glGetString(LIBMATTI_GL_GL_RENDERER);
    size_t len = 64;
    if (vendor != NULL) len += strlen(vendor);
    if (version != NULL) len += strlen(version);
    if (renderer != NULL) len += strlen(renderer);
    apiDescription = malloc(len);
    apiDescription[0] = '\0';
    if (vendor != NULL) strcat(apiDescription, vendor);
    if (version != NULL)
    {
        strcat(apiDescription, " ");
        strcat(apiDescription, version);
    }
    if (renderer != NULL)
    {
        strcat(apiDescription, " ");
        strcat(apiDescription, renderer);
    }
    LIBMATTI_ML_Logger_Info(logger(), NULL, "GL device: {}", apiDescription);
}

void LIBMATTI_B3D_RenderSystem_SetupDefaultState(void)
{
    // Java: modelViewStack.clear() - identity, position 0.
    modelViewStackPosition = 0;
    LIBMATTI_JOML_Matrix4f_Identity(&modelViewStack);
}

void LIBMATTI_B3D_RenderSystem_SetProjectionMatrix(const float *matrix, LIBMATTI_B3D_ProjectionType type)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    memcpy(projectionMatrix, matrix, sizeof(projectionMatrix));
    projectionMatrixSet = 1;
    projectionType = type;
}

void LIBMATTI_B3D_RenderSystem_BackupProjectionMatrix(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    memcpy(savedProjectionMatrix, projectionMatrix, sizeof(projectionMatrix));
    savedProjectionType = projectionType;
}

void LIBMATTI_B3D_RenderSystem_RestoreProjectionMatrix(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    memcpy(projectionMatrix, savedProjectionMatrix, sizeof(projectionMatrix));
    projectionType = savedProjectionType;
}

const float *LIBMATTI_B3D_RenderSystem_GetProjectionMatrix(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return projectionMatrixSet ? projectionMatrix : NULL;
}

LIBMATTI_B3D_ProjectionType LIBMATTI_B3D_RenderSystem_GetProjectionType(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return projectionType;
}

LIBMATTI_JOML_Matrix4f *LIBMATTI_B3D_RenderSystem_GetModelViewStack(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return &modelViewStack;
}

void LIBMATTI_B3D_RenderSystem_PushModelView(void)
{
    // Java: Matrix4fStack.push() - copies the current matrix one level down.
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (modelViewStackPosition >= MODELVIEW_STACK_DEPTH)
        return;
    modelViewStackEntries[modelViewStackPosition] = modelViewStack;
    modelViewStackPosition++;
}

void LIBMATTI_B3D_RenderSystem_PopModelView(void)
{
    // Java: Matrix4fStack.pop() - restores the matrix one level down.
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (modelViewStackPosition <= 0)
        return;
    modelViewStackPosition--;
    modelViewStack = modelViewStackEntries[modelViewStackPosition];
}

// Java: AutoStorageIndexBuffer.IndexGenerator - the three built-in generators.
static void index_generator_fill(const AutoStorageIndexBuffer *storage, unsigned short *out, int base)
{
    int i = 0;
    switch (storage->kind)
    {
        case 1:  // sharedSequentialQuad: v, v+1, v+2, v+2, v+3, v
            out[i++] = (unsigned short) (base + 0);
            out[i++] = (unsigned short) (base + 1);
            out[i++] = (unsigned short) (base + 2);
            out[i++] = (unsigned short) (base + 2);
            out[i++] = (unsigned short) (base + 3);
            out[i++] = (unsigned short) (base + 0);
            break;
        case 2:  // sharedSequentialLines: v, v+1, v+2, v+3, v+2, v+1
            out[i++] = (unsigned short) (base + 0);
            out[i++] = (unsigned short) (base + 1);
            out[i++] = (unsigned short) (base + 2);
            out[i++] = (unsigned short) (base + 3);
            out[i++] = (unsigned short) (base + 2);
            out[i++] = (unsigned short) (base + 1);
            break;
        default:  // sharedSequential: v
            out[i++] = (unsigned short) base;
            break;
    }
}

// Java: ensureStorage(int) - grows the shared buffer to at least indexCount.
static void ensure_storage(AutoStorageIndexBuffer *storage, int count)
{
    if (count <= storage->indexCount)
        return;

    // Java: p_157477_ = Mth.roundToward(p_157477_ * 2, this.indexStride);
    count = LIBMATTI_MC_Mth_RoundToward(count * 2, storage->indexStride);
    LIBMATTI_ML_Logger_Debug(logger(), NULL, "Growing IndexBuffer: Old limit {}, new limit {}.",
                             storage->indexCount, count);
    int vertices = count / storage->indexStride;
    // Java: VertexFormat.IndexType.least(j) - INT when the vertex count exceeds
    // 65535, SHORT otherwise.
    LIBMATTI_B3D_VertexFormat_IndexType type = (vertices & -65536) != 0 ? LIBMATTI_B3D_IndexType_INT
                                                                        : LIBMATTI_B3D_IndexType_SHORT;
    int typeBytes = type == LIBMATTI_B3D_IndexType_INT ? 4 : 2;
    int byteSize = LIBMATTI_MC_Mth_RoundToward(count * typeBytes, 4);

    storage->type = type;
    if (storage->buffer != 0)
        LIBMATTI_B3D_GlStateManager_DeleteBuffers(storage->buffer);

    storage->buffer = LIBMATTI_B3D_GlStateManager_GenBuffers();
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, storage->buffer);
    // Java: fill a ByteBuffer through the IndexGenerator and upload it.
    {
        unsigned short *indices = malloc((size_t) byteSize);
        int index = 0;
        for (int i = 0; i < count; i += storage->indexStride)
        {
            index_generator_fill(storage, indices + index, i * storage->vertexStride / storage->indexStride);
            index += storage->indexStride;
        }
        LIBMATTI_B3D_GlStateManager_BufferData(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, indices, byteSize,
                                               LIBMATTI_GL_GL_STATIC_DRAW);
        free(indices);
    }
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    storage->indexCount = count;
}

void *LIBMATTI_B3D_RenderSystem_GetSequentialBuffer(LIBMATTI_B3D_VertexFormat_Mode mode,
                                                    LIBMATTI_B3D_VertexFormat_IndexType *outType,
                                                    int *outIndexCount)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    AutoStorageIndexBuffer *storage;
    switch (mode)
    {
        case LIBMATTI_B3D_Mode_QUADS:
            storage = &sharedSequentialQuad;
            break;
        case LIBMATTI_B3D_Mode_LINES:
            storage = &sharedSequentialLines;
            break;
        default:
            storage = &sharedSequential;
            break;
    }
    if (outType != NULL)
        *outType = storage->type;
    if (outIndexCount != NULL)
        *outIndexCount = storage->indexCount;
    return (void *) (size_t) storage->buffer;
}

void LIBMATTI_B3D_RenderSystem_QueueFencedTask(void (*callback)(void *), void *userdata)
{
    GpuAsyncTask *task = malloc(sizeof(GpuAsyncTask));
    task->callback = callback;
    task->userdata = userdata;
    task->next = NULL;
    if (pendingTasksTail != NULL)
        pendingTasksTail->next = task;
    else
        pendingTasksHead = task;
    pendingTasksTail = task;
}

void LIBMATTI_B3D_RenderSystem_ExecutePendingTasks(void)
{
    // Java: the loop only runs the task when the fence reports completion with
    // a zero timeout; the port has no fences, every task is ready immediately.
    while (pendingTasksHead != NULL)
    {
        GpuAsyncTask *task = pendingTasksHead;
        pendingTasksHead = task->next;
        if (pendingTasksHead == NULL)
            pendingTasksTail = NULL;
        task->callback(task->userdata);
        free(task);
    }
}
