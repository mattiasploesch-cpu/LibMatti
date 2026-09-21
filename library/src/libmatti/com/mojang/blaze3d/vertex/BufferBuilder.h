// Port of com.mojang.blaze3d.vertex.BufferBuilder + the VertexConsumer surface
// it implements. The writer methods route through beginElement() like Java:
// an element outside the format is a no-op, a missing element fails the
// following build.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_BUFFERBUILDER_H
#define MATTICRAFT_BLAZE3D_VERTEX_BUFFERBUILDER_H

#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/MeshData.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class BufferBuilder implements VertexConsumer
typedef struct LIBMATTI_B3D_BufferBuilder LIBMATTI_B3D_BufferBuilder;

// Java: public BufferBuilder(ByteBufferBuilder, VertexFormat.Mode, VertexFormat)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_New(LIBMATTI_B3D_ByteBufferBuilder *buffer,
                                                           LIBMATTI_B3D_VertexFormat_Mode mode,
                                                           const LIBMATTI_B3D_VertexFormat *format);
// Java: no close - the builder borrows the shared ByteBufferBuilder.
void LIBMATTI_B3D_BufferBuilder_Free(LIBMATTI_B3D_BufferBuilder *builder);

// ---- VertexConsumer (the writing surface) ---------------------------------
// Java: VertexConsumer addVertex(float, float, float)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_AddVertex(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z);
// Java: VertexConsumer setColor(int r, int g, int b, int a)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColorRGBA(
    LIBMATTI_B3D_BufferBuilder *builder, int r, int g, int b, int a);
// Java: VertexConsumer setColor(int argb)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColor(
    LIBMATTI_B3D_BufferBuilder *builder, int argb);
// Java: default VertexConsumer setColor(float, float, float, float)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetColorFloat(
    LIBMATTI_B3D_BufferBuilder *builder, float r, float g, float b, float a);
// Java: VertexConsumer setUv(float, float)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv(
    LIBMATTI_B3D_BufferBuilder *builder, float u, float v);
// Java: VertexConsumer setUv1(int, int)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv1(
    LIBMATTI_B3D_BufferBuilder *builder, int u, int v);
// Java: default VertexConsumer setOverlay(int packed) -> setUv1(u & 65535, v >> 16)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetOverlay(
    LIBMATTI_B3D_BufferBuilder *builder, int packed);
// Java: VertexConsumer setUv2(int, int)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetUv2(
    LIBMATTI_B3D_BufferBuilder *builder, int u, int v);
// Java: default VertexConsumer setLight(int packed) -> setUv2(u & 65535, v >> 16)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetLight(
    LIBMATTI_B3D_BufferBuilder *builder, int packed);
// Java: VertexConsumer setNormal(float, float, float)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetNormal(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z);
// Java: VertexConsumer setLineWidth(float)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_BufferBuilder_SetLineWidth(
    LIBMATTI_B3D_BufferBuilder *builder, float width);
// Java: default void addVertex(x, y, z, color, u, v, overlay, light, nx, ny, nz)
void LIBMATTI_B3D_BufferBuilder_AddVertexFull(
    LIBMATTI_B3D_BufferBuilder *builder, float x, float y, float z, int color,
    float u, float v, int overlay, int light, float nx, float ny, float nz);

// Java: public @Nullable MeshData build()
LIBMATTI_B3D_MeshData *LIBMATTI_B3D_BufferBuilder_Build(LIBMATTI_B3D_BufferBuilder *builder);

#ifdef __cplusplus
}
#endif

#endif
