// Port of net.minecraft.world.phys.AABB.

#include "libmatti/net/minecraft/world/phys/AABB.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_New(double x1, double y1, double z1, double x2, double y2, double z2)
{
    LIBMATTI_MC_AABB *box = malloc(sizeof(LIBMATTI_MC_AABB));
    box->minX = fmin(x1, x2);
    box->minY = fmin(y1, y2);
    box->minZ = fmin(z1, z2);
    box->maxX = fmax(x1, x2);
    box->maxY = fmax(y1, y2);
    box->maxZ = fmax(z1, z2);
    return box;
}

// Java: public AABB(BlockPos) - (x, y, z, x + 1, y + 1, z + 1)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_FromBlockPos(const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_AABB_New(pos->base.x, pos->base.y, pos->base.z,
                                pos->base.x + 1, pos->base.y + 1, pos->base.z + 1);
}

// Java: public AABB(Vec3, Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_FromCorners(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to)
{
    return LIBMATTI_MC_AABB_New(from->x, from->y, from->z, to->x, to->y, to->z);
}

// Java: public static AABB unitCubeFromLowerCorner(Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_UnitCubeFromLowerCorner(const LIBMATTI_MC_Vec3 *lowerCorner)
{
    return LIBMATTI_MC_AABB_New(lowerCorner->x, lowerCorner->y, lowerCorner->z,
                                lowerCorner->x + 1.0, lowerCorner->y + 1.0, lowerCorner->z + 1.0);
}

// Java: public static AABB encapsulatingFullBlocks(BlockPos, BlockPos)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_EncapsulatingFullBlocks(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b)
{
    return LIBMATTI_MC_AABB_New(fmin(a->base.x, b->base.x), fmin(a->base.y, b->base.y), fmin(a->base.z, b->base.z),
                                fmax(a->base.x, b->base.x) + 1, fmax(a->base.y, b->base.y) + 1,
                                fmax(a->base.z, b->base.z) + 1);
}

// Java: public static AABB ofSize(Vec3 center, double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_OfSize(const LIBMATTI_MC_Vec3 *center, double x, double y, double z)
{
    return LIBMATTI_MC_AABB_New(center->x - x / 2.0, center->y - y / 2.0, center->z - z / 2.0,
                                center->x + x / 2.0, center->y + y / 2.0, center->z + z / 2.0);
}

// Java: public AABB setMinX(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinX(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(value, box->minY, box->minZ, box->maxX, box->maxY, box->maxZ);
}

// Java: public AABB setMinY(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinY(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(box->minX, value, box->minZ, box->maxX, box->maxY, box->maxZ);
}

// Java: public AABB setMinZ(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinZ(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(box->minX, box->minY, value, box->maxX, box->maxY, box->maxZ);
}

// Java: public AABB setMaxX(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxX(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(box->minX, box->minY, box->minZ, value, box->maxY, box->maxZ);
}

// Java: public AABB setMaxY(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxY(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(box->minX, box->minY, box->minZ, box->maxX, value, box->maxZ);
}

// Java: public AABB setMaxZ(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxZ(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_New(box->minX, box->minY, box->minZ, box->maxX, box->maxY, value);
}

// Java: public double min(Axis)
double LIBMATTI_MC_AABB_Min(const LIBMATTI_MC_AABB *box, LIBMATTI_MC_Direction_Axis axis)
{
    return LIBMATTI_MC_Direction_Axis_ChooseD(axis, box->minX, box->minY, box->minZ);
}

// Java: public double max(Axis)
double LIBMATTI_MC_AABB_Max(const LIBMATTI_MC_AABB *box, LIBMATTI_MC_Direction_Axis axis)
{
    return LIBMATTI_MC_Direction_Axis_ChooseD(axis, box->maxX, box->maxY, box->maxZ);
}

// Java: public AABB contract(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Contract(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    double minX = box->minX;
    double minY = box->minY;
    double minZ = box->minZ;
    double maxX = box->maxX;
    double maxY = box->maxY;
    double maxZ = box->maxZ;
    if (x < 0.0)
        minX -= x;
    else if (x > 0.0)
        maxX -= x;
    if (y < 0.0)
        minY -= y;
    else if (y > 0.0)
        maxY -= y;
    if (z < 0.0)
        minZ -= z;
    else if (z > 0.0)
        maxZ -= z;
    return LIBMATTI_MC_AABB_New(minX, minY, minZ, maxX, maxY, maxZ);
}

// Java: public AABB expandTowards(Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_ExpandTowards(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *towards)
{
    return LIBMATTI_MC_AABB_ExpandTowards3(box, towards->x, towards->y, towards->z);
}

// Java: public AABB expandTowards(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_ExpandTowards3(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    double minX = box->minX;
    double minY = box->minY;
    double minZ = box->minZ;
    double maxX = box->maxX;
    double maxY = box->maxY;
    double maxZ = box->maxZ;
    if (x < 0.0)
        minX += x;
    else if (x > 0.0)
        maxX += x;
    if (y < 0.0)
        minY += y;
    else if (y > 0.0)
        maxY += y;
    if (z < 0.0)
        minZ += z;
    else if (z > 0.0)
        maxZ += z;
    return LIBMATTI_MC_AABB_New(minX, minY, minZ, maxX, maxY, maxZ);
}

// Java: public AABB inflate(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Inflate3(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    return LIBMATTI_MC_AABB_New(box->minX - x, box->minY - y, box->minZ - z,
                                box->maxX + x, box->maxY + y, box->maxZ + z);
}

// Java: public AABB inflate(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Inflate(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_Inflate3(box, value, value, value);
}

// Java: public AABB deflate(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Deflate3(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    return LIBMATTI_MC_AABB_Inflate3(box, -x, -y, -z);
}

// Java: public AABB deflate(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Deflate(const LIBMATTI_MC_AABB *box, double value)
{
    return LIBMATTI_MC_AABB_Inflate3(box, -value, -value, -value);
}

// Java: public AABB intersect(AABB)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Intersect(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b)
{
    return LIBMATTI_MC_AABB_New(fmax(a->minX, b->minX), fmax(a->minY, b->minY), fmax(a->minZ, b->minZ),
                                fmin(a->maxX, b->maxX), fmin(a->maxY, b->maxY), fmin(a->maxZ, b->maxZ));
}

// Java: public AABB minmax(AABB)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Minmax(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b)
{
    return LIBMATTI_MC_AABB_New(fmin(a->minX, b->minX), fmin(a->minY, b->minY), fmin(a->minZ, b->minZ),
                                fmax(a->maxX, b->maxX), fmax(a->maxY, b->maxY), fmax(a->maxZ, b->maxZ));
}

// Java: public AABB move(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Move3(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    return LIBMATTI_MC_AABB_New(box->minX + x, box->minY + y, box->minZ + z,
                                box->maxX + x, box->maxY + y, box->maxZ + z);
}

// Java: public AABB move(BlockPos)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_MoveBlockPos(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_AABB_Move3(box, pos->base.x, pos->base.y, pos->base.z);
}

// Java: public AABB move(Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_MoveVec(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_AABB_Move3(box, v->x, v->y, v->z);
}

// Java: public boolean intersects(AABB)
int LIBMATTI_MC_AABB_Intersects(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b)
{
    return LIBMATTI_MC_AABB_Intersects6(a, b->minX, b->minY, b->minZ, b->maxX, b->maxY, b->maxZ);
}

// Java: public boolean intersects(double, ..., double)
int LIBMATTI_MC_AABB_Intersects6(const LIBMATTI_MC_AABB *box, double x1, double y1, double z1,
                                 double x2, double y2, double z2)
{
    return box->minX < x2 && box->maxX > x1
        && box->minY < y2 && box->maxY > y1
        && box->minZ < z2 && box->maxZ > z1;
}

// Java: public boolean intersects(BlockPos)
int LIBMATTI_MC_AABB_IntersectsBlockPos(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_AABB_Intersects6(box, pos->base.x, pos->base.y, pos->base.z,
                                        pos->base.x + 1, pos->base.y + 1, pos->base.z + 1);
}

// Java: public boolean contains(Vec3)
int LIBMATTI_MC_AABB_Contains(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v)
{
    return LIBMATTI_MC_AABB_Contains3(box, v->x, v->y, v->z);
}

// Java: public boolean contains(double, double, double)
int LIBMATTI_MC_AABB_Contains3(const LIBMATTI_MC_AABB *box, double x, double y, double z)
{
    return x >= box->minX && x < box->maxX
        && y >= box->minY && y < box->maxY
        && z >= box->minZ && z < box->maxZ;
}

// Java: public double getSize()
double LIBMATTI_MC_AABB_GetSize(const LIBMATTI_MC_AABB *box)
{
    return (LIBMATTI_MC_AABB_GetXsize(box) + LIBMATTI_MC_AABB_GetYsize(box) + LIBMATTI_MC_AABB_GetZsize(box)) / 3.0;
}

// Java: public double getXsize() / getYsize() / getZsize()
double LIBMATTI_MC_AABB_GetXsize(const LIBMATTI_MC_AABB *box)
{
    return box->maxX - box->minX;
}

double LIBMATTI_MC_AABB_GetYsize(const LIBMATTI_MC_AABB *box)
{
    return box->maxY - box->minY;
}

double LIBMATTI_MC_AABB_GetZsize(const LIBMATTI_MC_AABB *box)
{
    return box->maxZ - box->minZ;
}

// Java: private static Direction clipPoint(double[] tMin, Direction current, double dx, double dy, double dz,
//       double planeMin, double planeOtherMin, double planeOtherMax, double planeOtherMin2, double planeOtherMax2,
//       Direction hitFace, double originX, double originY, double originZ)
// The Java parameter juggling collapses to: distance along the axis, the two transversal
// ranges, the face the ray enters through and the ray origin components.
static int clip_point(double *tMin, int current, double axisDelta, double transA, double transB,
                      double planeMin, double aMin, double aMax, double bMin, double bMax,
                      int hitFace, double originA, double originB)
{
    double t = planeMin / axisDelta;
    double hitA = originA + t * transA;
    double hitB = originB + t * transB;
    if (0.0 < t && t < *tMin && aMin - 1.0E-7 < hitA && hitA < aMax + 1.0E-7 && bMin - 1.0E-7 < hitB
        && hitB < bMax + 1.0E-7)
    {
        *tMin = t;
        return hitFace;
    }
    return current;
}

// Java: private static Direction getDirection(...) - the axis-by-axis entry scan
static int clip_direction(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *from, double *tMin,
                          int current, double dx, double dy, double dz)
{
    if (dx > 1.0E-7)
    {
        current = clip_point(tMin, current, dx, dy, dz, box->minX - from->x, box->minY, box->maxY,
                             box->minZ, box->maxZ, LIBMATTI_MC_Direction_WEST, from->y, from->z);
    }
    else if (dx < -1.0E-7)
    {
        current = clip_point(tMin, current, dx, dy, dz, box->maxX - from->x, box->minY, box->maxY,
                             box->minZ, box->maxZ, LIBMATTI_MC_Direction_EAST, from->y, from->z);
    }

    if (dy > 1.0E-7)
    {
        current = clip_point(tMin, current, dy, dz, dx, box->minY - from->y, box->minZ, box->maxZ,
                             box->minX, box->maxX, LIBMATTI_MC_Direction_DOWN, from->z, from->x);
    }
    else if (dy < -1.0E-7)
    {
        current = clip_point(tMin, current, dy, dz, dx, box->maxY - from->y, box->minZ, box->maxZ,
                             box->minX, box->maxX, LIBMATTI_MC_Direction_UP, from->z, from->x);
    }

    if (dz > 1.0E-7)
    {
        current = clip_point(tMin, current, dz, dx, dy, box->minZ - from->z, box->minX, box->maxX,
                             box->minY, box->maxY, LIBMATTI_MC_Direction_NORTH, from->x, from->y);
    }
    else if (dz < -1.0E-7)
    {
        current = clip_point(tMin, current, dz, dx, dy, box->maxZ - from->z, box->minX, box->maxX,
                             box->minY, box->maxY, LIBMATTI_MC_Direction_SOUTH, from->x, from->y);
    }

    return current;
}

// Java: public Optional<Vec3> clip(Vec3 from, Vec3 to)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_Clip(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *from,
                                        const LIBMATTI_MC_Vec3 *to, int *found)
{
    double tMin = 1.0;
    double dx = to->x - from->x;
    double dy = to->y - from->y;
    double dz = to->z - from->z;
    int direction = clip_direction(box, from, &tMin, -1, dx, dy, dz);
    if (direction < 0)
    {
        if (found != NULL) *found = 0;
        return NULL;
    }
    if (found != NULL) *found = 1;
    return LIBMATTI_MC_Vec3_Add3(from, tMin * dx, tMin * dy, tMin * dz);
}

// Java: the direction part of clip - the block face the ray entered through
int LIBMATTI_MC_AABB_ClipDirection(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *from,
                                   const LIBMATTI_MC_Vec3 *to)
{
    double tMin = 1.0;
    double dx = to->x - from->x;
    double dy = to->y - from->y;
    double dz = to->z - from->z;
    return clip_direction(box, from, &tMin, -1, dx, dy, dz);
}

// Java: public double distanceToSqr(Vec3)
double LIBMATTI_MC_AABB_DistanceToSqr(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v)
{
    double dx = fmax(fmax(box->minX - v->x, v->x - box->maxX), 0.0);
    double dy = fmax(fmax(box->minY - v->y, v->y - box->maxY), 0.0);
    double dz = fmax(fmax(box->minZ - v->z, v->z - box->maxZ), 0.0);
    return LIBMATTI_MC_Mth_LengthSquared3(dx, dy, dz);
}

// Java: public boolean hasNaN()
int LIBMATTI_MC_AABB_HasNaN(const LIBMATTI_MC_AABB *box)
{
    return isnan(box->minX) || isnan(box->minY) || isnan(box->minZ) || isnan(box->maxX)
        || isnan(box->maxY) || isnan(box->maxZ);
}

// Java: public Vec3 getCenter()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetCenter(const LIBMATTI_MC_AABB *box)
{
    return LIBMATTI_MC_Vec3_New(LIBMATTI_MC_Mth_LerpD(0.5, box->minX, box->maxX),
                                LIBMATTI_MC_Mth_LerpD(0.5, box->minY, box->maxY),
                                LIBMATTI_MC_Mth_LerpD(0.5, box->minZ, box->maxZ));
}

// Java: public Vec3 getBottomCenter()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetBottomCenter(const LIBMATTI_MC_AABB *box)
{
    return LIBMATTI_MC_Vec3_New(LIBMATTI_MC_Mth_LerpD(0.5, box->minX, box->maxX), box->minY,
                                LIBMATTI_MC_Mth_LerpD(0.5, box->minZ, box->maxZ));
}

// Java: public Vec3 getMinPosition() / getMaxPosition()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetMinPosition(const LIBMATTI_MC_AABB *box)
{
    return LIBMATTI_MC_Vec3_New(box->minX, box->minY, box->minZ);
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetMaxPosition(const LIBMATTI_MC_AABB *box)
{
    return LIBMATTI_MC_Vec3_New(box->maxX, box->maxY, box->maxZ);
}
