#include "libmatti/com/mojang/blaze3d/vertex/MeshData.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_B3D_MeshData *LIBMATTI_B3D_MeshData_New(LIBMATTI_B3D_ByteBufferBuilder_Result *vertexBuffer,
                                                 const LIBMATTI_B3D_MeshData_DrawState *drawState)
{
    LIBMATTI_B3D_MeshData *mesh = calloc(1, sizeof(LIBMATTI_B3D_MeshData));
    mesh->vertexBuffer = vertexBuffer;
    mesh->drawState = *drawState;
    return mesh;
}

void LIBMATTI_B3D_MeshData_Free(LIBMATTI_B3D_MeshData *mesh)
{
    if (mesh == NULL)
        return;
    // Java: public void close() - vertex buffer, then the optional index buffer.
    LIBMATTI_B3D_ByteBufferBuilder_Result_Free(mesh->vertexBuffer);
    if (mesh->indexBuffer != NULL)
        LIBMATTI_B3D_ByteBufferBuilder_Result_Free(mesh->indexBuffer);
    free(mesh);
}

unsigned char *LIBMATTI_B3D_MeshData_VertexBuffer(const LIBMATTI_B3D_MeshData *mesh, size_t *outSize)
{
    return LIBMATTI_B3D_ByteBufferBuilder_Result_ByteBuffer(mesh->vertexBuffer, outSize);
}

unsigned char *LIBMATTI_B3D_MeshData_IndexBuffer(const LIBMATTI_B3D_MeshData *mesh, size_t *outSize)
{
    if (mesh->indexBuffer == NULL)
    {
        if (outSize != NULL)
            *outSize = 0;
        return NULL;
    }
    return LIBMATTI_B3D_ByteBufferBuilder_Result_ByteBuffer(mesh->indexBuffer, outSize);
}

const LIBMATTI_B3D_MeshData_DrawState *LIBMATTI_B3D_MeshData_DrawState_(const LIBMATTI_B3D_MeshData *mesh)
{
    return &mesh->drawState;
}

// Java: private static CompactVectorArray unpackQuadCentroids(ByteBuffer, int,
// VertexFormat) - reads the first and third vertex positions of every quad.
static LIBMATTI_B3D_CompactVectorArray *unpack_quad_centroids(const unsigned char *vertexBytes, size_t vertexBytesSize,
                                                              int vertexCount, const LIBMATTI_B3D_VertexFormat *format)
{
    int positionOffset = LIBMATTI_B3D_VertexFormat_GetOffset(format, 0);
    (void) vertexBytesSize;
    const float *vertices = (const float *) vertexBytes;
    // Java: int j = format.getVertexSize() / 4; int k = j * 4; int l = vertexCount / 4;
    int strideFloats = LIBMATTI_B3D_VertexFormat_GetVertexSize(format) / 4;
    int strideBytes = strideFloats * 4;
    int quadCount = vertexCount / 4;
    LIBMATTI_B3D_CompactVectorArray *centroids = LIBMATTI_B3D_CompactVectorArray_New(quadCount);

    for (int i = 0; i < quadCount; i++)
    {
        int first = (i * strideBytes + positionOffset) / 4;
        int third = first + strideFloats * 2;
        // Java: (f + f3) / 2.0F per component over vertices 0 and 2.
        float x = (vertices[first + 0] + vertices[third + 0]) / 2.0f;
        float y = (vertices[first + 1] + vertices[third + 1]) / 2.0f;
        float z = (vertices[first + 2] + vertices[third + 2]) / 2.0f;
        LIBMATTI_B3D_CompactVectorArray_Set(centroids, i, x, y, z);
    }

    return centroids;
}

// Java: SortState.indexWriter - writes one index with the width of indexType
// straight into the reserved region (MemoryUtil.memPutShort/memPutInt).
static void index_writer(unsigned char *base, LIBMATTI_B3D_VertexFormat_IndexType type, int index,
                         long long *cursor)
{
    if (base == NULL)
        return;
    if (type == LIBMATTI_B3D_IndexType_SHORT)
    {
        unsigned short value = (unsigned short) index;
        memcpy(base + *cursor, &value, sizeof(value));
        *cursor += 2;
    }
    else
    {
        unsigned int value = (unsigned int) index;
        memcpy(base + *cursor, &value, sizeof(value));
        *cursor += 4;
    }
}

LIBMATTI_B3D_ByteBufferBuilder_Result *LIBMATTI_B3D_MeshData_SortState_BuildSortedIndexBuffer(
    const LIBMATTI_B3D_MeshData_SortState *sortState, LIBMATTI_B3D_ByteBufferBuilder *allocator,
    LIBMATTI_B3D_VertexSorting_Sort sorting)
{
    // Java: int[] aint = p_342363_.sort(this.centroids);
    size_t orderCount = 0;
    int *order = sorting(sortState->centroids, &orderCount);
    int typeBytes = sortState->indexType == LIBMATTI_B3D_IndexType_INT ? 4 : 2;
    // Java: long i = p_342323_.reserve(aint.length * 6 * this.indexType.bytes);
    // The returned pointer is the base the memPuts write through.
    unsigned char *base = LIBMATTI_B3D_ByteBufferBuilder_Reserve(allocator, (int) (orderCount * 6 * typeBytes));

    long long cursor = 0;
    for (size_t i = 0; i < orderCount; i++)
    {
        int quad = order[i];
        // Java: quad*4+0, +1, +2, +2, +3, +0.
        index_writer(base, sortState->indexType, quad * 4 + 0, &cursor);
        index_writer(base, sortState->indexType, quad * 4 + 1, &cursor);
        index_writer(base, sortState->indexType, quad * 4 + 2, &cursor);
        index_writer(base, sortState->indexType, quad * 4 + 2, &cursor);
        index_writer(base, sortState->indexType, quad * 4 + 3, &cursor);
        index_writer(base, sortState->indexType, quad * 4 + 0, &cursor);
    }

    free(order);
    return LIBMATTI_B3D_ByteBufferBuilder_Build(allocator);
}

LIBMATTI_B3D_MeshData_SortState *LIBMATTI_B3D_MeshData_SortQuads(
    LIBMATTI_B3D_MeshData *mesh, LIBMATTI_B3D_ByteBufferBuilder *allocator,
    LIBMATTI_B3D_VertexSorting_Sort sorting)
{
    // Java: if (this.drawState.mode() != VertexFormat.Mode.QUADS) return null;
    if (mesh->drawState.mode != LIBMATTI_B3D_Mode_QUADS)
        return NULL;

    size_t vertexSize = 0;
    unsigned char *vertexBytes = LIBMATTI_B3D_MeshData_VertexBuffer(mesh, &vertexSize);
    LIBMATTI_B3D_CompactVectorArray *centroids =
        unpack_quad_centroids(vertexBytes, vertexSize, mesh->drawState.vertexCount, mesh->drawState.format);

    LIBMATTI_B3D_MeshData_SortState *sortState = malloc(sizeof(LIBMATTI_B3D_MeshData_SortState));
    sortState->centroids = centroids;
    sortState->indexType = mesh->drawState.indexType;

    mesh->indexBuffer = LIBMATTI_B3D_MeshData_SortState_BuildSortedIndexBuffer(sortState, allocator, sorting);
    return sortState;
}

void LIBMATTI_B3D_MeshData_SortState_Free(LIBMATTI_B3D_MeshData_SortState *sortState)
{
    if (sortState == NULL)
        return;
    LIBMATTI_B3D_CompactVectorArray_Free(sortState->centroids);
    free(sortState);
}
