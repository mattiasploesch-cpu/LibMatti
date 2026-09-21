#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#include "libmatti/com/mojang/blaze3d/buffers/GlBuffer.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdlib.h>
#include <string.h>

// Java: the elements register with global ids - POSITION=0, COLOR=1, UV0=2,
// UV1=3, UV2=4, NORMAL=5, LINE_WIDTH=6 - and the offsetsByElement table is
// indexed by exactly that id.
static int element_slot(const LIBMATTI_B3D_VertexFormatElement *element)
{
    if (element->usage == LIBMATTI_B3D_VFE_UV)
        return 2 + element->index;  // UV0 -> 2, UV1 -> 3, UV2 -> 4
    switch (element->usage)
    {
        case LIBMATTI_B3D_VFE_POSITION: return 0;
        case LIBMATTI_B3D_VFE_COLOR: return 1;
        case LIBMATTI_B3D_VFE_NORMAL: return 5;
        case LIBMATTI_B3D_VFE_GENERIC: return 6;
        default: return 0;
    }
}

// Java: VertexFormat(List elements, List names, IntList offsets, int vertexSize)
static LIBMATTI_B3D_VertexFormat *format_create(const LIBMATTI_B3D_VertexFormatElement *const *elements,
                                                char **names, int elementCount,
                                                const int *offsets, int vertexSize)
{
    LIBMATTI_B3D_VertexFormat *format = calloc(1, sizeof(LIBMATTI_B3D_VertexFormat));
    format->elementCount = elementCount;
    format->vertexSize = vertexSize;
    format->elements = malloc((size_t) (elementCount + 1) * sizeof(const LIBMATTI_B3D_VertexFormatElement *));
    format->names = malloc((size_t) (elementCount + 1) * sizeof(char *));
    format->offsets = malloc((size_t) elementCount * sizeof(int));

    int mask = 0;
    for (int i = 0; i < elementCount; i++)
    {
        format->elements[i] = elements[i];
        format->names[i] = names[i];
        format->offsets[i] = offsets[i];
        // Java: this.elementsMask = stream.mapToInt(VertexFormatElement::mask).reduce(0, (a, b) -> a | b);
        mask |= 1 << element_slot(elements[i]);
    }
    format->elements[elementCount] = NULL;
    format->names[elementCount] = NULL;
    format->elementsMask = mask;

    // Java: for every id 0..31, offsetsByElement[id] = offset of the element or
    // UNKNOWN_ELEMENT (the BY_ID registry order, see element_slot).
    for (int i = 0; i < 32; i++)
        format->offsetsByElement[i] = LIBMATTI_B3D_VERTEXFORMAT_UNKNOWN_ELEMENT;
    for (int i = 0; i < elementCount; i++)
        format->offsetsByElement[element_slot(elements[i])] = offsets[i];

    return format;
}

// ---- Builder ---------------------------------------------------------------

struct LIBMATTI_B3D_VertexFormat_Builder
{
    const LIBMATTI_B3D_VertexFormatElement **elements;
    char **names;
    int elementCount;
    int elementCapacity;
    int *offsets;
    int offset;
};

LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_New(void)
{
    return calloc(1, sizeof(LIBMATTI_B3D_VertexFormat_Builder));
}

LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_Add(
    LIBMATTI_B3D_VertexFormat_Builder *builder, const char *name, const LIBMATTI_B3D_VertexFormatElement *element)
{
    if (builder->elementCount == builder->elementCapacity)
    {
        builder->elementCapacity = builder->elementCapacity == 0 ? 4 : builder->elementCapacity * 2;
        builder->elements = realloc(builder->elements, (size_t) builder->elementCapacity * sizeof(*builder->elements));
        builder->names = realloc(builder->names, (size_t) builder->elementCapacity * sizeof(*builder->names));
        builder->offsets = realloc(builder->offsets, (size_t) builder->elementCapacity * sizeof(*builder->offsets));
    }
    // Java: ImmutableMap.builder() - duplicate keys throw at build; the port
    // overwrites nothing and just appends (the callers use unique names).
    builder->elements[builder->elementCount] = element;
    builder->names[builder->elementCount] = strdup(name);
    builder->offsets[builder->elementCount] = builder->offset;
    builder->elementCount++;
    // Java: this.offset = this.offset + p_345244_.byteSize();
    builder->offset += LIBMATTI_B3D_VertexFormatElement_ByteSize(element);
    return builder;
}

LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_Padding(
    LIBMATTI_B3D_VertexFormat_Builder *builder, int bytes)
{
    // Java: this.offset += p_345477_;
    builder->offset += bytes;
    return builder;
}

LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_VertexFormat_Builder_Build(LIBMATTI_B3D_VertexFormat_Builder *builder)
{
    LIBMATTI_B3D_VertexFormat *format =
        format_create(builder->elements, builder->names, builder->elementCount, builder->offsets, builder->offset);
    // The format took the names; the builder only owns the arrays.
    builder->names = NULL;
    LIBMATTI_B3D_VertexFormat_Builder_Free(builder);
    return format;
}

void LIBMATTI_B3D_VertexFormat_Builder_Free(LIBMATTI_B3D_VertexFormat_Builder *builder)
{
    if (builder == NULL)
        return;
    if (builder->names != NULL)
    {
        for (int i = 0; i < builder->elementCount; i++)
            free(builder->names[i]);
        free(builder->names);
    }
    free(builder->elements);
    free(builder->offsets);
    free(builder);
}

// ---- Format ----------------------------------------------------------------

void LIBMATTI_B3D_VertexFormat_Free(LIBMATTI_B3D_VertexFormat *format)
{
    if (format == NULL)
        return;
    for (int i = 0; i < format->elementCount; i++)
        free(format->names[i]);
    free(format->names);
    free(format->elements);
    free(format->offsets);
    // Java: the immediate buffers are GpuBuffers; close them with the format.
    if (format->immediateDrawVertexBuffer != NULL)
        LIBMATTI_B3D_GpuBuffer_Free(format->immediateDrawVertexBuffer);
    if (format->immediateDrawIndexBuffer != NULL)
        LIBMATTI_B3D_GpuBuffer_Free(format->immediateDrawIndexBuffer);
    free(format);
}

int LIBMATTI_B3D_VertexFormat_GetVertexSize(const LIBMATTI_B3D_VertexFormat *format)
{
    return format->vertexSize;
}

int LIBMATTI_B3D_VertexFormat_GetElementCount(const LIBMATTI_B3D_VertexFormat *format)
{
    return format->elementCount;
}

const LIBMATTI_B3D_VertexFormatElement *LIBMATTI_B3D_VertexFormat_GetElement(const LIBMATTI_B3D_VertexFormat *format,
                                                                             int index)
{
    if (index < 0 || index >= format->elementCount)
        return NULL;
    return format->elements[index];
}

int LIBMATTI_B3D_VertexFormat_GetOffsetOfElement(const LIBMATTI_B3D_VertexFormat *format,
                                                 const LIBMATTI_B3D_VertexFormatElement *element)
{
    return format->offsetsByElement[element_slot(element)];
}

const int *LIBMATTI_B3D_VertexFormat_GetOffsetsByElement(const LIBMATTI_B3D_VertexFormat *format)
{
    return format->offsetsByElement;
}

// Java has no index-based offset access; the port keeps it for the render
// path (VertexArrayCache/MeshData) - linear scan over the element list.
int LIBMATTI_B3D_VertexFormat_GetOffset(const LIBMATTI_B3D_VertexFormat *format, int elementIndex)
{
    if (elementIndex < 0 || elementIndex >= format->elementCount)
        return 0;
    return format->offsets[elementIndex];
}

int LIBMATTI_B3D_VertexFormat_Contains(const LIBMATTI_B3D_VertexFormat *format,
                                       const LIBMATTI_B3D_VertexFormatElement *element)
{
    // Java: (this.elementsMask & p_345196_.mask()) != 0 - the port shifts the
    // element's registry id (see element_slot).
    int slot = element_slot(element);
    return (format->elementsMask & (1 << slot)) != 0;
}

int LIBMATTI_B3D_VertexFormat_GetElementsMask(const LIBMATTI_B3D_VertexFormat *format)
{
    return format->elementsMask;
}

const char *LIBMATTI_B3D_VertexFormat_GetElementName(const LIBMATTI_B3D_VertexFormat *format,
                                                     const LIBMATTI_B3D_VertexFormatElement *element)
{
    for (int i = 0; i < format->elementCount; i++)
    {
        if (format->elements[i] == element)
            return format->names[i];
    }
    return NULL;
}

// ---- IndexType / Mode ------------------------------------------------------

int LIBMATTI_B3D_VertexFormat_IndexType_Bytes(LIBMATTI_B3D_VertexFormat_IndexType type)
{
    // Java: SHORT(2), INT(4).
    return type == LIBMATTI_B3D_IndexType_INT ? 4 : 2;
}

LIBMATTI_B3D_VertexFormat_IndexType LIBMATTI_B3D_VertexFormat_IndexType_Least(int vertices)
{
    // Java: return (p_166934_ & -65536) != 0 ? INT : SHORT;
    return (vertices & -65536) != 0 ? LIBMATTI_B3D_IndexType_INT : LIBMATTI_B3D_IndexType_SHORT;
}

int LIBMATTI_B3D_VertexFormat_IndexCount(const LIBMATTI_B3D_VertexFormat *format, int vertices)
{
    // Java: Mode.indexCount - LINES/QUADS scale to 6 indices per 4 vertices.
    switch (format->mode)
    {
        case LIBMATTI_B3D_Mode_LINES:
        case LIBMATTI_B3D_Mode_QUADS:
            return vertices / 4 * 6;
        case LIBMATTI_B3D_Mode_DEBUG_LINES:
        case LIBMATTI_B3D_Mode_DEBUG_LINE_STRIP:
        case LIBMATTI_B3D_Mode_POINTS:
        case LIBMATTI_B3D_Mode_TRIANGLES:
        case LIBMATTI_B3D_Mode_TRIANGLE_STRIP:
        case LIBMATTI_B3D_Mode_TRIANGLE_FAN:
        default:
            return vertices;
    }
}

// Java: private static GpuBuffer uploadToBuffer(GpuBuffer, ByteBuffer, usage,
// Supplier) - the port always rewrites the existing buffer when it is big
// enough (the GraphicsWorkarounds "fresh buffer" path closes and recreates).
static LIBMATTI_B3D_GpuBuffer *upload_to_buffer(LIBMATTI_B3D_GpuBuffer *existing, const unsigned char *data,
                                                size_t size, int usage)
{
    if (existing != NULL && LIBMATTI_B3D_GpuBuffer_Size(existing) >= (long) size)
    {
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, existing->handle);
        LIBMATTI_B3D_GlStateManager_BufferSubData(LIBMATTI_GL_GL_ARRAY_BUFFER, 0, data, (long) size);
        LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
        return existing;
    }
    if (existing != NULL)
        LIBMATTI_B3D_GpuBuffer_Free(existing);
    return LIBMATTI_B3D_GlBuffer_NewWithData(usage, (long) size, data);
}

unsigned int LIBMATTI_B3D_VertexFormat_UploadImmediateVertexBuffer(LIBMATTI_B3D_VertexFormat *format,
                                                                   const unsigned char *data, size_t size)
{
    // Java: usage 40 = MAP_WRITE | COPY_DST | VERTEX (32+8).
    format->immediateDrawVertexBuffer =
        upload_to_buffer(format->immediateDrawVertexBuffer, data, size,
                         LIBMATTI_B3D_USAGE_MAP_WRITE | LIBMATTI_B3D_USAGE_COPY_DST | LIBMATTI_B3D_USAGE_VERTEX);
    return format->immediateDrawVertexBuffer->handle;
}

unsigned int LIBMATTI_B3D_VertexFormat_UploadImmediateIndexBuffer(LIBMATTI_B3D_VertexFormat *format,
                                                                  const unsigned char *data, size_t size)
{
    // Java: usage 72 = MAP_WRITE | COPY_DST | VERTEX | INDEX (32+8+64).
    format->immediateDrawIndexBuffer =
        upload_to_buffer(format->immediateDrawIndexBuffer, data, size,
                         LIBMATTI_B3D_USAGE_MAP_WRITE | LIBMATTI_B3D_USAGE_COPY_DST | LIBMATTI_B3D_USAGE_VERTEX |
                             LIBMATTI_B3D_USAGE_INDEX);
    return format->immediateDrawIndexBuffer->handle;
}
