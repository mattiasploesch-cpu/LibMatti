// Port of net.minecraft.world.level.LevelReader.

#include "libmatti/net/minecraft/world/level/LevelReader.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#include <stdlib.h>

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelReader_GetChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ, bool load)
{
    // Java: getChunk(x, z, status, load) - the in-memory port has no load path,
    // so load=true creates an empty chunk like the cache's "should always be able
    // to create a chunk" fallback would
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunk(level, chunkX, chunkZ);
    if (chunk == NULL && load)
    {
        LIBMATTI_MC_ChunkPos pos = {chunkX, chunkZ};
        chunk = LIBMATTI_MC_LevelChunk_New(level, &pos);
        LIBMATTI_MC_Level_SetChunk(level, chunk);
    }
    return chunk;
}

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelReader_GetChunkAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_LevelReader_GetChunk(level,
                                            LIBMATTI_MC_SectionPos_BlockToSectionCoord(LIBMATTI_MC_Vec3i_GetX(&pos->base)),
                                            LIBMATTI_MC_SectionPos_BlockToSectionCoord(LIBMATTI_MC_Vec3i_GetZ(&pos->base)),
                                            true);
}

bool LIBMATTI_MC_LevelReader_HasChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ)
{
    return LIBMATTI_MC_Level_HasChunk(level, chunkX, chunkZ);
}

int LIBMATTI_MC_LevelReader_GetSeaLevel(struct LIBMATTI_MC_Level *level)
{
    // Java: DimensionType.minY() + seaLevel() - the overworld default 63
    (void) level;
    return 63;
}

static bool is_air(const LIBMATTI_MC_BlockState *state)
{
    return state == NULL || LIBMATTI_MC_BlockState_GetBlock(state) == (void *) LIBMATTI_MC_VanillaBlocks_AIR();
}

int LIBMATTI_MC_LevelReader_GetHeight(struct LIBMATTI_MC_Level *level, int x, int z)
{
    // Java: getHeight(type, x, z) through the heightmap - the port scans down from
    // the top for the first non-air block and returns the Y above it
    for (int y = LIBMATTI_MC_LevelHeightAccessor_GetMaxY(&level->heightAccessor); y >= level->heightAccessor.minY; y--)
    {
        LIBMATTI_MC_BlockPos *pos = LIBMATTI_MC_BlockPos_New(x, y, z);
        bool air = is_air(LIBMATTI_MC_Level_GetBlockState(level, pos));
        free(pos);
        if (!air)
            return y + 1;
    }
    return level->heightAccessor.minY;
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_LevelReader_GetHeightmapPos(struct LIBMATTI_MC_Level *level, int x, int z)
{
    // Java: new BlockPos(x, getHeight(type, x, z), z)
    return LIBMATTI_MC_BlockPos_New(x, LIBMATTI_MC_LevelReader_GetHeight(level, x, z), z);
}

bool LIBMATTI_MC_LevelReader_IsEmptyBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: getBlockState(pos).isAir()
    return is_air(LIBMATTI_MC_Level_GetBlockState(level, pos));
}

bool LIBMATTI_MC_LevelReader_CanSeeSky(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: no non-air block strictly above the position
    int x = LIBMATTI_MC_Vec3i_GetX(&pos->base);
    int z = LIBMATTI_MC_Vec3i_GetZ(&pos->base);
    for (int y = LIBMATTI_MC_Vec3i_GetY(&pos->base) + 1; y <= LIBMATTI_MC_LevelHeightAccessor_GetMaxY(&level->heightAccessor); y++)
    {
        LIBMATTI_MC_BlockPos *above = LIBMATTI_MC_BlockPos_New(x, y, z);
        bool air = is_air(LIBMATTI_MC_Level_GetBlockState(level, above));
        free(above);
        if (!air)
            return false;
    }
    return true;
}
