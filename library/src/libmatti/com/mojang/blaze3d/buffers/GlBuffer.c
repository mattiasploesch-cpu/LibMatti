#include "libmatti/com/mojang/blaze3d/buffers/GlBuffer.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GlBuffer_New(int usage, long size)
{
    LIBMATTI_B3D_GpuBuffer *buffer = LIBMATTI_B3D_GpuBuffer_New(usage, size);
    // Java: this.handle = dsa.createBuffer(...); GlStateManager._glBindBuffer +
    // _glBufferData with the size for the pre-allocation.
    buffer->handle = LIBMATTI_B3D_GlStateManager_GenBuffers();
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, buffer->handle);
    LIBMATTI_B3D_GlStateManager_BufferDataSized(LIBMATTI_GL_GL_ARRAY_BUFFER, size, LIBMATTI_GL_GL_DYNAMIC_DRAW);
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    return buffer;
}

LIBMATTI_B3D_GpuBuffer *LIBMATTI_B3D_GlBuffer_NewWithData(int usage, long size, const void *data)
{
    LIBMATTI_B3D_GpuBuffer *buffer = LIBMATTI_B3D_GpuBuffer_New(usage, size);
    buffer->handle = LIBMATTI_B3D_GlStateManager_GenBuffers();
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, buffer->handle);
    LIBMATTI_B3D_GlStateManager_BufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, data, size, LIBMATTI_GL_GL_DYNAMIC_DRAW);
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    return buffer;
}

void LIBMATTI_B3D_GlBuffer_Free(LIBMATTI_B3D_GpuBuffer *buffer)
{
    if (buffer == NULL)
        return;
    // Java: public void close() in GlBuffer - identical to the base port path.
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

void *LIBMATTI_B3D_GlBuffer_MapRead(LIBMATTI_B3D_GpuBuffer *buffer)
{
    if (buffer == NULL || buffer->closed)
        return NULL;
    // Java: dsa.mapBuffer(handle, usage) with MAP_READ - the port maps the
    // whole buffer through GlStateManager (target ARRAY_BUFFER like GlDevice).
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, buffer->handle);
    buffer->persistentBuffer = LIBMATTI_B3D_GlStateManager_MapBufferRange(
        LIBMATTI_GL_GL_ARRAY_BUFFER, 0, buffer->size, 0x0001 /* GL_MAP_READ_BIT */);
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    return buffer->persistentBuffer;
}

void LIBMATTI_B3D_GlBuffer_Unmap(LIBMATTI_B3D_GpuBuffer *buffer)
{
    if (buffer == NULL || buffer->persistentBuffer == NULL)
        return;
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, buffer->handle);
    LIBMATTI_B3D_GlStateManager_UnmapBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER);
    LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    buffer->persistentBuffer = NULL;
}
