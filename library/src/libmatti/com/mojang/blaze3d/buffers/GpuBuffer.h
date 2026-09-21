// Port of com.mojang.blaze3d.buffers.GpuBuffer - the abstract GPU buffer.
// Java keeps usage flags, a size and a slice type; the C port adds the GL
// handle and the VBO/index-flag the render path needs.

#ifndef MATTICRAFT_BLAZE3D_BUFFERS_GPUBUFFER_H
#define MATTICRAFT_BLAZE3D_BUFFERS_GPUBUFFER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: GpuBuffer.USAGE_* flags.
#define LIBMATTI_B3D_USAGE_MAP_READ 1
#define LIBMATTI_B3D_USAGE_MAP_WRITE 2
#define LIBMATTI_B3D_USAGE_HINT_CLIENT_STORAGE 4
#define LIBMATTI_B3D_USAGE_COPY_DST 8
#define LIBMATTI_B3D_USAGE_COPY_SRC 16
#define LIBMATTI_B3D_USAGE_VERTEX 32
#define LIBMATTI_B3D_USAGE_INDEX 64
#define LIBMATTI_B3D_USAGE_UNIFORM 128
#define LIBMATTI_B3D_USAGE_UNIFORM_TEXEL_BUFFER 256

// Java: public abstract class GpuBuffer implements AutoCloseable
typedef struct LIBMATTI_B3D_GpuBuffer
{
    int usage;
    long size;
    // GlBuffer subclass state (Java: GlBuffer.handle).
    unsigned int handle;
    int closed;
    // Java: persistentBuffer - the mapped storage for USAGE_MAP_READ buffers.
    void *persistentBuffer;
} LIBMATTI_B3D_GpuBuffer;

// Java: public GpuBuffer(@Usage int usage, long size)
LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GpuBuffer_New(int usage, long size);
void LIBMATTI_B3D_GpuBuffer_Free(LIBMATTI_B3D_GpuBuffer *buffer);

// Java: public boolean isClosed()
int LIBMATTI_B3D_GpuBuffer_IsClosed(const LIBMATTI_B3D_GpuBuffer *buffer);
// Java: public int usage() / public long size()
int LIBMATTI_B3D_GpuBuffer_Usage(const LIBMATTI_B3D_GpuBuffer *buffer);
long LIBMATTI_B3D_GpuBuffer_Size(const LIBMATTI_B3D_GpuBuffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
