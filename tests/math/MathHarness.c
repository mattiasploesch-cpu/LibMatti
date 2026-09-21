// Math harness: checks the Mth table behaviour, the packed BlockPos long round trip,
// the Vec3/BlockPos geometry, the Direction clockwise machinery, the ChunkPos packing
// and the AABB clip ray-cast with the exact Java expectations.

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/core/Vec3i.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/phys/AABB.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int checks = 0;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", what);
        exit(1);
    }
}

int main(void)
{
    // ------------------------------------------------------------------ Mth
    // Java: sin/cos go through the 65536-entry table - close to the libm but not exact
    check(fabs(LIBMATTI_MC_Mth_Sin(0.0)) < 1e-4, "sin(0)");
    check(fabs(LIBMATTI_MC_Mth_Sin(M_PI / 2) - 1.0f) < 1e-4, "sin(pi/2)");
    check(fabs(LIBMATTI_MC_Mth_Cos(0.0) - 1.0f) < 1e-4, "cos(0)");
    check(LIBMATTI_MC_Mth_FloorD(-1.5) == -2, "floor(-1.5)");
    check(LIBMATTI_MC_Mth_Floor(1.9f) == 1, "floor(1.9)");
    check(LIBMATTI_MC_Mth_CeilD(-1.5) == -1, "ceil(-1.5)");
    check(LIBMATTI_MC_Mth_Ceil(1.1f) == 2, "ceil(1.1)");
    check(LIBMATTI_MC_Mth_FloorDiv(7, 3) == 2 && LIBMATTI_MC_Mth_FloorDiv(-7, 3) == -3, "floorDiv");
    check(LIBMATTI_MC_Mth_PositiveModuloI(-1, 3) == 2, "positiveModulo(int)");
    check(LIBMATTI_MC_Mth_WrapDegreesI(350) == -10, "wrapDegrees(350)");
    check(LIBMATTI_MC_Mth_WrapDegreesI(-350) == 10, "wrapDegrees(-350)");
    check(LIBMATTI_MC_Mth_Clamp(5.0f, 0.0f, 1.0f) == 1.0f, "clamp");
    check(LIBMATTI_MC_Mth_LerpD(0.5, 0.0, 10.0) == 5.0, "lerp");
    check(LIBMATTI_MC_Mth_ClampedLerp(-0.5, 1.0, 2.0) == 1.0, "clampedLerp low");
    check(LIBMATTI_MC_Mth_ClampedLerp(1.5, 1.0, 2.0) == 2.0, "clampedLerp high");
    check(LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(17) == 32, "smallestEncompassingPowerOfTwo");
    // Java: log2 = ceillog2 - (isPowerOfTwo ? 0 : 1) - 17 is not a power of two, so 4
    check(LIBMATTI_MC_Mth_Log2(16) == 4 && LIBMATTI_MC_Mth_Log2(17) == 4, "log2");
    check(LIBMATTI_MC_Mth_Sign(-3.5) == -1 && LIBMATTI_MC_Mth_Sign(0.0) == 0, "sign");
    check(LIBMATTI_MC_Mth_Quantize(25.0, 16) == 16, "quantize");
    check(LIBMATTI_MC_Mth_PositiveCeilDiv(7, 3) == 3, "positiveCeilDiv");
    check(LIBMATTI_MC_Mth_HsvToRgb(0.0f, 1.0f, 1.0f) == 0xFF0000, "hsvToRgb red");
    check(LIBMATTI_MC_Mth_HsvToRgb(120.0f / 360.0f, 1.0f, 1.0f) == 0x00FF00, "hsvToRgb green");

    // ------------------------------------------------------------------ Direction
    check(LIBMATTI_MC_Direction_Get3DDataValue(LIBMATTI_MC_Direction_DOWN) == 0, "down data3d");
    check(LIBMATTI_MC_Direction_Get3DDataValue(LIBMATTI_MC_Direction_EAST) == 5, "east data3d");
    check(LIBMATTI_MC_Direction_Get2DDataValue(LIBMATTI_MC_Direction_SOUTH) == 0, "south data2d");
    check(LIBMATTI_MC_Direction_Get2DDataValue(LIBMATTI_MC_Direction_EAST) == 3, "east data2d");
    check(LIBMATTI_MC_Direction_GetOpposite(LIBMATTI_MC_Direction_UP) == LIBMATTI_MC_Direction_DOWN,
          "opposite up");
    check(LIBMATTI_MC_Direction_GetClockWise(LIBMATTI_MC_Direction_NORTH) == LIBMATTI_MC_Direction_EAST,
          "clockwise north");
    check(LIBMATTI_MC_Direction_GetCounterClockWise(LIBMATTI_MC_Direction_NORTH) == LIBMATTI_MC_Direction_WEST,
          "counterclockwise north");
    check(LIBMATTI_MC_Direction_GetStepY(LIBMATTI_MC_Direction_UP) == 1, "step up");
    check(LIBMATTI_MC_Direction_GetStepZ(LIBMATTI_MC_Direction_SOUTH) == 1, "step south");
    check(LIBMATTI_MC_Direction_GetAxis(LIBMATTI_MC_Direction_WEST) == LIBMATTI_MC_Direction_Axis_X,
          "west axis");
    check(LIBMATTI_MC_Direction_Axis_IsHorizontal(LIBMATTI_MC_Direction_Axis_Z), "z horizontal");
    check(LIBMATTI_MC_Direction_Axis_IsVertical(LIBMATTI_MC_Direction_Axis_Y), "y vertical");
    check(LIBMATTI_MC_Direction_Axis_ChooseI(LIBMATTI_MC_Direction_Axis_Y, 1, 2, 3) == 2, "choose y");
    check(LIBMATTI_MC_Direction_From3DDataValue(1) == LIBMATTI_MC_Direction_UP, "from3DDataValue(1)");
    check(LIBMATTI_MC_Direction_FromYRot(0.0) == LIBMATTI_MC_Direction_SOUTH, "fromYRot(0)");
    check(LIBMATTI_MC_Direction_FromYRot(90.0) == LIBMATTI_MC_Direction_WEST, "fromYRot(90)");
    check(LIBMATTI_MC_Direction_ToYRot(LIBMATTI_MC_Direction_WEST) == 90.0f, "toYRot west");

    // ------------------------------------------------------------------ Vec3
    LIBMATTI_MC_Vec3 *a = LIBMATTI_MC_Vec3_New(1.0, 2.0, 3.0);
    LIBMATTI_MC_Vec3 *b = LIBMATTI_MC_Vec3_New(4.0, 6.0, 3.0);
    LIBMATTI_MC_Vec3 *sum = LIBMATTI_MC_Vec3_Add(a, b);
    check(sum->x == 5.0 && sum->y == 8.0 && sum->z == 6.0, "vec add");
    LIBMATTI_MC_Vec3 *cross = LIBMATTI_MC_Vec3_Cross(LIBMATTI_MC_Vec3_X_AXIS(), LIBMATTI_MC_Vec3_Y_AXIS());
    check(cross->x == 0.0 && cross->y == 0.0 && cross->z == 1.0, "x cross y = z");
    check(LIBMATTI_MC_Vec3_Dot(LIBMATTI_MC_Vec3_X_AXIS(), LIBMATTI_MC_Vec3_X_AXIS()) == 1.0, "dot");
    LIBMATTI_MC_Vec3 *norm = LIBMATTI_MC_Vec3_Normalize(LIBMATTI_MC_Vec3_New(3.0, 0.0, 4.0));
    check(fabs(norm->x - 0.6) < 1e-9 && fabs(norm->z - 0.8) < 1e-9, "normalize");
    check(LIBMATTI_MC_Vec3_DistanceToSqr(a, b) == 25.0, "distanceToSqr");
    check(LIBMATTI_MC_Vec3_HorizontalDistance(LIBMATTI_MC_Vec3_New(3.0, 10.0, 4.0)) == 5.0,
          "horizontalDistance");
    LIBMATTI_MC_Vec3 *center = LIBMATTI_MC_Vec3_AtCenterOf(&((LIBMATTI_MC_Vec3i){1, 0, 1}));
    check(center->x == 1.5 && center->y == 0.5 && center->z == 1.5, "atCenterOf");
    LIBMATTI_MC_Vec3 *rot = LIBMATTI_MC_Vec3_YRot(LIBMATTI_MC_Vec3_New(1.0, 0.0, 0.0), LIBMATTI_MC_Mth_HALF_PI);
    check(fabs(rot->x) < 1e-4 && fabs(rot->z + 1.0) < 1e-4, "yRot 90 degrees");
    LIBMATTI_MC_Vec3 *dir = LIBMATTI_MC_Vec3_DirectionFromRotation(0.0f, 0.0f);
    check(dir->z > 0.99, "directionFromRotation south");
    check(LIBMATTI_MC_Vec3_Get(LIBMATTI_MC_Vec3_New(1.0, 2.0, 3.0), LIBMATTI_MC_Direction_Axis_Y) == 2.0,
          "vec get axis");

    // ------------------------------------------------------------------ BlockPos
    LIBMATTI_MC_BlockPos *pos = LIBMATTI_MC_BlockPos_New(123456, -700, -654321);
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
    check(LIBMATTI_MC_BlockPos_GetX((long) packed) == 123456, "packed x");
    check(LIBMATTI_MC_BlockPos_GetY((long) packed) == -700, "packed y");
    check(LIBMATTI_MC_BlockPos_GetZ((long) packed) == -654321, "packed z");
    check(LIBMATTI_MC_Mth_FloorD(-2.5) == -3, "containing floor");
    LIBMATTI_MC_BlockPos *above = LIBMATTI_MC_BlockPos_Above(pos);
    check(above->base.y == -699, "above");
    LIBMATTI_MC_BlockPos *rel = LIBMATTI_MC_BlockPos_RelativeN(pos, LIBMATTI_MC_Direction_NORTH, 3);
    check(rel->base.z == pos->base.z - 3, "relative north 3");
    LIBMATTI_MC_Vec3 *centerV = LIBMATTI_MC_BlockPos_GetCenter(LIBMATTI_MC_BlockPos_New(1, 1, 1));
    check(centerV->x == 1.5 && centerV->y == 1.5 && centerV->z == 1.5, "block center");
    LIBMATTI_MC_BlockPos *mutated = LIBMATTI_MC_BlockPos_MutableSet(
        LIBMATTI_MC_BlockPos_MutableNew(), 5, 6, 7);
    LIBMATTI_MC_BlockPos_MutableMove(mutated, LIBMATTI_MC_Direction_EAST);
    check(mutated->base.x == 6 && mutated->base.y == 6 && mutated->base.z == 7, "mutable move east");

    // ------------------------------------------------------------------ ChunkPos
    LIBMATTI_MC_ChunkPos *chunk = LIBMATTI_MC_ChunkPos_FromBlockPos(LIBMATTI_MC_BlockPos_New(35, 0, -17));
    check(chunk->x == 2 && chunk->z == -2, "chunk of block pos");
    int64_t chunkPacked = LIBMATTI_MC_ChunkPos_ToLong(chunk);
    check(LIBMATTI_MC_ChunkPos_GetX(chunkPacked) == 2 && LIBMATTI_MC_ChunkPos_GetZ(chunkPacked) == -2,
          "chunk packed round trip");
    check(LIBMATTI_MC_ChunkPos_GetMinBlockX(chunk) == 32, "chunk min block x");
    check(LIBMATTI_MC_ChunkPos_GetMaxBlockZ(chunk) == -17, "chunk max block z");
    check(LIBMATTI_MC_ChunkPos_Contains(chunk, LIBMATTI_MC_BlockPos_New(33, 0, -20)), "chunk contains");
    check(!LIBMATTI_MC_ChunkPos_Contains(chunk, LIBMATTI_MC_BlockPos_New(31, 0, -20)), "chunk not contains");
    check(LIBMATTI_MC_ChunkPos_GetRegionX(chunk) == 0 && LIBMATTI_MC_ChunkPos_GetRegionLocalZ(chunk) == 30,
          "chunk region coords");
    check(LIBMATTI_MC_ChunkPos_Hash(0, 0) == LIBMATTI_MC_ChunkPos_Hash(0, 0), "chunk hash stable");

    // ------------------------------------------------------------------ AABB
    LIBMATTI_MC_AABB *box = LIBMATTI_MC_AABB_New(0.0, 0.0, 0.0, 2.0, 1.0, 2.0);
    check(box->minX == 0.0 && box->maxX == 2.0, "aabb normalises min/max");
    LIBMATTI_MC_AABB *inflated = LIBMATTI_MC_AABB_Inflate(box, 1.0);
    check(inflated->minX == -1.0 && inflated->maxX == 3.0, "aabb inflate");
    LIBMATTI_MC_AABB *moved = LIBMATTI_MC_AABB_Move3(box, 1.0, 0.0, 0.0);
    check(moved->minX == 1.0 && moved->maxX == 3.0, "aabb move");
    check(LIBMATTI_MC_AABB_Intersects(box, moved), "aabb intersects");
    LIBMATTI_MC_AABB *far = LIBMATTI_MC_AABB_Move3(box, 10.0, 0.0, 0.0);
    check(!LIBMATTI_MC_AABB_Intersects(box, far), "aabb not intersects far");
    check(LIBMATTI_MC_AABB_Contains3(box, 1.0, 0.5, 1.0), "aabb contains");
    check(!LIBMATTI_MC_AABB_Contains3(box, 2.0, 0.5, 1.0), "aabb excludes upper bound");
    check(fabs(LIBMATTI_MC_AABB_GetSize(box) - 5.0 / 3.0) < 1e-9, "aabb size");
    check(LIBMATTI_MC_AABB_DistanceToSqr(box, LIBMATTI_MC_Vec3_New(3.0, 0.5, 1.0)) == 1.0, "aabb distance");

    // Java: the clip ray-cast - a ray through the box hits the west face
    int found = 0;
    LIBMATTI_MC_Vec3 *hit = LIBMATTI_MC_AABB_Clip(box, LIBMATTI_MC_Vec3_New(-1.0, 0.5, 1.0),
                                                  LIBMATTI_MC_Vec3_New(3.0, 0.5, 1.0), &found);
    check(found && hit->x == 0.0, "aabb clip hit");
    int direction = LIBMATTI_MC_AABB_ClipDirection(box, LIBMATTI_MC_Vec3_New(-1.0, 0.5, 1.0),
                                                   LIBMATTI_MC_Vec3_New(3.0, 0.5, 1.0));
    check(direction == LIBMATTI_MC_Direction_WEST, "aabb clip direction west");
    direction = LIBMATTI_MC_AABB_ClipDirection(box, LIBMATTI_MC_Vec3_New(1.0, 2.0, 1.0),
                                               LIBMATTI_MC_Vec3_New(1.0, 3.0, 1.0));
    check(direction < 0, "aabb clip miss above");
    LIBMATTI_MC_Vec3 *miss = LIBMATTI_MC_AABB_Clip(box, LIBMATTI_MC_Vec3_New(1.0, 2.0, 1.0),
                                                   LIBMATTI_MC_Vec3_New(1.0, 3.0, 1.0), &found);
    check(!found && miss == NULL, "aabb clip miss returns empty");

    printf("math: %d checks passed\n", checks);
    return 0;
}
