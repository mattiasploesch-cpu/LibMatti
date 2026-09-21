// Port of net.minecraft.world.phys.Vec3 (the immutable double triple).
// Java allocates a fresh object per operation; the C port keeps the same semantics -
// every operation returns a new heap object the caller owns.

#ifndef MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_VEC3_H
#define MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_VEC3_H

#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/core/Vec3i.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class Vec3 implements Position
typedef struct LIBMATTI_MC_Vec3
{
    // Java: public final double x / y / z
    double x;
    double y;
    double z;
} LIBMATTI_MC_Vec3;

// Java: public Vec3(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_New(double x, double y, double z);
// Java: public Vec3(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_FromVec3i(const LIBMATTI_MC_Vec3i *v);

// Java: public static final Vec3 ZERO / X_AXIS / Y_AXIS / Z_AXIS
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ZERO(void);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_X_AXIS(void);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Y_AXIS(void);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Z_AXIS(void);

// Java: public static Vec3 atLowerCornerOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtLowerCornerOf(const LIBMATTI_MC_Vec3i *v);
// Java: public static Vec3 atLowerCornerWithOffset(Vec3i, double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtLowerCornerWithOffset(const LIBMATTI_MC_Vec3i *v, double dx, double dy, double dz);
// Java: public static Vec3 atCenterOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtCenterOf(const LIBMATTI_MC_Vec3i *v);
// Java: public static Vec3 atBottomCenterOf(Vec3i)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AtBottomCenterOf(const LIBMATTI_MC_Vec3i *v);
// Java: public static Vec3 upFromBottomCenterOf(Vec3i, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_UpFromBottomCenterOf(const LIBMATTI_MC_Vec3i *v, double up);

// Java: public Vec3 vectorTo(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_VectorTo(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *to);
// Java: public Vec3 normalize()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Normalize(const LIBMATTI_MC_Vec3 *v);
// Java: public double dot(Vec3)
double LIBMATTI_MC_Vec3_Dot(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b);
// Java: public Vec3 cross(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Cross(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b);

// Java: public Vec3 subtract(Vec3) / subtract(double) / subtract(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Subtract(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_SubtractScalar(const LIBMATTI_MC_Vec3 *v, double scalar);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Subtract3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z);
// Java: public Vec3 add(double) / add(Vec3) / add(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_AddScalar(const LIBMATTI_MC_Vec3 *v, double scalar);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Add(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Add3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z);

// Java: public double distanceTo(Vec3)
double LIBMATTI_MC_Vec3_DistanceTo(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b);
// Java: public double distanceToSqr(Vec3) / distanceToSqr(double, double, double)
double LIBMATTI_MC_Vec3_DistanceToSqr(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b);
double LIBMATTI_MC_Vec3_DistanceToSqr3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z);
// Java: public boolean closerThan(Position, double)
int LIBMATTI_MC_Vec3_CloserThan(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other, double max);
// Java: public boolean closerThan(Vec3, double horizontal, double vertical)
int LIBMATTI_MC_Vec3_CloserThanCylinder(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other,
                                        double horizontal, double vertical);

// Java: public Vec3 scale(double) / reverse()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Scale(const LIBMATTI_MC_Vec3 *v, double factor);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Reverse(const LIBMATTI_MC_Vec3 *v);
// Java: public Vec3 multiply(Vec3) / multiply(double, double, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Multiply(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *other);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Multiply3(const LIBMATTI_MC_Vec3 *v, double x, double y, double z);

// Java: public Vec3 horizontal() - (x, 0, z)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Horizontal(const LIBMATTI_MC_Vec3 *v);
// Java: public double length() / lengthSqr()
double LIBMATTI_MC_Vec3_Length(const LIBMATTI_MC_Vec3 *v);
double LIBMATTI_MC_Vec3_LengthSqr(const LIBMATTI_MC_Vec3 *v);
// Java: public double horizontalDistance() / horizontalDistanceSqr()
double LIBMATTI_MC_Vec3_HorizontalDistance(const LIBMATTI_MC_Vec3 *v);
double LIBMATTI_MC_Vec3_HorizontalDistanceSqr(const LIBMATTI_MC_Vec3 *v);

// Java: public Vec3 lerp(Vec3, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Lerp(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to, double delta);

// Java: public Vec3 xRot(float) / yRot(float) / zRot(float)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_XRot(const LIBMATTI_MC_Vec3 *v, float radians);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_YRot(const LIBMATTI_MC_Vec3 *v, float radians);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ZRot(const LIBMATTI_MC_Vec3 *v, float radians);
// Java: public Vec3 rotateClockwise90()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_RotateClockwise90(const LIBMATTI_MC_Vec3 *v);

// Java: public static Vec3 directionFromRotation(float pitch, float yaw)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_DirectionFromRotation(float pitch, float yaw);

// Java: public Vec3 align(EnumSet<Direction.Axis>) - the port takes a bitmask of Axis bits
#define LIBMATTI_MC_Vec3_AXIS_X_BIT 1
#define LIBMATTI_MC_Vec3_AXIS_Y_BIT 2
#define LIBMATTI_MC_Vec3_AXIS_Z_BIT 4
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Align(const LIBMATTI_MC_Vec3 *v, int axisBits);

// Java: public double get(Axis)
double LIBMATTI_MC_Vec3_Get(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction_Axis axis);
// Java: public Vec3 with(Axis, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_With(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction_Axis axis, double value);
// Java: public Vec3 relative(Direction, double)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_Relative(const LIBMATTI_MC_Vec3 *v, LIBMATTI_MC_Direction direction, double distance);

// Java: public Vec3 projectedOn(Vec3)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Vec3_ProjectedOn(const LIBMATTI_MC_Vec3 *v, const LIBMATTI_MC_Vec3 *onto);
// Java: public boolean isFinite()
int LIBMATTI_MC_Vec3_IsFinite(const LIBMATTI_MC_Vec3 *v);

// Java: public boolean equals(Object)
int LIBMATTI_MC_Vec3_Equals(const LIBMATTI_MC_Vec3 *a, const LIBMATTI_MC_Vec3 *b);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_VEC3_H
