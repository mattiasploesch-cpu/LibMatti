// Port of com.mojang.blaze3d.vertex.VertexFormat.
// Java: an immutable element list built through VertexFormat.builder() with
// per-element names, a 32-entry offsetsByElement table (UNKNOWN_ELEMENT = -1)
// and the immediate-draw GpuBuffer pair the upload methods maintain.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_VERTEXFORMAT_H
#define MATTICRAFT_BLAZE3D_VERTEX_VERTEXFORMAT_H

#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static final int UNKNOWN_ELEMENT = -1;
#define LIBMATTI_B3D_VERTEXFORMAT_UNKNOWN_ELEMENT -1

// Java: public class VertexFormat
typedef struct LIBMATTI_B3D_VertexFormat
{
    LIBMATTI_B3D_VertexFormat_Mode mode;  // the port binds one mode per format (Java passes it per begin())
    // Java: List<VertexFormatElement> elements - NULL-terminated pointer array,
    // the elements stay owned by the creator (the shared ELEMENT_* constants).
    const LIBMATTI_B3D_VertexFormatElement **elements;
    // Java: List<String> names - parallel array of attribute names.
    char **names;
    int elementCount;
    int *offsets;       // Java: the builder's IntList offsets
    int vertexSize;     // Java: private final int vertexSize
    int elementsMask;   // Java: private final int elementsMask
    // Java: private final int[] offsetsByElement = new int[32] - indexed by
    // element id, UNKNOWN_ELEMENT where the format lacks the element.
    int offsetsByElement[32];
    // Java: private @Nullable GpuBuffer immediateDrawVertexBuffer/IndexBuffer
    LIBMATTI_B3D_GpuBuffer *immediateDrawVertexBuffer;
    LIBMATTI_B3D_GpuBuffer *immediateDrawIndexBuffer;
} LIBMATTI_B3D_VertexFormat;

// Java: VertexFormat.Builder
typedef struct LIBMATTI_B3D_VertexFormat_Builder LIBMATTI_B3D_VertexFormat_Builder;

LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_New(void);
// Java: public VertexFormat.Builder add(String name, VertexFormatElement element)
LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_Add(
    LIBMATTI_B3D_VertexFormat_Builder *builder, const char *name, const LIBMATTI_B3D_VertexFormatElement *element);
// Java: public VertexFormat.Builder padding(int bytes)
LIBMATTI_B3D_VertexFormat_Builder *LIBMATTI_B3D_VertexFormat_Builder_Padding(
    LIBMATTI_B3D_VertexFormat_Builder *builder, int bytes);
// Java: public VertexFormat build()
LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_VertexFormat_Builder_Build(LIBMATTI_B3D_VertexFormat_Builder *builder);
void LIBMATTI_B3D_VertexFormat_Builder_Free(LIBMATTI_B3D_VertexFormat_Builder *builder);

void LIBMATTI_B3D_VertexFormat_Free(LIBMATTI_B3D_VertexFormat *format);

// Java: public int getVertexSize()
int LIBMATTI_B3D_VertexFormat_GetVertexSize(const LIBMATTI_B3D_VertexFormat *format);
// Java: public int getElementCount()
int LIBMATTI_B3D_VertexFormat_GetElementCount(const LIBMATTI_B3D_VertexFormat *format);
// Java: public List<VertexFormatElement> getElements() - by index.
const LIBMATTI_B3D_VertexFormatElement *LIBMATTI_B3D_VertexFormat_GetElement(const LIBMATTI_B3D_VertexFormat *format,
                                                                             int index);
// Java: public int getOffset(VertexFormatElement) - via the element id table.
int LIBMATTI_B3D_VertexFormat_GetOffsetOfElement(const LIBMATTI_B3D_VertexFormat *format,
                                                 const LIBMATTI_B3D_VertexFormatElement *element);
// The slot-based variant the render path uses (POSITION=0..LINE_WIDTH=5, UV0/1/2).
int LIBMATTI_B3D_VertexFormat_GetOffset(const LIBMATTI_B3D_VertexFormat *format, int elementIndex);
// Java: public int[] getOffsetsByElement()
const int *LIBMATTI_B3D_VertexFormat_GetOffsetsByElement(const LIBMATTI_B3D_VertexFormat *format);
// Java: public boolean contains(VertexFormatElement)
int LIBMATTI_B3D_VertexFormat_Contains(const LIBMATTI_B3D_VertexFormat *format,
                                       const LIBMATTI_B3D_VertexFormatElement *element);
// Java: public int getElementsMask()
int LIBMATTI_B3D_VertexFormat_GetElementsMask(const LIBMATTI_B3D_VertexFormat *format);
// Java: public String getElementName(VertexFormatElement) - NULL when absent.
const char *LIBMATTI_B3D_VertexFormat_GetElementName(const LIBMATTI_B3D_VertexFormat *format,
                                                     const LIBMATTI_B3D_VertexFormatElement *element);

// Java: VertexFormat.IndexType - the port adds the byte width accessors.
int LIBMATTI_B3D_VertexFormat_IndexType_Bytes(LIBMATTI_B3D_VertexFormat_IndexType type);
// Java: public static VertexFormat.IndexType least(int vertices)
LIBMATTI_B3D_VertexFormat_IndexType LIBMATTI_B3D_VertexFormat_IndexType_Least(int vertices);

// Java: Mode.indexCount(int) - the port keeps the vertex-side helper.
int LIBMATTI_B3D_VertexFormat_IndexCount(const LIBMATTI_B3D_VertexFormat *format, int vertices);

// Java: public GpuBuffer uploadImmediateVertexBuffer(ByteBuffer) / IndexBuffer -
// writes through the GL layer (GlDevice path); the port returns the handle.
unsigned int LIBMATTI_B3D_VertexFormat_UploadImmediateVertexBuffer(LIBMATTI_B3D_VertexFormat *format,
                                                                   const unsigned char *data, size_t size);
unsigned int LIBMATTI_B3D_VertexFormat_UploadImmediateIndexBuffer(LIBMATTI_B3D_VertexFormat *format,
                                                                  const unsigned char *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
