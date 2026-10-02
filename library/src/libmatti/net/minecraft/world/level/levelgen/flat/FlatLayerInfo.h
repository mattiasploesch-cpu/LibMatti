// Port of net.minecraft.world.level.levelgen.flat.FlatLayerInfo (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLAYERINFO_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLAYERINFO_H

#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class FlatLayerInfo - one layer of the superflat stack
typedef struct LIBMATTI_MC_FlatLayerInfo
{
    // Java: private final int height
    int height;
    // Java: private final BlockState blockState (the default state of the block)
    LIBMATTI_MC_BlockState *blockState;
} LIBMATTI_MC_FlatLayerInfo;

// Java: the codec read - height + the block's default state
void LIBMATTI_MC_FlatLayerInfo_Init(LIBMATTI_MC_FlatLayerInfo *layer, int height, LIBMATTI_MC_BlockState *state);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLAYERINFO_H
