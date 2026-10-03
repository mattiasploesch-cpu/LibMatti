// Port of net.minecraft.world.level.levelgen.FlatLevelSource: fillFromNoise
// writes the layer stack bottom-up over the chunk's sections (with the
// OCEAN_FLOOR_WG + WORLD_SURFACE_WG heightmap updates like the Java loop),
// getBaseHeight answers the first opaque layer top, getSpawnHeight rides
// min(getHeight, layers.size()).

#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLevelSource.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include <stdlib.h>

LIBMATTI_MC_FlatLevelSource *LIBMATTI_MC_FlatLevelSource_New(const LIBMATTI_MC_FlatLayerInfo *layers, int layerCount,
                                                             LIBMATTI_MC_Biome *biome)
{
    LIBMATTI_MC_FlatLevelSource *source = calloc(1, sizeof(LIBMATTI_MC_FlatLevelSource));
    if (source == NULL)
        return NULL;
    source->base.kind = LIBMATTI_MC_ChunkGenerator_FLAT;
    source->base.flat = source;
    source->base.biomeSource = LIBMATTI_MC_BiomeSource_Fixed(biome);
    LIBMATTI_MC_FlatLevelGeneratorSettings_Init(&source->settings, layers, layerCount, biome);
    return source;
}

LIBMATTI_MC_FlatLevelGeneratorSettings *LIBMATTI_MC_FlatLevelSource_Settings(LIBMATTI_MC_FlatLevelSource *source)
{
    return &source->settings;
}

void LIBMATTI_MC_FlatLevelSource_FillFromNoise(LIBMATTI_MC_FlatLevelSource *source, struct LIBMATTI_MC_ChunkAccess *chunk)
{
    // Java: List<BlockState> list = settings.getLayers()
    LIBMATTI_MC_FlatLevelGeneratorSettings *settings = &source->settings;
    LIBMATTI_MC_BlockState **list = settings->layersPerY;
    int listSize = settings->layersPerYCount;

    // Java: the two worldgen heightmaps the loop updates directly
    LIBMATTI_MC_Heightmap *oceanFloor = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(
        chunk, LIBMATTI_MC_Heightmap_OCEAN_FLOOR_WG);
    LIBMATTI_MC_Heightmap *worldSurface = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(
        chunk, LIBMATTI_MC_Heightmap_WORLD_SURFACE_WG);

    int minY = chunk->levelHeightAccessor.minY;
    int height = chunk->levelHeightAccessor.height;
    for (int i = 0; i < listSize && i < height; i++)
    {
        LIBMATTI_MC_BlockState *state = list[i];
        if (state == NULL)
            continue;
        int y = minY + i;
        for (int x = 0; x < 16; x++)
        {
            for (int z = 0; z < 16; z++)
            {
                // Java: chunk.setBlockState(mutable.set(k, j, l), blockstate) -
                // the ChunkAccess write rides the section directly (no level)
                int sectionIndex = LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk->levelHeightAccessor, y);
                if (sectionIndex < 0 || sectionIndex >= chunk->sectionCount)
                    continue;
                LIBMATTI_MC_LevelChunkSection *section = chunk->sections[sectionIndex];
                LIBMATTI_MC_LevelChunkSection_SetBlockState(section, x, y & 15, z & 15, state);
                // Java: heightmap.update(k, j, l, blockstate) on both worldgen maps
                LIBMATTI_MC_Heightmap_Update(oceanFloor, x, y, z, state);
                LIBMATTI_MC_Heightmap_Update(worldSurface, x, y, z, state);
            }
        }
    }
}

int LIBMATTI_MC_FlatLevelSource_GetBaseHeight(LIBMATTI_MC_FlatLevelSource *source, int x, int z,
                                              LIBMATTI_MC_HeightmapTypes type,
                                              const LIBMATTI_MC_LevelHeightAccessor *height)
{
    (void) x;
    (void) z;
    // Java: the top-down layer scan for the first state passing the type's
    // isOpaque test (the port treats every non-air state as opaque)
    LIBMATTI_MC_FlatLevelGeneratorSettings *settings = &source->settings;
    int maxScan = settings->layersPerYCount < height->height + 1 ? settings->layersPerYCount - 1 : height->height;
    for (int i = maxScan; i >= 0; i--)
    {
        LIBMATTI_MC_BlockState *state = i < settings->layersPerYCount ? settings->layersPerY[i] : NULL;
        if (state != NULL && LIBMATTI_MC_BlockState_GetBlock(state) != (void *) LIBMATTI_MC_VanillaBlocks_AIR())
            return height->minY + i + 1;
    }
    return height->minY;
}

int LIBMATTI_MC_FlatLevelSource_GetSpawnHeight(LIBMATTI_MC_FlatLevelSource *source,
                                               const LIBMATTI_MC_LevelHeightAccessor *height)
{
    // Java: minY + Math.min(getHeight(), settings.getLayers().size())
    int layers = source->settings.layersPerYCount;
    return height->minY + (height->height < layers ? height->height : layers);
}

void LIBMATTI_MC_FlatLevelSource_Free(LIBMATTI_MC_FlatLevelSource *source)
{
    if (source == NULL)
        return;
    LIBMATTI_MC_FlatLevelGeneratorSettings_Free(&source->settings);
    free(source);
}
