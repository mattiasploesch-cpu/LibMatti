// Port of com.mojang.blaze3d.opengl.GlBuffer (subclass of GpuBuffer).
// Java stores the GL handle, the DirectStateAccess strategy and an optional
// persistent mapped view; the port uses the GlStateManager layer for every
// handle operation.

#ifndef MATTICRAFT_BLAZE3D_BUFFERS_GLBUFFER_H
#define MATTICRAFT_BLAZE3D_BUFFERS_GLBUFFER_H

#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class GlBuffer extends GpuBuffer - the handle lives in the
// base struct; this header only adds the factory and the mapped view.
// Java: GlDevice uses GlBuffer with USAGE flags and a byte size.
LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GlBuffer_New(int usage, long size);
// Java: GlBuffer with initial data (GlDevice.preAllocateBuffer path).
LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GlBuffer_NewWithData(int usage, long size, const void *data);
void LIBMATTI_B3D_GlBuffer_Free(LIBMATTI_B3D_GpuBuffer *buffer);

// Java: GpuBuffer.MappedView - the persistent read view; close() unmaps.
void *LIBMATTI_B3D_GlBuffer_MapRead(LIBMATTI_B3D_GpuBuffer *buffer);
void LIBMATTI_B3D_GlBuffer_Unmap(LIBMATTI_B3D_GpuBuffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
