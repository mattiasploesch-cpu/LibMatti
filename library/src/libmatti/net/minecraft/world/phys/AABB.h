// Port of net.minecraft.world.phys.AABB (the immutable axis aligned bounding box).
// Every mutating method returns a fresh box like Java; the clip ray-cast carries the
// hit direction so the BlockHitResult side can be built on top.

#ifndef MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_AABB_H
#define MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_AABB_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class AABB
typedef struct LIBMATTI_MC_AABB
{
    // Java: public final double minX / minY / minZ / maxX / maxY / maxZ
    double minX;
    double minY;
    double minZ;
    double maxX;
    double maxY;
    double maxZ;
} LIBMATTI_MC_AABB;

// Java: public AABB(double, double, double, double, double, double) - min/max normalised
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_New(double x1, double y1, double z1, double x2, double y2, double z2);
// Java: public AABB(BlockPos) - the unit cube of the block
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_FromBlockPos(const LIBMATTI_MC_BlockPos *pos);
// Java: public AABB(Vec3, Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_FromCorners(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to);
// Java: public static AABB unitCubeFromLowerCorner(Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_UnitCubeFromLowerCorner(const LIBMATTI_MC_Vec3 *lowerCorner);
// Java: public static AABB encapsulatingFullBlocks(BlockPos, BlockPos)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_EncapsulatingFullBlocks(const LIBMATTI_MC_BlockPos *a, const LIBMATTI_MC_BlockPos *b);
// Java: public static AABB ofSize(Vec3 center, double x, double y, double z)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_OfSize(const LIBMATTI_MC_Vec3 *center, double x, double y, double z);

// Java: public AABB setMinX/Y/Z(double) / setMaxX/Y/Z(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinX(const LIBMATTI_MC_AABB *box, double value);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinY(const LIBMATTI_MC_AABB *box, double value);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMinZ(const LIBMATTI_MC_AABB *box, double value);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxX(const LIBMATTI_MC_AABB *box, double value);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxY(const LIBMATTI_MC_AABB *box, double value);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_SetMaxZ(const LIBMATTI_MC_AABB *box, double value);

// Java: public double min(Axis) / max(Axis)
double LIBMATTI_MC_AABB_Min(const LIBMATTI_MC_AABB *box, LIBMATTI_MC_Direction_Axis axis);
double LIBMATTI_MC_AABB_Max(const LIBMATTI_MC_AABB *box, LIBMATTI_MC_Direction_Axis axis);

// Java: public AABB contract(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Contract(const LIBMATTI_MC_AABB *box, double x, double y, double z);
// Java: public AABB expandTowards(Vec3) / expandTowards(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_ExpandTowards(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *towards);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_ExpandTowards3(const LIBMATTI_MC_AABB *box, double x, double y, double z);
// Java: public AABB inflate(double, double, double) / inflate(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Inflate3(const LIBMATTI_MC_AABB *box, double x, double y, double z);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Inflate(const LIBMATTI_MC_AABB *box, double value);
// Java: public AABB deflate(double, double, double) / deflate(double)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Deflate3(const LIBMATTI_MC_AABB *box, double x, double y, double z);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Deflate(const LIBMATTI_MC_AABB *box, double value);

// Java: public AABB intersect(AABB)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Intersect(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b);
// Java: public AABB minmax(AABB)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Minmax(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b);
// Java: public AABB move(double, double, double) / move(BlockPos) / move(Vec3)
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_Move3(const LIBMATTI_MC_AABB *box, double x, double y, double z);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_MoveBlockPos(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_BlockPos *pos);
LIBMATTI_MC_AABB *LIBMATTI_MC_AABB_MoveVec(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v);

// Java: public boolean intersects(AABB) / intersects(double, ..., double) / intersects(BlockPos)
int LIBMATTI_MC_AABB_Intersects(const LIBMATTI_MC_AABB *a, const LIBMATTI_MC_AABB *b);
int LIBMATTI_MC_AABB_Intersects6(const LIBMATTI_MC_AABB *box, double x1, double y1, double z1,
                                 double x2, double y2, double z2);
int LIBMATTI_MC_AABB_IntersectsBlockPos(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_BlockPos *pos);

// Java: public boolean contains(Vec3) / contains(double, double, double)
int LIBMATTI_MC_AABB_Contains(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v);
int LIBMATTI_MC_AABB_Contains3(const LIBMATTI_MC_AABB *box, double x, double y, double z);

// Java: public double getSize() / getXsize() / getYsize() / getZsize()
double LIBMATTI_MC_AABB_GetSize(const LIBMATTI_MC_AABB *box);
double LIBMATTI_MC_AABB_GetXsize(const LIBMATTI_MC_AABB *box);
double LIBMATTI_MC_AABB_GetYsize(const LIBMATTI_MC_AABB *box);
double LIBMATTI_MC_AABB_GetZsize(const LIBMATTI_MC_AABB *box);

// Java: public Optional<Vec3> clip(Vec3 from, Vec3 to) - found = 0 when the ray misses
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_Clip(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *from,
                                        const LIBMATTI_MC_Vec3 *to, int *found);
// Java: the clip hit direction (the block face the ray entered through); -1 when no hit
int LIBMATTI_MC_AABB_ClipDirection(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *from,
                                   const LIBMATTI_MC_Vec3 *to);

// Java: public double distanceToSqr(Vec3)
double LIBMATTI_MC_AABB_DistanceToSqr(const LIBMATTI_MC_AABB *box, const LIBMATTI_MC_Vec3 *v);

// Java: public boolean hasNaN()
int LIBMATTI_MC_AABB_HasNaN(const LIBMATTI_MC_AABB *box);
// Java: public Vec3 getCenter() / getBottomCenter() / getMinPosition() / getMaxPosition()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetCenter(const LIBMATTI_MC_AABB *box);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetBottomCenter(const LIBMATTI_MC_AABB *box);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetMinPosition(const LIBMATTI_MC_AABB *box);
LIBMATTI_MC_Vec3 *LIBMATTI_MC_AABB_GetMaxPosition(const LIBMATTI_MC_AABB *box);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_AABB_H
