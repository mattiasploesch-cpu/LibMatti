// Port of the BiomeSource surface: the fixed source answers the same biome
// everywhere, the checkerboard folds the floor-divided grid coords through the
// biome cycle (Java's Math.floorDiv over the 4-block-per-biome scale).

#include "libmatti/net/minecraft/world/level/biome/BiomeSource.h"

#include <stddef.h>

LIBMATTI_MC_BiomeSource LIBMATTI_MC_BiomeSource_Fixed(LIBMATTI_MC_Biome *biome)
{
    LIBMATTI_MC_BiomeSource source;
    source.kind = LIBMATTI_MC_BiomeSource_FIXED;
    source.fixedBiome = biome;
    source.possibleBiomes = NULL;
    source.possibleBiomeCount = 0;
    source.scale = 0;
    return source;
}

LIBMATTI_MC_BiomeSource LIBMATTI_MC_BiomeSource_Checkerboard(LIBMATTI_MC_Biome **biomes, int count, int scale)
{
    LIBMATTI_MC_BiomeSource source;
    source.kind = LIBMATTI_MC_BiomeSource_CHECKERBOARD;
    source.fixedBiome = NULL;
    source.possibleBiomes = biomes;
    source.possibleBiomeCount = count;
    source.scale = scale > 0 ? scale : 2;
    return source;
}

LIBMATTI_MC_Biome *LIBMATTI_MC_BiomeSource_GetBiome(const LIBMATTI_MC_BiomeSource *source, int x, int y, int z)
{
    switch (source->kind)
    {
        case LIBMATTI_MC_BiomeSource_FIXED:
            return source->fixedBiome;
        case LIBMATTI_MC_BiomeSource_CHECKERBOARD:
        {
            if (source->possibleBiomeCount <= 0)
                return LIBMATTI_MC_Biomes_Plains();
            int scale = source->scale * 4; // Java: the quart-pos unit (4 blocks)
            int gx = x / scale - (x % scale < 0 ? 1 : 0);
            int gz = z / scale - (z % scale < 0 ? 1 : 0);
            int index = gx + gz; // Java: the (x + z) diagonal fold
            index = index % source->possibleBiomeCount;
            if (index < 0)
                index += source->possibleBiomeCount;
            return source->possibleBiomes[index];
        }
    }
    return LIBMATTI_MC_Biomes_Plains();
}
