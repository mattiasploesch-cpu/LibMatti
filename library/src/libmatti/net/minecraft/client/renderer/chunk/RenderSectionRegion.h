// Port of net.minecraft.client.renderer.chunk.RenderSectionRegion - the 18^3
// block-state window a section compiles against (the center section plus the
// one-block border from the neighbours).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_RENDERSECTIONREGION_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_RENDERSECTIONREGION_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/SectionPos.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: private static final int SIZE = 18
#define LIBMATTI_MC_RenderSectionRegion_SIZE 18
// The section-relative y offset of the center section inside the region.
#define LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET 1

// Java: public class RenderSectionRegion
typedef struct LIBMATTI_MC_RenderSectionRegion
{
    // Java: private final int centerX, centerY, centerZ (the section coords)
    int centerX;
    int centerY;
    int centerZ;
    // Java: private final LevelChunkSection[] sections - 18^3, index
    // y * 18 * 18 + z * 18 + x, NULL where no section exists.
    LIBMATTI_MC_LevelChunkSection **sections;
    // The level the block-entity lookups run against (Java: LevelReader).
    struct LIBMATTI_MC_Level *level;
} LIBMATTI_MC_RenderSectionRegion;

// Java: public RenderSectionRegion(int, int, int, LevelChunkSection[]) - the
// array is taken over (the caller built it with the NULL outside-sections).
LIBMATTI_MC_RenderSectionRegion *LIBMATTI_MC_RenderSectionRegion_New(int centerX, int centerY, int centerZ,
                                                                     struct LIBMATTI_MC_Level *level,
                                                                     LIBMATTI_MC_LevelChunkSection **sections);
// Java: public void close() - releases the section reference array.
void LIBMATTI_MC_RenderSectionRegion_Free(LIBMATTI_MC_RenderSectionRegion *region);

// Java: public BlockState getBlockState(BlockPos) - the global block position
// is translated into the 18^3 region (NULL outside the window).
LIBMATTI_MC_BlockState *LIBMATTI_MC_RenderSectionRegion_GetBlockState(const LIBMATTI_MC_RenderSectionRegion *region,
                                                                      const LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_RENDERSECTIONREGION_H
