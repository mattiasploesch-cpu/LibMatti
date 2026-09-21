#include "libmatti/com/mojang/blaze3d/vertex/Tesselator.h"

#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"

#include <stdlib.h>

struct LIBMATTI_B3D_Tesselator
{
    LIBMATTI_B3D_ByteBufferBuilder *buffer;
};

static LIBMATTI_B3D_Tesselator *instance = NULL;

int LIBMATTI_B3D_Tesselator_Init(void)
{
    // Java: if (instance != null) throw "Tesselator has already been initialized".
    if (instance != NULL)
        return 0;
    instance = calloc(1, sizeof(LIBMATTI_B3D_Tesselator));
    // Java: public Tesselator() { this(786432); }
    instance->buffer = LIBMATTI_B3D_ByteBufferBuilder_New(LIBMATTI_B3D_TESSELATOR_MAX_BYTES);
    if (instance->buffer == NULL)
    {
        free(instance);
        instance = NULL;
        return 0;
    }
    return 1;
}

LIBMATTI_B3D_Tesselator *LIBMATTI_B3D_Tesselator_GetInstance(void)
{
    // Java: getInstance throws before init; the port returns NULL.
    return instance;
}

void LIBMATTI_B3D_Tesselator_Free(LIBMATTI_B3D_Tesselator *tesselator)
{
    if (tesselator == NULL)
        return;
    LIBMATTI_B3D_ByteBufferBuilder_Free(tesselator->buffer);
    if (instance == tesselator)
        instance = NULL;
    free(tesselator);
}

LIBMATTI_B3D_BufferBuilder *LIBMATTI_B3D_Tesselator_Begin(
    LIBMATTI_B3D_Tesselator *tesselator, LIBMATTI_B3D_VertexFormat_Mode mode,
    const LIBMATTI_B3D_VertexFormat *format)
{
    // Java: return new BufferBuilder(this.buffer, p_342351_, p_344902_);
    return LIBMATTI_B3D_BufferBuilder_New(tesselator->buffer, mode, format);
}

void LIBMATTI_B3D_Tesselator_Clear(LIBMATTI_B3D_Tesselator *tesselator)
{
    // Java: public void clear() { this.buffer.clear(); }
    LIBMATTI_B3D_ByteBufferBuilder_Clear(tesselator->buffer);
}
