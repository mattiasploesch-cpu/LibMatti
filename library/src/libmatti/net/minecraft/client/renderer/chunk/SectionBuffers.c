// Port of net.minecraft.client.renderer.chunk.SectionBuffers (implementation).

#include "libmatti/net/minecraft/client/renderer/chunk/SectionBuffers.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdlib.h>

LIBMATTI_MC_SectionBuffers *LIBMATTI_MC_SectionBuffers_New(LIBMATTI_B3D_GpuBuffer *vertexBuffer,
                                                           LIBMATTI_B3D_GpuBuffer *indexBuffer,
                                                           int indexCount,
                                                           LIBMATTI_B3D_VertexFormat_IndexType indexType)
{
    LIBMATTI_MC_SectionBuffers *buffers = calloc(1, sizeof(LIBMATTI_MC_SectionBuffers));
    buffers->vertexBuffer = vertexBuffer;
    buffers->indexBuffer = indexBuffer;
    buffers->indexCount = indexCount;
    buffers->indexType = indexType;
    return buffers;
}

void LIBMATTI_MC_SectionBuffers_Free(LIBMATTI_MC_SectionBuffers *buffers)
{
    if (buffers == NULL)
        return;
    // Java: this.vertexBuffer.close(); if (this.indexBuffer != null) ...
    if (buffers->vertexBuffer != NULL)
        LIBMATTI_B3D_GpuBuffer_Free(buffers->vertexBuffer);
    if (buffers->indexBuffer != NULL)
        LIBMATTI_B3D_GpuBuffer_Free(buffers->indexBuffer);
    free(buffers);
}

LIBMATTI_B3D_GpuBuffer *LIBMATTI_MC_SectionBuffers_GetVertexBuffer(const LIBMATTI_MC_SectionBuffers *buffers)
{
    return buffers->vertexBuffer;
}

LIBMATTI_B3D_GpuBuffer *LIBMATTI_MC_SectionBuffers_GetIndexBuffer(const LIBMATTI_MC_SectionBuffers *buffers)
{
    return buffers->indexBuffer;
}

void LIBMATTI_MC_SectionBuffers_SetIndexBuffer(LIBMATTI_MC_SectionBuffers *buffers, LIBMATTI_B3D_GpuBuffer *indexBuffer)
{
    buffers->indexBuffer = indexBuffer;
}

int LIBMATTI_MC_SectionBuffers_GetIndexCount(const LIBMATTI_MC_SectionBuffers *buffers)
{
    return buffers->indexCount;
}

void LIBMATTI_MC_SectionBuffers_SetIndexCount(LIBMATTI_MC_SectionBuffers *buffers, int indexCount)
{
    buffers->indexCount = indexCount;
}

LIBMATTI_B3D_VertexFormat_IndexType LIBMATTI_MC_SectionBuffers_GetIndexType(const LIBMATTI_MC_SectionBuffers *buffers)
{
    return buffers->indexType;
}

// Java: SectionRenderDispatcher.uploadSection - BindBuffer + BufferData per
// store, the index store only when the layer carries index data.
void LIBMATTI_MC_SectionBuffers_Upload(LIBMATTI_MC_SectionBuffers *buffers,
                                       const void *vertexData, long vertexSize,
                                       const void *indexData, long indexSize)
{
    if (buffers == NULL || vertexData == NULL || vertexSize <= 0)
        return;
    if (buffers->vertexBuffer != NULL)
    {
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, buffers->vertexBuffer->handle);
        LIBMATTI_B3D_GlStateManager_BufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, vertexData, vertexSize,
                                               LIBMATTI_GL_GL_STATIC_DRAW);
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
        buffers->vertexBuffer->size = vertexSize;
    }
    if (indexData != NULL && indexSize > 0 && buffers->indexBuffer != NULL)
    {
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, buffers->indexBuffer->handle);
        LIBMATTI_B3D_GlStateManager_BufferData(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, indexData, indexSize,
                                               LIBMATTI_GL_GL_STATIC_DRAW);
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
        buffers->indexBuffer->size = indexSize;
    }
}
void LIBMATTI_MC_SectionBuffers_SetIndexType(LIBMATTI_MC_SectionBuffers *buffers, LIBMATTI_B3D_VertexFormat_IndexType indexType)
{
    buffers->indexType = indexType;
}
