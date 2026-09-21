// Port of net.minecraft.core.Direction.

#include "libmatti/net/minecraft/core/Direction.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Java: DOWN(0, 1, -1, "down", NEGATIVE, Axis.Y, new Vec3i(0, -1, 0)), ...
// one entry per constant in declaration order: data3d, oppositeIndex, data2d, name,
// axis, axisDirection, normal
typedef struct DirectionData
{
    int data3d;
    int oppositeIndex;
    int data2d;
    const char *name;
    LIBMATTI_MC_Direction_Axis axis;
    LIBMATTI_MC_Direction_AxisDirection axisDirection;
    int stepX;
    int stepY;
    int stepZ;
} DirectionData;

static const DirectionData DATA[LIBMATTI_MC_Direction_COUNT] = {
    {0, 1, -1, "down", LIBMATTI_MC_Direction_Axis_Y, LIBMATTI_MC_Direction_AxisDirection_NEGATIVE, 0, -1, 0},
    {1, 0, -1, "up", LIBMATTI_MC_Direction_Axis_Y, LIBMATTI_MC_Direction_AxisDirection_POSITIVE, 0, 1, 0},
    {2, 3, 2, "north", LIBMATTI_MC_Direction_Axis_Z, LIBMATTI_MC_Direction_AxisDirection_NEGATIVE, 0, 0, -1},
    {3, 2, 0, "south", LIBMATTI_MC_Direction_Axis_Z, LIBMATTI_MC_Direction_AxisDirection_POSITIVE, 0, 0, 1},
    {4, 5, 1, "west", LIBMATTI_MC_Direction_Axis_X, LIBMATTI_MC_Direction_AxisDirection_NEGATIVE, -1, 0, 0},
    {5, 4, 3, "east", LIBMATTI_MC_Direction_Axis_X, LIBMATTI_MC_Direction_AxisDirection_POSITIVE, 1, 0, 0},
};

// Java: private static final Direction[] BY_3D_DATA - sorted by data3d (DOWN, UP, NORTH, SOUTH, WEST, EAST)
static const LIBMATTI_MC_Direction BY_3D_DATA[LIBMATTI_MC_Direction_COUNT] = {
    LIBMATTI_MC_Direction_DOWN, LIBMATTI_MC_Direction_UP, LIBMATTI_MC_Direction_NORTH,
    LIBMATTI_MC_Direction_SOUTH, LIBMATTI_MC_Direction_WEST, LIBMATTI_MC_Direction_EAST};

// Java: private static final Direction[] BY_2D_DATA - horizontals sorted by data2d (SOUTH 0, WEST 1, NORTH 2, EAST 3)
static const LIBMATTI_MC_Direction BY_2D_DATA[4] = {
    LIBMATTI_MC_Direction_SOUTH, LIBMATTI_MC_Direction_WEST, LIBMATTI_MC_Direction_NORTH,
    LIBMATTI_MC_Direction_EAST};

const char *LIBMATTI_MC_Direction_GetName(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].name;
}

// Java: public int get3DDataValue()
int LIBMATTI_MC_Direction_Get3DDataValue(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].data3d;
}

// Java: public int get2DDataValue()
int LIBMATTI_MC_Direction_Get2DDataValue(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].data2d;
}

// Java: public static Direction byName(String) - the codec byName
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_ByName(const char *name, int *found)
{
    for (int i = 0; i < LIBMATTI_MC_Direction_COUNT; i++)
    {
        if (strcmp(DATA[i].name, name) == 0)
        {
            if (found != NULL) *found = 1;
            return (LIBMATTI_MC_Direction) i;
        }
    }
    if (found != NULL) *found = 0;
    return LIBMATTI_MC_Direction_DOWN;
}

// Java: public static Direction from3DDataValue(int) - BY_3D_DATA[Mth.abs(index % 6)]
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_From3DDataValue(int index)
{
    return BY_3D_DATA[LIBMATTI_MC_Mth_AbsI(index % LIBMATTI_MC_Direction_COUNT)];
}

// Java: public static Direction from2DDataValue(int)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_From2DDataValue(int index)
{
    return BY_2D_DATA[LIBMATTI_MC_Mth_AbsI(index % 4)];
}

// Java: public static Direction fromYRot(double) - from2DDataValue(floor(yRot / 90.0 + 0.5) & 3)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_FromYRot(double yRot)
{
    return LIBMATTI_MC_Direction_From2DDataValue(LIBMATTI_MC_Mth_FloorD(yRot / 90.0 + 0.5) & 3);
}

// Java: public static Direction fromAxisAndDirection(Axis, AxisDirection)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_FromAxisAndDirection(LIBMATTI_MC_Direction_Axis axis,
                                                                 LIBMATTI_MC_Direction_AxisDirection axisDirection)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X:
        return axisDirection == LIBMATTI_MC_Direction_AxisDirection_POSITIVE ? LIBMATTI_MC_Direction_EAST
                                                                            : LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_Axis_Y:
        return axisDirection == LIBMATTI_MC_Direction_AxisDirection_POSITIVE ? LIBMATTI_MC_Direction_UP
                                                                            : LIBMATTI_MC_Direction_DOWN;
    case LIBMATTI_MC_Direction_Axis_Z:
        return axisDirection == LIBMATTI_MC_Direction_AxisDirection_POSITIVE ? LIBMATTI_MC_Direction_SOUTH
                                                                            : LIBMATTI_MC_Direction_NORTH;
    }
    return LIBMATTI_MC_Direction_DOWN;
}

// Java: public static Direction get(AxisDirection, Axis)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Get(LIBMATTI_MC_Direction_AxisDirection axisDirection,
                                                LIBMATTI_MC_Direction_Axis axis)
{
    for (int i = 0; i < LIBMATTI_MC_Direction_COUNT; i++)
    {
        if (DATA[i].axisDirection == axisDirection && DATA[i].axis == axis)
            return (LIBMATTI_MC_Direction) i;
    }
    // Java: throw new IllegalArgumentException("No such direction: " + ...)
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
    LIBMATTI_ML_Logger_Error(logger, NULL, "No such direction");
    return LIBMATTI_MC_Direction_DOWN;
}

// Java: public Direction.Axis getAxis()
LIBMATTI_MC_Direction_Axis LIBMATTI_MC_Direction_GetAxis(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].axis;
}

// Java: public Direction.AxisDirection getAxisDirection()
LIBMATTI_MC_Direction_AxisDirection LIBMATTI_MC_Direction_GetAxisDirection(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].axisDirection;
}

// Java: public Direction getOpposite() - from3DDataValue(oppositeIndex)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetOpposite(LIBMATTI_MC_Direction direction)
{
    return LIBMATTI_MC_Direction_From3DDataValue(DATA[direction].oppositeIndex);
}

// Java: private Direction getClockWiseX()
static LIBMATTI_MC_Direction get_clock_wise_x(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_DOWN: return LIBMATTI_MC_Direction_SOUTH;
    case LIBMATTI_MC_Direction_UP: return LIBMATTI_MC_Direction_NORTH;
    case LIBMATTI_MC_Direction_NORTH: return LIBMATTI_MC_Direction_DOWN;
    case LIBMATTI_MC_Direction_SOUTH: return LIBMATTI_MC_Direction_UP;
    default:
        // Java: throw new IllegalStateException("Unable to get X-rotated facing of " + this)
        return direction;
    }
}

// Java: private Direction getCounterClockWiseX()
static LIBMATTI_MC_Direction get_counter_clock_wise_x(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_DOWN: return LIBMATTI_MC_Direction_NORTH;
    case LIBMATTI_MC_Direction_UP: return LIBMATTI_MC_Direction_SOUTH;
    case LIBMATTI_MC_Direction_NORTH: return LIBMATTI_MC_Direction_UP;
    case LIBMATTI_MC_Direction_SOUTH: return LIBMATTI_MC_Direction_DOWN;
    default:
        return direction;
    }
}

// Java: private Direction getClockWiseZ()
static LIBMATTI_MC_Direction get_clock_wise_z(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_DOWN: return LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_UP: return LIBMATTI_MC_Direction_EAST;
    case LIBMATTI_MC_Direction_WEST: return LIBMATTI_MC_Direction_UP;
    case LIBMATTI_MC_Direction_EAST: return LIBMATTI_MC_Direction_DOWN;
    default:
        return direction;
    }
}

// Java: private Direction getCounterClockWiseZ()
static LIBMATTI_MC_Direction get_counter_clock_wise_z(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_DOWN: return LIBMATTI_MC_Direction_EAST;
    case LIBMATTI_MC_Direction_UP: return LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_WEST: return LIBMATTI_MC_Direction_DOWN;
    case LIBMATTI_MC_Direction_EAST: return LIBMATTI_MC_Direction_UP;
    default:
        return direction;
    }
}

// Java: public Direction getClockWise()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetClockWise(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_NORTH: return LIBMATTI_MC_Direction_EAST;
    case LIBMATTI_MC_Direction_SOUTH: return LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_WEST: return LIBMATTI_MC_Direction_NORTH;
    case LIBMATTI_MC_Direction_EAST: return LIBMATTI_MC_Direction_SOUTH;
    default:
        // Java: throw new IllegalStateException("Unable to get Y-rotated facing of " + this)
        return direction;
    }
}

// Java: public Direction getCounterClockWise()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetCounterClockWise(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_NORTH: return LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_SOUTH: return LIBMATTI_MC_Direction_EAST;
    case LIBMATTI_MC_Direction_WEST: return LIBMATTI_MC_Direction_SOUTH;
    case LIBMATTI_MC_Direction_EAST: return LIBMATTI_MC_Direction_NORTH;
    default:
        return direction;
    }
}

// Java: public Direction getClockWise(Axis)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetClockWiseAround(LIBMATTI_MC_Direction direction,
                                                               LIBMATTI_MC_Direction_Axis axis)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X:
        return direction != LIBMATTI_MC_Direction_WEST && direction != LIBMATTI_MC_Direction_EAST
                   ? get_clock_wise_x(direction)
                   : direction;
    case LIBMATTI_MC_Direction_Axis_Y:
        return direction != LIBMATTI_MC_Direction_UP && direction != LIBMATTI_MC_Direction_DOWN
                   ? LIBMATTI_MC_Direction_GetClockWise(direction)
                   : direction;
    case LIBMATTI_MC_Direction_Axis_Z:
        return direction != LIBMATTI_MC_Direction_NORTH && direction != LIBMATTI_MC_Direction_SOUTH
                   ? get_clock_wise_z(direction)
                   : direction;
    }
    return direction;
}

// Java: public Direction getCounterClockWise(Axis)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetCounterClockWiseAround(LIBMATTI_MC_Direction direction,
                                                                      LIBMATTI_MC_Direction_Axis axis)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X:
        return direction != LIBMATTI_MC_Direction_WEST && direction != LIBMATTI_MC_Direction_EAST
                   ? get_counter_clock_wise_x(direction)
                   : direction;
    case LIBMATTI_MC_Direction_Axis_Y:
        return direction != LIBMATTI_MC_Direction_UP && direction != LIBMATTI_MC_Direction_DOWN
                   ? LIBMATTI_MC_Direction_GetCounterClockWise(direction)
                   : direction;
    case LIBMATTI_MC_Direction_Axis_Z:
        return direction != LIBMATTI_MC_Direction_NORTH && direction != LIBMATTI_MC_Direction_SOUTH
                   ? get_counter_clock_wise_z(direction)
                   : direction;
    }
    return direction;
}

// Java: public int getStepX() / getStepY() / getStepZ()
int LIBMATTI_MC_Direction_GetStepX(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].stepX;
}

int LIBMATTI_MC_Direction_GetStepY(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].stepY;
}

int LIBMATTI_MC_Direction_GetStepZ(LIBMATTI_MC_Direction direction)
{
    return DATA[direction].stepZ;
}

// Java: public Vec3i getUnitVec3i() - the shared normal vector per direction
const LIBMATTI_MC_Vec3i *LIBMATTI_MC_Direction_GetUnitVec3i(LIBMATTI_MC_Direction direction)
{
    static LIBMATTI_MC_Vec3i normals[LIBMATTI_MC_Direction_COUNT];
    normals[direction].x = DATA[direction].stepX;
    normals[direction].y = DATA[direction].stepY;
    normals[direction].z = DATA[direction].stepZ;
    return &normals[direction];
}

// Java: public float toYRot() - (data2d & 3) * 90
float LIBMATTI_MC_Direction_ToYRot(LIBMATTI_MC_Direction direction)
{
    return (float) ((DATA[direction].data2d & 3) * 90);
}

// Java: public static float getYRot(Direction)
float LIBMATTI_MC_Direction_GetYRot(LIBMATTI_MC_Direction direction)
{
    switch (direction)
    {
    case LIBMATTI_MC_Direction_NORTH: return 180.0f;
    case LIBMATTI_MC_Direction_SOUTH: return 0.0f;
    case LIBMATTI_MC_Direction_WEST: return 90.0f;
    case LIBMATTI_MC_Direction_EAST: return -90.0f;
    default:
        // Java: throw new IllegalStateException("No y-Rot for vertical axis: " + ...)
        return 0.0f;
    }
}

// Java: public static Direction getApproximateNearest(float, float, float)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetApproximateNearest(float x, float y, float z)
{
    LIBMATTI_MC_Direction direction = LIBMATTI_MC_Direction_NORTH;
    float best = -FLT_MAX;
    for (int i = 0; i < LIBMATTI_MC_Direction_COUNT; i++)
    {
        float dot = x * (float) DATA[i].stepX + y * (float) DATA[i].stepY + z * (float) DATA[i].stepZ;
        if (dot > best)
        {
            best = dot;
            direction = (LIBMATTI_MC_Direction) i;
        }
    }
    return direction;
}

// Java: public static Direction getNearest(int, int, int, Direction)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetNearest(int x, int y, int z, LIBMATTI_MC_Direction fallback)
{
    int absX = abs(x);
    int absY = abs(y);
    int absZ = abs(z);
    if (absX > absZ && absX > absY)
        return x < 0 ? LIBMATTI_MC_Direction_WEST : LIBMATTI_MC_Direction_EAST;
    if (absZ > absX && absZ > absY)
        return z < 0 ? LIBMATTI_MC_Direction_NORTH : LIBMATTI_MC_Direction_SOUTH;
    if (absY > absX && absY > absZ)
        return y < 0 ? LIBMATTI_MC_Direction_DOWN : LIBMATTI_MC_Direction_UP;
    return fallback;
}

// Java: public boolean isFacingAngle(float)
int LIBMATTI_MC_Direction_IsFacingAngle(LIBMATTI_MC_Direction direction, float angle)
{
    float radians = angle * (float) (M_PI / 180.0);
    float sinNeg = -LIBMATTI_MC_Mth_Sin(radians);
    float cos = LIBMATTI_MC_Mth_Cos(radians);
    return (float) DATA[direction].stepX * sinNeg + (float) DATA[direction].stepZ * cos > 0.0f;
}

// Java: Axis.isVertical() - this == Y
int LIBMATTI_MC_Direction_Axis_IsVertical(LIBMATTI_MC_Direction_Axis axis)
{
    return axis == LIBMATTI_MC_Direction_Axis_Y;
}

// Java: Axis.isHorizontal() - this == X || this == Z
int LIBMATTI_MC_Direction_Axis_IsHorizontal(LIBMATTI_MC_Direction_Axis axis)
{
    return axis == LIBMATTI_MC_Direction_Axis_X || axis == LIBMATTI_MC_Direction_Axis_Z;
}

// Java: Axis.getName()
const char *LIBMATTI_MC_Direction_Axis_GetName(LIBMATTI_MC_Direction_Axis axis)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X: return "x";
    case LIBMATTI_MC_Direction_Axis_Y: return "y";
    case LIBMATTI_MC_Direction_Axis_Z: return "z";
    }
    return "x";
}

// Java: Axis.byName(String)
LIBMATTI_MC_Direction_Axis LIBMATTI_MC_Direction_Axis_ByName(const char *name, int *found)
{
    if (found != NULL) *found = 1;
    if (strcmp(name, "x") == 0) return LIBMATTI_MC_Direction_Axis_X;
    if (strcmp(name, "y") == 0) return LIBMATTI_MC_Direction_Axis_Y;
    if (strcmp(name, "z") == 0) return LIBMATTI_MC_Direction_Axis_Z;
    if (found != NULL) *found = 0;
    return LIBMATTI_MC_Direction_Axis_X;
}

// Java: Axis.choose(int, int, int)
int LIBMATTI_MC_Direction_Axis_ChooseI(LIBMATTI_MC_Direction_Axis axis, int x, int y, int z)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X: return x;
    case LIBMATTI_MC_Direction_Axis_Y: return y;
    case LIBMATTI_MC_Direction_Axis_Z: return z;
    }
    return x;
}

// Java: Axis.choose(double, double, double)
double LIBMATTI_MC_Direction_Axis_ChooseD(LIBMATTI_MC_Direction_Axis axis, double x, double y, double z)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X: return x;
    case LIBMATTI_MC_Direction_Axis_Y: return y;
    case LIBMATTI_MC_Direction_Axis_Z: return z;
    }
    return x;
}

// Java: Axis.getPositive()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Axis_GetPositive(LIBMATTI_MC_Direction_Axis axis)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X: return LIBMATTI_MC_Direction_EAST;
    case LIBMATTI_MC_Direction_Axis_Y: return LIBMATTI_MC_Direction_UP;
    case LIBMATTI_MC_Direction_Axis_Z: return LIBMATTI_MC_Direction_SOUTH;
    }
    return LIBMATTI_MC_Direction_EAST;
}

// Java: Axis.getNegative()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Axis_GetNegative(LIBMATTI_MC_Direction_Axis axis)
{
    switch (axis)
    {
    case LIBMATTI_MC_Direction_Axis_X: return LIBMATTI_MC_Direction_WEST;
    case LIBMATTI_MC_Direction_Axis_Y: return LIBMATTI_MC_Direction_DOWN;
    case LIBMATTI_MC_Direction_Axis_Z: return LIBMATTI_MC_Direction_NORTH;
    }
    return LIBMATTI_MC_Direction_WEST;
}

// Java: Axis.getPlane()
LIBMATTI_MC_Direction_Plane LIBMATTI_MC_Direction_Axis_GetPlane(LIBMATTI_MC_Direction_Axis axis)
{
    return axis == LIBMATTI_MC_Direction_Axis_Y ? LIBMATTI_MC_Direction_Plane_VERTICAL
                                                : LIBMATTI_MC_Direction_Plane_HORIZONTAL;
}

// Java: Plane.faces - HORIZONTAL = [NORTH, EAST, SOUTH, WEST], VERTICAL = [UP, DOWN]
const LIBMATTI_MC_Direction *LIBMATTI_MC_Direction_Plane_Faces(LIBMATTI_MC_Direction_Plane plane, size_t *count)
{
    static const LIBMATTI_MC_Direction HORIZONTAL[4] = {
        LIBMATTI_MC_Direction_NORTH, LIBMATTI_MC_Direction_EAST, LIBMATTI_MC_Direction_SOUTH,
        LIBMATTI_MC_Direction_WEST};
    static const LIBMATTI_MC_Direction VERTICAL[2] = {LIBMATTI_MC_Direction_UP, LIBMATTI_MC_Direction_DOWN};
    if (plane == LIBMATTI_MC_Direction_Plane_HORIZONTAL)
    {
        if (count != NULL) *count = 4;
        return HORIZONTAL;
    }
    if (count != NULL) *count = 2;
    return VERTICAL;
}
