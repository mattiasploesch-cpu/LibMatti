// Port of the ChunkGenerator dispatch: the kind switch routes to the concrete
// source (the flat source is the only one; the noise-based generator lands
// with the full NoiseRouter port).

#include "libmatti/net/minecraft/world/level/chunk/ChunkGenerator.h"

#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLevelSource.h"
#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

void LIBMATTI_MC_ChunkGenerator_FillFromNoise(LIBMATTI_MC_ChunkGenerator *generator,
                                              struct LIBMATTI_MC_ChunkAccess *chunk)
{
    if (generator == NULL || chunk == NULL)
        return;
    switch (generator->kind)
    {
        case LIBMATTI_MC_ChunkGenerator_FLAT:
            if (generator->flat != NULL)
                LIBMATTI_MC_FlatLevelSource_FillFromNoise(generator->flat, chunk);
            break;
    }
}

int LIBMATTI_MC_ChunkGenerator_GetBaseHeight(LIBMATTI_MC_ChunkGenerator *generator, int x, int z,
                                             LIBMATTI_MC_HeightmapTypes type,
                                             const LIBMATTI_MC_LevelHeightAccessor *height)
{
    if (generator == NULL)
        return 0;
    switch (generator->kind)
    {
        case LIBMATTI_MC_ChunkGenerator_FLAT:
            if (generator->flat != NULL)
                return LIBMATTI_MC_FlatLevelSource_GetBaseHeight(generator->flat, x, z, type, height);
            break;
    }
    return 0;
}

int LIBMATTI_MC_ChunkGenerator_GetSpawnHeight(LIBMATTI_MC_ChunkGenerator *generator,
                                              const LIBMATTI_MC_LevelHeightAccessor *height)
{
    if (generator == NULL)
        return 0;
    switch (generator->kind)
    {
        case LIBMATTI_MC_ChunkGenerator_FLAT:
            if (generator->flat != NULL)
                return LIBMATTI_MC_FlatLevelSource_GetSpawnHeight(generator->flat, height);
            break;
    }
    return 0;
}

void LIBMATTI_MC_ChunkGenerator_FillBiomes(LIBMATTI_MC_ChunkGenerator *generator,
                                           struct LIBMATTI_MC_ChunkAccess *chunk)
{
    if (generator == NULL || chunk == NULL)
        return;
    // Java: the BIOMES status task - one sample per section (the section's
    // biome container is single-valued at the sample position)
    int minSectionY = chunk->levelHeightAccessor.minY / 16;
    for (int i = 0; i < chunk->sectionCount; i++)
    {
        LIBMATTI_MC_LevelChunkSection *section = chunk->sections[i];
        if (section == NULL)
            continue;
        int sectionY = minSectionY + i;
        LIBMATTI_MC_Biome *biome = LIBMATTI_MC_BiomeSource_GetBiome(&generator->biomeSource,
                                                                    chunk->chunkPos.x * 16 + 8,
                                                                    sectionY * 16 + 8,
                                                                    chunk->chunkPos.z * 16 + 8);
        LIBMATTI_MC_LevelChunkSection_SetBiome(section, biome);
    }
}
