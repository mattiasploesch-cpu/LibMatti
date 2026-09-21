// Port of com.mojang.blaze3d.systems.RenderSystem (1.21.11 static API).

#ifndef MATTICRAFT_BLAZE3D_SYSTEMS_RENDERSYSTEM_H
#define MATTICRAFT_BLAZE3D_SYSTEMS_RENDERSYSTEM_H

#include "libmatti/org/joml/Matrix4f.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: VertexFormat.Mode - the primitive mode with its index layout.
typedef enum LIBMATTI_B3D_VertexFormat_Mode
{
    LIBMATTI_B3D_Mode_LINES = 0,
    LIBMATTI_B3D_Mode_DEBUG_LINES,
    LIBMATTI_B3D_Mode_DEBUG_LINE_STRIP,
    LIBMATTI_B3D_Mode_POINTS,
    LIBMATTI_B3D_Mode_TRIANGLES,
    LIBMATTI_B3D_Mode_TRIANGLE_STRIP,
    LIBMATTI_B3D_Mode_TRIANGLE_FAN,
    LIBMATTI_B3D_Mode_QUADS
} LIBMATTI_B3D_VertexFormat_Mode;

// Java: VertexFormat.IndexType.
typedef enum LIBMATTI_B3D_VertexFormat_IndexType
{
    LIBMATTI_B3D_IndexType_SHORT = 0,
    LIBMATTI_B3D_IndexType_INT
} LIBMATTI_B3D_VertexFormat_IndexType;

// Java: ProjectionType.
typedef enum LIBMATTI_B3D_ProjectionType
{
    LIBMATTI_B3D_ProjectionType_PERSPECTIVE = 0,
    LIBMATTI_B3D_ProjectionType_ORTHOGRAPHIC
} LIBMATTI_B3D_ProjectionType;

// Java: static Logger LOGGER - routes through the launcher's LogManager.
void LIBMATTI_B3D_RenderSystem_AssertOnRenderThread(void);
int LIBMATTI_B3D_RenderSystem_IsOnRenderThread(void);
void LIBMATTI_B3D_RenderSystem_InitRenderThread(void);

// Java: pollEvents + isFrozenAtPollEvents
void LIBMATTI_B3D_RenderSystem_PollEvents(void);
int LIBMATTI_B3D_RenderSystem_IsFrozenAtPollEvents(void);

// Java: flipFrame(Window, TracyFrameCapture) - the port takes the GLFW window
// handle; the Tesselator/LevelRenderer/DynamicUniforms reset hooks run through
// the game-side registration (nothing to clear yet in the skeleton).
void LIBMATTI_B3D_RenderSystem_FlipFrame(long window);

// Java: limitDisplayFPS(int) - sleeps until the target frame time via
// glfwWaitEventsTimeout, exactly like the Java original.
void LIBMATTI_B3D_RenderSystem_LimitDisplayFPS(int fps);

// Java: setShaderFog / getShaderFog / setShaderLights / getShaderLights
void LIBMATTI_B3D_RenderSystem_SetShaderFog(void *shaderFog);
void *LIBMATTI_B3D_RenderSystem_GetShaderFog(void);
void LIBMATTI_B3D_RenderSystem_SetShaderLights(void *shaderLightDirections);
void *LIBMATTI_B3D_RenderSystem_GetShaderLights(void);

// Java: enableScissorForRenderTypeDraws / disableScissorForRenderTypeDraws
void LIBMATTI_B3D_RenderSystem_EnableScissorForRenderTypeDraws(int x, int y, int width, int height);
void LIBMATTI_B3D_RenderSystem_DisableScissorForRenderTypeDraws(void);

// Java: getBackendDescription / getApiDescription
const char *LIBMATTI_B3D_RenderSystem_GetBackendDescription(void);
const char *LIBMATTI_B3D_RenderSystem_GetApiDescription(void);

// Java: initRenderer - the port loads the GL binding for the current context
// (the GlDevice ctor resolves everything eagerly in Java).
void LIBMATTI_B3D_RenderSystem_InitRenderer(long window);

// Java: setupDefaultState
void LIBMATTI_B3D_RenderSystem_SetupDefaultState(void);

// Java: setProjectionMatrix / backupProjectionMatrix / restoreProjectionMatrix
void LIBMATTI_B3D_RenderSystem_SetProjectionMatrix(const float *projectionMatrix, LIBMATTI_B3D_ProjectionType type);
void LIBMATTI_B3D_RenderSystem_BackupProjectionMatrix(void);
void LIBMATTI_B3D_RenderSystem_RestoreProjectionMatrix(void);
const float *LIBMATTI_B3D_RenderSystem_GetProjectionMatrix(void);
LIBMATTI_B3D_ProjectionType LIBMATTI_B3D_RenderSystem_GetProjectionType(void);

// Java: getModelViewMatrix / getModelViewStack - the stack is a Matrix4fStack
// (a Matrix4f with push/pop positions). The port exposes the raw 16 floats.
LIBMATTI_JOML_Matrix4f *LIBMATTI_B3D_RenderSystem_GetModelViewStack(void);
void LIBMATTI_B3D_RenderSystem_PushModelView(void);
void LIBMATTI_B3D_RenderSystem_PopModelView(void);

// Java: getSequentialBuffer(Mode) - the three shared AutoStorageIndexBuffers.
// Returns the GL buffer handle, sets *outType and *outIndexCount.
void *LIBMATTI_B3D_RenderSystem_GetSequentialBuffer(LIBMATTI_B3D_VertexFormat_Mode mode,
                                                    LIBMATTI_B3D_VertexFormat_IndexType *outType,
                                                    int *outIndexCount);

// Java: queueFencedTask / executePendingTasks - the port runs the tasks
// directly (no GPU fences yet, every task is "complete" immediately like a
// fence that awaits with a 0 timeout returning ALREADY_SIGNALED).
void LIBMATTI_B3D_RenderSystem_QueueFencedTask(void (*callback)(void *), void *userdata);
void LIBMATTI_B3D_RenderSystem_ExecutePendingTasks(void);

#ifdef __cplusplus
}
#endif

#endif
