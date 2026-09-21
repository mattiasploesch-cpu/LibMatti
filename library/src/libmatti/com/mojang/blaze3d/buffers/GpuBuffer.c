#include "libmatti/com/mojang/blaze3d/buffers/GpuBuffer.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"

#include <stdlib.h>

LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GpuBuffer_New(int usage, long size)
{
    LIBMATTI_B3D_GpuBuffer *buffer = calloc(1, sizeof(LIBMATTI_B3D_GpuBuffer));
    buffer->usage = usage;
    buffer->size = size;
    return buffer;
}

void LIBMATTI_B3D_GpuBuffer_Free(LIBMATTI_B3D_GpuBuffer *buffer)
{
    if (buffer == NULL)
        return;
    // Java: public void close() - unmap the persistent view, delete the handle.
    if (!buffer->closed)
    {
        buffer->closed = 1;
        if (buffer->persistentBuffer != NULL)
        {
            LIBMATTI_B3D_GlStateManager_UnmapBuffer(buffer->handle);
            buffer->persistentBuffer = NULL;
        }
        LIBMATTI_B3D_GlStateManager_DeleteBuffers(buffer->handle);
    }
    free(buffer);
}

int LIBMATTI_B3D_GpuBuffer_IsClosed(const LIBMATTI_B3D_GpuBuffer *buffer)
{
    return buffer->closed;
}

int LIBMATTI_B3D_GpuBuffer_Usage(const LIBMATTI_B3D_GpuBuffer *buffer)
{
    return buffer->usage;
}

long LIBMATTI_B3D_GpuBuffer_Size(const LIBMATTI_B3D_GpuBuffer *buffer)
{
    return buffer->size;
}
