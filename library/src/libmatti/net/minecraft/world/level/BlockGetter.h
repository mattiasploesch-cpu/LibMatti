// Port of net.minecraft.world.level.BlockGetter (interface).
//
// Java's interface becomes the C free-function surface over any object that
// provides getBlockEntity/getBlockState/getFluidState - the Level and the
// LevelChunk both implement it.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCKGETTER_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCKGETTER_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

// the getter works on the Level (BlockGetter.h stays level-free to avoid the
// Level.h cycle; Level.h includes this header)
struct LIBMATTI_MC_Level;

#ifdef __cplusplus
extern "C" {
#endif

// Java: BlockEntity getBlockEntity(BlockPos) - the typed entity from the chunk
struct LIBMATTI_MC_BlockEntity;
struct LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockGetter_GetBlockEntity(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: BlockState getBlockState(BlockPos)
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockGetter_GetBlockState(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: int getLightEmission(BlockPos) - the block's light emission (0 in the port
// until the block model carries luminance)
int LIBMATTI_MC_BlockGetter_GetLightEmission(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCKGETTER_H
