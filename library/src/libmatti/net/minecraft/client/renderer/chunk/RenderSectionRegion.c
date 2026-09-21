// Port of net.minecraft.client.renderer.chunk.RenderSectionRegion (implementation).
//
// Java: private static int index(int x, int y, int z) -> (y * 18 + z) * 18 + x
// over the 18^3 sections; a block position maps into the region by shifting
// from the center section origin by +1 (the one-block border).

#include "libmatti/net/minecraft/client/renderer/chunk/RenderSectionRegion.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"

#include <stdlib.h>

static int section_index(int x, int y, int z)
{
    return (y * LIBMATTI_MC_RenderSectionRegion_SIZE + z) * LIBMATTI_MC_RenderSectionRegion_SIZE + x;
}

LIBMATTI_MC_RenderSectionRegion *LIBMATTI_MC_RenderSectionRegion_New(int centerX, int centerY, int centerZ,
                                                                     struct LIBMATTI_MC_Level *level,
                                                                     LIBMATTI_MC_LevelChunkSection **sections)
{
    LIBMATTI_MC_RenderSectionRegion *region = calloc(1, sizeof(LIBMATTI_MC_RenderSectionRegion));
    region->centerX = centerX;
    region->centerY = centerY;
    region->centerZ = centerZ;
    region->level = level;
    region->sections = sections;
    return region;
}

void LIBMATTI_MC_RenderSectionRegion_Free(LIBMATTI_MC_RenderSectionRegion *region)
{
    if (region == NULL)
        return;
    free(region->sections);
    free(region);
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_RenderSectionRegion_GetBlockState(const LIBMATTI_MC_RenderSectionRegion *region,
                                                                      const LIBMATTI_MC_BlockPos *pos)
{
    if (region == NULL || pos == NULL)
        return NULL;

    // Java: int i = SectionPos.blockToSectionCoord(pos.getX()) - centerX + 1;
    int sx = LIBMATTI_MC_SectionPos_BlockToSectionCoord(pos->base.x) - region->centerX + 1;
    int sy = LIBMATTI_MC_SectionPos_BlockToSectionCoord(pos->base.y) - region->centerY + 1;
    int sz = LIBMATTI_MC_SectionPos_BlockToSectionCoord(pos->base.z) - region->centerZ + 1;
    if (sx >= 0 && sx < 18 && sy >= 0 && sy < 18 && sz >= 0 && sz < 18)
    {
        LIBMATTI_MC_LevelChunkSection *section = region->sections[section_index(sx, sy, sz)];
        if (section != NULL)
        {
            return LIBMATTI_MC_LevelChunkSection_GetBlockState(
                section, LIBMATTI_MC_SectionPos_SectionRelative(pos->base.x),
                LIBMATTI_MC_SectionPos_SectionRelative(pos->base.y),
                LIBMATTI_MC_SectionPos_SectionRelative(pos->base.z));
        }
    }
    // Java: Blocks.VOID_AIR outside the window / missing sections.
    return LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR());
}
