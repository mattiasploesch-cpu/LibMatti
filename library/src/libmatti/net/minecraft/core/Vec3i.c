// Port of net.minecraft.core.Vec3i.
// Java allocates a fresh object per operation (the type is immutable); the C port keeps
// the same semantics - every operation returns a new heap object the caller owns.

#include "libmatti/net/minecraft/core/Vec3i.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_New(int x, int y, int z)
{
    LIBMATTI_MC_Vec3i *v = malloc(sizeof(LIBMATTI_MC_Vec3i));
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

// Java: public static final Vec3i ZERO = new Vec3i(0, 0, 0)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_ZERO(void)
{
    static LIBMATTI_MC_Vec3i zero = {0, 0, 0};
    return &zero;
}

int LIBMATTI_MC_Vec3i_GetX(const LIBMATTI_MC_Vec3i *v)
{
    return v->x;
}

int LIBMATTI_MC_Vec3i_GetY(const LIBMATTI_MC_Vec3i *v)
{
    return v->y;
}

int LIBMATTI_MC_Vec3i_GetZ(const LIBMATTI_MC_Vec3i *v)
{
    return v->z;
}

// Java: public Vec3i offset(int dx, int dy, int dz)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Offset(const LIBMATTI_MC_Vec3i *v, int dx, int dy, int dz)
{
    return dx == 0 && dy == 0 && dz == 0 ? LIBMATTI_MC_Vec3i_New(v->x, v->y, v->z)
                                         : LIBMATTI_MC_Vec3i_New(v->x + dx, v->y + dy, v->z + dz);
}

// Java: public Vec3i subtract(Vec3i other)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Subtract(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other)
{
    return LIBMATTI_MC_Vec3i_Offset(v, -other->x, -other->y, -other->z);
}

// Java: public Vec3i multiply(int factor)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Multiply(const LIBMATTI_MC_Vec3i *v, int factor)
{
    if (factor == 1)
        return LIBMATTI_MC_Vec3i_New(v->x, v->y, v->z);
    return factor == 0 ? LIBMATTI_MC_Vec3i_New(0, 0, 0)
                       : LIBMATTI_MC_Vec3i_New(v->x * factor, v->y * factor, v->z * factor);
}

// Java: public double distSqr(Vec3i other)
double LIBMATTI_MC_Vec3i_DistSqr(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other)
{
    int dx = other->x - v->x;
    int dy = other->y - v->y;
    int dz = other->z - v->z;
    return (double) (dx * dx + dy * dy + dz * dz);
}

// Java: public int distManhattan(Vec3i other)
int LIBMATTI_MC_Vec3i_DistManhattan(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other)
{
    return LIBMATTI_MC_Mth_AbsI(other->x - v->x) + LIBMATTI_MC_Mth_AbsI(other->y - v->y)
         + LIBMATTI_MC_Mth_AbsI(other->z - v->z);
}

// Java: public int distChebyshev(Vec3i other)
int LIBMATTI_MC_Vec3i_DistChebyshev(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other)
{
    return LIBMATTI_MC_Mth_AbsMaxI(LIBMATTI_MC_Mth_AbsMaxI(LIBMATTI_MC_Mth_AbsI(other->x - v->x),
                                                           LIBMATTI_MC_Mth_AbsI(other->y - v->y)),
                                   LIBMATTI_MC_Mth_AbsI(other->z - v->z));
}

// Java: public Vec3i cross(Vec3i other)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Cross(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other)
{
    return LIBMATTI_MC_Vec3i_New(v->y * other->z - v->z * other->y,
                                 v->z * other->x - v->x * other->z,
                                 v->x * other->y - v->y * other->x);
}

// Java: public boolean closerThan(Vec3i other, double max)
int LIBMATTI_MC_Vec3i_CloserThan(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other, double max)
{
    return LIBMATTI_MC_Vec3i_DistSqr(v, other) < max * max;
}

// Java: public boolean closerThan(Vec3i other, double dx, double dy, double dz)
int LIBMATTI_MC_Vec3i_CloserThan3(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other,
                                  double dx, double dy, double dz)
{
    int ddx = other->x - v->x;
    int ddy = other->y - v->y;
    int ddz = other->z - v->z;
    return (double) (ddx * ddx + ddz * ddz) < dx * dx && (double) ddy < dy * dy
        && fabs((double) ddz) < dz;
}

// Java: public boolean equals(Object)
int LIBMATTI_MC_Vec3i_Equals(const LIBMATTI_MC_Vec3i *a, const LIBMATTI_MC_Vec3i *b)
{
    if (a == b)
        return 1;
    if (a == NULL || b == NULL)
        return 0;
    return a->x == b->x && a->y == b->y && a->z == b->z;
}

// Java: public int hashCode() - 31 * (y + 31 * (31 * x + z)) ... Java's actual mix
int LIBMATTI_MC_Vec3i_HashCode(const LIBMATTI_MC_Vec3i *v)
{
    int result = v->y + 31 * v->z;
    return v->x + 31 * result;
}
