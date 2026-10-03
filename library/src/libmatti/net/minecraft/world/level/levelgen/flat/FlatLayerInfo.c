// Port of net.minecraft.world.level.levelgen.flat.FlatLayerInfo.

#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLayerInfo.h"

void LIBMATTI_MC_FlatLayerInfo_Init(LIBMATTI_MC_FlatLayerInfo *layer, int height, LIBMATTI_MC_BlockState *state)
{
    layer->height = height;
    layer->blockState = state;
}
