// Port of net.minecraft.world.level.chunk.ChunkGenerator (P7.2) - the port
// keeps the generation surface the flat/noise sources share: fillFromNoise
// over the chunk's sections, the base height/column queries and the biome
// source. Structures, carvers and feature placement are later-phase content
// (the flat source stubs them like vanilla's empty overrides).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKGENERATOR_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKGENERATOR_H

#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"
#include "libmatti/net/minecraft/world/level/biome/BiomeSource.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_ChunkAccess;
struct LIBMATTI_MC_FlatLevelSource;

// the generator dispatch (the port switches on the kind until the C-side
// vtable lands - flat is the only concrete source so far)
typedef enum LIBMATTI_MC_ChunkGenerator_Kind
{
    LIBMATTI_MC_ChunkGenerator_FLAT,
} LIBMATTI_MC_ChunkGenerator_Kind;

// Java: public abstract class ChunkGenerator
typedef struct LIBMATTI_MC_ChunkGenerator
{
    // the dispatch kind
    LIBMATTI_MC_ChunkGenerator_Kind kind;
    // Java: public final BiomeSource biomeSource
    LIBMATTI_MC_BiomeSource biomeSource;
    // the flat source payload (borrowed - FlatLevelSource owns the settings)
    struct LIBMATTI_MC_FlatLevelSource *flat;
} LIBMATTI_MC_ChunkGenerator;

// Java: fillFromNoise(Blender, RandomState, StructureManager, ChunkAccess)
// - the noise pass over the chunk (the flat source writes the layer stack)
void LIBMATTI_MC_ChunkGenerator_FillFromNoise(LIBMATTI_MC_ChunkGenerator *generator,
                                              struct LIBMATTI_MC_ChunkAccess *chunk);
// Java: getBaseHeight(int, int, Heightmap.Types, LevelHeightAccessor, RandomState)
int LIBMATTI_MC_ChunkGenerator_GetBaseHeight(LIBMATTI_MC_ChunkGenerator *generator, int x, int z,
                                             LIBMATTI_MC_HeightmapTypes type,
                                             const LIBMATTI_MC_LevelHeightAccessor *height);
// Java: getSpawnHeight(LevelHeightAccessor)
int LIBMATTI_MC_ChunkGenerator_GetSpawnHeight(LIBMATTI_MC_ChunkGenerator *generator,
                                              const LIBMATTI_MC_LevelHeightAccessor *height);
// Java: the ChunkStatus.BIOMES task - every section samples the biome source
// at the section's world position (the port stores the section's biome)
void LIBMATTI_MC_ChunkGenerator_FillBiomes(LIBMATTI_MC_ChunkGenerator *generator,
                                           struct LIBMATTI_MC_ChunkAccess *chunk);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKGENERATOR_H
