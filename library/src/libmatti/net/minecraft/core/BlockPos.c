// Port of net.minecraft.core.BlockPos.

#include "libmatti/net/minecraft/core/BlockPos.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>

// Java: private static final int PACKED_X_MASK = (1L << PACKED_HORIZONTAL_LENGTH) - 1L
#define PACKED_X_MASK ((1L << LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH) - 1L)
// Java: private static final long PACKED_Y_MASK = (1L << PACKED_Y_LENGTH) - 1L
#define PACKED_Y_MASK ((1L << LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH) - 1L)
// Java: private static final long PACKED_Z_MASK = (1L << PACKED_HORIZONTAL_LENGTH) - 1L
#define PACKED_Z_MASK ((1L << LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH) - 1L)
// Java: private static final int Y_OFFSET = 0, Z_OFFSET = PACKED_Y_LENGTH,
//       X_OFFSET = PACKED_Y_LENGTH + PACKED_HORIZONTAL_LENGTH
#define Y_OFFSET 0
#define Z_OFFSET LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH
#define X_OFFSET (LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH + LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH)

// Java: public static final BlockPos ZERO = new BlockPos(0, 0, 0)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_ZERO(void)
{
    static LIBMATTI_MC_BlockPos zero;
    static int initialized = 0;
    if (!initialized)
    {
        zero.base.x = 0;
        zero.base.y = 0;
        zero.base.z = 0;
        initialized = 1;
    }
    return &zero;
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_New(int x, int y, int z)
{
    LIBMATTI_MC_BlockPos *pos = malloc(sizeof(LIBMATTI_MC_BlockPos));
    pos->base.x = x;
    pos->base.y = y;
    pos->base.z = z;
    return pos;
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_FromVec3i(const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_BlockPos_New(v->x, v->y, v->z);
}

// Java: public static long asLong(int x, int y, int z)
int64_t LIBMATTI_MC_BlockPos_AsLong3(int x, int y, int z)
{
    int64_t i = 0;
    i |= (int64_t) ((uint64_t) (x & PACKED_X_MASK) << X_OFFSET);
    i |= (int64_t) ((uint64_t) (y & PACKED_Y_MASK) << 0);
    return i | (int64_t) ((uint64_t) (z & PACKED_Z_MASK) << Z_OFFSET);
}

// Java: public long asLong()
int64_t LIBMATTI_MC_BlockPos_AsLong(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_AsLong3(pos->base.x, pos->base.y, pos->base.z);
}

// Java: public static int getX(long packed)
int LIBMATTI_MC_BlockPos_GetX(long packed)
{
    return (int) ((int64_t) packed << (64 - X_OFFSET - LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH)
                  >> (64 - LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH));
}

// Java: public static int getY(long packed)
int LIBMATTI_MC_BlockPos_GetY(long packed)
{
    return (int) ((int64_t) packed << (64 - LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH)
                  >> (64 - LIBMATTI_MC_BlockPos_PACKED_Y_LENGTH));
}

// Java: public static int getZ(long packed)
int LIBMATTI_MC_BlockPos_GetZ(long packed)
{
    return (int) ((int64_t) packed << (64 - Z_OFFSET - LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH)
                  >> (64 - LIBMATTI_MC_BlockPos_PACKED_HORIZONTAL_LENGTH));
}

// Java: public static BlockPos of(long)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Of(long packed)
{
    return LIBMATTI_MC_BlockPos_New(LIBMATTI_MC_BlockPos_GetX(packed), LIBMATTI_MC_BlockPos_GetY(packed),
                                    LIBMATTI_MC_BlockPos_GetZ(packed));
}

// Java: public static long offset(long packed, int dx, int dy, int dz)
int64_t LIBMATTI_MC_BlockPos_OffsetLong(int64_t packed, int dx, int dy, int dz)
{
    return LIBMATTI_MC_BlockPos_AsLong3(LIBMATTI_MC_BlockPos_GetX((long) packed) + dx,
                                        LIBMATTI_MC_BlockPos_GetY((long) packed) + dy,
                                        LIBMATTI_MC_BlockPos_GetZ((long) packed) + dz);
}

// Java: public static long offset(long packed, Direction)
int64_t LIBMATTI_MC_BlockPos_OffsetLongDirection(int64_t packed, LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_BlockPos_OffsetLong(packed, LIBMATTI_MC_Direction_GetStepX(direction),
                                           LIBMATTI_MC_Direction_GetStepY(direction),
                                           LIBMATTI_MC_Direction_GetStepZ(direction));
}

// Java: public static long getFlatIndex(long) - packed & -16L
int64_t LIBMATTI_MC_BlockPos_GetFlatIndex(int64_t packed)
{
    return packed & -16L;
}

// Java: public static BlockPos containing(double, double, double)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Containing(double x, double y, double z)
{
    return LIBMATTI_MC_BlockPos_New(LIBMATTI_MC_Mth_FloorD(x), LIBMATTI_MC_Mth_FloorD(y), LIBMATTI_MC_Mth_FloorD(z));
}

// Java: public static BlockPos containing(Vec3)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_ContainingVec(const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_BlockPos_Containing(v->x, v->y, v->z);
}

// Java: public static BlockPos min(BlockPos, BlockPos)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Min(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b)
{
    return LIBMATTI_MC_BlockPos_New(a->base.x < b->base.x ? a->base.x : b->base.x,
                                    a->base.y < b->base.y ? a->base.y : b->base.y,
                                    a->base.z < b->base.z ? a->base.z : b->base.z);
}

// Java: public static BlockPos max(BlockPos, BlockPos)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Max(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b)
{
    return LIBMATTI_MC_BlockPos_New(a->base.x > b->base.x ? a->base.x : b->base.x,
                                    a->base.y > b->base.y ? a->base.y : b->base.y,
                                    a->base.z > b->base.z ? a->base.z : b->base.z);
}

// Java: public BlockPos offset(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Offset(const LIBMATTI_MC_BlockPos *pos, int dx, int dy, int dz)
{
    return dx == 0 && dy == 0 && dz == 0 ? LIBMATTI_MC_BlockPos_Immutable(pos)
                                         : LIBMATTI_MC_BlockPos_New(pos->base.x + dx, pos->base.y + dy,
                                                                    pos->base.z + dz);
}

// Java: public BlockPos offset(Vec3i)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_OffsetVec(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_BlockPos_Offset(pos, v->x, v->y, v->z);
}

// Java: public BlockPos subtract(Vec3i)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Subtract(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_BlockPos_Offset(pos, -v->x, -v->y, -v->z);
}

// Java: public BlockPos multiply(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Multiply(const LIBMATTI_MC_BlockPos *pos, int factor)
{
    if (factor == 1)
        return LIBMATTI_MC_BlockPos_Immutable(pos);
    return factor == 0 ? LIBMATTI_MC_BlockPos_New(0, 0, 0)
                       : LIBMATTI_MC_BlockPos_New(pos->base.x * factor, pos->base.y * factor,
                                                  pos->base.z * factor);
}

// Java: public BlockPos above()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Above(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_UP);
}

// Java: public BlockPos above(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_AboveN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_UP, n);
}

// Java: public BlockPos below()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Below(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_DOWN);
}

// Java: public BlockPos below(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_BelowN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_DOWN, n);
}

// Java: public BlockPos north()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_North(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_NORTH);
}

// Java: public BlockPos north(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_NorthN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_NORTH, n);
}

// Java: public BlockPos south()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_South(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_SOUTH);
}

// Java: public BlockPos south(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_SouthN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_SOUTH, n);
}

// Java: public BlockPos west()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_West(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_WEST);
}

// Java: public BlockPos west(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_WestN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_WEST, n);
}

// Java: public BlockPos east()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_East(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_Relative(pos, LIBMATTI_MC_Direction_EAST);
}

// Java: public BlockPos east(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_EastN(const LIBMATTI_MC_BlockPos *pos, int n)
{
    return LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_EAST, n);
}

// Java: public BlockPos relative(Direction)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Relative(const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_BlockPos_New(pos->base.x + LIBMATTI_MC_Direction_GetStepX(direction),
                                    pos->base.y + LIBMATTI_MC_Direction_GetStepY(direction),
                                    pos->base.z + LIBMATTI_MC_Direction_GetStepZ(direction));
}

// Java: public BlockPos relative(Direction, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_RelativeN(const LIBMATTI_MC_BlockPos *pos,
                                                     LIBMATTI_MC_Direction direction, int n)
{
    return n == 0 ? LIBMATTI_MC_BlockPos_Immutable(pos)
                  : LIBMATTI_MC_BlockPos_New(pos->base.x + LIBMATTI_MC_Direction_GetStepX(direction) * n,
                                             pos->base.y + LIBMATTI_MC_Direction_GetStepY(direction) * n,
                                             pos->base.z + LIBMATTI_MC_Direction_GetStepZ(direction) * n);
}

// Java: public BlockPos relative(Axis, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_RelativeAxis(const LIBMATTI_MC_BlockPos *pos,
                                                        LIBMATTI_MC_Direction_Axis axis, int n)
{
    if (n == 0)
        return LIBMATTI_MC_BlockPos_Immutable(pos);
    int dx = axis == LIBMATTI_MC_Direction_Axis_X ? n : 0;
    int dy = axis == LIBMATTI_MC_Direction_Axis_Y ? n : 0;
    int dz = axis == LIBMATTI_MC_Direction_Axis_Z ? n : 0;
    return LIBMATTI_MC_BlockPos_New(pos->base.x + dx, pos->base.y + dy, pos->base.z + dz);
}

// Java: public BlockPos cross(Vec3i)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Cross(const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_BlockPos_New(pos->base.y * v->z - pos->base.z * v->y,
                                    pos->base.z * v->x - pos->base.x * v->z,
                                    pos->base.x * v->y - pos->base.y * v->x);
}

// Java: public BlockPos atY(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_AtY(const LIBMATTI_MC_BlockPos *pos, int y)
{
    return LIBMATTI_MC_BlockPos_New(pos->base.x, y, pos->base.z);
}

// Java: public Vec3 getCenter()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_BlockPos_GetCenter(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_Vec3_AtCenterOf(&pos->base);
}

// Java: public Vec3 getBottomCenter()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_BlockPos_GetBottomCenter(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_Vec3_AtBottomCenterOf(&pos->base);
}

// Java: public BlockPos immutable()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Immutable(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_New(pos->base.x, pos->base.y, pos->base.z);
}

// Java: public BlockPos.MutableBlockPos mutable()
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_Mutable(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockPos_MutableNew3(pos->base.x, pos->base.y, pos->base.z);
}

// -----------------------------------------------------------------------
// MutableBlockPos
// -----------------------------------------------------------------------

LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableNew(void)
{
    return LIBMATTI_MC_BlockPos_MutableNew3(0, 0, 0);
}

LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableNew3(int x, int y, int z)
{
    return LIBMATTI_MC_BlockPos_New(x, y, z);
}

// Java: public MutableBlockPos set(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSet(LIBMATTI_MC_BlockPos *pos, int x, int y, int z)
{
    pos->base.x = x;
    pos->base.y = y;
    pos->base.z = z;
    return pos;
}

// Java: public MutableBlockPos set(long)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetLong(LIBMATTI_MC_BlockPos *pos, int64_t packed)
{
    return LIBMATTI_MC_BlockPos_MutableSet(pos, LIBMATTI_MC_BlockPos_GetX((long) packed),
                                           LIBMATTI_MC_BlockPos_GetY((long) packed),
                                           LIBMATTI_MC_BlockPos_GetZ((long) packed));
}

// Java: public MutableBlockPos setWithOffset(Vec3i, Direction)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetWithOffset(LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3i *v,
                                                                LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_BlockPos_MutableSet(pos, v->x + LIBMATTI_MC_Direction_GetStepX(direction),
                                           v->y + LIBMATTI_MC_Direction_GetStepY(direction),
                                           v->z + LIBMATTI_MC_Direction_GetStepZ(direction));
}

// Java: public MutableBlockPos move(Direction)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMove(LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_BlockPos_MutableMoveN(pos, direction, 1);
}

// Java: public MutableBlockPos move(Direction, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMoveN(LIBMATTI_MC_BlockPos *pos,
                                                        LIBMATTI_MC_Direction direction, int n)
{
    return LIBMATTI_MC_BlockPos_MutableSet(pos, pos->base.x + LIBMATTI_MC_Direction_GetStepX(direction) * n,
                                           pos->base.y + LIBMATTI_MC_Direction_GetStepY(direction) * n,
                                           pos->base.z + LIBMATTI_MC_Direction_GetStepZ(direction) * n);
}

// Java: public MutableBlockPos move(int, int, int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableMove3(LIBMATTI_MC_BlockPos *pos, int dx, int dy, int dz)
{
    return LIBMATTI_MC_BlockPos_MutableSet(pos, pos->base.x + dx, pos->base.y + dy, pos->base.z + dz);
}

// Java: public MutableBlockPos setX(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetX(LIBMATTI_MC_BlockPos *pos, int x)
{
    pos->base.x = x;
    return pos;
}

// Java: public MutableBlockPos setY(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetY(LIBMATTI_MC_BlockPos *pos, int y)
{
    pos->base.y = y;
    return pos;
}

// Java: public MutableBlockPos setZ(int)
LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockPos_MutableSetZ(LIBMATTI_MC_BlockPos *pos, int z)
{
    pos->base.z = z;
    return pos;
}
