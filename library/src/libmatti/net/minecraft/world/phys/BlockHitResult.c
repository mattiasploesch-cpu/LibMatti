// Port of net.minecraft.world.phys.BlockHitResult (implementation). See the
// header for the value-struct note.

#include "libmatti/net/minecraft/world/phys/BlockHitResult.h"

#include <stdlib.h>

LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_Miss(const LIBMATTI_MC_Vec3 *location,
                                                           LIBMATTI_MC_Direction direction,
                                                           const LIBMATTI_MC_BlockPos *blockPos)
{
    LIBMATTI_MC_BlockHitResult hit;
    hit.location = *location;
    hit.type = LIBMATTI_MC_HitResult_MISS;
    hit.direction = direction;
    hit.blockPos = *blockPos;
    hit.inside = false;
    hit.worldBorder = false;
    return hit;
}

LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_Block(const LIBMATTI_MC_Vec3 *location,
                                                            LIBMATTI_MC_Direction direction,
                                                            const LIBMATTI_MC_BlockPos *blockPos, bool inside)
{
    LIBMATTI_MC_BlockHitResult hit;
    hit.location = *location;
    hit.type = LIBMATTI_MC_HitResult_BLOCK;
    hit.direction = direction;
    hit.blockPos = *blockPos;
    hit.inside = inside;
    hit.worldBorder = false;
    return hit;
}

LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_WithDirection(const LIBMATTI_MC_BlockHitResult *hit,
                                                                    LIBMATTI_MC_Direction direction)
{
    LIBMATTI_MC_BlockHitResult copy = *hit;
    copy.direction = direction;
    return copy;
}

LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_DefaultMiss(void)
{
    // Java: GameRenderer's field initializer builds the shared miss once
    LIBMATTI_MC_Vec3 zero = {0.0, 0.0, 0.0};
    LIBMATTI_MC_BlockPos zeroPos = {{0, 0, 0}};
    return LIBMATTI_MC_BlockHitResult_Miss(&zero, LIBMATTI_MC_Direction_DOWN, &zeroPos);
}
