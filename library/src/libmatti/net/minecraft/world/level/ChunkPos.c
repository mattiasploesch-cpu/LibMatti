// Port of net.minecraft.world.level.ChunkPos.

#include "libmatti/net/minecraft/world/level/ChunkPos.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>

// Java: private static final long COORD_BITS = 32L, COORD_MASK = 4294967295L
#define COORD_MASK 4294967295LL

// SectionPos helpers (SectionPos itself stays out until the level sections exist):
// Java: SectionPos.blockToSectionCoord(int) - coord >> 4
static int block_to_section_coord(int coord)
{
    return coord >> 4;
}

// Java: SectionPos.sectionToBlockCoord(int) - coord << 4
static int section_to_block_coord(int coord)
{
    return coord << 4;
}

// Java: SectionPos.sectionToBlockCoord(int, int) - (coord << 4) + offset
static int section_to_block_coord_offset(int coord, int offset)
{
    return (coord << 4) + offset;
}

LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_New(int x, int z)
{
    LIBMATTI_MC_ChunkPos *pos = malloc(sizeof(LIBMATTI_MC_ChunkPos));
    pos->x = x;
    pos->z = z;
    return pos;
}

// Java: public ChunkPos(BlockPos)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_FromBlockPos(const LIBMATTI_MC_BlockPos *blockPos)
{
    return LIBMATTI_MC_ChunkPos_New(block_to_section_coord(blockPos->base.x),
                                    block_to_section_coord(blockPos->base.z));
}

// Java: public ChunkPos(long)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_FromLong(int64_t packed)
{
    return LIBMATTI_MC_ChunkPos_New((int) packed, (int) (packed >> 32));
}

// Java: public static final ChunkPos ZERO = new ChunkPos(0, 0)
LIBMATTI_MC_ChunkPos *LIBMATTI_MC_ChunkPos_ZERO(void)
{
    static LIBMATTI_MC_ChunkPos zero = {0, 0};
    return &zero;
}

// Java: public long toLong()
int64_t LIBMATTI_MC_ChunkPos_ToLong(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_AsLong(pos->x, pos->z);
}

// Java: public static long asLong(int x, int z)
int64_t LIBMATTI_MC_ChunkPos_AsLong(int x, int z)
{
    return (int64_t) (x & COORD_MASK) | (int64_t) ((z & COORD_MASK) << 32);
}

// Java: public static long asLong(BlockPos)
int64_t LIBMATTI_MC_ChunkPos_AsLongBlockPos(const LIBMATTI_MC_BlockPos *blockPos)
{
    return LIBMATTI_MC_ChunkPos_AsLong(block_to_section_coord(blockPos->base.x),
                                       block_to_section_coord(blockPos->base.z));
}

// Java: public static int getX(long)
int LIBMATTI_MC_ChunkPos_GetX(int64_t packed)
{
    return (int) (packed & COORD_MASK);
}

// Java: public static int getZ(long)
int LIBMATTI_MC_ChunkPos_GetZ(int64_t packed)
{
    return (int) (packed >> 32 & COORD_MASK);
}

// Java: public static int hash(int x, int z)
int LIBMATTI_MC_ChunkPos_Hash(int x, int z)
{
    int i = 1664525 * x + 1013904223;
    int j = 1664525 * (z ^ -559038737) + 1013904223;
    return i ^ j;
}

// Java: public int getMinBlockX() / getMinBlockZ()
int LIBMATTI_MC_ChunkPos_GetMinBlockX(const LIBMATTI_MC_ChunkPos *pos)
{
    return section_to_block_coord(pos->x);
}

int LIBMATTI_MC_ChunkPos_GetMinBlockZ(const LIBMATTI_MC_ChunkPos *pos)
{
    return section_to_block_coord(pos->z);
}

// Java: public int getMaxBlockX() / getMaxBlockZ()
int LIBMATTI_MC_ChunkPos_GetMaxBlockX(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_GetBlockX(pos, 15);
}

int LIBMATTI_MC_ChunkPos_GetMaxBlockZ(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_GetBlockZ(pos, 15);
}

// Java: public int getMiddleBlockX() / getMiddleBlockZ()
int LIBMATTI_MC_ChunkPos_GetMiddleBlockX(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_GetBlockX(pos, 8);
}

int LIBMATTI_MC_ChunkPos_GetMiddleBlockZ(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_GetBlockZ(pos, 8);
}

// Java: public int getBlockX(int offset) / getBlockZ(int offset)
int LIBMATTI_MC_ChunkPos_GetBlockX(const LIBMATTI_MC_ChunkPos *pos, int offset)
{
    return section_to_block_coord_offset(pos->x, offset);
}

int LIBMATTI_MC_ChunkPos_GetBlockZ(const LIBMATTI_MC_ChunkPos *pos, int offset)
{
    return section_to_block_coord_offset(pos->z, offset);
}

// Java: public BlockPos getBlockAt(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetBlockAt(const LIBMATTI_MC_ChunkPos *pos, int offsetX, int y, int offsetZ)
{
    return LIBMATTI_MC_BlockPos_New(LIBMATTI_MC_ChunkPos_GetBlockX(pos, offsetX), y,
                                    LIBMATTI_MC_ChunkPos_GetBlockZ(pos, offsetZ));
}

// Java: public BlockPos getMiddleBlockPosition(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetMiddleBlockPosition(const LIBMATTI_MC_ChunkPos *pos, int y)
{
    return LIBMATTI_MC_BlockPos_New(LIBMATTI_MC_ChunkPos_GetMiddleBlockX(pos), y,
                                    LIBMATTI_MC_ChunkPos_GetMiddleBlockZ(pos));
}

// Java: public BlockPos getWorldPosition()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_ChunkPos_GetWorldPosition(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_BlockPos_New(LIBMATTI_MC_ChunkPos_GetMinBlockX(pos), 0,
                                    LIBMATTI_MC_ChunkPos_GetMinBlockZ(pos));
}

// Java: public int getRegionX() / getRegionZ() / getRegionLocalX() / getRegionLocalZ()
int LIBMATTI_MC_ChunkPos_GetRegionX(const LIBMATTI_MC_ChunkPos *pos)
{
    return pos->x >> 5;
}

int LIBMATTI_MC_ChunkPos_GetRegionZ(const LIBMATTI_MC_ChunkPos *pos)
{
    return pos->z >> 5;
}

int LIBMATTI_MC_ChunkPos_GetRegionLocalX(const LIBMATTI_MC_ChunkPos *pos)
{
    return pos->x & 31;
}

int LIBMATTI_MC_ChunkPos_GetRegionLocalZ(const LIBMATTI_MC_ChunkPos *pos)
{
    return pos->z & 31;
}

// Java: public boolean contains(BlockPos)
int LIBMATTI_MC_ChunkPos_Contains(const LIBMATTI_MC_ChunkPos *pos, const LIBMATTI_MC_BlockPos *blockPos)
{
    return blockPos->base.x >= LIBMATTI_MC_ChunkPos_GetMinBlockX(pos)
        && blockPos->base.z >= LIBMATTI_MC_ChunkPos_GetMinBlockZ(pos)
        && blockPos->base.x <= LIBMATTI_MC_ChunkPos_GetMaxBlockX(pos)
        && blockPos->base.z <= LIBMATTI_MC_ChunkPos_GetMaxBlockZ(pos);
}

// Java: public boolean isValid() - absMax <= MAX_COORDINATE_VALUE; the port uses the
// BlockPos horizontal bound like Java's SectionPos chain resolves to
int LIBMATTI_MC_ChunkPos_IsValid(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_Mth_AbsMaxI(pos->x, pos->z)
        <= LIBMATTI_MC_Mth_Quantize((double) LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE,
                                    LIBMATTI_MC_ChunkPos_REGION_SIZE) / 2;
}

// Java: public int getChessboardDistance(ChunkPos)
int LIBMATTI_MC_ChunkPos_GetChessboardDistance(const LIBMATTI_MC_ChunkPos *a, const LIBMATTI_MC_ChunkPos *b)
{
    return LIBMATTI_MC_Mth_ChessboardDistance(b->x, b->z, a->x, a->z);
}

// Java: public int distanceSquared(ChunkPos)
int LIBMATTI_MC_ChunkPos_DistanceSquared(const LIBMATTI_MC_ChunkPos *a, const LIBMATTI_MC_ChunkPos *b)
{
    int dx = b->x - a->x;
    int dz = b->z - a->z;
    return dx * dx + dz * dz;
}
