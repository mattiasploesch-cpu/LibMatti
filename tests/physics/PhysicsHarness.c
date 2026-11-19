





#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/phys/AABB.h"
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

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();



    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    build_arena(level);



    LIBMATTI_MC_AABB *hits[16];
    int count = LIBMATTI_MC_Level_GetBlockCollisions(level, 9.0, 60.0, -1.0, 12.0, 70.0, 4.0, hits, 16);
    check(count == 16, "scan finds the 16 wall blocks");
    if (count == 16)
    {
        int saw00 = 0, saw01 = 0, saw02 = 0, saw03 = 0;
        for (int i = 0; i < count; i++)
        {
            if (fabs(hits[i]->minX - 10.0) < 1e-9 && fabs(hits[i]->maxY - 65.0) < 1e-9 && fabs(hits[i]->minZ) < 1e-9)
                saw00 = 1;
            if (fabs(hits[i]->minY - 66.0) < 1e-9 && fabs(hits[i]->minZ - 1.0) < 1e-9)
                saw01 = 1;
            if (fabs(hits[i]->minY - 67.0) < 1e-9 && fabs(hits[i]->minZ - 2.0) < 1e-9)
                saw02 = 1;


            if (fabs(hits[i]->minY - 67.0) < 1e-9 && fabs(hits[i]->minZ - 3.0) < 1e-9)
                saw03 = 1;
        }
        check(saw00 && saw01 && saw02 && saw03, "scan returns the full cubes per position");
        for (int i = 0; i < count; i++)
            free(hits[i]);
    }


    count = LIBMATTI_MC_Level_GetBlockCollisions(level, 0.0, 200.0, 0.0, 1.0, 201.0, 1.0, hits, 16);
    check(count == 0, "empty sky scan finds nothing");
    for (int i = 0; i < count; i++)
        free(hits[i]);




    LIBMATTI_MC_AABB *inWall = LIBMATTI_MC_AABB_New(10.1, 64.1, 0.1, 10.4, 64.5, 0.4);
    LIBMATTI_MC_AABB *inOpen = LIBMATTI_MC_AABB_New(5.0, 65.1, 0.0, 5.3, 65.5, 0.3);
    check(!LIBMATTI_MC_Level_NoBlockCollision(level, inWall), "noBlockCollision false inside the wall");
    check(LIBMATTI_MC_Level_NoBlockCollision(level, inOpen), "noBlockCollision true in the open");
    free(inWall);
    free(inOpen);





    LIBMATTI_MC_Player *walker = LIBMATTI_MC_Player_Create(level, "Walker");


    LIBMATTI_MC_Entity_SetPos(&walker->base.base, 9.0, 65.0, 1.5);
    LIBMATTI_MC_Vec3 walk = {1.0, 0.0, 0.0};
    LIBMATTI_MC_Vec3 clipped;
    LIBMATTI_MC_Entity_Collide(&walker->base.base, &walk, &clipped);




    check(fabs(clipped.x - 0.7) < 1e-5, "x clip stops at the wall face");
    check(fabs(clipped.z) < 1e-9, "untouched axes stay zero");



    LIBMATTI_MC_Vec3 parallel = {0.0, 0.0, 2.0};
    LIBMATTI_MC_Entity_Collide(&walker->base.base, &parallel, &clipped);
    check(fabs(clipped.z - 2.0) < 1e-9, "parallel movement passes unclipped");



    LIBMATTI_MC_Vec3 push = {2.0, 0.0, 0.0};
    LIBMATTI_MC_AABB *box = LIBMATTI_MC_AABB_New(8.0, 64.0, 1.0, 9.0, 66.0, 2.0);
    LIBMATTI_MC_Entity_CollideBoundingBox(NULL, &push, box, level, &clipped);
    check(fabs(clipped.x - 1.0) < 1e-9, "collideBoundingBox clips at the face");
    free(box);

// the zero movement returns untouched (Java: lengthSqr == 0 branch)
    LIBMATTI_MC_Vec3 zero = {0.0, 0.0, 0.0};
    LIBMATTI_MC_Entity_Collide(&walker->base.base, &zero, &clipped);
    check(clipped.x == 0.0 && clipped.y == 0.0 && clipped.z == 0.0, "zero movement stays zero");



    LIBMATTI_MC_Player *faller = LIBMATTI_MC_Player_Create(level, "Faller");



    LIBMATTI_MC_Entity_SetPos(&faller->base.base, 0.5, 65.5, 0.5);
    LIBMATTI_MC_Vec3 fall = {0.0, -0.08, 0.0};
    LIBMATTI_MC_Entity_Move(&faller->base.base, LIBMATTI_MC_MoverType_SELF, &fall);
    check(fabs(LIBMATTI_MC_Entity_GetY(&faller->base.base) - 65.42) < 1e-9, "the gravity tick falls unclipped");


    LIBMATTI_MC_Vec3 deepFall = {0.0, -0.5, 0.0};
    LIBMATTI_MC_Entity_Move(&faller->base.base, LIBMATTI_MC_MoverType_SELF, &deepFall);
    check(fabs(LIBMATTI_MC_Entity_GetY(&faller->base.base) - 65.0) < 1e-9, "fall lands on the platform");
    check(LIBMATTI_MC_Entity_VerticalCollisionBelow(&faller->base.base), "verticalCollisionBelow set");
    check(LIBMATTI_MC_Entity_OnGround(&faller->base.base), "onGround after landing");
    check(!LIBMATTI_MC_Entity_HorizontalCollision(&faller->base.base), "no horizontal collision on the fall");



    LIBMATTI_MC_Vec3 motion = {0.3, 0.0, 0.2};
    LIBMATTI_MC_Entity_SetDeltaMovement(&faller->base.base, &motion);
    LIBMATTI_MC_Vec3 intoWall = {0.3, 0.0, 0.2};
    LIBMATTI_MC_Entity_Move(&faller->base.base, LIBMATTI_MC_MoverType_SELF, &intoWall);



    check(!LIBMATTI_MC_Entity_HorizontalCollision(&faller->base.base), "free walk has no horizontal collision");

    LIBMATTI_MC_Entity_SetPos(&walker->base.base, 9.5, 65.0, 1.5);
    LIBMATTI_MC_Vec3 walkMotion = {0.5, 0.0, 0.0};
    LIBMATTI_MC_Entity_SetDeltaMovement(&walker->base.base, &walkMotion);
    LIBMATTI_MC_Entity_Move(&walker->base.base, LIBMATTI_MC_MoverType_SELF, &walkMotion);
    check(LIBMATTI_MC_Entity_HorizontalCollision(&walker->base.base), "walking into the wall sets horizontalCollision");
    check(fabs(LIBMATTI_MC_Entity_GetX(&walker->base.base) - 9.7) < 1e-5, "the move stops at the wall face");
    LIBMATTI_MC_Vec3 after;
    LIBMATTI_MC_Entity_GetDeltaMovement(&walker->base.base, &after);
    check(after.x == 0.0 && fabs(after.z) < 1e-9, "the horizontal motion resets on collision");



    LIBMATTI_MC_Player *ghost = LIBMATTI_MC_Player_Create(level, "Ghost");
    LIBMATTI_MC_Entity_SetPos(&ghost->base.base, 9.5, 66.0, 1.5);
    LIBMATTI_MC_Entity_SetNoPhysics(&ghost->base.base, true);
    LIBMATTI_MC_Vec3 through = {0.0, 0.0, 0.0};
    (void) through;
    LIBMATTI_MC_Vec3 ghostMove = {2.0, 0.0, 0.0};
    LIBMATTI_MC_Entity_Move(&ghost->base.base, LIBMATTI_MC_MoverType_SELF, &ghostMove);
    check(fabs(LIBMATTI_MC_Entity_GetX(&ghost->base.base) - 11.5) < 1e-9, "noPhysics moves straight through the wall");
    check(!LIBMATTI_MC_Entity_HorizontalCollision(&ghost->base.base), "noPhysics clears the collision flags");



    LIBMATTI_MC_Player *slider = LIBMATTI_MC_Player_Create(level, "Slider");


    LIBMATTI_MC_Entity_SetPos(&slider->base.base, 7.0, 66.5, 5.0);



    LIBMATTI_MC_Vec3 diagonal = {-0.2, -2.0, 0.0};
    LIBMATTI_MC_Entity_Move(&slider->base.base, LIBMATTI_MC_MoverType_SELF, &diagonal);
    check(LIBMATTI_MC_Entity_OnGround(&slider->base.base), "diagonal fall lands");
    check(fabs(LIBMATTI_MC_Entity_GetX(&slider->base.base) - 6.8) < 1e-5, "the horizontal slide rides unclipped");
    check(fabs(LIBMATTI_MC_Entity_GetY(&slider->base.base) - 65.0) < 1e-5, "the vertical clips at the platform");

    LIBMATTI_MC_Player_Free(ghost);
    LIBMATTI_MC_Player_Free(slider);
    LIBMATTI_MC_Player_Free(walker);
    LIBMATTI_MC_Player_Free(faller);
    LIBMATTI_MC_Level_Free(level);

    printf("physics: %d checks passed (%s)\n", checks, failures ? "FAILURES" : "ok");
    return failures != 0;
}
