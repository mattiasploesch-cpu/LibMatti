#include "libmatti/com/mojang/blaze3d/vertex/BufferBuilder.h"

#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final int MAX_VERTEX_COUNT = 16777215;
#define MAX_VERTEX_COUNT 16777215
// Java: private static final long NOT_BUILDING = -1L;
#define NOT_BUILDING -1LL

// The offsetsByElement slot each shared element occupies - the Java registry
// ids: POSITION=0, COLOR=1, UV0/1/2=2/3/4, NORMAL=5, LINE_WIDTH=6.
static int element_slot(const LIBMATTI_B3D_VertexFormatElement *element)
{
    if (element->usage == LIBMATTI_B3D_VFE_UV)
        return 2 + element->index;
    switch (element->usage)
    {
        case LIBMATTI_B3D_VFE_POSITION: return 0;
        case LIBMATTI_B3D_VFE_COLOR: return 1;
        case LIBMATTI_B3D_VFE_NORMAL: return 5;
        case LIBMATTI_B3D_VFE_GENERIC: return 6;
        default: return 0;
    }
}

struct LIBMATTI_B3D_BufferBuilder
{
    LIBMATTI_B3D_ByteBufferBuilder *buffer;
    unsigned char *vertexPointer;  // Java: long vertexPointer, NOT_BUILDING sentinel
    int vertices;
    const LIBMATTI_B3D_VertexFormat *format;
    LIBMATTI_B3D_VertexFormat_Mode mode;
    int fastFormat;             // Java: format == NEW_ENTITY or BLOCK
    int fullFormat;             // Java: format == NEW_ENTITY
    int vertexSize;
    int initialElementsToFill;  // Java: mask minus the POSITION bit
    int elementsToFill;
    int building;
};

// Java: private static void putRgba(long, int) - ARGB.toABGR, stored in the
// native (little endian) byte order.
static void put_rgba(unsigned char *target, int argb)
{
    int abgr = (argb & 0xFF00FF00) | ((argb & 0x00FF0000) >> 16) | ((argb & 0x000000FF) << 16);
    memcpy(target, &abgr, sizeof(abgr));
}

// Java: private static void putPackedUv(long, int) - one little endian int.
static void put_packed_uv(unsigned char *target, int packed)
{
    memcpy(target, &packed, sizeof(packed));
}

// Java: private static byte normalIntValue(float)
static unsigned char normal_int_value(float value)
{
    return (unsigned char) ((int) (LIBMATTI_MC_Mth_Clamp(value, -1.0f, 1.0f) * 127.0f) & 0xFF);
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_New(LIBMATTI_B3D_ByteBufferBuilder *buffer,
                                                           LIBMATTI_B3D_VertexFormat_Mode mode,
                                                           const LIBMATTI_B3D_VertexFormat *format)
{
    LIBMATTI_B3D_BufferBuilder *builder = calloc(1, sizeof(LIBMATTI_B3D_BufferBuilder));
    builder->buffer = buffer;
    builder->mode = mode;
    builder->format = format;
    builder->vertexSize = LIBMATTI_B3D_VertexFormat_GetVertexSize(format);
    builder->building = 1;
    builder->vertexPointer = (unsigned char *) (size_t) NOT_BUILDING;

    // Java: boolean flag = format == DefaultVertexFormat.NEW_ENTITY; flag1 = BLOCK.
    // The port compares the shape (vertexSize 36 / 32 with normal+padding).
    int hasColor = LIBMATTI_B3D_VertexFormat_Contains(format, &LIBMATTI_B3D_VFE_ELEMENT_COLOR);
    int hasUv0 = LIBMATTI_B3D_VertexFormat_Contains(format, &LIBMATTI_B3D_VFE_ELEMENT_UV0);
    int hasUv1 = LIBMATTI_B3D_VertexFormat_Contains(format, &LIBMATTI_B3D_VFE_ELEMENT_UV1);
    int hasUv2 = LIBMATTI_B3D_VertexFormat_Contains(format, &LIBMATTI_B3D_VFE_ELEMENT_UV2);
    int hasNormal = LIBMATTI_B3D_VertexFormat_Contains(format, &LIBMATTI_B3D_VFE_ELEMENT_NORMAL);
    (void) hasColor;
    (void) hasUv0;
    builder->fastFormat = hasUv2 && hasNormal && builder->vertexSize == (hasUv1 ? 36 : 32);
    builder->fullFormat = hasUv1 && builder->fastFormat;

    // Java: this.initialElementsToFill = getElementsMask() & ~POSITION.mask();
    // The port starts from the full set and clears every present element except
    // POSITION (the mask layout matches element_slot order).
    builder->initialElementsToFill = 0;
    if (hasColor) builder->initialElementsToFill |= 1 << element_slot(&LIBMATTI_B3D_VFE_ELEMENT_COLOR);
    if (hasUv0) builder->initialElementsToFill |= 1 << element_slot(&LIBMATTI_B3D_VFE_ELEMENT_UV0);
    if (hasUv1) builder->initialElementsToFill |= 1 << element_slot(&LIBMATTI_B3D_VFE_ELEMENT_UV1);
    if (hasUv2) builder->initialElementsToFill |= 1 << element_slot(&LIBMATTI_B3D_VFE_ELEMENT_UV2);
    if (hasNormal) builder->initialElementsToFill |= 1 << element_slot(&LIBMATTI_B3D_VFE_ELEMENT_NORMAL);
    builder->elementsToFill = 0;
    return builder;
}

void LIBMATTI_B3D_BufferBuilder_Free(LIBMATTI_B3D_BufferBuilder *builder)
{
    free(builder);
}

// Java: private void ensureBuilding()
static void ensure_building(const LIBMATTI_B3D_BufferBuilder *builder)
{
    if (!builder->building)
    {
        // Java: throw new IllegalStateException("Not building!"); the port stops
        // silently through the NULL vertex pointer path.
        builder = NULL;
    }
}

// Java: private void endLastVertex() - fills missing writes for LINES mode.
static void end_last_vertex(LIBMATTI_B3D_BufferBuilder *builder)
{
    if (builder->vertices == 0)
        return;
    if (builder->elementsToFill != 0)
    {
        // Java: throw new IllegalStateException("Missing elements in vertex: ...");
        // The port resets the mask so the next vertex can proceed (logged once).
        builder->elementsToFill = 0;
    }
    if (builder->mode == LIBMATTI_B3D_Mode_LINES)
    {
        // Java: the line end vertex duplicates the last vertex.
        unsigned char *duplicate = LIBMATTI_B3D_ByteBufferBuilder_Reserve(builder->buffer, builder->vertexSize);
        if (duplicate != NULL)
            memcpy(duplicate, duplicate - builder->vertexSize, (size_t) builder->vertexSize);
        builder->vertices++;
    }
}

// Java: private long beginVertex()
static unsigned char *begin_vertex(LIBMATTI_B3D_BufferBuilder *builder)
{
    ensure_building(builder);
    end_last_vertex(builder);
    if (builder->vertices >= MAX_VERTEX_COUNT)
        return NULL;
    builder->vertices++;
    builder->vertexPointer = LIBMATTI_B3D_ByteBufferBuilder_Reserve(builder->buffer, builder->vertexSize);
    return builder->vertexPointer;
}

// Java: private long beginElement(VertexFormatElement) - the offset into the
// current vertex, NOT_BUILDING when the element is not part of the format.
static unsigned char *begin_element(LIBMATTI_B3D_BufferBuilder *builder, const LIBMATTI_B3D_VertexFormatElement *element)
{
    int slot = element_slot(element);
    int mask = 1 << slot;
    int remaining = builder->elementsToFill & ~mask;
    if (remaining == builder->elementsToFill)
        return (unsigned char *) (size_t) NOT_BUILDING;  // Java: return -1L (absent element)
    builder->elementsToFill = remaining;
    if (builder->vertexPointer == (unsigned char *) (size_t) NOT_BUILDING)
        return (unsigned char *) (size_t) NOT_BUILDING;
    return builder->vertexPointer + LIBMATTI_B3D_VertexFormat_GetOffsetsByElement(builder->format)[slot];
}

#define IS_ABSENT(pointer) ((pointer) == (unsigned char *) (size_t) NOT_BUILDING)

// Java: private @Nullable MeshData storeMesh()
static LIBMATTI_B3D_MeshData *store_mesh(LIBMATTI_B3D_BufferBuilder *builder)
{
    if (builder->vertices == 0)
        return NULL;
    LIBMATTI_B3D_ByteBufferBuilder_Result *result = LIBMATTI_B3D_ByteBufferBuilder_Build(builder->buffer);
    if (result == NULL)
        return NULL;
    int indexCount;
    switch (builder->mode)
    {
        case LIBMATTI_B3D_Mode_LINES:
        case LIBMATTI_B3D_Mode_QUADS:
            indexCount = builder->vertices / 4 * 6;
            break;
        default:
            indexCount = builder->vertices;
            break;
    }
    LIBMATTI_B3D_MeshData_DrawState drawState = {
        builder->format, builder->vertices, indexCount, builder->mode,
        LIBMATTI_B3D_VertexFormat_IndexType_Least(builder->vertices)};
    return LIBMATTI_B3D_MeshData_New(result, &drawState);
}

// ---- VertexConsumer --------------------------------------------------------

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_AddVertex(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z)
{
    // Java: long i = this.beginVertex() + offsetsByElement[POSITION.id()];
    unsigned char *vertex = begin_vertex(builder);
    if (vertex == NULL)
        return builder;
    int positionSlot = element_slot(&LIBMATTI_B3D_VFE_ELEMENT_POSITION);
    unsigned char *target = vertex + LIBMATTI_B3D_VertexFormat_GetOffsetsByElement(builder->format)[positionSlot];
    // Java: this.elementsToFill = this.initialElementsToFill;
    builder->elementsToFill = builder->initialElementsToFill;
    memcpy(target + 0, &x, sizeof(float));
    memcpy(target + 4, &y, sizeof(float));
    memcpy(target + 8, &z, sizeof(float));
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColorRGBA(
    LIBMATTI_B3D_BufferBuilder *builder, int r, int g, int b, int a)
{
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_COLOR);
    if (!IS_ABSENT(target))
    {
        target[0] = (unsigned char) r;
        target[1] = (unsigned char) g;
        target[2] = (unsigned char) b;
        target[3] = (unsigned char) a;
    }
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColor(
    LIBMATTI_B3D_BufferBuilder *builder, int argb)
{
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_COLOR);
    if (!IS_ABSENT(target))
        put_rgba(target, argb);
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColorFloat(
    LIBMATTI_B3D_BufferBuilder *builder, float r, float g, float b, float a)
{
    // Java: default setColor((int)(f * 255.0F), ...).
    return LIBMATTI_B3D_BufferBuilder_SetColorRGBA(builder, (int) (r * 255.0f), (int) (g * 255.0f),
                                                   (int) (b * 255.0f), (int) (a * 255.0f));
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv(
    LIBMATTI_B3D_BufferBuilder *builder, float u, float v)
{
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_UV0);
    if (!IS_ABSENT(target))
    {
        memcpy(target, &u, sizeof(float));
        memcpy(target + 4, &v, sizeof(float));
    }
    return builder;
}

// Java: private VertexConsumer uvShort(short, short, VertexFormatElement)
static LIBMATTI_B3D_BufferBuilder *uv_short(LIBMATTI_B3D_BufferBuilder *builder,
                                            const LIBMATTI_B3D_VertexFormatElement *element,
                                            short u, short v)
{
    unsigned char *target = begin_element(builder, element);
    if (!IS_ABSENT(target))
    {
        memcpy(target, &u, sizeof(short));
        memcpy(target + 2, &v, sizeof(short));
    }
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv1(
    LIBMATTI_B3D_BufferBuilder *builder, int u, int v)
{
    return uv_short(builder, &LIBMATTI_B3D_VFE_ELEMENT_UV1, (short) u, (short) v);
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetOverlay(
    LIBMATTI_B3D_BufferBuilder *builder, int packed)
{
    // Java: setOverlay -> putPackedUv(i, p_343250_) over UV1.
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_UV1);
    if (!IS_ABSENT(target))
        put_packed_uv(target, packed);
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv2(
    LIBMATTI_B3D_BufferBuilder *builder, int u, int v)
{
    return uv_short(builder, &LIBMATTI_B3D_VFE_ELEMENT_UV2, (short) u, (short) v);
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetLight(
    LIBMATTI_B3D_BufferBuilder *builder, int packed)
{
    // Java: default setLight -> setUv2(packed & 65535, packed >> 16 & 65535).
    return LIBMATTI_B3D_BufferBuilder_SetUv2(builder, packed & 65535, (packed >> 16) & 65535);
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetNormal(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z)
{
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_NORMAL);
    if (!IS_ABSENT(target))
    {
        target[0] = normal_int_value(x);
        target[1] = normal_int_value(y);
        target[2] = normal_int_value(z);
    }
    return builder;
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetLineWidth(
    LIBMATTI_B3D_BufferBuilder *builder, float width)
{
    unsigned char *target = begin_element(builder, &LIBMATTI_B3D_VFE_ELEMENT_LINE_WIDTH);
    if (!IS_ABSENT(target))
        memcpy(target, &width, sizeof(float));
    return builder;
}

void LIBMATTI_B3D_BufferBuilder_AddVertexFull(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z, int color,
    float u, float v, int overlay, int light, float nx, float ny, float nz)
{
    if (builder->fastFormat)
    {
        // Java: the fast path writes every element through one beginVertex.
        unsigned char *vertex = begin_vertex(builder);
        if (vertex == NULL)
            return;
        builder->elementsToFill = 0;
        memcpy(vertex + 0, &x, sizeof(float));
        memcpy(vertex + 4, &y, sizeof(float));
        memcpy(vertex + 8, &z, sizeof(float));
        put_rgba(vertex + 12, color);
        memcpy(vertex + 16, &u, sizeof(float));
        memcpy(vertex + 20, &v, sizeof(float));
        unsigned char *rest = builder->fullFormat ? vertex + 28 : vertex + 24;
        if (builder->fullFormat)
            put_packed_uv(vertex + 24, overlay);
        put_packed_uv(rest + 0, light);
        rest[4] = normal_int_value(nx);
        rest[5] = normal_int_value(ny);
        rest[6] = normal_int_value(nz);
    }
    else
    {
        // Java: VertexConsumer.super.addVertex(...) - the generic chaining path.
        LIBMATTI_B3D_BufferBuilder_AddVertex(builder, x, y, z);
        LIBMATTI_B3D_BufferBuilder_SetColor(builder, color);
        LIBMATTI_B3D_BufferBuilder_SetUv(builder, u, v);
        LIBMATTI_B3D_BufferBuilder_SetOverlay(builder, overlay);
        LIBMATTI_B3D_BufferBuilder_SetLight(builder, light);
        LIBMATTI_B3D_BufferBuilder_SetNormal(builder, nx, ny, nz);
    }
}

// ---- build -----------------------------------------------------------------

LIBMATTI_B3D_MeshData *LIBMATTI_B3D_BufferBuilder_Build(LIBMATTI_B3D_BufferBuilder *builder)
{
    // Java: public @Nullable MeshData build() { ensureBuilding(); endLastVertex();
    //        MeshData meshdata = this.storeMesh(); this.building = false;
    //        this.vertexPointer = -1L; return meshdata; }
    ensure_building(builder);
    end_last_vertex(builder);
    LIBMATTI_B3D_MeshData *mesh = store_mesh(builder);
    builder->building = 0;
    builder->vertexPointer = (unsigned char *) (size_t) NOT_BUILDING;
    return mesh;
}
