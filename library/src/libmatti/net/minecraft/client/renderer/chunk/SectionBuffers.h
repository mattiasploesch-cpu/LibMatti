// Port of net.minecraft.client.renderer.chunk.SectionBuffers - the uploaded
// GPU buffer pair of one section layer.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONBUFFERS_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONBUFFERS_H

#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public final class SectionBuffers implements AutoCloseable
typedef struct LIBMATTI_MC_SectionBuffers
{
    // Java: private GpuBuffer vertexBuffer / @Nullable indexBuffer
    LIBMATTI_B3D_GpuBuffer *vertexBuffer;
    LIBMATTI_B3D_GpuBuffer *indexBuffer;
    // Java: private int indexCount / VertexFormat.IndexType indexType
    int indexCount;
    LIBMATTI_B3D_VertexFormat_IndexType indexType;
} LIBMATTI_MC_SectionBuffers;

// Java: public SectionBuffers(GpuBuffer, @Nullable GpuBuffer, int, IndexType)
LIBMATTI_MC_SectionBuffers *LIBMATTI_MC_SectionBuffers_New(LIBMATTI_B3D_GpuBuffer *vertexBuffer,
                                                           LIBMATTI_B3D_GpuBuffer *indexBuffer,
                                                           int indexCount,
                                                           LIBMATTI_B3D_VertexFormat_IndexType indexType);
// Java: public void close() - frees both buffers.
void LIBMATTI_MC_SectionBuffers_Free(LIBMATTI_MC_SectionBuffers *buffers);

// The getters/setters (Java: getVertexBuffer, setIndexBuffer, getIndexCount, ...).
LIBMATTI_B3D_GpuBuffer *LIBMATTI_MC_SectionBuffers_GetVertexBuffer(const LIBMATTI_MC_SectionBuffers *buffers);
LIBMATTI_B3D_GpuBuffer *LIBMATTI_MC_SectionBuffers_GetIndexBuffer(const LIBMATTI_MC_SectionBuffers *buffers);
void LIBMATTI_MC_SectionBuffers_SetIndexBuffer(LIBMATTI_MC_SectionBuffers *buffers, LIBMATTI_B3D_GpuBuffer *indexBuffer);
int LIBMATTI_MC_SectionBuffers_GetIndexCount(const LIBMATTI_MC_SectionBuffers *buffers);
void LIBMATTI_MC_SectionBuffers_SetIndexCount(LIBMATTI_MC_SectionBuffers *buffers, int indexCount);
LIBMATTI_B3D_VertexFormat_IndexType LIBMATTI_MC_SectionBuffers_GetIndexType(const LIBMATTI_MC_SectionBuffers *buffers);
void LIBMATTI_MC_SectionBuffers_SetIndexType(LIBMATTI_MC_SectionBuffers *buffers, LIBMATTI_B3D_VertexFormat_IndexType indexType);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONBUFFERS_H
