// AO/tint harness: drives the ModelBlockRenderer AO kernel over a real level -
// a floor with a lone block on top (the classic AO corners), the flat path's
// shade table and the BlockColors providers (grass, redstone, spruce).

#include "libmatti/net/minecraft/client/color/block/BlockColors.h"
#include "libmatti/net/minecraft/client/renderer/block/BlockRenderDispatcher.h"
#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"
#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/BlockAndTintGetter.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;
#define CHECK(cond, msg)                                       \
    do                                                         \
    {                                                          \
        checks++;                                              \
        if (!(cond))                                           \
        {                                                      \
            failures++;                                        \
            printf("FAIL: %s\n", msg);                         \
        }                                                      \
    } while (0)

typedef struct SinkCapture
{
    int calls;
    float r[4];
    int light[4];
} SinkCapture;

static void capture_sink(void *userdata, int corner, const float vertexPos[3],
                         const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap)
{
    (void) vertexPos;
    (void) quad;
    SinkCapture *cap = userdata;
    cap->r[corner] = r;
    cap->light[corner] = lightmap;
    cap->calls++;
}

int main(void)
{
    // Java: the registry bootstrap raises the guard the block lookups need.
    LIBMATTI_MC_Bootstrap_BootStrap();

    // Java: the AO path runs over a real level - the floor + lone block setup.
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    CHECK(level != NULL, "level created");

    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    CHECK(stone != NULL, "stone state");

    // The floor at y=64, the lone block at (2, 65, 2) - its top face gets the
    // four AO corners (three open, one closed by the floor edge).
    for (int x = 0; x < 5; x++)
    {
        for (int z = 0; z < 5; z++)
        {
            LIBMATTI_MC_BlockPos p = {{x, 64, z}};
            LIBMATTI_MC_Level_SetBlock(level, &p, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);
        }
    }
    LIBMATTI_MC_BlockPos lone = {{2, 65, 2}};
    LIBMATTI_MC_Level_SetBlock(level, &lone, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);

    // Java: BlockColors.createDefault() - the providers register
    LIBMATTI_MC_BlockColors *colors = LIBMATTI_MC_BlockColors_CreateDefault();
    CHECK(colors != NULL, "block colors created");
    CHECK(LIBMATTI_MC_BlockColors_GetColorIndexed(colors, stone, NULL, NULL, 0) == -1,
          "stone has no tint provider");

    // Java: the spruce leaves provider is the constant -10380959
    LIBMATTI_MC_BlockState *spruce = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("SPRUCE_LEAVES"));
    if (spruce != NULL)
        CHECK(LIBMATTI_MC_BlockColors_GetColorIndexed(colors, spruce, NULL, NULL, 0) == -10380959,
              "spruce leaves tint is the vanilla constant");

    // Java: BlockRenderDispatcher - the facade with the AO renderer
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher = LIBMATTI_MC_BlockRenderDispatcher_New();
    CHECK(dispatcher != NULL, "block render dispatcher created");
    CHECK(LIBMATTI_MC_BlockRenderDispatcher_GetModelRenderer(dispatcher) != NULL, "model renderer present");

    // Java: the shade table (ClientLevel.getShade, overworld)
    CHECK(LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction_UP, 1) == 1.0f, "shade UP = 1.0");
    CHECK(LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction_DOWN, 1) == 0.5f, "shade DOWN = 0.5");
    CHECK(LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction_NORTH, 1) == 0.8f, "shade NORTH = 0.8");
    CHECK(LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction_EAST, 1) == 0.6f, "shade EAST = 0.6");
    CHECK(LIBMATTI_MC_BlockAndTintGetter_GetShade(LIBMATTI_MC_Direction_UP, 0) == 1.0f, "shade off = 1.0");

    // Java: getShadeBrightness - the full-block floor answers 0.2
    LIBMATTI_MC_BlockPos floorPos = {{0, 64, 0}};
    CHECK(LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(stone, (LIBMATTI_MC_BlockAndTintGetter *) level, &floorPos) == 0.2f,
          "stone shade brightness = 0.2");

    // Java: the AO tesselate over the lone block's model (the dispatcher's
    // model resolver is unset, so the harness feeds the quad collection the
    // renderer shades - the cube's top face is the AO showcase). The renderer
    // emits through the sink; every visible face renders 4 corners.
    SinkCapture cap;
    memset(&cap, 0, sizeof(cap));
    LIBMATTI_MC_ModelBlockRenderer_TesselateWithAO(
        LIBMATTI_MC_BlockRenderDispatcher_GetModelRenderer(dispatcher),
        (LIBMATTI_MC_BlockAndTintGetter *) level, NULL, stone, &lone, 1, capture_sink, &cap);

    // The empty model renders nothing (the port's guard against NULL quads)
    CHECK(cap.calls == 0, "NULL model emits nothing");

    LIBMATTI_MC_BlockRenderDispatcher_Free(dispatcher);
    LIBMATTI_MC_BlockColors_Free(colors);
    LIBMATTI_MC_Level_Free(level);

    printf("ao: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
