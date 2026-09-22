// Port of the vanilla block model bootstrap slice (implementation).
//
// The bake needs two lookups the ModelManager's file path cannot answer for
// the embedded models: the model JSON (VanillaModels carries it) and the
// sprite rect (the atlas carries it once the block textures are stitched).
// The bootstrap resolves the parent chain over the embedded JSONs and bakes
// each model through the ModelBaker.

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockModels.h"

#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/client/resources/model/VanillaModels.h"
#include "libmatti/net/minecraft/client/renderer/block/model/BlockModel.h"

#include <stdlib.h>
#include <string.h>

// (ModelManager_RegisterBaked comes from ModelManager.h via the include.)

// The parent-resolved model for an id: the model's own JSON when it carries
// elements, otherwise the parent chain's first element carrier. Java's
// ModelDiscovery walks the chain and merges; the port resolves to the first
// model with elements (cube_all carries the cube for every vanilla cube_all
// child - the textures map rides along on the child).
static LIBMATTI_MC_BlockModel *resolve_model(const char *modelId)
{
    const char *json = LIBMATTI_MC_VanillaModels_ModelById(modelId);
    if (json == NULL)
        return NULL;
    LIBMATTI_MC_BlockModel *model = LIBMATTI_MC_BlockModel_Parse(json);
    if (model == NULL)
        return NULL;
    if (model->elementsPresent)
        return model;

    // Java: the parent walk - merge the child textures over the parent's.
    const char *parentJson = model->parent != NULL ? LIBMATTI_MC_VanillaModels_ModelById(model->parent) : NULL;
    if (parentJson == NULL)
    {
        LIBMATTI_MC_BlockModel_Free(model);
        return NULL;
    }
    LIBMATTI_MC_BlockModel *parent = LIBMATTI_MC_BlockModel_Parse(parentJson);
    LIBMATTI_MC_BlockModel_Free(model);
    return parent;
}

// Java: SpriteGetter.resolveSlot - the atlas rect for a texture id.
static int resolve_sprite(void *userdata, const char *textureId, float uvRect[4])
{
    const LIBMATTI_MC_TextureAtlas *atlas = (const LIBMATTI_MC_TextureAtlas *) userdata;
    return LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, textureId, uvRect);
}

int LIBMATTI_MC_VanillaBlockModels_Bootstrap(LIBMATTI_MC_ModelManager *manager)
{
    if (manager == NULL || manager->blockAtlas == NULL)
        return 0;

    // Java: the ids are compile-time constants; C needs the literal array
    // inline (the extern const char* variables are not constant initializers).
    const char *ids[2];
    ids[0] = LIBMATTI_MC_VanillaModels_STONE_ID;
    ids[1] = LIBMATTI_MC_VanillaModels_DIRT_ID;
    int ok = 1;
    for (size_t i = 0; i < 2; i++)
    {
        if (LIBMATTI_MC_ModelManager_GetModel(manager, ids[i]) != NULL)
            continue;
        LIBMATTI_MC_BlockModel *model = resolve_model(ids[i]);
        if (model == NULL)
        {
            ok = 0;
            continue;
        }
        LIBMATTI_MC_QuadCollection collection;
        if (!LIBMATTI_MC_ModelBaker_Bake(model, resolve_sprite, manager->blockAtlas, &collection))
        {
            ok = 0;
        }
        else if (!LIBMATTI_MC_ModelManager_RegisterBaked(manager, ids[i], &collection))
        {
            ok = 0;
        }
        LIBMATTI_MC_BlockModel_Free(model);
    }
    return ok;
}
