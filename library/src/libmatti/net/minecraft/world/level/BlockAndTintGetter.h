// Port of net.minecraft.world.level.BlockAndTintGetter (the interface slice
// the render paths need on top of BlockGetter: getShade, getBlockTint).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCKANDTINTGETTER_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCKANDTINTGETTER_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: the interface is implemented by Level; the port resolves the calls
// through the Level surface (the header stays level-free).
struct LIBMATTI_MC_Level;

typedef struct LIBMATTI_MC_BlockAndTintGetter LIBMATTI_MC_BlockAndTintGetter;

// Java: float getShade(Direction, boolean shade) - the cardinal light table of
// the dimension (ClientLevel's overworld switch).
float LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction direction, int shade);

// Java: boolean canSeeSky(BlockPos) - rides on LevelReader's column scan
// (declared there; the BlockAndTintGetter method is the same surface).

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCKANDTINTGETTER_H
