// RenderSystem harness: exercises the state-cache layer and the RenderSystem
// API without a GL context - every call takes the cached/no-driver path, the
// same fallbacks LWJGL's generated calls use when the driver is absent.

#include "libmatti/com/mojang/blaze3d/buffers/GlBuffer.h"
#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/opengl/VertexArrayCache.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"

#include <stdio.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

static void fenced_task(void *userdata)
{
    *(int *) userdata = 1;
}

#define CHECK(cond)                                          \
    do                                                       \
    {                                                        \
        checks++;                                            \
        if (!(cond))                                         \
        {                                                    \
            failures++;                                      \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                    \
    } while (0)

int main(void)
{
    // Java: RenderSystem.initRenderThread + assertOnRenderThread.
    LIBMATTI_B3D_RenderSystem_InitRenderThread();
    CHECK(LIBMATTI_B3D_RenderSystem_IsOnRenderThread());
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();

    // Java: setupDefaultState - model-view stack starts at the identity.
    LIBMATTI_B3D_RenderSystem_SetupDefaultState();
    LIBMATTI_JOML_Matrix4f *mv = LIBMATTI_B3D_RenderSystem_GetModelViewStack();
    CHECK(mv != NULL);
    CHECK(mv->m00 == 1.0f && mv->m11 == 1.0f && mv->m22 == 1.0f && mv->m33 == 1.0f);
    CHECK(mv->m01 == 0.0f && mv->m32 == 0.0f);

    // Java: Matrix4fStack push/pop round trip.
    LIBMATTI_JOML_Matrix4f_Translate(mv, 1.0f, 2.0f, 3.0f);
    CHECK(mv->m30 == 1.0f && mv->m31 == 2.0f && mv->m32 == 3.0f);
    LIBMATTI_B3D_RenderSystem_PushModelView();
    LIBMATTI_JOML_Matrix4f_Translate(mv, 10.0f, 0.0f, 0.0f);
    CHECK(mv->m30 == 11.0f);
    LIBMATTI_B3D_RenderSystem_PopModelView();
    CHECK(mv->m30 == 1.0f && mv->m31 == 2.0f);

    // Java: setProjectionMatrix / backup / restore.
    float proj[16] = {0};
    proj[0] = 1.0f;
    proj[5] = 1.0f;
    proj[10] = -1.0f;
    proj[15] = 1.0f;
    LIBMATTI_B3D_RenderSystem_SetProjectionMatrix(proj, LIBMATTI_B3D_ProjectionType_PERSPECTIVE);
    CHECK(LIBMATTI_B3D_RenderSystem_GetProjectionMatrix() == proj || memcmp(LIBMATTI_B3D_RenderSystem_GetProjectionMatrix(), proj, sizeof(proj)) == 0);
    CHECK(LIBMATTI_B3D_RenderSystem_GetProjectionType() == LIBMATTI_B3D_ProjectionType_PERSPECTIVE);
    LIBMATTI_B3D_RenderSystem_BackupProjectionMatrix();
    float other[16] = {0};
    other[15] = 2.0f;
    LIBMATTI_B3D_RenderSystem_SetProjectionMatrix(other, LIBMATTI_B3D_ProjectionType_ORTHOGRAPHIC);
    CHECK(LIBMATTI_B3D_RenderSystem_GetProjectionType() == LIBMATTI_B3D_ProjectionType_ORTHOGRAPHIC);
    LIBMATTI_B3D_RenderSystem_RestoreProjectionMatrix();
    CHECK(LIBMATTI_B3D_RenderSystem_GetProjectionType() == LIBMATTI_B3D_ProjectionType_PERSPECTIVE);
    CHECK(LIBMATTI_B3D_RenderSystem_GetProjectionMatrix()[15] == 1.0f);

    // Java: setShaderFog / setShaderLights round trip.
    int fog = 0;
    LIBMATTI_B3D_RenderSystem_SetShaderFog(&fog);
    CHECK(LIBMATTI_B3D_RenderSystem_GetShaderFog() == &fog);
    int lights = 0;
    LIBMATTI_B3D_RenderSystem_SetShaderLights(&lights);
    CHECK(LIBMATTI_B3D_RenderSystem_GetShaderLights() == &lights);

    // Java: getSequentialBuffer - the three shared storages grow lazily; without
    // a driver the handles stay 0 but the types are reported correctly.
    LIBMATTI_B3D_VertexFormat_IndexType type = LIBMATTI_B3D_IndexType_SHORT;
    int indexCount = -1;
    void *buf = LIBMATTI_B3D_RenderSystem_GetSequentialBuffer(LIBMATTI_B3D_Mode_QUADS, &type, &indexCount);
    CHECK(type == LIBMATTI_B3D_IndexType_SHORT);
    CHECK(buf == (void *) 0);
    (void) LIBMATTI_B3D_RenderSystem_GetSequentialBuffer(LIBMATTI_B3D_Mode_LINES, &type, &indexCount);
    CHECK(type == LIBMATTI_B3D_IndexType_SHORT);

    // Java: GpuBuffer usage flags + close path (no driver: handle stays 0).
    LIBMATTI_B3D_GpuBuffer *buffer = LIBMATTI_B3D_GlBuffer_New(LIBMATTI_B3D_USAGE_VERTEX | LIBMATTI_B3D_USAGE_MAP_WRITE, 256);
    CHECK(buffer != NULL);
    CHECK(LIBMATTI_B3D_GpuBuffer_Usage(buffer) == (LIBMATTI_B3D_USAGE_VERTEX | LIBMATTI_B3D_USAGE_MAP_WRITE));
    CHECK(LIBMATTI_B3D_GpuBuffer_Size(buffer) == 256);
    CHECK(!LIBMATTI_B3D_GpuBuffer_IsClosed(buffer));
    LIBMATTI_B3D_GlBuffer_Free(buffer);

    // Java: VertexFormat offsets/vertexSize for the POSITION|COLOR|UV0 format.
    const LIBMATTI_B3D_VertexFormatElement *elements[] = {
        &LIBMATTI_B3D_VFE_ELEMENT_POSITION,
        &LIBMATTI_B3D_VFE_ELEMENT_COLOR,
        &LIBMATTI_B3D_VFE_ELEMENT_UV0,
    };
    LIBMATTI_B3D_VertexFormat *format = LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
            "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0));
    // The port keeps the mode on the format for the index-count helper.
    format->mode = LIBMATTI_B3D_Mode_QUADS;
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(format) == 12 + 4 + 8);
    CHECK(LIBMATTI_B3D_VertexFormat_GetOffset(format, 0) == 0);
    CHECK(LIBMATTI_B3D_VertexFormat_GetOffset(format, 1) == 12);
    CHECK(LIBMATTI_B3D_VertexFormat_GetOffset(format, 2) == 16);
    CHECK(LIBMATTI_B3D_VertexFormat_GetElement(format, 2)->type == LIBMATTI_B3D_VFE_FLOAT);
    CHECK(LIBMATTI_B3D_VertexFormat_IndexCount(format, 8) == 12); // 2 quads -> 12 indices
    LIBMATTI_B3D_VertexFormat_Free(format);

    // Java: VertexArrayCache create/bind - no context, the bind degrades to the
    // cached path (no VAO generated, no crash).
    LIBMATTI_B3D_VertexArrayCache *cache = LIBMATTI_B3D_VertexArrayCache_Create(0);
    CHECK(cache != NULL);
    LIBMATTI_B3D_VertexFormat *format2 = LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
            "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0));
    LIBMATTI_B3D_VertexArrayCache_BindVertexArray(cache, format2, NULL);
    LIBMATTI_B3D_VertexArrayCache_BindVertexArray(cache, format2, NULL);
    LIBMATTI_B3D_VertexArrayCache_Free(cache);
    LIBMATTI_B3D_VertexFormat_Free(format2);

    // Java: GlStateManager state-cache getters report the initial values.
    CHECK(LIBMATTI_B3D_GlStateManager_GetDepth()->func == 513); // GL_LESS
    CHECK(LIBMATTI_B3D_GlStateManager_GetBlend()->srcRgb == 1); // GL_ONE
    CHECK(LIBMATTI_B3D_GlStateManager_GetColorMask()->red == 1);
    CHECK(LIBMATTI_B3D_GlStateManager_GetActiveTexture() == 0);

    // Java: the descriptions default like RenderSystem declares them.
    CHECK(strcmp(LIBMATTI_B3D_RenderSystem_GetApiDescription(), "Unknown") == 0);
    CHECK(LIBMATTI_B3D_RenderSystem_GetBackendDescription() != NULL);

    // Java: queueFencedTask - a queued task flushes on execute.
    int taskRan = 0;
    LIBMATTI_B3D_RenderSystem_QueueFencedTask(fenced_task, &taskRan);
    LIBMATTI_B3D_RenderSystem_ExecutePendingTasks();
    CHECK(taskRan == 1);

    printf("rendersystem harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
