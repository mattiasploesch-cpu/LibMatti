// Port of net.minecraft.core.BlockPos (the packed-long format, the relative helpers and
// MutableBlockPos). The withinManhattan/betweenClosed iterators stay the game port's part
// until their callers exist - the packed representation and the geometry are complete.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_BLOCKPOS_H
#define MATTICRAFT_NET_MINECRAFT_CORE_BLOCKPOS_H

#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/core/Vec3i.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class BlockPos extends Vec3i - the port is a Vec3i typedef
typedef struct LIBMATTI_MC_BlockPos LIBMATTI_MC_BlockPos;
struct LIBMATTI_MC_BlockPos
{
    LIBMATTI_MC_Vec3i base;
};

// Java: public static final BlockPos ZERO
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_ZERO(void);

// Java: public BlockPos(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_New(int x, int y, int z);
// Java: public BlockPos(Vec3i)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_FromVec3i(const LIBMATTI_MC_Vec3i *v);

// -----------------------------------------------------------------------
// The packed long representation
// -----------------------------------------------------------------------

// Java: public static final int PACKED_HORIZONTAL_LENGTH (1 + log2(smallestEncompassingPowerOfTwo(30000000)))
#define LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH 26
// Java: public static final int PACKED_Y_LENGTH = 64 - 2 * PACKED_HORIZONTAL_LENGTH
#define LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH 12
// Java: public static final int MAX_HORIZONTAL_COORDINATE
#define LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE 33554431

// Java: public static long asLong(int, int, int)
int64_t LIBMATTI_MC_BlockPos_AsLong3(int x, int y, int z);
// Java: public long asLong()
int64_t LIBMATTI_MC_BlockPos_AsLong(const LIBMATTI_MC_BlockPos *pos);
// Java: public static int getX(long) / getY(long) / getZ(long)
int LIBMATTI_MC_BlockPos_GetX(long packed);
int LIBMATTI_MC_BlockPos_GetY(long packed);
int LIBMATTI_MC_BlockPos_GetZ(long packed);
// Java: public static BlockPos of(long)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Of(long packed);
// Java: public static long offset(long, int, int, int) / offset(long, Direction)
int64_t LIBMATTI_MC_BlockPos_OffsetLong(int64_t packed, int dx, int dy, int dz);
int64_t LIBMATTI_MC_BlockPos_OffsetLongDirection(int64_t packed, LIBMATTI_MC_Direction direction);
// Java: public static long getFlatIndex(long) - clears the y bits
int64_t LIBMATTI_MC_BlockPos_GetFlatIndex(int64_t packed);

// -----------------------------------------------------------------------
// The geometry
// -----------------------------------------------------------------------

// Java: public static BlockPos containing(double, double, double)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Containing(double x, double y, double z);
// Java: public static BlockPos containing(Vec3)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_ContainingVec(const LIBMATTI_MC_Vec3 *v);
// Java: public static BlockPos min(BlockPos, BlockPos) / max(BlockPos, BlockPos)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Min(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Max(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b);

// Java: public BlockPos offset(int, int, int) / offset(Vec3i) / subtract(Vec3i) / multiply(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Offset(const LIBMATTI_MC_BlockPos *pos, int dx, int dy, int dz);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_OffsetVec(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Subtract(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Multiply(const LIBMATTI_MC_BlockPos *pos, int factor);

// Java: public BlockPos above() / above(int) / below() / below(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Above(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_AboveN(const LIBMATTI_MC_BlockPos *pos, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Below(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_BelowN(const LIBMATTI_MC_BlockPos *pos, int n);
// Java: public BlockPos north() / south() / west() / east() (+ the int overloads)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_North(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_NorthN(const LIBMATTI_MC_BlockPos *pos, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_South(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_SouthN(const LIBMATTI_MC_BlockPos *pos, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_West(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_WestN(const LIBMATTI_MC_BlockPos *pos, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_East(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_EastN(const LIBMATTI_MC_BlockPos *pos, int n);

// Java: public BlockPos relative(Direction) / relative(Direction, int) / relative(Axis, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Relative(const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_RelativeN(const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_RelativeAxis(const LIBMATTI_MC_BlockPos *pos,
                                                        LIBMATTI_MC_Direction_Axis axis, int n);

// Java: public BlockPos cross(Vec3i)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Cross(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v);
// Java: public BlockPos atY(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_AtY(const LIBMATTI_MC_BlockPos *pos, int y);

// Java: public Vec3 getCenter() / getBottomCenter()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_BlockPos_GetCenter(const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_BlockPos_GetBottomCenter(const LIBMATTI_MC_BlockPos *pos);

// Java: public BlockPos immutable()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Immutable(const LIBMATTI_MC_BlockPos *pos);
// Java: public BlockPos.MutableBlockPos mutable()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Mutable(const LIBMATTI_MC_BlockPos *pos);

// -----------------------------------------------------------------------
// MutableBlockPos - the same struct with the set/move mutators
// -----------------------------------------------------------------------

// Java: public static class MutableBlockPos extends BlockPos
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableNew(void);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableNew3(int x, int y, int z);
// Java: public MutableBlockPos set(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSet(LIBMATTI_MC_BlockPos *pos, int x, int y, int z);
// Java: public MutableBlockPos set(long)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetLong(LIBMATTI_MC_BlockPos *pos, int64_t packed);
// Java: public MutableBlockPos setWithOffset(Vec3i, Direction)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetWithOffset(LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v,
                                                                LIBMATTI_MC_Direction direction);
// Java: public MutableBlockPos move(Direction) / move(Direction, int) / move(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMove(LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMoveN(LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction, int n);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMove3(LIBMATTI_MC_BlockPos *pos, int dx, int dy, int dz);
// Java: public MutableBlockPos setX/Y/Z(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetX(LIBMATTI_MC_BlockPos *pos, int x);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetY(LIBMATTI_MC_BlockPos *pos, int y);
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetZ(LIBMATTI_MC_BlockPos *pos, int z);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CORE_BLOCKPOS_H
