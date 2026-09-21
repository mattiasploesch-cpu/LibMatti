// Port of net.minecraft.core.Direction (the enum with Axis, AxisDirection and Plane).
// Java's enum constants become one struct per direction plus the shared constants table
// in declaration order; the getClockWise/rotate machinery follows the Java switches 1:1.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_DIRECTION_H
#define MATTICRAFT_NET_MINECRAFT_CORE_DIRECTION_H

#include "libmatti/net/minecraft/core/Vec3i.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static enum Direction.Axis
typedef enum
{
    LIBMATTI_MC_Direction_Axis_X,
    LIBMATTI_MC_Direction_Axis_Y,
    LIBMATTI_MC_Direction_Axis_Z
} LIBMATTI_MC_Direction_Axis;

// Java: public static enum Direction.AxisDirection
typedef enum
{
    LIBMATTI_MC_Direction_AxisDirection_POSITIVE = 1,
    LIBMATTI_MC_Direction_AxisDirection_NEGATIVE = -1
} LIBMATTI_MC_Direction_AxisDirection;

// Java: public static enum Direction.Plane
typedef enum
{
    LIBMATTI_MC_Direction_Plane_HORIZONTAL,
    LIBMATTI_MC_Direction_Plane_VERTICAL
} LIBMATTI_MC_Direction_Plane;

// Java: public enum Direction implements StringRepresentable
typedef enum
{
    LIBMATTI_MC_Direction_DOWN,
    LIBMATTI_MC_Direction_UP,
    LIBMATTI_MC_Direction_NORTH,
    LIBMATTI_MC_Direction_SOUTH,
    LIBMATTI_MC_Direction_WEST,
    LIBMATTI_MC_Direction_EAST,
    LIBMATTI_MC_Direction_COUNT
} LIBMATTI_MC_Direction;

// Java: private final String name (the serialized name of every constant)
const char *LIBMATTI_MC_Direction_GetName(LIBMATTI_MC_Direction direction);

// Java: public int get3DDataValue() / get2DDataValue()
int LIBMATTI_MC_Direction_Get3DDataValue(LIBMATTI_MC_Direction direction);
int LIBMATTI_MC_Direction_Get2DDataValue(LIBMATTI_MC_Direction direction);

// Java: public static Direction byName(String) - NULL when unknown
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_ByName(const char *name, int *found);
// Java: public static Direction from3DDataValue(int) - the BY_3D_DATA table with wrap
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_From3DDataValue(int index);
// Java: public static Direction from2DDataValue(int)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_From2DDataValue(int index);
// Java: public static Direction fromYRot(double)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_FromYRot(double yRot);
// Java: public static Direction fromAxisAndDirection(Axis, AxisDirection)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_FromAxisAndDirection(LIBMATTI_MC_Direction_Axis axis,
                                                                 LIBMATTI_MC_Direction_AxisDirection axisDirection);
// Java: public static Direction get(AxisDirection, Axis)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Get(LIBMATTI_MC_Direction_AxisDirection axisDirection,
                                                LIBMATTI_MC_Direction_Axis axis);

// Java: public Direction.Axis getAxis()
LIBMATTI_MC_Direction_Axis LIBMATTI_MC_Direction_GetAxis(LIBMATTI_MC_Direction direction);
// Java: public Direction.AxisDirection getAxisDirection()
LIBMATTI_MC_Direction_AxisDirection LIBMATTI_MC_Direction_GetAxisDirection(LIBMATTI_MC_Direction direction);

// Java: public Direction getOpposite()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetOpposite(LIBMATTI_MC_Direction direction);
// Java: public Direction getClockWise()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetClockWise(LIBMATTI_MC_Direction direction);
// Java: public Direction getCounterClockWise()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetCounterClockWise(LIBMATTI_MC_Direction direction);
// Java: public Direction getClockWise(Axis) / getCounterClockWise(Axis)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetClockWiseAround(LIBMATTI_MC_Direction direction,
                                                               LIBMATTI_MC_Direction_Axis axis);
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetCounterClockWiseAround(LIBMATTI_MC_Direction direction,
                                                                      LIBMATTI_MC_Direction_Axis axis);

// Java: public int getStepX() / getStepY() / getStepZ()
int LIBMATTI_MC_Direction_GetStepX(LIBMATTI_MC_Direction direction);
int LIBMATTI_MC_Direction_GetStepY(LIBMATTI_MC_Direction direction);
int LIBMATTI_MC_Direction_GetStepZ(LIBMATTI_MC_Direction direction);

// Java: public Vec3i getUnitVec3i() - the shared (0/±1) normal vector
const LIBMATTI_MC_Vec3i *LIBMATTI_MC_Direction_GetUnitVec3i(LIBMATTI_MC_Direction direction);

// Java: public float toYRot()
float LIBMATTI_MC_Direction_ToYRot(LIBMATTI_MC_Direction direction);
// Java: public static float getYRot(Direction) - aborts on vertical directions
float LIBMATTI_MC_Direction_GetYRot(LIBMATTI_MC_Direction direction);

// Java: public static Direction getApproximateNearest(float, float, float)
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetApproximateNearest(float x, float y, float z);
// Java: public static Direction getNearest(int, int, int, Direction) - the fallback when tied
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_GetNearest(int x, int y, int z, LIBMATTI_MC_Direction fallback);

// Java: public boolean isFacingAngle(float)
int LIBMATTI_MC_Direction_IsFacingAngle(LIBMATTI_MC_Direction direction, float angle);

// Java: Axis.isVertical() / isHorizontal()
int LIBMATTI_MC_Direction_Axis_IsVertical(LIBMATTI_MC_Direction_Axis axis);
int LIBMATTI_MC_Direction_Axis_IsHorizontal(LIBMATTI_MC_Direction_Axis axis);
// Java: Axis.getName()
const char *LIBMATTI_MC_Direction_Axis_GetName(LIBMATTI_MC_Direction_Axis axis);
// Java: Axis.byName(String) - found = 0 when unknown
LIBMATTI_MC_Direction_Axis LIBMATTI_MC_Direction_Axis_ByName(const char *name, int *found);
// Java: Axis.choose(int, int, int) / choose(double, double, double)
int LIBMATTI_MC_Direction_Axis_ChooseI(LIBMATTI_MC_Direction_Axis axis, int x, int y, int z);
double LIBMATTI_MC_Direction_Axis_ChooseD(LIBMATTI_MC_Direction_Axis axis, double x, double y, double z);
// Java: Axis.getPositive() / getNegative()
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Axis_GetPositive(LIBMATTI_MC_Direction_Axis axis);
LIBMATTI_MC_Direction LIBMATTI_MC_Direction_Axis_GetNegative(LIBMATTI_MC_Direction_Axis axis);
// Java: Axis.getPlane()
LIBMATTI_MC_Direction_Plane LIBMATTI_MC_Direction_Axis_GetPlane(LIBMATTI_MC_Direction_Axis axis);

// Java: Plane.faces - HORIZONTAL = [NORTH, EAST, SOUTH, WEST], VERTICAL = [UP, DOWN]
const LIBMATTI_MC_Direction *LIBMATTI_MC_Direction_Plane_Faces(LIBMATTI_MC_Direction_Plane plane, size_t *count);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CORE_DIRECTION_H
