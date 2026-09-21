// Port of net.minecraft.world.level.LevelReader (interface).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELREADER_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELREADER_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/BlockGetter.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: ChunkAccess getChunk(int chunkX, int chunkZ, ChunkStatus, boolean load) -
// the in-memory port returns the stored chunk or NULL (no load path)
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelReader_GetChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ, bool load);
// Java: default ChunkAccess getChunk(BlockPos)
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelReader_GetChunkAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: boolean hasChunk(int, int)
bool LIBMATTI_MC_LevelReader_HasChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ);
// Java: int getSeaLevel() - the overworld default (63); the dimension model is
// game-port content
int LIBMATTI_MC_LevelReader_GetSeaLevel(struct LIBMATTI_MC_Level *level);
// Java: int getHeight(Heightmap.Types, int x, int z) - the port scans from the top
// for the first non-air block (the heightmap model lands with the save port)
int LIBMATTI_MC_LevelReader_GetHeight(struct LIBMATTI_MC_Level *level, int x, int z);
// Java: BlockPos getHeightmapPos(Heightmap.Types, BlockPos)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_LevelReader_GetHeightmapPos(struct LIBMATTI_MC_Level *level, int x, int z);
// Java: boolean isEmptyBlock(BlockPos)
bool LIBMATTI_MC_LevelReader_IsEmptyBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: BlockAndTintGetter.canSeeSky(BlockPos) = getBrightness(SKY, pos) >= 15 -
// the light engine is game-port content; the port approximates it with the
// column scan (no non-air block strictly above the position)
bool LIBMATTI_MC_LevelReader_CanSeeSky(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELREADER_H
