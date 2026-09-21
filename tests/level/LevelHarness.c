// Level harness: builds an in-memory Level, writes blocks through setBlock and
// checks the read path through getBlockState - chunk lookup, section indexing,
// bounds handling (VOID_AIR below the world, air in unloaded chunks) and the
// LevelReader surface (height scan, canSeeSky).

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/LevelReader.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void)
{
    // overworld: minY -64, height 384 (like the Java DimensionType overworld)
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    check(level != NULL, "level created");
    check(LIBMATTI_MC_LevelHeightAccessor_GetSectionsCount(&level->heightAccessor) == 24, "24 sections (384/16)");
    check(LIBMATTI_MC_LevelHeightAccessor_GetMaxY(&level->heightAccessor) == 319, "maxY 319");

    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    LIBMATTI_MC_BlockState *oakPlanks = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("OAK_PLANKS"));

    // write through setBlock - this loads two chunks on the fly
    LIBMATTI_MC_BlockPos *p1 = LIBMATTI_MC_BlockPos_New(0, -60, 0);
    LIBMATTI_MC_BlockPos *p2 = LIBMATTI_MC_BlockPos_New(30, 100, -47);
    check(LIBMATTI_MC_Level_SetBlock(level, p1, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS), "setBlock stone at (0,-60,0)");
    check(LIBMATTI_MC_Level_SetBlock(level, p2, oakPlanks, LIBMATTI_MC_Level_UPDATE_CLIENTS), "setBlock planks at (30,100,-47)");
    check(LIBMATTI_MC_Level_GetChunkCount(level) == 2, "two chunks loaded");

    // read back - different chunk, different section
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, p1)) == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("STONE"), "read back stone");
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, p2)) == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("OAK_PLANKS"), "read back planks");

    // the section counts moved (non-air in two different sections)
    LIBMATTI_MC_LevelChunk *chunk1 = LIBMATTI_MC_Level_GetChunk(level, 0, 0);
    check(chunk1 != NULL, "chunk (0,0) loaded");
    LIBMATTI_MC_LevelChunkSection *section1 = LIBMATTI_MC_LevelChunk_GetSections(chunk1)[LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk1->base.levelHeightAccessor, -60)];
    check(section1->nonEmptyBlockCount == 1, "section has 1 non-air block");

    // bounds: below the world is VOID_AIR, above is air, unloaded chunk is air
    LIBMATTI_MC_BlockPos *below = LIBMATTI_MC_BlockPos_New(0, -65, 0);
    LIBMATTI_MC_BlockPos *above = LIBMATTI_MC_BlockPos_New(0, 320, 0);
    LIBMATTI_MC_BlockPos *unloaded = LIBMATTI_MC_BlockPos_New(1000, 64, 1000);
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, below)) == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("VOID_AIR"), "below world is VOID_AIR");
    // Java: isInValidBounds fails above the world too -> VOID_AIR there as well
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, above)) == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("VOID_AIR"), "above world is VOID_AIR");
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, unloaded)) == (void *) LIBMATTI_MC_VanillaBlocks_AIR(), "unloaded chunk is AIR");
    check(!LIBMATTI_MC_Level_SetBlock(level, below, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS), "setBlock below world rejected");

    // remove + destroy
    check(LIBMATTI_MC_Level_RemoveBlock(level, p1, LIBMATTI_MC_Level_UPDATE_CLIENTS), "removeBlock stone");
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, p1)) == (void *) LIBMATTI_MC_VanillaBlocks_AIR(), "removed block is air");

    // LevelReader surface
    check(LIBMATTI_MC_LevelReader_GetSeaLevel(level) == 63, "sea level 63");
    check(LIBMATTI_MC_LevelReader_GetHeight(level, 30, -47) == 101, "height scan above planks at y=100");

    // heightmap data layout - Java's LevelChunk.setBlockState creates and updates
    // the four live heightmaps on every write, so the planks write already primed
    // WORLD_SURFACE
    LIBMATTI_MC_LevelChunk *planksChunkHm = LIBMATTI_MC_Level_GetChunk(level, 1, -3);
    check(LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(&planksChunkHm->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE), "heightmap primed by setBlock");
    check(!LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(&planksChunkHm->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE_WG), "WG heightmap untouched (worldgen only)");
    check(LIBMATTI_MC_ChunkAccess_GetHeight(&planksChunkHm->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE, 14, 1) == 100, "chunk heightmap (x=14,z=1 = block 30,-47) -> 100 (top block y)");
    // incremental update: place a block above, the primed heightmap rises
    // (Java: getHeight = firstAvailable - 1 = the top block's Y, so 200 for the
    // block at y=200; the LevelReader scan returns y+1 = 201 - different conventions)
    LIBMATTI_MC_BlockPos *high = LIBMATTI_MC_BlockPos_New(30, 200, -47);
    check(LIBMATTI_MC_Level_SetBlock(level, high, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS), "place block at y=200");
    check(LIBMATTI_MC_ChunkAccess_GetHeight(&planksChunkHm->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE, 14, 1) == 200, "heightmap rose to 200 (top block y)");

    // canSeeSky after the y=200 block: Java checks SKY light (>= 15); the port's
    // column approximation scans ABOVE pos - the y=200 block shadows both spots
    LIBMATTI_MC_BlockPos *atPlanks = LIBMATTI_MC_BlockPos_New(30, 100, -47);
    LIBMATTI_MC_BlockPos *abovePlanks = LIBMATTI_MC_BlockPos_New(30, 101, -47);
    check(!LIBMATTI_MC_LevelReader_CanSeeSky(level, atPlanks), "planks covered by y=200 block");
    check(!LIBMATTI_MC_LevelReader_CanSeeSky(level, abovePlanks), "above planks also covered");
    check(LIBMATTI_MC_LevelReader_IsEmptyBlock(level, unloaded), "unloaded block empty");

    // hasChunk + block entity lifecycle - a chest at planks' position gets the real
    // BlockEntity through the ChunkAccess containers
    check(LIBMATTI_MC_Level_HasChunk(level, 0, 0), "hasChunk (0,0)");
    check(!LIBMATTI_MC_Level_HasChunk(level, 5, 5), "no hasChunk (5,5)");
    LIBMATTI_MC_LevelChunk *planksChunk = LIBMATTI_MC_Level_GetChunk(level, 1, -3);
    LIBMATTI_MC_BlockState *chestState = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("CHEST"));
    LIBMATTI_MC_BlockPos *chestPos = LIBMATTI_MC_BlockPos_New(31, 100, -47);
    check(LIBMATTI_MC_Level_SetBlock(level, chestPos, chestState, LIBMATTI_MC_Level_UPDATE_CLIENTS), "place chest");
    LIBMATTI_MC_BlockEntity *chest = LIBMATTI_MC_LevelChunk_GetBlockEntityWithCreation(
        planksChunk, chestPos, LIBMATTI_MC_LevelChunkEntityCreationType_IMMEDIATE);
    check(chest != NULL, "chest block entity created (IMMEDIATE)");
    check(chest != NULL && LIBMATTI_MC_BlockEntity_GetType(chest) == LIBMATTI_MC_BlockEntityType_CHEST(), "type is chest");
    check(chest != NULL && LIBMATTI_MC_BlockEntity_GetLevel(chest) == level, "entity wired to the level");
    check(LIBMATTI_MC_LevelChunk_GetBlockEntity(planksChunk, chestPos) == chest, "read back through CHECK");
    // empty positions have no entity
    check(LIBMATTI_MC_LevelChunk_GetBlockEntity(planksChunk, atPlanks) == NULL, "no entity for plain block");
    // remove path
    LIBMATTI_MC_LevelChunk_RemoveBlockEntity(planksChunk, chestPos);
    check(LIBMATTI_MC_LevelChunk_GetBlockEntity(planksChunk, chestPos) == NULL, "entity removed");
    check(LIBMATTI_MC_Level_GetBlockEntity(level, atPlanks) == NULL, "no entity through level");

    free(p1);
    free(p2);
    free(below);
    free(above);
    free(unloaded);
    free(atPlanks);
    free(abovePlanks);
    LIBMATTI_MC_Level_Free(level);

    printf("level: %d checks passed (%s)\n", checks, failures == 0 ? "ok" : "FAILURES");
    return failures == 0 ? 0 : 1;
}
