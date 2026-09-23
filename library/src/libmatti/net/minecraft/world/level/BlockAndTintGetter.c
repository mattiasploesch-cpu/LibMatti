// Port of net.minecraft.world.level.BlockAndTintGetter (the slice the render
// paths call directly).

#include "libmatti/net/minecraft/world/level/BlockAndTintGetter.h"

// Java: ClientLevel.getShade - the overworld cardinal light table
// (DimensionType.CardinalLightType overworld: 0.5/1.0/0.8/0.6).
float LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction direction, int shade)
{
    if (!shade)
        return 1.0f;
    switch (direction)
    {
    case LIBMATTI_MC_Direction_DOWN: return 0.5f;
    case LIBMATTI_MC_Direction_UP: return 1.0f;
    case LIBMATTI_MC_Direction_NORTH:
    case LIBMATTI_MC_Direction_SOUTH: return 0.8f;
    case LIBMATTI_MC_Direction_WEST:
    case LIBMATTI_MC_Direction_EAST: return 0.6f;
    default: return 1.0f;
    }
}
