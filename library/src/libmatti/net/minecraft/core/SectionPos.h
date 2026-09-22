// Port of net.minecraft.core.SectionPos - the section coordinate triple with
// the packed long form and the block <-> section conversions.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_SECTIONPOS_H
#define MATTICRAFT_NET_MINECRAFT_CORE_SECTIONPOS_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class SectionPos extends Vec3i
typedef struct LIBMATTI_MC_SectionPos
{
    LIBMATTI_MC_Vec3i base;
} LIBMATTI_MC_SectionPos;

// Java: public static final int SECTION_BITS = 4, SECTION_SIZE = 16, SECTION_MASK = 15
#define LIBMATTI_MC_SectionPos_SECTION_BITS 4
#define LIBMATTI_MC_SectionPos_SECTION_SIZE 16
#define LIBMATTI_MC_SectionPos_SECTION_MASK 15
// Java: SECTION_HALF_SIZE = SECTION_SIZE / 2, SECTION_FULL_SIZE = SECTION_SIZE * 2
#define LIBMATTI_MC_SectionPos_SECTION_HALF_SIZE 8
#define LIBMATTI_MC_SectionPos_SECTION_FULL_SIZE 32

// Java: public static SectionPos of(int x, int y, int z)
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_Of(int x, int y, int z);
// Java: public static SectionPos of(BlockPos) / of(ChunkPos, int)
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_OfBlockPos(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_OfChunk(const LIBMATTI_MC_ChunkPos *chunkPos, int y);
// Java: public static SectionPos of(Entity) is the game port's part (entity x/y/z) -
// the port exposes the three-coordinate form instead (above).

// Java: public static SectionPos create(long packed)
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_Create(int64_t packed);

// Java: public static long blockToSection(long blockPos)
int64_t LIBMATTI_MC_SectionPos_BlockToSectionLong(int64_t blockPos);
// Java: public static long getZeroNode(long packed)
int64_t LIBMATTI_MC_SectionPos_GetZeroNode(int64_t packed);
// Java: public static int blockToSectionCoord(int coord)
int LIBMATTI_MC_SectionPos_BlockToSectionCoord(int coord);
// Java: public static int sectionToBlockCoord(int coord)
int LIBMATTI_MC_SectionPos_SectionToBlockCoord(int coord);
// Java: public static int sectionRelative(int coord)
int LIBMATTI_MC_SectionPos_SectionRelative(int coord);
// Java: public static short sectionRelativePos(BlockPos)
short LIBMATTI_MC_SectionPos_SectionRelativePos(const LIBMATTI_MC_BlockPos *pos);

// Java: public int sectionX() / sectionY() / sectionZ()
int LIBMATTI_MC_SectionPos_SectionX(const LIBMATTI_MC_SectionPos *pos);
int LIBMATTI_MC_SectionPos_SectionY(const LIBMATTI_MC_SectionPos *pos);
int LIBMATTI_MC_SectionPos_SectionZ(const LIBMATTI_MC_SectionPos *pos);

// Java: public short relativeTo(BlockPos)
short LIBMATTI_MC_SectionPos_RelativeTo(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos);

// Java: public BlockPos origin()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_Origin(const LIBMATTI_MC_SectionPos *pos);
// Java: the min/max corners of the section (1.21 helper form)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_LowCornerPos(const LIBMATTI_MC_SectionPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_HighCornerPos(const LIBMATTI_MC_SectionPos *pos);

// Java: public BlockPos relativeToBlockPos(short offset)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_SectionPos_RelativeToBlockPos(const LIBMATTI_MC_SectionPos *pos, short offset);

// Java: public boolean isAdjacent(BlockPos) / contains(BlockPos)
int LIBMATTI_MC_SectionPos_IsAdjacent(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos);
int LIBMATTI_MC_SectionPos_Contains(const LIBMATTI_MC_SectionPos *pos, const LIBMATTI_MC_BlockPos *blockPos);

// Java: public long asLong() / public static long asLong(int, int, int)
int64_t LIBMATTI_MC_SectionPos_AsLong(const LIBMATTI_MC_SectionPos *pos);
int64_t LIBMATTI_MC_SectionPos_AsLong3(int x, int y, int z);
// Java: public static long offset(long, Direction)
int64_t LIBMATTI_MC_SectionPos_OffsetDirection(int64_t packed, LIBMATTI_MC_Direction direction);

// Java: SectionPos.betweenClosedStream - the port exposes the index-based walk
// (x from min..max) over a cube of sections between two origins.
size_t LIBMATTI_MC_SectionPos_BetweenClosedCount(const LIBMATTI_MC_SectionPos *min, const LIBMATTI_MC_SectionPos *max);
LIBMATTI_MC_SectionPos *LIBMATTI_MC_SectionPos_BetweenClosedGet(const LIBMATTI_MC_SectionPos *min,
                                                               const LIBMATTI_MC_SectionPos *max,
                                                               size_t index);

// Java: the coordinate shifts - the port exposes them as the functions above
// (blockToSectionCoord / sectionToBlockCoord), like every other helper.

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CORE_SECTIONPOS_H
