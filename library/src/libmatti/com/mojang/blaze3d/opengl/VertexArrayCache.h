// Port of com.mojang.blaze3d.opengl.VertexArrayCache.
// Java picks the Separate (ARB_vertex_attrib_binding) or Emulated strategy at
// creation; the port keeps both and defaults to Emulated (the driver probe
// happens through LIBMATTI_B3D_VertexArrayCache_Create like Java's factory).

#ifndef MATTICRAFT_BLAZE3D_OPENGL_VERTEXARRAYCACHE_H
#define MATTICRAFT_BLAZE3D_OPENGL_VERTEXARRAYCACHE_H

#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: abstract class VertexArrayCache.
typedef struct LIBMATTI_B3D_VertexArrayCache LIBMATTI_B3D_VertexArrayCache;

// Java: public static VertexArrayCache create(GLCapabilities, GlDebugLabel, Set)
// useSeparate != 0 selects the ARB_vertex_attrib_binding path.
LIBMATTI_B3D_VertexArrayCache *LIBMATTI_B3D_VertexArrayCache_Create(int useSeparate);
void LIBMATTI_B3D_VertexArrayCache_Free(LIBMATTI_B3D_VertexArrayCache *cache);

// Java: public abstract void bindVertexArray(VertexFormat, @Nullable GlBuffer)
void LIBMATTI_B3D_VertexArrayCache_BindVertexArray(LIBMATTI_B3D_VertexArrayCache *cache,
                                                   const LIBMATTI_B3D_VertexFormat *format,
                                                   const LIBMATTI_B3D_GpuBuffer *vertexBuffer);

#ifdef __cplusplus
}
#endif

#endif
