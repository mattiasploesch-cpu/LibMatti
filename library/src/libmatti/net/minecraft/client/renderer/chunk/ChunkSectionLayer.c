// Port of net.minecraft.client.renderer.chunk.ChunkSectionLayer (implementation).
//
// Java: SOLID(RenderPipelines.SOLID_TERRAIN, 4194304, false),
//       CUTOUT(RenderPipelines.CUTOUT_TERRAIN, 4194304, false),
//       TRANSLUCENT(RenderPipelines.TRANSLUCENT_TERRAIN, 786432, true),
//       TRIPWIRE(RenderPipelines.TRIPWIRE_TERRAIN, 1536, true)

#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"

static const int BUFFER_SIZES[LIBMATTI_MC_ChunkSectionLayer_COUNT] = {
    4194304, // SOLID
    4194304, // CUTOUT
    786432,  // TRANSLUCENT
    1536     // TRIPWIRE
};

static const int SORT_ON_UPLOAD[LIBMATTI_MC_ChunkSectionLayer_COUNT] = {
    0, // SOLID
    0, // CUTOUT
    1, // TRANSLUCENT
    1  // TRIPWIRE
};

static const char *const LABELS[LIBMATTI_MC_ChunkSectionLayer_COUNT] = {
    "solid", "cutout", "translucent", "tripwire"
};

int LIBMATTI_MC_ChunkSectionLayer_BufferSize(LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return 0;
    return BUFFER_SIZES[layer];
}

int LIBMATTI_MC_ChunkSectionLayer_SortOnUpload(LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return 0;
    return SORT_ON_UPLOAD[layer];
}

const char *LIBMATTI_MC_ChunkSectionLayer_Label(LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return "unknown";
    return LABELS[layer];
}
