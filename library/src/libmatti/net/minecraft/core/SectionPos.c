// Port of net.minecraft.core.SectionPos (implementation).

#include "libmatti/net/minecraft/core/SectionPos.h"

#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdlib.h>

// Java: public static final long SECTION_MASK = 4294967280L - the low 32 bits
// with the bottom 4 cleared (the section-relative part).
#define SECTION_MASK_LONG 0xFFFFFF00LL

// Java: private static int sectionCoord(int coord) -> coord >> 4
static int section_coord(int coord)
{
    return coord >> LIBMATTI_MC_SectionPos_SECTION_BITS;
}

// Java: private static int relPos(int coord) -> coord & 15
static int rel_pos(int coord)
{
    return coord & LIBMATTI_MC_SectionPos_SECTION_MASK;
}

// Java: private static int chunkToSection(int coord) -> coord >> 2
static int chunk_to_section(int coord)
{
    return coord >> 2;
}

// The Vec3i base carries the section coordinates directly (Java: super(x, y, z)).
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_Of(int x, int y, int z)
{
    LIBMATTI_MC_SectionPos *pos = calloc(1, sizeof(LIBMATTI_MC_SectionPos));
    pos->base.x = x;
    pos->base.y = y;
    pos->base.z = z;
    return pos;
}

LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_OfBlockPos(const LIBMATTI_MC_BlockPos *pos)
{
    if (pos == NULL)
        return NULL;
    return LIBMATTI_MC_SectionPos_Of(section_coord(pos->base.x), section_coord(pos->base.y),
                                     section_coord(pos->base.z));
}

LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_OfChunk(const LIBMATTI_MC_ChunkPos *chunkPos, int y)
{
    if (chunkPos == NULL)
        return NULL;
    return LIBMATTI_MC_SectionPos_Of(chunk_to_section(chunkPos->x), y, chunk_to_section(chunkPos->z));
}

LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_Create(int64_t packed)
{
    return LIBMATTI_MC_SectionPos_Of((int) (packed & 0x3FFFFF), (int) ((packed >> 22) & 0xFFFFF),
                                     (int) ((packed >> 42) & 0x3FFFFF));
}

int64_t LIBMATTI_MC_SectionPos_BlockToSectionLong(int64_t blockPos)
{
    return (blockPos & SECTION_MASK_LONG) >> 2;
}

int64_t LIBMATTI_MC_SectionPos_GetZeroNode(int64_t packed)
{
    return packed & SECTION_MASK_LONG;
}

int LIBMATTI_MC_SectionPos_BlockToSectionCoord(int coord)
{
    return section_coord(coord);
}

// Java: public static int sectionToBlockCoord(int coord)
int LIBMATTI_MC_SectionPos_SectionToBlockCoord(int coord)
{
    return coord << LIBMATTI_MC_SectionPos_SECTION_BITS;
}

int LIBMATTI_MC_SectionPos_SectionRelative(int coord)
{
    return rel_pos(coord);
}

short LIBMATTI_MC_SectionPos_SectionRelativePos(const LIBMATTI_MC_BlockPos *pos)
{
    if (pos == NULL)
        return 0;
    return (short) (rel_pos(pos->base.x) << 8 | rel_pos(pos->base.z) << 4 | rel_pos(pos->base.y));
}

int LIBMATTI_MC_SectionPos_SectionX(const LIBMATTI_MC_SectionPos *pos)
{
    return pos->base.x;
}

int LIBMATTI_MC_SectionPos_SectionY(const LIBMATTI_MC_SectionPos *pos)
{
    return pos->base.y;
}

int LIBMATTI_MC_SectionPos_SectionZ(const LIBMATTI_MC_SectionPos *pos)
{
    return pos->base.z;
}

short LIBMATTI_MC_SectionPos_RelativeTo(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos)
{
    return LIBMATTI_MC_SectionPos_SectionRelativePos(blockPos);
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_Origin(const LIBMATTI_MC_SectionPos *pos)
{
    return LIBMATTI_MC_BlockPos_New(pos->base.x << 4, pos->base.y << 4, pos->base.z << 4);
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_LowCornerPos(const LIBMATTI_MC_SectionPos *pos)
{
    return LIBMATTI_MC_SectionPos_Origin(pos);
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_HighCornerPos(const LIBMATTI_MC_SectionPos *pos)
{
    return LIBMATTI_MC_BlockPos_New((pos->base.x << 4) + LIBMATTI_MC_SectionPos_SECTION_MASK,
                                    (pos->base.y << 4) + LIBMATTI_MC_SectionPos_SECTION_MASK,
                                    (pos->base.z << 4) + LIBMATTI_MC_SectionPos_SECTION_MASK);
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_RelativeToBlockPos(const LIBMATTI_MC_SectionPos *pos, short offset)
{
    // Java: BlockPos.containing(offset & 15, offset >> 4 & 15, offset >> 8 & 15)
    // relative to the origin (the offset packs y << 8 | z << 4 | x).
    int relX = offset & 15;
    int relY = (offset >> 8) & 15;
    int relZ = (offset >> 4) & 15;
    return LIBMATTI_MC_BlockPos_New((pos->base.x << 4) + relX, (pos->base.y << 4) + relY,
                                    (pos->base.z << 4) + relZ);
}

int LIBMATTI_MC_SectionPos_IsAdjacent(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos)
{
    if (blockPos == NULL)
        return 0;
    return (blockPos->base.x >> 4) == pos->base.x && (blockPos->base.y >> 4) == pos->base.y
           && (blockPos->base.z >> 4) == pos->base.z;
}

int LIBMATTI_MC_SectionPos_Contains(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos)
{
    if (blockPos == NULL)
        return 0;
    return (blockPos->base.x >> 4) == pos->base.x && (blockPos->base.y >> 4) == pos->base.y
           && (blockPos->base.z >> 4) == pos->base.z;
}

int64_t LIBMATTI_MC_SectionPos_AsLong(const LIBMATTI_MC_SectionPos *pos)
{
    return LIBMATTI_MC_SectionPos_AsLong3(pos->base.x, pos->base.y, pos->base.z);
}

int64_t LIBMATTI_MC_SectionPos_AsLong3(int x, int y, int z)
{
    return ((int64_t) x & 0x3FFFFF) | (((int64_t) y & 0xFFFFF) << 22) | (((int64_t) z & 0x3FFFFF) << 42);
}

int64_t LIBMATTI_MC_SectionPos_OffsetDirection(int64_t packed, LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_BlockPos_OffsetLongDirection(packed, direction);
}

size_t LIBMATTI_MC_SectionPos_BetweenClosedCount(const LIBMATTI_MC_SectionPos *min, const LIBMATTI_MC_SectionPos *max)
{
    if (min == NULL || max == NULL)
        return 0;
    long long dx = max->base.x - min->base.x + 1;
    long long dy = max->base.y - min->base.y + 1;
    long long dz = max->base.z - min->base.z + 1;
    if (dx <= 0 || dy <= 0 || dz <= 0)
        return 0;
    return (size_t) (dx * dy * dz);
}

LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_BetweenClosedGet(const LIBMATTI_MC_SectionPos *min,
                                                               const LIBMATTI_MC_SectionPos *max,
                                                               size_t index)
{
    if (min == NULL || max == NULL)
        return NULL;
    long long dx = max->base.x - min->base.x + 1;
    long long dy = max->base.y - min->base.y + 1;
    long long dz = max->base.z - min->base.z + 1;
    if (dx <= 0 || dy <= 0 || dz <= 0)
        return NULL;
    if ((long long) index >= dx * dy * dz)
        return NULL;
    int x = min->base.x + (int) (index % dx);
    int y = min->base.y + (int) (index / (dx * dz));
    int z = min->base.z + (int) ((index / dx) % dz);
    return LIBMATTI_MC_SectionPos_Of(x, y, z);
}
