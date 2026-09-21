// Port of net.minecraft.world.phys.Vec3.

#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_New(double x, double y, double z)
{
    LIBMATTI_MC_Vec3 *v = malloc(sizeof(LIBMATTI_MC_Vec3));
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

// Java: public Vec3(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_FromVec3i(const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_Vec3_New(v->x, v->y, v->z);
}

// Java: public static final Vec3 ZERO = new Vec3(0.0, 0.0, 0.0)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ZERO(void)
{
    static LIBMATTI_MC_Vec3 zero = {0.0, 0.0, 0.0};
    return &zero;
}

// Java: public static final Vec3 X_AXIS = new Vec3(1.0, 0.0, 0.0)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_X_AXIS(void)
{
    static LIBMATTI_MC_Vec3 axis = {1.0, 0.0, 0.0};
    return &axis;
}

// Java: public static final Vec3 Y_AXIS = new Vec3(0.0, 1.0, 0.0)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Y_AXIS(void)
{
    static LIBMATTI_MC_Vec3 axis = {0.0, 1.0, 0.0};
    return &axis;
}

// Java: public static final Vec3 Z_AXIS = new Vec3(0.0, 0.0, 1.0)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Z_AXIS(void)
{
    static LIBMATTI_MC_Vec3 axis = {0.0, 0.0, 1.0};
    return &axis;
}

// Java: public static Vec3 atLowerCornerOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtLowerCornerOf(const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_Vec3_New(v->x, v->y, v->z);
}

// Java: public static Vec3 atLowerCornerWithOffset(Vec3i, double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtLowerCornerWithOffset(const LIBMATTI_MC_Vec3i *v, double dx, double dy, double dz)
{
    return LIBMATTI_MC_Vec3_New(v->x + dx, v->y + dy, v->z + dz);
}

// Java: public static Vec3 atCenterOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtCenterOf(const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_Vec3_AtLowerCornerWithOffset(v, 0.5, 0.5, 0.5);
}

// Java: public static Vec3 atBottomCenterOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtBottomCenterOf(const LIBMATTI_MC_Vec3i *v)
{
    return LIBMATTI_MC_Vec3_AtLowerCornerWithOffset(v, 0.5, 0.0, 0.5);
}

// Java: public static Vec3 upFromBottomCenterOf(Vec3i, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_UpFromBottomCenterOf(const LIBMATTI_MC_Vec3i *v, double up)
{
    return LIBMATTI_MC_Vec3_AtLowerCornerWithOffset(v, 0.5, up, 0.5);
}

// Java: public Vec3 vectorTo(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_VectorTo(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *to)
{
    return LIBMATTI_MC_Vec3_New(to->x - v->x, to->y - v->y, to->z - v->z);
}

// Java: public Vec3 normalize()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Normalize(const LIBMATTI_MC_Vec3 *v)
{
    double d0 = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    return d0 < 1.0E-5 ? LIBMATTI_MC_Vec3_New(0.0, 0.0, 0.0)
                       : LIBMATTI_MC_Vec3_New(v->x / d0, v->y / d0, v->z / d0);
}

// Java: public double dot(Vec3)
double LIBMATTI_MC_Vec3_Dot(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Java: public Vec3 cross(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Cross(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b)
{
    return LIBMATTI_MC_Vec3_New(a->y * b->z - a->z * b->y,
                                a->z * b->x - a->x * b->z,
                                a->x * b->y - a->y * b->x);
}

// Java: public Vec3 subtract(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Subtract(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other)
{
    return LIBMATTI_MC_Vec3_Subtract3(v, other->x, other->y, other->z);
}

// Java: public Vec3 subtract(double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_SubtractScalar(const LIBMATTI_MC_Vec3 *v, double scalar)
{
    return LIBMATTI_MC_Vec3_Subtract3(v, scalar, scalar, scalar);
}

// Java: public Vec3 subtract(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Subtract3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z)
{
    return LIBMATTI_MC_Vec3_Add3(v, -x, -y, -z);
}

// Java: public Vec3 add(double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AddScalar(const LIBMATTI_MC_Vec3 *v, double scalar)
{
    return LIBMATTI_MC_Vec3_Add3(v, scalar, scalar, scalar);
}

// Java: public Vec3 add(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Add(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other)
{
    return LIBMATTI_MC_Vec3_Add3(v, other->x, other->y, other->z);
}

// Java: public Vec3 add(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Add3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z)
{
    return LIBMATTI_MC_Vec3_New(v->x + x, v->y + y, v->z + z);
}

// Java: public double distanceTo(Vec3)
double LIBMATTI_MC_Vec3_DistanceTo(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b)
{
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    double dz = b->z - a->z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

// Java: public double distanceToSqr(Vec3)
double LIBMATTI_MC_Vec3_DistanceToSqr(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b)
{
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    double dz = b->z - a->z;
    return dx * dx + dy * dy + dz * dz;
}

// Java: public double distanceToSqr(double, double, double)
double LIBMATTI_MC_Vec3_DistanceToSqr3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z)
{
    double dx = x - v->x;
    double dy = y - v->y;
    double dz = z - v->z;
    return dx * dx + dy * dy + dz * dz;
}

// Java: public boolean closerThan(Position, double)
int LIBMATTI_MC_Vec3_CloserThan(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other, double max)
{
    return LIBMATTI_MC_Vec3_DistanceToSqr3(v, other->x, other->y, other->z) < max * max;
}

// Java: public boolean closerThan(Vec3, double horizontal, double vertical)
int LIBMATTI_MC_Vec3_CloserThanCylinder(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other,
                                        double horizontal, double vertical)
{
    double dx = other->x - v->x;
    double dy = other->y - v->y;
    double dz = other->z - v->z;
    return LIBMATTI_MC_Mth_LengthSquared(dx, dz) < LIBMATTI_MC_Mth_SquareD(horizontal)
        && fabs(dy) < vertical;
}

// Java: public Vec3 scale(double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Scale(const LIBMATTI_MC_Vec3 *v, double factor)
{
    return LIBMATTI_MC_Vec3_Multiply3(v, factor, factor, factor);
}

// Java: public Vec3 reverse()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Reverse(const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_Vec3_Scale(v, -1.0);
}

// Java: public Vec3 multiply(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Multiply(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other)
{
    return LIBMATTI_MC_Vec3_Multiply3(v, other->x, other->y, other->z);
}

// Java: public Vec3 multiply(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Multiply3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z)
{
    return LIBMATTI_MC_Vec3_New(v->x * x, v->y * y, v->z * z);
}

// Java: public Vec3 horizontal()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Horizontal(const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_Vec3_New(v->x, 0.0, v->z);
}

// Java: public double length()
double LIBMATTI_MC_Vec3_Length(const LIBMATTI_MC_Vec3 *v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

// Java: public double lengthSqr()
double LIBMATTI_MC_Vec3_LengthSqr(const LIBMATTI_MC_Vec3 *v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

// Java: public double horizontalDistance()
double LIBMATTI_MC_Vec3_HorizontalDistance(const LIBMATTI_MC_Vec3 *v)
{
    return sqrt(v->x * v->x + v->z * v->z);
}

// Java: public double horizontalDistanceSqr()
double LIBMATTI_MC_Vec3_HorizontalDistanceSqr(const LIBMATTI_MC_Vec3 *v)
{
    return v->x * v->x + v->z * v->z;
}

// Java: public Vec3 lerp(Vec3, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Lerp(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to, double delta)
{
    return LIBMATTI_MC_Vec3_New(LIBMATTI_MC_Mth_LerpD(delta, from->x, to->x),
                                LIBMATTI_MC_Mth_LerpD(delta, from->y, to->y),
                                LIBMATTI_MC_Mth_LerpD(delta, from->z, to->z));
}

// Java: public Vec3 xRot(float)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_XRot(const LIBMATTI_MC_Vec3 *v, float radians)
{
    float f = LIBMATTI_MC_Mth_Cos(radians);
    float f1 = LIBMATTI_MC_Mth_Sin(radians);
    double x = v->x;
    double y = v->y * (double) f + v->z * (double) f1;
    double z = v->z * (double) f - v->y * (double) f1;
    return LIBMATTI_MC_Vec3_New(x, y, z);
}

// Java: public Vec3 yRot(float)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_YRot(const LIBMATTI_MC_Vec3 *v, float radians)
{
    float f = LIBMATTI_MC_Mth_Cos(radians);
    float f1 = LIBMATTI_MC_Mth_Sin(radians);
    double x = v->x * (double) f + v->z * (double) f1;
    double y = v->y;
    double z = v->z * (double) f - v->x * (double) f1;
    return LIBMATTI_MC_Vec3_New(x, y, z);
}

// Java: public Vec3 zRot(float)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ZRot(const LIBMATTI_MC_Vec3 *v, float radians)
{
    float f = LIBMATTI_MC_Mth_Cos(radians);
    float f1 = LIBMATTI_MC_Mth_Sin(radians);
    double x = v->x * (double) f + v->y * (double) f1;
    double y = v->y * (double) f - v->x * (double) f1;
    double z = v->z;
    return LIBMATTI_MC_Vec3_New(x, y, z);
}

// Java: public Vec3 rotateClockwise90()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_RotateClockwise90(const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_Vec3_New(-v->z, v->y, v->x);
}

// Java: public static Vec3 directionFromRotation(float pitch, float yaw)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_DirectionFromRotation(float pitch, float yaw)
{
    float f = LIBMATTI_MC_Mth_Cos(-yaw * (float) (M_PI / 180.0) - (float) M_PI);
    float f1 = LIBMATTI_MC_Mth_Sin(-yaw * (float) (M_PI / 180.0) - (float) M_PI);
    float f2 = -LIBMATTI_MC_Mth_Cos(-pitch * (float) (M_PI / 180.0));
    float f3 = LIBMATTI_MC_Mth_Sin(-pitch * (float) (M_PI / 180.0));
    return LIBMATTI_MC_Vec3_New((double) (f1 * f2), (double) f3, (double) (f * f2));
}

// Java: public Vec3 align(EnumSet<Direction.Axis>)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Align(const LIBMATTI_MC_Vec3 *v, int axisBits)
{
    double x = (axisBits & LIBMATTI_MC_Vec3_AXIS_X_BIT) ? (double) LIBMATTI_MC_Mth_FloorD(v->x) : v->x;
    double y = (axisBits & LIBMATTI_MC_Vec3_AXIS_Y_BIT) ? (double) LIBMATTI_MC_Mth_FloorD(v->y) : v->y;
    double z = (axisBits & LIBMATTI_MC_Vec3_AXIS_Z_BIT) ? (double) LIBMATTI_MC_Mth_FloorD(v->z) : v->z;
    return LIBMATTI_MC_Vec3_New(x, y, z);
}

// Java: public double get(Axis)
double LIBMATTI_MC_Vec3_Get(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction_Axis axis)
{
    return LIBMATTI_MC_Direction_Axis_ChooseD(axis, v->x, v->y, v->z);
}

// Java: public Vec3 with(Axis, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_With(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction_Axis axis, double value)
{
    double x = axis == LIBMATTI_MC_Direction_Axis_X ? value : v->x;
    double y = axis == LIBMATTI_MC_Direction_Axis_Y ? value : v->y;
    double z = axis == LIBMATTI_MC_Direction_Axis_Z ? value : v->z;
    return LIBMATTI_MC_Vec3_New(x, y, z);
}

// Java: public Vec3 relative(Direction, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Relative(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction direction, double distance)
{
    const LIBMATTI_MC_Vec3i *normal = LIBMATTI_MC_Direction_GetUnitVec3i(direction);
    return LIBMATTI_MC_Vec3_New(v->x + distance * normal->x,
                                v->y + distance * normal->y,
                                v->z + distance * normal->z);
}

// Java: public Vec3 projectedOn(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ProjectedOn(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *onto)
{
    if (LIBMATTI_MC_Vec3_LengthSqr(onto) == 0.0)
        return LIBMATTI_MC_Vec3_New(onto->x, onto->y, onto->z);
    return LIBMATTI_MC_Vec3_Scale(LIBMATTI_MC_Vec3_Scale(onto, LIBMATTI_MC_Vec3_Dot(v, onto)),
                                  1.0 / LIBMATTI_MC_Vec3_LengthSqr(onto));
}

// Java: public boolean isFinite()
int LIBMATTI_MC_Vec3_IsFinite(const LIBMATTI_MC_Vec3 *v)
{
    return isfinite(v->x) && isfinite(v->y) && isfinite(v->z);
}

// Java: public boolean equals(Object)
int LIBMATTI_MC_Vec3_Equals(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b)
{
    if (a == b)
        return 1;
    if (a == NULL || b == NULL)
        return 0;
    return a->x == b->x && a->y == b->y && a->z == b->z;
}
