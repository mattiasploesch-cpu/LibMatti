// Port of com.mojang.blaze3d.vertex.MeshData - one built mesh: vertex bytes,
// optional sorted index bytes and the DrawState record.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_MESHDATA_H
#define MATTICRAFT_BLAZE3D_VERTEX_MESHDATA_H

#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/CompactVectorArray.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexSorting.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public record DrawState(VertexFormat format, int vertexCount,
//       int indexCount, VertexFormat.Mode mode, VertexFormat.IndexType indexType)
typedef struct LIBMATTI_B3D_MeshData_DrawState
{
    const LIBMATTI_B3D_VertexFormat *format;
    int vertexCount;
    int indexCount;
    LIBMATTI_B3D_VertexFormat_Mode mode;
    LIBMATTI_B3D_VertexFormat_IndexType indexType;
} LIBMATTI_B3D_MeshData_DrawState;

// Java: public record SortState(CompactVectorArray centroids, VertexFormat.IndexType indexType)
typedef struct LIBMATTI_B3D_MeshData_SortState
{
    LIBMATTI_B3D_CompactVectorArray *centroids;
    LIBMATTI_B3D_VertexFormat_IndexType indexType;
} LIBMATTI_B3D_MeshData_SortState;

// Java: public class MeshData implements AutoCloseable
typedef struct LIBMATTI_B3D_MeshData
{
    LIBMATTI_B3D_ByteBufferBuilder_Result *vertexBuffer;
    LIBMATTI_B3D_ByteBufferBuilder_Result *indexBuffer;  // Java: @Nullable
    LIBMATTI_B3D_MeshData_DrawState drawState;
} LIBMATTI_B3D_MeshData;

// Java: public MeshData(Result vertexBuffer, DrawState drawState)
LIBMATTI_B3D_MeshData *LIBMATTI_B3D_MeshData_New(LIBMATTI_B3D_ByteBufferBuilder_Result *vertexBuffer,
                                                 const LIBMATTI_B3D_MeshData_DrawState *drawState);
void LIBMATTI_B3D_MeshData_Free(LIBMATTI_B3D_MeshData *mesh);

// Java: public ByteBuffer vertexBuffer()
unsigned char *LIBMATTI_B3D_MeshData_VertexBuffer(const LIBMATTI_B3D_MeshData *mesh, size_t *outSize);
// Java: public @Nullable ByteBuffer indexBuffer()
unsigned char *LIBMATTI_B3D_MeshData_IndexBuffer(const LIBMATTI_B3D_MeshData *mesh, size_t *outSize);
// Java: public DrawState drawState()
const LIBMATTI_B3D_MeshData_DrawState *LIBMATTI_B3D_MeshData_DrawState_(const LIBMATTI_B3D_MeshData *mesh);

// Java: public @Nullable SortState sortQuads(ByteBufferBuilder, VertexSorting) -
// NULL for non-QUADS modes; the built index buffer is stored on the mesh.
LIBMATTI_B3D_MeshData_SortState *LIBMATTI_B3D_MeshData_SortQuads(
    LIBMATTI_B3D_MeshData *mesh, LIBMATTI_B3D_ByteBufferBuilder *allocator,
    LIBMATTI_B3D_VertexSorting_Sort sorting);

// Java: SortState.buildSortedIndexBuffer - exposed for the harness.
LIBMATTI_B3D_ByteBufferBuilder_Result *LIBMATTI_B3D_MeshData_SortState_BuildSortedIndexBuffer(
    const LIBMATTI_B3D_MeshData_SortState *sortState, LIBMATTI_B3D_ByteBufferBuilder *allocator,
    LIBMATTI_B3D_VertexSorting_Sort sorting);
void LIBMATTI_B3D_MeshData_SortState_Free(LIBMATTI_B3D_MeshData_SortState *sortState);

#ifdef __cplusplus
}
#endif

#endif
