// Port of net.minecraft.world.level.levelgen.FlatLevelSource (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELSOURCE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELSOURCE_H

#include "libmatti/net/minecraft/world/level/chunk/ChunkGenerator.h"
#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLevelGeneratorSettings.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_ChunkAccess;

// Java: public class FlatLevelSource extends ChunkGenerator
typedef struct LIBMATTI_MC_FlatLevelSource
{
    // Java: the ChunkGenerator base (the kind + the biome source)
    LIBMATTI_MC_ChunkGenerator base;
    // Java: private final FlatLevelGeneratorSettings settings
    LIBMATTI_MC_FlatLevelGeneratorSettings settings;
} LIBMATTI_MC_FlatLevelSource;

// Java: new FlatLevelSource(FlatLevelGeneratorSettings) - the ctor wires the
// FixedBiomeSource(settings.getBiome()) into the base
LIBMATTI_MC_FlatLevelSource *LIBMATTI_MC_FlatLevelSource_New(const LIBMATTI_MC_FlatLayerInfo *layers, int layerCount,
                                                             LIBMATTI_MC_Biome *biome);
// Java: public FlatLevelGeneratorSettings settings()
LIBMATTI_MC_FlatLevelGeneratorSettings *LIBMATTI_MC_FlatLevelSource_Settings(LIBMATTI_MC_FlatLevelSource *source);
// the generation surface (the ChunkGenerator dispatch calls these)
void LIBMATTI_MC_FlatLevelSource_FillFromNoise(LIBMATTI_MC_FlatLevelSource *source, struct LIBMATTI_MC_ChunkAccess *chunk);
int LIBMATTI_MC_FlatLevelSource_GetBaseHeight(LIBMATTI_MC_FlatLevelSource *source, int x, int z,
                                              LIBMATTI_MC_HeightmapTypes type,
                                              const LIBMATTI_MC_LevelHeightAccessor *height);
int LIBMATTI_MC_FlatLevelSource_GetSpawnHeight(LIBMATTI_MC_FlatLevelSource *source,
                                               const LIBMATTI_MC_LevelHeightAccessor *height);
void LIBMATTI_MC_FlatLevelSource_Free(LIBMATTI_MC_FlatLevelSource *source);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELSOURCE_H
