// Port of com.mojang.blaze3d.vertex.VertexFormatElement.
// Java: an immutable element descriptor (type, usage, count) plus the seven
// built-in constants the vanilla formats are built from.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_VERTEXFORMATELEMENT_H
#define MATTICRAFT_BLAZE3D_VERTEX_VERTEXFORMATELEMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static enum Type - byteSize + the GL enum from GlConst.toGl.
typedef enum LIBMATTI_B3D_VFE_Type
{
    LIBMATTI_B3D_VFE_FLOAT = 0,  // GL_FLOAT 5126
    LIBMATTI_B3D_VFE_UBYTE,      // GL_UNSIGNED_BYTE 5121
    LIBMATTI_B3D_VFE_BYTE,       // GL_BYTE 5120
    LIBMATTI_B3D_VFE_SHORT,      // GL_SHORT 5122
    LIBMATTI_B3D_VFE_USHORT,     // GL_UNSIGNED_SHORT 5123
    LIBMATTI_B3D_VFE_INT,        // GL_INT 5124
    LIBMATTI_B3D_VFE_UINT        // GL_UNSIGNED_INT 5125
} LIBMATTI_B3D_VFE_Type;

// Java: public static enum Usage.
typedef enum LIBMATTI_B3D_VFE_Usage
{
    LIBMATTI_B3D_VFE_POSITION = 0,
    LIBMATTI_B3D_VFE_NORMAL,
    LIBMATTI_B3D_VFE_COLOR,
    LIBMATTI_B3D_VFE_UV,
    LIBMATTI_B3D_VFE_GENERIC
} LIBMATTI_B3D_VFE_Usage;

// Java: public static final class VertexFormatElement (a record with type(),
// usage(), count()).
typedef struct LIBMATTI_B3D_VertexFormatElement
{
    LIBMATTI_B3D_VFE_Type type;
    LIBMATTI_B3D_VFE_Usage usage;
    int count;
    int index;  // Java: the UV index (0..2), 0 for everything else.
} LIBMATTI_B3D_VertexFormatElement;

// Java: public int byteSize() - type.byteSize * count.
int LIBMATTI_B3D_VertexFormatElement_ByteSize(const LIBMATTI_B3D_VertexFormatElement *element);
// Java: GlConst.toGl(type) - the GL type enum for the vertexAttribPointer call.
unsigned int LIBMATTI_B3D_VertexFormatElement_GlType(const LIBMATTI_B3D_VertexFormatElement *element);

// Java: the seven shared elements (register(...) at class-init).
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_POSITION;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_COLOR;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV0;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV1;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV2;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_NORMAL;
extern const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_LINE_WIDTH;

#ifdef __cplusplus
}
#endif

#endif
