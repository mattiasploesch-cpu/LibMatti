// Port of the ModelManager slice the block rendering needs (Java:
// net.minecraft.client.resources.model.ModelManager + ModelBakery): load the
// block-model JSONs, bake them against the block atlas and hand the baked
// quads to the renderers. The port keeps the baked state in a flat
// id -> QuadCollection table with the vanilla block model paths.

#ifndef MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELMANAGER_H
#define MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELMANAGER_H

#include "libmatti/net/minecraft/client/resources/model/ModelBaker.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: ModelManager - the baked registry.
typedef struct LIBMATTI_MC_ModelManager
{
    LIBMATTI_MC_TextureAtlas *blockAtlas;
    // Java: Map<ModelResourceLocation, BakedModel> - the port stores the
    // baked quads per model id ("block/stone" etc.), insertion ordered.
    size_t modelCount;
    char **modelIds;
    LIBMATTI_MC_QuadCollection *models;
} LIBMATTI_MC_ModelManager;

// Java: ModelManager::<init>(TextureAtlas) - the atlas must be uploaded.
LIBMATTI_MC_ModelManager *LIBMATTI_MC_ModelManager_New(LIBMATTI_MC_TextureAtlas *atlas);

// Java: ModelBakery.loadBlockModels - parse one model JSON from a resource
// root and bake it immediately. resourceRoot is the assets directory
// (".../assets"), modelId the "minecraft:block/stone" style id. Returns 0 on
// missing/invalid input (Java: the missing-model fallback takes over).
int LIBMATTI_MC_ModelManager_LoadModel(LIBMATTI_MC_ModelManager *manager, const char *resourceRoot,
                                       const char *modelId);

// Java: BakedModel registry access - NULL when the id was never loaded.
const LIBMATTI_MC_QuadCollection *LIBMATTI_MC_ModelManager_GetModel(const LIBMATTI_MC_ModelManager *manager,
                                                                    const char *modelId);

// Java: the registry.put step of the bake - registers an already-baked
// collection under the id (the vanilla bootstrap bakes outside LoadModel).
// Takes over the collection on success; returns 0 when the id exists.
int LIBMATTI_MC_ModelManager_RegisterBaked(LIBMATTI_MC_ModelManager *manager, const char *modelId,
                                           LIBMATTI_MC_QuadCollection *collection);

// The block atlas (the renderers bind it before drawing quads).
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_ModelManager_GetAtlas(const LIBMATTI_MC_ModelManager *manager);

// Java: close() - frees the baked models, keeps the atlas (the TextureManager owns it).
void LIBMATTI_MC_ModelManager_Free(LIBMATTI_MC_ModelManager *manager);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELMANAGER_H
