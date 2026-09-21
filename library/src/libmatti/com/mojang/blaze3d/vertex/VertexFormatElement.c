#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"

// Java: private final Type type; private final Usage usage; private final int count;
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_POSITION = {LIBMATTI_B3D_VFE_FLOAT, LIBMATTI_B3D_VFE_POSITION, 3, 0};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_COLOR = {LIBMATTI_B3D_VFE_UBYTE, LIBMATTI_B3D_VFE_COLOR, 4, 0};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV0 = {LIBMATTI_B3D_VFE_FLOAT, LIBMATTI_B3D_VFE_UV, 2, 0};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV1 = {LIBMATTI_B3D_VFE_SHORT, LIBMATTI_B3D_VFE_UV, 2, 1};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_UV2 = {LIBMATTI_B3D_VFE_SHORT, LIBMATTI_B3D_VFE_UV, 2, 2};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_NORMAL = {LIBMATTI_B3D_VFE_BYTE, LIBMATTI_B3D_VFE_NORMAL, 3, 0};
const LIBMATTI_B3D_VertexFormatElement LIBMATTI_B3D_VFE_ELEMENT_LINE_WIDTH = {LIBMATTI_B3D_VFE_FLOAT, LIBMATTI_B3D_VFE_GENERIC, 1, 0};

int LIBMATTI_B3D_VertexFormatElement_ByteSize(const LIBMATTI_B3D_VertexFormatElement *element)
{
    // Java: Type.BYTE_SIZE - the per-component byte size times the count.
    static const int byteSize[] = {4, 1, 1, 2, 2, 4, 4};
    return byteSize[element->type] * element->count;
}

unsigned int LIBMATTI_B3D_VertexFormatElement_GlType(const LIBMATTI_B3D_VertexFormatElement *element)
{
    // Java: GlConst.toGl(VertexFormatElement.Type) - the GL_* type enums.
    static const unsigned int glType[] = {5126, 5121, 5120, 5122, 5123, 5124, 5125};
    return glType[element->type];
}
