// Port of net.minecraft.client.renderer.SectionBufferBuilderPack (implementation).

#include "libmatti/net/minecraft/client/renderer/chunk/SectionBufferBuilderPack.h"

#include <stdlib.h>

// The buffers are freed on close(); the pack owns them.
LIBMATTI_MC_SectionBufferBuilderPack *LIBMATTI_MC_SectionBufferBuilderPack_New(void)
{
    LIBMATTI_MC_SectionBufferBuilderPack *pack = calloc(1, sizeof(LIBMATTI_MC_SectionBufferBuilderPack));
    pack->totalBytes = 0;
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        int size = LIBMATTI_MC_ChunkSectionLayer_BufferSize((LIBMATTI_MC_ChunkSectionLayer) layer);
        pack->buffers[layer] = LIBMATTI_B3D_ByteBufferBuilder_New(size);
        pack->totalBytes += size;
    }
    return pack;
}

void LIBMATTI_MC_SectionBufferBuilderPack_Free(LIBMATTI_MC_SectionBufferBuilderPack *pack)
{
    if (pack == NULL)
        return;
    if (!pack->closed)
    {
        pack->closed = 1;
        for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
        {
            LIBMATTI_B3D_ByteBufferBuilder_Free(pack->buffers[layer]);
            pack->buffers[layer] = NULL;
        }
    }
    free(pack);
}

LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_MC_SectionBufferBuilderPack_Buffer(
    LIBMATTI_MC_SectionBufferBuilderPack *pack, LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (pack == NULL || layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return NULL;
    return pack->buffers[layer];
}

void LIBMATTI_MC_SectionBufferBuilderPack_ClearAll(LIBMATTI_MC_SectionBufferBuilderPack *pack)
{
    if (pack == NULL)
        return;
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        LIBMATTI_B3D_ByteBufferBuilder_Clear(pack->buffers[layer]);
    }
}

long long LIBMATTI_MC_SectionBufferBuilderPack_MaxBytes(void)
{
    // Java: the four layer sizes summed.
    long long total = 0;
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
        total += LIBMATTI_MC_ChunkSectionLayer_BufferSize((LIBMATTI_MC_ChunkSectionLayer) layer);
    return total;
}
