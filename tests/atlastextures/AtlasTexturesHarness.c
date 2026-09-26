// The atlas-texture harness (P4.3): the procedural vanilla block textures
// stitch into the block atlas, the vanilla models bake against it with real
// sprite UV rects and the registry hands the quads to the renderer path.

#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/client/resources/model/VanillaModels.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockTextures.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockModels.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;
#define CHECK(cond)                                                          \
    do                                                                       \
    {                                                                        \
        checks++;                                                            \
        if (!(cond))                                                         \
        {                                                                    \
            failures++;                                                      \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
        }                                                                    \
    } while (0)

// The sprite rect the bake resolves through (the SpriteGetter contract).
static int resolve_sprite(void *userdata, const char *textureId, float uvRect[4])
{
    return LIBMATTI_MC_SpriteGetter_SpriteRect((const LIBMATTI_MC_TextureAtlas *) userdata, textureId, uvRect);
}

int main(void)
{
    // Java: the MODEL_ATLAS bootstrap - the procedural stone/dirt page.
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_VanillaBlockTextures_Bootstrap(1024);
    CHECK(atlas != NULL);
    if (atlas == NULL)
        return 1;
    // Java: the demo slice stitches the hotbar palette + the missing fallback
    // (10 sprites on the 160x16 page).
    CHECK(atlas->spriteCount == 10);
    CHECK(atlas->width == 160 && atlas->height == 16);
    CHECK(atlas->missingSprite != NULL);

    // The sprite rects are real atlas coordinates (missing cell 0, then the
    // block cells left to right on the 160x16 page).
    float missingRect[4], stoneRect[4], dirtRect[4];
    CHECK(LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "missingno", missingRect));
    CHECK(LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "block/stone", stoneRect));
    CHECK(LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "block/dirt", dirtRect));
    CHECK(missingRect[0] == 0.0f && missingRect[2] == 0.1f); // 16/160
    CHECK(stoneRect[0] == 0.1f && stoneRect[2] == 0.2f);
    CHECK(dirtRect[0] == 0.2f && dirtRect[2] == 0.3f);
    // Java: getSprite -> the missing sprite for unknown ids (the rect still
    // resolves - the checkerboard fallback, never the full-atlas smear).
    CHECK(LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "block/unknown", missingRect));
    CHECK(missingRect[0] == 0.0f && missingRect[2] == 0.1f);

    // Java: ModelBakery - the vanilla models bake against the atlas.
    LIBMATTI_MC_ModelManager *manager = LIBMATTI_MC_ModelManager_New(atlas);
    CHECK(LIBMATTI_MC_VanillaBlockModels_Bootstrap(manager));
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_STONE_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_DIRT_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, "minecraft:block/missing") == NULL);

    // The baked stone model: the cube_all element bakes to 6 quads (one per
    // face, all culled buckets - the unculled list stays empty).
    const LIBMATTI_MC_QuadCollection *stone =
        LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_STONE_ID);
    size_t unculled = 0, culled = 0;
    LIBMATTI_MC_QuadCollection_GetUnculled(stone, &unculled);
    CHECK(unculled == 0);
    for (int d = 0; d < 6; d++)
    {
        LIBMATTI_MC_QuadCollection_GetCulled(stone, (LIBMATTI_MC_Direction) d, &culled);
        CHECK(culled == 1);
    }

    // Java: the hotbar palette's models all bake (cube_all family + the log's
    // cube_column).
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_COBBLESTONE_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_OAK_PLANKS_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_GLASS_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_BRICKS_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_SAND_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_GRAVEL_ID) != NULL);
    CHECK(LIBMATTI_MC_ModelManager_GetModel(manager, LIBMATTI_MC_VanillaModels_OAK_LOG_ID) != NULL);

    // The quads carry real atlas UVs (the stone rect, not the full 0..1).
    const LIBMATTI_MC_BakedQuad *quads =
        LIBMATTI_MC_QuadCollection_GetCulled(stone, LIBMATTI_MC_Direction_UP, &culled);
    CHECK(quads != NULL && culled == 1);
    if (quads != NULL)
    {
        CHECK(quads[0].uv[0][0] >= stoneRect[0] - 0.001f);
        CHECK(quads[0].uv[0][0] <= stoneRect[2] + 0.001f);
        CHECK(quads[0].direction == LIBMATTI_MC_Direction_UP);
    }

    // The bake is idempotent (Java: the registry dedupes by id).
    CHECK(LIBMATTI_MC_VanillaBlockModels_Bootstrap(manager));

    LIBMATTI_MC_ModelManager_Free(manager);
    LIBMATTI_MC_TextureAtlas_Free(atlas);

    printf("atlastextures: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
