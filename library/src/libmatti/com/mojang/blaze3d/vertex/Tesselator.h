// Port of com.mojang.blaze3d.vertex.Tesselator - the immediate-mode entry:
// one shared ByteBufferBuilder the BufferBuilders write into.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_TESSELATOR_H
#define MATTICRAFT_BLAZE3D_VERTEX_TESSELATOR_H

#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/com/mojang/blaze3d/vertex/BufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class Tesselator - private static final int MAX_BYTES = 786432;
#define LIBMATTI_B3D_TESSELATOR_MAX_BYTES 786432

// Java: public class Tesselator
typedef struct LIBMATTI_B3D_Tesselator LIBMATTI_B3D_Tesselator;

// Java: public static void init() - 0 when already initialized.
int LIBMATTI_B3D_Tesselator_Init(void);
// Java: public static Tesselator getInstance() - NULL before init().
LIBMATTI_B3D_Tesselator *LIBMATTI_B3D_Tesselator_GetInstance(void);

// Java: no close on the singleton; the port frees the instance explicitly.
void LIBMATTI_B3D_Tesselator_Free(LIBMATTI_B3D_Tesselator *tesselator);

// Java: public BufferBuilder begin(VertexFormat.Mode, VertexFormat)
LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_Tesselator_Begin(
    LIBMATTI_B3D_Tesselator *tesselator, LIBMATTI_B3D_VertexFormat_Mode mode,
    const LIBMATTI_B3D_VertexFormat *format);

// Java: public void clear()
void LIBMATTI_B3D_Tesselator_Clear(LIBMATTI_B3D_Tesselator *tesselator);

#ifdef __cplusplus
}
#endif

#endif
