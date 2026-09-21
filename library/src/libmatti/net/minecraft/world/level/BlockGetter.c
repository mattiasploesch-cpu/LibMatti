// Port of net.minecraft.world.level.BlockGetter - the surface forwards to Level.

#include "libmatti/net/minecraft/world/level/BlockGetter.h"

#include "libmatti/net/minecraft/world/level/Level.h"

struct LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockGetter_GetBlockEntity(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_Level_GetBlockEntity(level, pos);
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockGetter_GetBlockState(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_Level_GetBlockState(level, pos);
}

int LIBMATTI_MC_BlockGetter_GetLightEmission(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: getBlockState(pos).getLightEmission() - the block model carries the
    // luminance; the port has no luminance data yet
    (void) level;
    (void) pos;
    return 0;
}
