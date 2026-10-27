// Pick harness: the P5.4 core - the level ray cast (BlockGetter.clip over the
// traverseBlocks DDA), the per-cell cube clip with the entry face + inside
// path, the ClipContext shape resolution through the hasCollision gate and the
// reach cutting the ray off (the miss direction rides the reverse ray).

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/ClipContext.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/phys/BlockHitResult.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

// the ground slab (y = 64) plus the wall column on x = 10 (the same arena the
// physics harness walks against)
static void build_arena(LIBMATTI_MC_Level *level)
{
    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    for (int y = 64; y < 68; y++)
    {
        for (int z = 0; z < 4; z++)
        {
            LIBMATTI_MC_BlockPos pos = {{10, y, z}};
            LIBMATTI_MC_Level_SetBlock(level, &pos, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);
        }
    }
    for (int x = -2; x < 8; x++)
    {
        for (int z = -2; z < 8; z++)
        {
            LIBMATTI_MC_BlockPos pos = {{x, 64, z}};
            LIBMATTI_MC_Level_SetBlock(level, &pos, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);
        }
    }
}

// the clip over the context, owned by the caller
static LIBMATTI_MC_BlockHitResult clip(LIBMATTI_MC_Level *level, double fx, double fy, double fz,
                                       double tx, double ty, double tz)
{
    LIBMATTI_MC_Vec3 from = {fx, fy, fz};
    LIBMATTI_MC_Vec3 to = {tx, ty, tz};
    LIBMATTI_MC_ClipContext context = LIBMATTI_MC_ClipContext_New(
        &from, &to, LIBMATTI_MC_ClipContext_Block_COLLIDER, LIBMATTI_MC_ClipContext_Fluid_NONE, NULL, NULL);
    return LIBMATTI_MC_Level_Clip(level, &context);
}

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    build_arena(level);

    // ---- the straight-down ray: hits the slab's UP face at the eye's column ----
    LIBMATTI_MC_BlockHitResult hit = clip(level, 3.5, 66.0, 2.5, 3.5, 60.0, 2.5);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the down ray hits the slab");
    if (hit.type == LIBMATTI_MC_HitResult_BLOCK)
    {
        check(hit.blockPos.base.x == 3 && hit.blockPos.base.y == 64 && hit.blockPos.base.z == 2,
              "the down ray hits the block under the eye");
        check(hit.direction == LIBMATTI_MC_Direction_UP, "the down ray enters through the UP face");
        check(fabs(hit.location.y - 65.0) < 1.0e-9, "the hit location rides the slab surface");
    }

    // ---- the horizontal ray over the slab: the wall blocks it ----
    hit = clip(level, 8.5, 64.5, 1.5, 13.0, 64.5, 1.5);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the horizontal ray hits the wall");
    if (hit.type == LIBMATTI_MC_HitResult_BLOCK)
    {
        check(hit.blockPos.base.x == 10, "the horizontal ray stops at the wall column");
        check(hit.direction == LIBMATTI_MC_Direction_WEST, "the horizontal ray enters through the WEST face");
        check(fabs(hit.location.x - 10.0) < 1.0e-9, "the hit location rides the wall surface");
    }

    // ---- the diagonal ray toward the slab corner ----
    hit = clip(level, 2.0, 66.0, 2.0, 2.0, 64.0, 2.0);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the diagonal ray hits the slab");
    if (hit.type == LIBMATTI_MC_HitResult_BLOCK)
    {
        check(hit.blockPos.base.y == 64, "the diagonal ray stops at the slab row");
        check(hit.direction == LIBMATTI_MC_Direction_UP, "the diagonal ray enters through the UP face");
    }

    // ---- the air ray: miss with the ray end + the reverse-ray nearest ----
    hit = clip(level, 3.5, 70.0, 2.5, 3.5, 70.0, 12.5);
    check(hit.type == LIBMATTI_MC_HitResult_MISS, "the air ray misses");
    if (hit.type == LIBMATTI_MC_HitResult_MISS)
    {
        check(fabs(hit.location.z - 12.5) < 1.0e-9, "the miss location rides the ray end");
        check(hit.direction == LIBMATTI_MC_Direction_NORTH, "the miss direction rides the reverse ray (+z -> north)");
    }

    // ---- the reach cuts the ray off: the wall beyond the ray end stays unhit ----
    hit = clip(level, 8.5, 64.5, 1.5, 9.7, 64.5, 1.5);
    check(hit.type == LIBMATTI_MC_HitResult_MISS, "the ray ending before the wall misses it");
    hit = clip(level, 8.5, 64.5, 1.5, 13.5, 64.5, 1.5);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the longer ray reaches the wall");

    // ---- the start inside a block: the immediate inside hit ----
    hit = clip(level, 10.5, 65.5, 1.5, 14.0, 65.5, 1.5);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the ray starting inside the wall hits");
    if (hit.type == LIBMATTI_MC_HitResult_BLOCK)
    {
        check(hit.inside, "the inside start reports inside=true");
        check(hit.blockPos.base.x == 10 && hit.blockPos.base.y == 65, "the inside hit names the start block");
    }

    // ---- the zero-length ray misses immediately ----
    hit = clip(level, 3.5, 66.0, 2.5, 3.5, 66.0, 2.5);
    check(hit.type == LIBMATTI_MC_HitResult_MISS, "the zero-length ray misses");

    // ---- the degenerate inside-slab ray hits the surrounding cell (the ray is
    // long enough to clear Java's lengthSqr 1.0E-7 gate) ----
    hit = clip(level, 3.5, 64.5, 2.5, 3.5, 64.5, 2.51);
    check(hit.type == LIBMATTI_MC_HitResult_BLOCK, "the ray inside the slab hits its own cell");
    if (hit.type == LIBMATTI_MC_HitResult_BLOCK)
        check(hit.inside, "the degenerate slab ray reports inside=true");

    // ---- the shape gate: the non-colliding block clips empty ----
    LIBMATTI_MC_BlockState *grass = LIBMATTI_MC_Block_DefaultBlockState(
        LIBMATTI_MC_VanillaBlocks_GetByName("SHORT_GRASS"));
    if (grass != NULL)
    {
        LIBMATTI_MC_BlockPos pos = {{4, 65, 3}};
        LIBMATTI_MC_Level_SetBlock(level, &pos, grass, LIBMATTI_MC_Level_UPDATE_CLIENTS);
        // the ray ends just above the slab surface (65), so a collidable grass
        // cube (65..66) would have caught it - the empty clip walks through
        hit = clip(level, 4.5, 67.0, 3.5, 4.5, 65.01, 3.5);
        check(hit.type == LIBMATTI_MC_HitResult_MISS, "the non-colliding plant clips empty");
    }
    else
    {
        checks++;
        failures++;
        printf("FAIL: SHORT_GRASS missing from the vanilla data\n");
    }

    // ---- the BlockHitResult factories ----
    LIBMATTI_MC_Vec3 location = {1.0, 2.0, 3.0};
    LIBMATTI_MC_BlockPos pos = {{5, 6, 7}};
    LIBMATTI_MC_BlockHitResult built = LIBMATTI_MC_BlockHitResult_Block(&location, LIBMATTI_MC_Direction_NORTH, &pos, false);
    check(built.type == LIBMATTI_MC_HitResult_BLOCK && !built.inside, "the block factory carries the type");
    LIBMATTI_MC_BlockHitResult flipped = LIBMATTI_MC_BlockHitResult_WithDirection(&built, LIBMATTI_MC_Direction_EAST);
    check(flipped.direction == LIBMATTI_MC_Direction_EAST && built.direction == LIBMATTI_MC_Direction_NORTH,
          "withDirection copies without touching the source");
    LIBMATTI_MC_BlockHitResult miss = LIBMATTI_MC_BlockHitResult_Miss(&location, LIBMATTI_MC_Direction_DOWN, &pos);
    check(miss.type == LIBMATTI_MC_HitResult_MISS, "the miss factory carries the type");
    LIBMATTI_MC_BlockHitResult defaultMiss = LIBMATTI_MC_BlockHitResult_DefaultMiss();
    check(defaultMiss.type == LIBMATTI_MC_HitResult_MISS && defaultMiss.blockPos.base.x == 0,
          "the default miss rides the zero block");

    LIBMATTI_MC_Level_Free(level);
    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
