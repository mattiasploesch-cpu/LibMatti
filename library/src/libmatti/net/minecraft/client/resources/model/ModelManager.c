// Port of the ModelManager slice (implementation).

#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"

#include "libmatti/net/minecraft/client/renderer/block/model/BlockModel.h"
#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the file read helper - reads the whole model JSON into a buffer.
static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length <= 0)
    {
        fclose(file);
        return NULL;
    }
    char *data = malloc((size_t) length + 1);
    if (fread(data, 1, (size_t) length, file) != (size_t) length)
    {
        free(data);
        fclose(file);
        return NULL;
    }
    data[length] = '\0';
    fclose(file);
    return data;
}

// Java: the bake-time sprite resolver callback shape - the manager binds the
// atlas through the SpriteGetter.
typedef struct ResolverContext
{
    const LIBMATTI_MC_TextureAtlas *atlas;
} ResolverContext;

LIBMATTI_MC_ModelManager *LIBMATTI_MC_ModelManager_New(LIBMATTI_MC_TextureAtlas *atlas)
{
    LIBMATTI_MC_ModelManager *manager = calloc(1, sizeof(LIBMATTI_MC_ModelManager));
    manager->blockAtlas = atlas;
    return manager;
}

int LIBMATTI_MC_ModelManager_LoadModel(LIBMATTI_MC_ModelManager *manager, const char *resourceRoot,
                                       const char *modelId)
{
    if (manager == NULL || resourceRoot == NULL || modelId == NULL)
        return 0;
    if (LIBMATTI_MC_ModelManager_GetModel(manager, modelId) != NULL)
        return 1; // Java: the registry dedupes by id.

    // Java: ModelResourceLocation -> the file path
    // assets/<namespace>/models/<path>.json
    LIBMATTI_MC_Identifier *id = strchr(modelId, ':') != NULL
                                     ? LIBMATTI_MC_Identifier_Parse(modelId)
                                     : LIBMATTI_MC_Identifier_FromNamespaceAndPath("minecraft", modelId);
    if (id == NULL)
        return 0;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s/models/%s.json", resourceRoot,
             LIBMATTI_MC_Identifier_GetNamespace(id), LIBMATTI_MC_Identifier_GetPath(id));
    LIBMATTI_MC_Identifier_Free(id);

    char *json = read_file(path);
    if (json == NULL)
        return 0; // Java: the missing-model fallback.

    LIBMATTI_MC_BlockModel *model = LIBMATTI_MC_BlockModel_Parse(json);
    free(json);
    if (model == NULL)
        return 0;

    // Java: the bake against the block atlas through the SpriteGetter.
    ResolverContext context = {manager->blockAtlas};
    LIBMATTI_MC_QuadCollection collection;
    int ok = LIBMATTI_MC_ModelBaker_Bake(model, (LIBMATTI_MC_ModelBaker_SpriteResolver) NULL, &context, &collection);
    LIBMATTI_MC_BlockModel_Free(model);
    if (!ok)
        return 0;

    // Java: BakedModel registry.put
    char **ids = realloc(manager->modelIds, (manager->modelCount + 1) * sizeof(char *));
    LIBMATTI_MC_QuadCollection *models =
        realloc(manager->models, (manager->modelCount + 1) * sizeof(LIBMATTI_MC_QuadCollection));
    if (ids == NULL || models == NULL)
    {
        LIBMATTI_MC_QuadCollection_Free(&collection);
        return 0;
    }
    manager->modelIds = ids;
    manager->models = models;
    manager->modelIds[manager->modelCount] = strdup(modelId);
    manager->models[manager->modelCount] = collection;
    manager->modelCount++;
    return 1;
}

const LIBMATTI_MC_QuadCollection *LIBMATTI_MC_ModelManager_GetModel(const LIBMATTI_MC_ModelManager *manager,
                                                                    const char *modelId)
{
    if (manager == NULL || modelId == NULL)
        return NULL;
    for (size_t i = 0; i < manager->modelCount; i++)
    {
        if (strcmp(manager->modelIds[i], modelId) == 0)
            return &manager->models[i];
    }
    return NULL;
}

// Java: the registry.put step of the bake - the bootstrap registers
// already-baked collections here.
int LIBMATTI_MC_ModelManager_RegisterBaked(LIBMATTI_MC_ModelManager *manager, const char *modelId,
                                           LIBMATTI_MC_QuadCollection *collection)
{
    if (manager == NULL || modelId == NULL || collection == NULL)
        return 0;
    if (LIBMATTI_MC_ModelManager_GetModel(manager, modelId) != NULL)
    {
        LIBMATTI_MC_QuadCollection_Free(collection);
        return 0; // Java: the registry dedupes.
    }
    char **ids = realloc(manager->modelIds, (manager->modelCount + 1) * sizeof(char *));
    LIBMATTI_MC_QuadCollection *models =
        realloc(manager->models, (manager->modelCount + 1) * sizeof(LIBMATTI_MC_QuadCollection));
    if (ids == NULL || models == NULL)
    {
        LIBMATTI_MC_QuadCollection_Free(collection);
        return 0;
    }
    manager->modelIds = ids;
    manager->models = models;
    manager->modelIds[manager->modelCount] = strdup(modelId);
    manager->models[manager->modelCount] = *collection;
    manager->modelCount++;
    return 1;
}

LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_ModelManager_GetAtlas(const LIBMATTI_MC_ModelManager *manager)
{
    return manager != NULL ? manager->blockAtlas : NULL;
}

void LIBMATTI_MC_ModelManager_Free(LIBMATTI_MC_ModelManager *manager)
{
    if (manager == NULL)
        return;
    for (size_t i = 0; i < manager->modelCount; i++)
    {
        free(manager->modelIds[i]);
        LIBMATTI_MC_QuadCollection_Free(&manager->models[i]);
    }
    free(manager->modelIds);
    free(manager->models);
    free(manager);
}
