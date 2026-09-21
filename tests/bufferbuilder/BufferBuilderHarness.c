// BufferBuilder harness: drives the vertex pipeline end to end - the builder
// byte layout for POSITION_COLOR, the element-skipping behaviour for formats
// without the element, the fast path for NEW_ENTITY, quad sorting through
// MeshData.sortQuads and the Tesselator singleton.

#include "libmatti/com/mojang/blaze3d/vertex/BufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/CompactVectorArray.h"
#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/Tesselator.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexSorting.h"

#include <stdio.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                            \
    do                                                         \
    {                                                          \
        checks++;                                              \
        if (!(cond))                                           \
        {                                                      \
            failures++;                                        \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                      \
    } while (0)

static float read_float(const unsigned char *base, size_t offset)
{
    float value;
    memcpy(&value, base + offset, sizeof(float));
    return value;
}

int main(void)
{
    // ---- ByteBufferBuilder reserve/build/discard ---------------------------
    LIBMATTI_B3D_ByteBufferBuilder *allocator = LIBMATTI_B3D_ByteBufferBuilder_New(16);
    CHECK(allocator != NULL);
    unsigned char *slot = LIBMATTI_B3D_ByteBufferBuilder_Reserve(allocator, 12);
    CHECK(slot != NULL);
    memset(slot, 0xAB, 12);
    LIBMATTI_B3D_ByteBufferBuilder_Result *result = LIBMATTI_B3D_ByteBufferBuilder_Build(allocator);
    CHECK(result != NULL);
    size_t size = 0;
    unsigned char *bytes = LIBMATTI_B3D_ByteBufferBuilder_Result_ByteBuffer(result, &size);
    CHECK(size == 12 && bytes[0] == 0xAB && bytes[11] == 0xAB);
    // Java: build() with nothing new written returns null.
    CHECK(LIBMATTI_B3D_ByteBufferBuilder_Build(allocator) == NULL);
    LIBMATTI_B3D_ByteBufferBuilder_Result_Free(result);
    LIBMATTI_B3D_ByteBufferBuilder_Free(allocator);

    // ---- POSITION_COLOR quad layout ----------------------------------------
    const LIBMATTI_B3D_VertexFormat *positionColor = LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR();
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(positionColor) == 16);
    CHECK(LIBMATTI_B3D_VertexFormat_Contains(positionColor, &LIBMATTI_B3D_VFE_ELEMENT_POSITION));
    CHECK(LIBMATTI_B3D_VertexFormat_Contains(positionColor, &LIBMATTI_B3D_VFE_ELEMENT_COLOR));
    CHECK(!LIBMATTI_B3D_VertexFormat_Contains(positionColor, &LIBMATTI_B3D_VFE_ELEMENT_UV0));
    const int *offsets = LIBMATTI_B3D_VertexFormat_GetOffsetsByElement(positionColor);
    CHECK(offsets[0] == 0);  // POSITION (Java registry id 0)
    CHECK(offsets[1] == 12); // COLOR (id 1)

    allocator = LIBMATTI_B3D_ByteBufferBuilder_New(256);
    LIBMATTI_B3D_BufferBuilder *builder =
        LIBMATTI_B3D_BufferBuilder_New(allocator, LIBMATTI_B3D_Mode_QUADS, positionColor);
    // One quad with two colors; setUv is a documented no-op (no UV0 element).
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 0.0f, 0.0f, 0.0f);
    LIBMATTI_B3D_BufferBuilder_SetColorRGBA(builder, 255, 0, 0, 255);
    LIBMATTI_B3D_BufferBuilder_SetUv(builder, 1.0f, 2.0f); // skipped
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 1.0f, 0.0f, 0.0f);
    LIBMATTI_B3D_BufferBuilder_SetColor(builder, 0xFF00FF00); // ARGB green
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 1.0f, 1.0f, 0.0f);
    LIBMATTI_B3D_BufferBuilder_SetColorRGBA(builder, 0, 0, 255, 255);
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 0.0f, 1.0f, 0.0f);
    LIBMATTI_B3D_BufferBuilder_SetColorRGBA(builder, 255, 255, 255, 128);

    LIBMATTI_B3D_MeshData *mesh = LIBMATTI_B3D_BufferBuilder_Build(builder);
    CHECK(mesh != NULL);
    const LIBMATTI_B3D_MeshData_DrawState *drawState = LIBMATTI_B3D_MeshData_DrawState_(mesh);
    CHECK(drawState->vertexCount == 4);
    CHECK(drawState->indexCount == 6); // QUADS: 4/4*6
    CHECK(drawState->indexType == LIBMATTI_B3D_IndexType_SHORT);
    CHECK(drawState->mode == LIBMATTI_B3D_Mode_QUADS);

    bytes = LIBMATTI_B3D_MeshData_VertexBuffer(mesh, &size);
    CHECK(size == 4 * 16);
    CHECK(read_float(bytes, 0) == 0.0f);
    CHECK(read_float(bytes, 8) == 0.0f);
    // Java: putRgba converts ARGB green to ABGR, stored little endian -> the
    // memory layout is R,G,B,A per byte.
    CHECK(bytes[16 + 12] == 0);   // r
    CHECK(bytes[16 + 13] == 255); // g
    CHECK(bytes[16 + 14] == 0);   // b
    CHECK(bytes[16 + 15] == 255); // a
    // Vertex 0 color: r=255 g=0 b=0 a=255 (RGBA order in memory).
    CHECK(bytes[12] == 255 && bytes[13] == 0 && bytes[14] == 0 && bytes[15] == 255);

    // ---- MeshData.sortQuads -------------------------------------------------
    allocator = LIBMATTI_B3D_ByteBufferBuilder_New(256);
    LIBMATTI_B3D_MeshData_SortState *sortState =
        LIBMATTI_B3D_MeshData_SortQuads(mesh, allocator, LIBMATTI_B3D_VertexSorting_DistanceToOrigin());
    CHECK(sortState != NULL);
    size_t indexSize = 0;
    const unsigned char *indexBytes = LIBMATTI_B3D_MeshData_IndexBuffer(mesh, &indexSize);
    CHECK(indexBytes != NULL && indexSize == 6 * 2);
    // Centroid of the quad sits at (0.5, 0.5, 0).
    CHECK(LIBMATTI_B3D_CompactVectorArray_Size(sortState->centroids) == 1);
    CHECK(LIBMATTI_B3D_CompactVectorArray_GetX(sortState->centroids, 0) == 0.5f);
    CHECK(LIBMATTI_B3D_CompactVectorArray_GetZ(sortState->centroids, 0) == 0.0f);
    // The sorted index buffer holds quad 0 six times as shorts.
    unsigned short i0, i1, i2, i3, i4, i5;
    memcpy(&i0, indexBytes + 0, 2); memcpy(&i1, indexBytes + 2, 2); memcpy(&i2, indexBytes + 4, 2);
    memcpy(&i3, indexBytes + 6, 2); memcpy(&i4, indexBytes + 8, 2); memcpy(&i5, indexBytes + 10, 2);
    CHECK(i0 == 0 && i1 == 1 && i2 == 2 && i3 == 2 && i4 == 3 && i5 == 0);
    LIBMATTI_B3D_MeshData_SortState_Free(sortState);
    LIBMATTI_B3D_MeshData_Free(mesh);
    LIBMATTI_B3D_BufferBuilder_Free(builder);
    LIBMATTI_B3D_ByteBufferBuilder_Free(allocator);

    // ---- sortQuads on a non-QUADS mesh returns NULL --------------------------
    allocator = LIBMATTI_B3D_ByteBufferBuilder_New(256);
    builder = LIBMATTI_B3D_BufferBuilder_New(allocator, LIBMATTI_B3D_Mode_TRIANGLES,
                                              LIBMATTI_B3D_DefaultVertexFormat_POSITION());
    for (int i = 0; i < 3; i++)
        LIBMATTI_B3D_BufferBuilder_AddVertex(builder, (float) i, 0.0f, 0.0f);
    mesh = LIBMATTI_B3D_BufferBuilder_Build(builder);
    CHECK(mesh != NULL && LIBMATTI_B3D_MeshData_DrawState_(mesh)->vertexCount == 3);
    CHECK(LIBMATTI_B3D_MeshData_SortQuads(mesh, allocator, LIBMATTI_B3D_VertexSorting_DistanceToOrigin()) == NULL);
    LIBMATTI_B3D_MeshData_Free(mesh);
    LIBMATTI_B3D_BufferBuilder_Free(builder);
    LIBMATTI_B3D_ByteBufferBuilder_Free(allocator);

    // ---- NEW_ENTITY fast path ------------------------------------------------
    const LIBMATTI_B3D_VertexFormat *newEntity = LIBMATTI_B3D_DefaultVertexFormat_NEW_ENTITY();
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(newEntity) == 36);
    allocator = LIBMATTI_B3D_ByteBufferBuilder_New(256);
    builder = LIBMATTI_B3D_BufferBuilder_New(allocator, LIBMATTI_B3D_Mode_QUADS, newEntity);
    LIBMATTI_B3D_BufferBuilder_AddVertexFull(builder, 1.0f, 2.0f, 3.0f, 0x80FF0000, 0.5f, 0.25f,
                                             0x00030003, 0x00C800C8, 0.0f, 1.0f, 0.0f);
    mesh = LIBMATTI_B3D_BufferBuilder_Build(builder);
    CHECK(mesh != NULL);
    bytes = LIBMATTI_B3D_MeshData_VertexBuffer(mesh, &size);
    CHECK(size == 36);
    CHECK(read_float(bytes, 0) == 1.0f && read_float(bytes, 4) == 2.0f && read_float(bytes, 8) == 3.0f);
    CHECK(read_float(bytes, 16) == 0.5f && read_float(bytes, 20) == 0.25f);
    // normal y = 1.0 -> byte 127: ny sits at 32 + 1 (normal starts after
    // 12 pos + 4 color + 8 uv0 + 4 uv1 + 4 uv2 = 32).
    CHECK(bytes[33] == 127);
    LIBMATTI_B3D_MeshData_Free(mesh);
    LIBMATTI_B3D_BufferBuilder_Free(builder);
    LIBMATTI_B3D_ByteBufferBuilder_Free(allocator);

    // ---- LINES mode duplicates the last vertex -------------------------------
    allocator = LIBMATTI_B3D_ByteBufferBuilder_New(256);
    builder = LIBMATTI_B3D_BufferBuilder_New(allocator, LIBMATTI_B3D_Mode_LINES,
                                              LIBMATTI_B3D_DefaultVertexFormat_POSITION());
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 0.0f, 0.0f, 0.0f);
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 1.0f, 0.0f, 0.0f);
    mesh = LIBMATTI_B3D_BufferBuilder_Build(builder);
    CHECK(mesh != NULL);
    // Java: endLastVertex duplicates every written vertex in LINES mode
    // (A A B B) - the wide-line quad the lines shader expands.
    CHECK(LIBMATTI_B3D_MeshData_DrawState_(mesh)->vertexCount == 4);
    CHECK(LIBMATTI_B3D_MeshData_DrawState_(mesh)->indexCount == 6); // 4/4*6 like QUADS
    LIBMATTI_B3D_MeshData_Free(mesh);
    LIBMATTI_B3D_BufferBuilder_Free(builder);
    LIBMATTI_B3D_ByteBufferBuilder_Free(allocator);

    // ---- Tesselator singleton -------------------------------------------------
    CHECK(LIBMATTI_B3D_Tesselator_Init() == 1);
    CHECK(LIBMATTI_B3D_Tesselator_Init() == 0); // Java: already initialized
    void *tesselator = LIBMATTI_B3D_Tesselator_GetInstance();
    CHECK(tesselator != NULL);
    builder = LIBMATTI_B3D_Tesselator_Begin(tesselator, LIBMATTI_B3D_Mode_QUADS,
                                            LIBMATTI_B3D_DefaultVertexFormat_POSITION());
    LIBMATTI_B3D_BufferBuilder_AddVertex(builder, 0.0f, 0.0f, 0.0f);
    mesh = LIBMATTI_B3D_BufferBuilder_Build(builder);
    CHECK(mesh != NULL);
    LIBMATTI_B3D_MeshData_Free(mesh);
    LIBMATTI_B3D_BufferBuilder_Free(builder);
    LIBMATTI_B3D_Tesselator_Clear(tesselator);
    LIBMATTI_B3D_Tesselator_Free(tesselator);

    // ---- other DefaultVertexFormat shapes ------------------------------------
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(LIBMATTI_B3D_DefaultVertexFormat_BLOCK()) == 32);
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(LIBMATTI_B3D_DefaultVertexFormat_POSITION()) == 12);
    CHECK(LIBMATTI_B3D_VertexFormat_GetVertexSize(LIBMATTI_B3D_DefaultVertexFormat_POSITION_TEX()) == 20);
    CHECK(LIBMATTI_B3D_VertexFormat_GetElementName(LIBMATTI_B3D_DefaultVertexFormat_BLOCK(),
                                                   &LIBMATTI_B3D_VFE_ELEMENT_UV2) != NULL &&
          strcmp(LIBMATTI_B3D_VertexFormat_GetElementName(LIBMATTI_B3D_DefaultVertexFormat_BLOCK(),
                                                          &LIBMATTI_B3D_VFE_ELEMENT_UV2),
                 "UV2") == 0);

    printf("bufferbuilder harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
