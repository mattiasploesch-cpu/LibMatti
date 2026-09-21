// Port of net.minecraft.world.level.ChunkPos (the chunk coordinate pair with its packed
// long form). The codec/stream parts stay out until the network port needs them.

#ifndef MATTICRAFT_NET_MINECRAFT_WORLD_LEVEL_CHUNKPOS_H
#define MATTICRAFT_NET_MINECRAFT_WORLD_LEVEL_CHUNKPOS_H

#include "libmatti/net/minecraft/core/BlockPos.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class ChunkPos
typedef struct LIBMATTI_MC_ChunkPos
{
    // Java: public final int x / z
    int x;
    int z;
} LIBMATTI_MC_ChunkPos;

// Java: public static final long INVALID_CHUNK_POS = asLong(1875066, 1875066)
#define LIBMATTI_MC_ChunkPos_INVALID_CHUNK_POS 1875066L
// Java: public static final int REGION_SIZE = 32, REGION_MAX_INDEX = 31
#define LIBMATTI_MC_ChunkPos_REGION_SIZE 32
#define LIBMATTI_MC_ChunkPos_REGION_MAX_INDEX 31

// Java: public ChunkPos(int, int)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_New(int x, int z);
// Java: public ChunkPos(BlockPos)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_FromBlockPos(const LIBMATTI_MC_BlockPos *pos);
// Java: public ChunkPos(long)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_FromLong(int64_t packed);
// Java: public static final ChunkPos ZERO
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_ZERO(void);

// Java: public long toLong()
int64_t LIBMATTI_MC_ChunkPos_ToLong(const LIBMATTI_MC_ChunkPos *pos);
// Java: public static long asLong(int, int)
int64_t LIBMATTI_MC_ChunkPos_AsLong(int x, int z);
// Java: public static long asLong(BlockPos)
int64_t LIBMATTI_MC_ChunkPos_AsLongBlockPos(const LIBMATTI_MC_BlockPos *pos);
// Java: public static int getX(long) / getZ(long)
int LIBMATTI_MC_ChunkPos_GetX(int64_t packed);
int LIBMATTI_MC_ChunkPos_GetZ(int64_t packed);

// Java: public static int hash(int, int)
int LIBMATTI_MC_ChunkPos_Hash(int x, int z);

// Java: public int getMinBlockX() / getMinBlockZ() / getMaxBlockX() / getMaxBlockZ()
int LIBMATTI_MC_ChunkPos_GetMinBlockX(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetMinBlockZ(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetMaxBlockX(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetMaxBlockZ(const LIBMATTI_MC_ChunkPos *pos);
// Java: public int getMiddleBlockX() / getMiddleBlockZ()
int LIBMATTI_MC_ChunkPos_GetMiddleBlockX(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetMiddleBlockZ(const LIBMATTI_MC_ChunkPos *pos);

// Java: public int getBlockX(int offset) / getBlockZ(int offset)
int LIBMATTI_MC_ChunkPos_GetBlockX(const LIBMATTI_MC_ChunkPos *pos, int offset);
int LIBMATTI_MC_ChunkPos_GetBlockZ(const LIBMATTI_MC_ChunkPos *pos, int offset);
// Java: public BlockPos getBlockAt(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetBlockAt(const LIBMATTI_MC_ChunkPos *pos, int offsetX, int y, int offsetZ);
// Java: public BlockPos getMiddleBlockPosition(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetMiddleBlockPosition(const LIBMATTI_MC_ChunkPos *pos, int y);
// Java: public BlockPos getWorldPosition()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetWorldPosition(const LIBMATTI_MC_ChunkPos *pos);

// Java: public int getRegionX() / getRegionZ() / getRegionLocalX() / getRegionLocalZ()
int LIBMATTI_MC_ChunkPos_GetRegionX(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetRegionZ(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetRegionLocalX(const LIBMATTI_MC_ChunkPos *pos);
int LIBMATTI_MC_ChunkPos_GetRegionLocalZ(const LIBMATTI_MC_ChunkPos *pos);

// Java: public boolean contains(BlockPos)
int LIBMATTI_MC_ChunkPos_Contains(const LIBMATTI_MC_ChunkPos *pos, const LIBMATTI_MC_BlockPos *blockPos);
// Java: public boolean isValid()
int LIBMATTI_MC_ChunkPos_IsValid(const LIBMATTI_MC_ChunkPos *pos);

// Java: public int getChessboardDistance(ChunkPos) / distanceSquared(ChunkPos)
int LIBMATTI_MC_ChunkPos_GetChessboardDistance(const LIBMATTI_MC_ChunkPos *a, const LIBMATTI_MC_ChunkPos *b);
int LIBMATTI_MC_ChunkPos_DistanceSquared(const LIBMATTI_MC_ChunkPos *a, const LIBMATTI_MC_ChunkPos *b);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_WORLD_LEVEL_CHUNKPOS_H
