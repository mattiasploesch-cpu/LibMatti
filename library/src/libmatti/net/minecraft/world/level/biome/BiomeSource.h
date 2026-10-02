// Port of the net.minecraft.world.level.biome.BiomeSource surface (P7.2):
// the fixed source (FlatLevelSource's ctor pair) and the checkerboard source
// (the vanilla "checkerboard" superflat variant's 2D grid).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BIOME_BIOMESOURCE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_BIOMESOURCE_H

#include "libmatti/net/minecraft/world/level/biome/Biome.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LIBMATTI_MC_BiomeSource_Kind
{
    LIBMATTI_MC_BiomeSource_FIXED,
    LIBMATTI_MC_BiomeSource_CHECKERBOARD,
} LIBMATTI_MC_BiomeSource_Kind;

// Java: FixedBiomeSource / CheckerboardBiomeSource (the scale in biomes per
// grid cell; Java's default zoom is 2)
typedef struct LIBMATTI_MC_BiomeSource
{
    LIBMATTI_MC_BiomeSource_Kind kind;
    LIBMATTI_MC_Biome *fixedBiome;
    LIBMATTI_MC_Biome **possibleBiomes;
    int possibleBiomeCount;
    int scale;
} LIBMATTI_MC_BiomeSource;

// Java: new FixedBiomeSource(Holder<Biome>)
LIBMATTI_MC_BiomeSource LIBMATTI_MC_BiomeSource_Fixed(LIBMATTI_MC_Biome *biome);
// Java: new CheckerboardBiomeSource(...) - the biomes cycle per grid cell
LIBMATTI_MC_BiomeSource LIBMATTI_MC_BiomeSource_Checkerboard(LIBMATTI_MC_Biome **biomes, int count, int scale);
// Java: getBiome(x, y, z) - the sampled biome at the world position
LIBMATTI_MC_Biome *LIBMATTI_MC_BiomeSource_GetBiome(const LIBMATTI_MC_BiomeSource *source, int x, int y, int z);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_BIOMESOURCE_H
