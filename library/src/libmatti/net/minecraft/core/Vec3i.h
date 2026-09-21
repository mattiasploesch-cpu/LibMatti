// Port of net.minecraft.core.Vec3i - the immutable integer triple BlockPos and
// ChunkPos build on. Java carries x/y/z as final ints; the C port keeps the same
// struct layout so the accessors are the Java methods.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_VEC3I_H
#define MATTICRAFT_NET_MINECRAFT_CORE_VEC3I_H

// Java: public class Vec3i
typedef struct LIBMATTI_MC_Vec3i
{
    // Java: private final int x / y / z
    int x;
    int y;
    int z;
} LIBMATTI_MC_Vec3i;

// Java: the record-like constructor
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_New(int x, int y, int z);
// Java: the ZERO constant - one shared instance (Java: public static final Vec3i ZERO)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_ZERO(void);

// Java: public int getX() / getY() / getZ()
int LIBMATTI_MC_Vec3i_GetX(const LIBMATTI_MC_Vec3i *v);
int LIBMATTI_MC_Vec3i_GetY(const LIBMATTI_MC_Vec3i *v);
int LIBMATTI_MC_Vec3i_GetZ(const LIBMATTI_MC_Vec3i *v);

// Java: public Vec3i offset(int, int, int)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Offset(const LIBMATTI_MC_Vec3i *v, int dx, int dy, int dz);
// Java: public Vec3i subtract(Vec3i)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Subtract(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other);
// Java: public Vec3i multiply(int)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Multiply(const LIBMATTI_MC_Vec3i *v, int factor);

// Java: public double distSqr(Vec3i)
double LIBMATTI_MC_Vec3i_DistSqr(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other);
// Java: public int distManhattan(Vec3i)
int LIBMATTI_MC_Vec3i_DistManhattan(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other);
// Java: public int distChebyshev(Vec3i)
int LIBMATTI_MC_Vec3i_DistChebyshev(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other);

// Java: public Vec3i cross(Vec3i)
LIBMATTI_MC_Vec3i *LIBMATTI_MC_Vec3i_Cross(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other);

// Java: public boolean closerThan(Vec3i, double)
int LIBMATTI_MC_Vec3i_CloserThan(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other, double max);
// Java: public boolean closerThan(Vec3i, double, double, double)
int LIBMATTI_MC_Vec3i_CloserThan3(const LIBMATTI_MC_Vec3i *v, const LIBMATTI_MC_Vec3i *other,
                                  double dx, double dy, double dz);

// Java: public boolean equals(Object) - component equality
int LIBMATTI_MC_Vec3i_Equals(const LIBMATTI_MC_Vec3i *a, const LIBMATTI_MC_Vec3i *b);
// Java: public int hashCode()
int LIBMATTI_MC_Vec3i_HashCode(const LIBMATTI_MC_Vec3i *v);

#endif //MATTICRAFT_NET_MINECRAFT_CORE_VEC3I_H
