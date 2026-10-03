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
#include "libmatti/net/minecraft/world/level/biome/Biome.h"
#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLevelSource.h"

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

// ---- the chunk source: a missing chunk is generated, never handed out empty ----
// The in-memory level used to answer getChunk(..., load=true) with a fresh
// all-air chunk. Walking past the border then dropped the player through it into
// the void, because the empty chunk carries no collision.
static void test_chunk_generation(void)
{
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);

    // without a generator there is nothing to generate - the lookup stays empty
    check(LIBMATTI_MC_LevelReader_GetChunk(level, 7, 7, true) == NULL,
          "load=true without a generator returns NULL (no empty chunk)");
    check(LIBMATTI_MC_Level_GetChunk(level, 7, 7) == NULL, "the empty chunk was not stored");

    LIBMATTI_MC_BlockState *bedrock = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("BEDROCK"));
    LIBMATTI_MC_BlockState *dirt = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("DIRT"));
    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    LIBMATTI_MC_FlatLayerInfo layers[3];
    LIBMATTI_MC_FlatLayerInfo_Init(&layers[0], 1, bedrock);
    LIBMATTI_MC_FlatLayerInfo_Init(&layers[1], 2, dirt);
    LIBMATTI_MC_FlatLayerInfo_Init(&layers[2], 126, stone);
    LIBMATTI_MC_FlatLevelSource *flat = LIBMATTI_MC_FlatLevelSource_New(layers, 3, LIBMATTI_MC_Biomes_Plains());
    LIBMATTI_MC_Level_SetChunkGenerator(level, &flat->base);
    check(LIBMATTI_MC_Level_GetChunkGenerator(level) == &flat->base, "the generator is wired to the level");

    // the block access path does NOT generate - the keep-alive pass does
    check(LIBMATTI_MC_LevelReader_GetChunk(level, 7, 7, true) == NULL,
          "load=true does not generate (the cache fills from the keep-alive pass)");

    LIBMATTI_MC_LevelChunk *generated = LIBMATTI_MC_Level_GenerateChunk(level, 7, 7);
    check(generated != NULL, "the missing chunk generates");
    check(LIBMATTI_MC_Level_GetChunk(level, 7, 7) == generated, "the generated chunk is stored in the level");
    check(LIBMATTI_MC_LevelReader_GetChunk(level, 7, 7, true) == generated, "the cache answers the second ask");

    // the superflat stack: bedrock 1 / dirt 2 / stone 126 over the -64 base, so the
    // stone surface lands at y=64 and the air above it is what a walker used to
    // fall through
    LIBMATTI_MC_BlockPos *floorPos = LIBMATTI_MC_BlockPos_New(7 * 16 + 3, 64, 7 * 16 + 5);
    LIBMATTI_MC_BlockState *floor = LIBMATTI_MC_Level_GetBlockState(level, floorPos);
    check(LIBMATTI_MC_BlockState_GetBlock(floor) == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("STONE"),
          "the generated chunk has the stone floor at y=64");
    LIBMATTI_MC_BlockPos *airPos = LIBMATTI_MC_BlockPos_New(7 * 16 + 3, 65, 7 * 16 + 5);
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, airPos)) ==
              (void *) LIBMATTI_MC_VanillaBlocks_AIR(),
          "air above the surface at y=65");
    LIBMATTI_MC_BlockPos *dirtPos = LIBMATTI_MC_BlockPos_New(7 * 16 + 3, -63, 7 * 16 + 5);
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, dirtPos)) ==
              (void *) LIBMATTI_MC_VanillaBlocks_GetByName("DIRT"),
          "dirt at y=-63");
    LIBMATTI_MC_BlockPos *basePos = LIBMATTI_MC_BlockPos_New(7 * 16 + 3, -64, 7 * 16 + 5);
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, basePos)) ==
              (void *) LIBMATTI_MC_VanillaBlocks_GetByName("BEDROCK"),
          "bedrock at y=-64");
    free(floorPos);
    free(airPos);
    free(dirtPos);
    free(basePos);

    // the heightmaps ride along (MOTION_BLOCKING is primed, not just allocated)
    check(LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(&generated->base, LIBMATTI_MC_Heightmap_MOTION_BLOCKING),
          "the generated chunk carries its primed heightmaps");

    // the keep-alive pass fills the square around a centre, and only the gaps
    check(LIBMATTI_MC_Level_EnsureChunksAround(level, 1, 0, 1) == 9, "the radius-1 pass filled all 9 chunks around the centre");
    check(LIBMATTI_MC_Level_GetChunk(level, 2, 1) != NULL, "the pass reached the corner chunk (2,1)");
    check(LIBMATTI_MC_Level_EnsureChunksAround(level, 1, 0, 1) == 0, "the second pass has nothing left to do");
    check(LIBMATTI_MC_Level_GetChunk(level, 7, 7) == generated, "the first chunk survived the pass");

    // the pass must not overwrite a loaded chunk - a built chunk keeps its blocks
    LIBMATTI_MC_BlockPos *built = LIBMATTI_MC_BlockPos_New(0 * 16 + 2, 70, 0 * 16 + 2);
    LIBMATTI_MC_Level_SetBlock(level, built, LIBMATTI_MC_Block_DefaultBlockState(
                                             LIBMATTI_MC_VanillaBlocks_GetByName("OAK_PLANKS")),
                               LIBMATTI_MC_Level_UPDATE_CLIENTS);
    LIBMATTI_MC_Level_EnsureChunksAround(level, 0, 0, 2);
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(level, built)) ==
              (void *) LIBMATTI_MC_VanillaBlocks_GetByName("OAK_PLANKS"),
          "the keep-alive pass kept the block in a loaded chunk");
    free(built);

    LIBMATTI_MC_FlatLevelSource_Free(flat);
    LIBMATTI_MC_Level_Free(level);
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

    // Java: setBlock needs the chunk to be there - the chunk source generates a
    // missing one, and this level has no generator, so the two write chunks are
    // created explicitly here
    const int WRITE_CHUNKS[][2] = {{0, 0}, {1, -3}};
    for (size_t i = 0; i < sizeof(WRITE_CHUNKS) / sizeof(WRITE_CHUNKS[0]); i++)
    {
        LIBMATTI_MC_ChunkPos writePos = {WRITE_CHUNKS[i][0], WRITE_CHUNKS[i][1]};
        LIBMATTI_MC_Level_SetChunk(level, LIBMATTI_MC_LevelChunk_New(level, &writePos));
    }

    // write through setBlock
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

    test_chunk_generation();

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
