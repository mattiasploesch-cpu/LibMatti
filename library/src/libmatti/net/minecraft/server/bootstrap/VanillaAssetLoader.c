// The dynamic vanilla asset loader (implementation). Java's startup reload
// runs SpriteSourceList -> SpriteLoader -> TextureAtlas over the resource
// manager, then ModelBakery bakes the block models the blockstates resolve
// to - all driven by the pack's JSON + PNG files. The port runs the same
// pipeline over the embedded pack (the gzip blob in RAM): a new block ships
// as blockstate + model + texture and renders without a code change.

#include "libmatti/net/minecraft/server/bootstrap/VanillaAssetLoader.h"

#include "libmatti/net/minecraft/client/renderer/texture/SpriteLoader.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/resources/model/ModelBaker.h"
#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/client/resources/model/VanillaModels.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"
#include "libmatti/net/minecraft/server/packs/resources/Resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: SpriteGetter.resolveSlot - the atlas rect for a texture id (the
// bake resolves the model's texture slots through it).
static int resolve_sprite(void *userdata, const char *textureId, float uvRect[4])
{
    return LIBMATTI_MC_SpriteGetter_SpriteRect((const LIBMATTI_MC_TextureAtlas *) userdata, textureId, uvRect);
}

static LIBMATTI_MC_ModelManager *modelManager = NULL;
static LIBMATTI_MC_MultiPackResourceManager *resourceManager = NULL;
static size_t packEntryCount = 0;

// Java: ModelBakery - the model id the blockstate resolves to ("block/<path>"
// for the cube family the demo renders). The loader walks the blockstate
// directory through the pack's listResources and bakes every referenced
// block model it finds.
typedef struct ModelIdList
{
    char **ids;
    size_t count;
    size_t capacity;
} ModelIdList;

static void model_list_add(ModelIdList *list, const char *id)
{
    for (size_t i = 0; i < list->count; i++)
    {
        if (strcmp(list->ids[i], id) == 0)
            return; // Java: the registry dedupes by id
    }
    if (list->count == list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 64;
        list->ids = realloc(list->ids, list->capacity * sizeof(char *));
        if (list->ids == NULL)
            return;
    }
    list->ids[list->count] = strdup(id);
    if (list->ids[list->count] != NULL)
        list->count++;
}

// Java: BlockModelDefinition - the blockstate JSON's "variants" map the
// model ids per state. The port scans the raw JSON for "model" values
// (the state machine the demo's blocks use is the identity state).
static void collect_models_from_blockstate(const unsigned char *data, size_t length, ModelIdList *list)
{
    // the "model": "..." values over the blockstate JSON (the variants carry
    // the model resource ids, "minecraft:block/stone" style)
    char *text = malloc(length + 1);
    if (text == NULL)
        return;
    memcpy(text, data, length);
    text[length] = '\0';

    const char *cursor = text;
    while ((cursor = strstr(cursor, "\"model\"")) != NULL)
    {
        cursor += strlen("\"model\"");
        while (*cursor == ' ' || *cursor == ':' || *cursor == '\n' || *cursor == '\t')
            cursor++;
        if (*cursor != '"')
            continue;
        cursor++;
        const char *end = strchr(cursor, '"');
        if (end == NULL)
            break;
        char model[256];
        size_t length2 = (size_t) (end - cursor);
        if (length2 >= sizeof(model))
            continue;
        memcpy(model, cursor, length2);
        model[length2] = '\0';
        // Java: the model id normalizes to the namespace:path form
        if (strchr(model, ':') == NULL)
        {
            char withNamespace[276];
            snprintf(withNamespace, sizeof(withNamespace), "minecraft:%s", model);
            model_list_add(list, withNamespace);
        }
        else
        {
            model_list_add(list, model);
        }
        cursor = end;
    }
    free(text);
}

// The blockstate walk callback: read every state JSON, harvest its model ids.
typedef struct StateWalkContext
{
    LIBMATTI_MC_MultiPackResourceManager *resources;
    ModelIdList *list;
} StateWalkContext;

// Java: ResourceManager.getResource -> the bytes (the port's Resource.open
// mallocs the stream contents; the loader reads the blockstate JSON through
// it and hands the buffer to the state walker).
static unsigned char *read_pack_resource(LIBMATTI_MC_MultiPackResourceManager *resources, const char *ns,
                                         const char *path, size_t *outLength)
{
    *outLength = 0;
    LIBMATTI_MC_Resource *resource =
        LIBMATTI_MC_MultiPackResourceManager_GetResource(resources, ns, path);
    if (resource == NULL)
        return NULL;
    unsigned char *data = LIBMATTI_MC_Resource_Open(resource, outLength);
    return data;
}

static void state_walk_callback(void *userData, const char *ns, const char *path, int hasResource, size_t length)
{
    (void) length;
    StateWalkContext *context = userData;
    if (!hasResource)
        return; // the directory members pass through (the walk structure)
    if (strncmp(path, "blockstates/", 12) != 0)
        return;
    // Java: the blockstate resource id drops the .json suffix (the block
    // registry key path) - the pack stores the FULL path, so the read goes to
    // "blockstates/<name>.json" as-is.
    size_t jsonLength = 0;
    unsigned char *data = read_pack_resource(context->resources, ns, path, &jsonLength);
    if (data == NULL)
        return;
    collect_models_from_blockstate(data, jsonLength, context->list);
    free(data);
}

// Java: ModelBakery.loadBlockModels - the model walk follows the parent chain
// (the child's bake needs the parent's ELEMENTS - cube_all et al. carry the
// cube) + the texture slots through the bake. The port resolves the parent
// chain here: merge the child's textures over the parsed parent and bake the
// element carrier (the VanillaBlockModels.merge_textures flow over the pack).
static LIBMATTI_MC_BlockModel *load_pack_model(LIBMATTI_MC_MultiPackResourceManager *resources,
                                               const char *modelId)
{
    char namespaceBuffer[128];
    char pathBuffer[512];
    const char *colon = strchr(modelId, ':');
    if (colon == NULL)
    {
        snprintf(namespaceBuffer, sizeof(namespaceBuffer), "minecraft");
        snprintf(pathBuffer, sizeof(pathBuffer), "models/%s.json", modelId);
    }
    else
    {
        snprintf(namespaceBuffer, sizeof(namespaceBuffer), "%.*s", (int) (colon - modelId), modelId);
        snprintf(pathBuffer, sizeof(pathBuffer), "models/%s.json", colon + 1);
    }
    size_t jsonLength = 0;
    unsigned char *data = read_pack_resource(resources, namespaceBuffer, pathBuffer, &jsonLength);
    if (data == NULL)
        return NULL;
    LIBMATTI_MC_BlockModel *model = LIBMATTI_MC_BlockModel_Parse((const char *) data);
    free(data);
    return model;
}

// Java: TextureSlots.Data - the child's textures override the parent's map
// entries by slot name; new slots append (the VanillaBlockModels merge flow
// over the pack-resolved chain).
static void merge_model_textures(LIBMATTI_MC_BlockModel *parent, const LIBMATTI_MC_BlockModel *child)
{
    for (size_t c = 0; c < child->textureCount; c++)
    {
        size_t slot = parent->textureCount;
        for (size_t p = 0; p < parent->textureCount; p++)
        {
            if (strcmp(parent->textureNames[p], child->textureNames[c]) == 0)
            {
                slot = p;
                break;
            }
        }
        if (slot == parent->textureCount)
        {
            char **names = realloc(parent->textureNames, (parent->textureCount + 1) * sizeof(char *));
            char **values = realloc(parent->textureValues, (parent->textureCount + 1) * sizeof(char *));
            if (names == NULL || values == NULL)
                continue;
            parent->textureNames = names;
            parent->textureValues = values;
            parent->textureNames[slot] = strdup(child->textureNames[c]);
            parent->textureValues[slot] = strdup(child->textureValues[c]);
            parent->textureCount++;
        }
        else
        {
            free(parent->textureValues[slot]);
            parent->textureValues[slot] = strdup(child->textureValues[c]);
        }
    }
}

// Java: the parent walk (ModelDiscovery) - the child inherits the parent's
// ELEMENTS and the child's textures merge over the parent's slots (the
// cube_all carrier + the "all": "block/stone" mapping the child describes).
static LIBMATTI_MC_BlockModel *resolve_pack_model(LIBMATTI_MC_MultiPackResourceManager *resources,
                                                  const char *modelId, int depth)
{
    if (depth > 8)
        return NULL; // Java: the parent cycle guard
    LIBMATTI_MC_BlockModel *model = load_pack_model(resources, modelId);
    if (model == NULL)
        return NULL;
    if (model->elementsPresent || model->parent == NULL)
        return model;
    // the parent id normalizes to the namespaced form ("block/cube_all")
    char parentBuffer[512];
    const char *parentPath = model->parent;
    if (strchr(parentPath, ':') != NULL)
        parentPath = strchr(parentPath, ':') + 1;
    snprintf(parentBuffer, sizeof(parentBuffer), "minecraft:%s", parentPath);
    LIBMATTI_MC_BlockModel *resolvedParent = resolve_pack_model(resources, parentBuffer, depth + 1);
    if (resolvedParent == NULL)
        return model; // the child bakes alone (the parent is broken)
    // Java: the resolved model = the parent's elements + the merged textures
    merge_model_textures(resolvedParent, model);
    LIBMATTI_MC_BlockModel_Free(model);
    return resolvedParent;
}

// Java: ModelBakery.bake - the parent-resolved model bakes against the atlas
// and registers under its id (the registry the renderers walk).
static int bake_model(LIBMATTI_MC_ModelManager *manager, LIBMATTI_MC_MultiPackResourceManager *resources,
                      const char *modelId)
{
    if (LIBMATTI_MC_ModelManager_GetModel(manager, modelId) != NULL)
        return 1; // Java: the registry dedupes by id

    LIBMATTI_MC_BlockModel *model = resolve_pack_model(resources, modelId, 0);
    if (model == NULL)
    {
        static int loadFails = 0;
        if (loadFails++ < 3)
            fprintf(stderr, "[ASSETS] bake %s: model NULL\n", modelId);
        return 0;
    }

    LIBMATTI_MC_QuadCollection collection;
    int baked = LIBMATTI_MC_ModelBaker_Bake(model, resolve_sprite, manager->blockAtlas, &collection);
    LIBMATTI_MC_BlockModel_Free(model);
    if (!baked)
    {
        static int bakeFails = 0;
        if (bakeFails++ < 3)
            fprintf(stderr, "[ASSETS] bake %s: baker refused (missing sprite?)\n", modelId);
        return 0;
    }
    if (!LIBMATTI_MC_ModelManager_RegisterBaked(manager, modelId, &collection))
    {
        LIBMATTI_MC_QuadCollection_Free(&collection);
        return 0;
    }
    return 1;
}

int LIBMATTI_MC_VanillaAssetLoader_Load(void)
{
    // Java: the resource manager over the built-in pack (the embedded blob)
    LIBMATTI_MC_EmbeddedPackResources *pack = LIBMATTI_MC_EmbeddedPackResources_New();
    if (pack == NULL)
    {
        fprintf(stderr, "[ASSETS] embedded pack unavailable\n");
        return 0;
    }
    LIBMATTI_MC_PackResources *packs[1] = {(LIBMATTI_MC_PackResources *) pack};
    resourceManager =
        LIBMATTI_MC_MultiPackResourceManager_New(LIBMATTI_MC_PackType_CLIENT_RESOURCES, packs, 1);
    packEntryCount = LIBMATTI_MC_EmbeddedPackResources_EntryCount(pack);

    // Java: MODEL_ATLAS - the block atlas stitches through the atlas JSON
    LIBMATTI_MC_Identifier *atlasId = LIBMATTI_MC_Identifier_WithDefaultNamespace("blocks");
    LIBMATTI_MC_SpriteLoader *loader = LIBMATTI_MC_SpriteLoader_Create(atlasId, 1024);
    LIBMATTI_MC_SpriteLoader_Preparations *preparations =
        LIBMATTI_MC_SpriteLoader_LoadAndStitch(loader, resourceManager, atlasId, 0);
    LIBMATTI_MC_SpriteLoader_Free(loader);
    if (preparations == NULL)
    {
        fprintf(stderr, "[ASSETS] atlas stitch failed - the procedural fallback stays\n");
        LIBMATTI_MC_Identifier_Free(atlasId);
        return 0;
    }
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_TextureAtlas_New(atlasId, 1024);
    if (!LIBMATTI_MC_TextureAtlas_Upload(atlas, preparations))
    {
        fprintf(stderr, "[ASSETS] atlas upload failed\n");
        LIBMATTI_MC_TextureAtlas_Free(atlas);
        LIBMATTI_MC_SpriteLoader_Preparations_Free(preparations);
        LIBMATTI_MC_Identifier_Free(atlasId);
        return 0;
    }
    // Java: TextureAtlas.upload TAKES the preparations' sprites over (the
    // atlas owns them now) - the preparations struct drops its references
    // through ClearTextureData; freeing the sprites here would be the
    // use-after-free the window harness hit (the atlas then reads freed
    // SpriteContents on the first GetSprite).
    // LIBMATTI_MC_SpriteLoader_Preparations_Free(preparations);
    LIBMATTI_MC_Identifier_Free(atlasId);

    modelManager = LIBMATTI_MC_ModelManager_New(atlas);
    fprintf(stderr, "[ASSETS] block atlas stitched (%d sprites, page %dx%d)\n",
            (int) atlas->spriteCount, atlas->width, atlas->height);

    // Java: MODEL_BAKERY - bake the block models the blockstates reference.
    // The atlas carries the cube_all family's sprites; the blockstate walk
    // harvests the model ids dynamically (a new block = new JSON files).
    ModelIdList list = {0};
    StateWalkContext context = {resourceManager, &list};
    // Java: listResources("blockstates", ...) over every namespace - the
    // FallbackResourceManager fans the walk out per namespace; the callback
    // reads each blockstate JSON through getResource.
    size_t namespaceCount = 0;
    char **namespaces = LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(resourceManager, &namespaceCount);
    for (size_t i = 0; i < namespaceCount; i++)
    {
        LIBMATTI_MC_MultiPackResourceManager_ListResources(resourceManager, "blockstates", &context,
                                                            state_walk_callback);
    }
    for (size_t i = 0; i < namespaceCount; i++)
        free(namespaces[i]);
    free(namespaces);

    size_t bakedCount = 0;
    for (size_t i = 0; i < list.count; i++)
    {
        if (bake_model(modelManager, resourceManager, list.ids[i]))
            bakedCount++;
        free(list.ids[i]);
    }
    free(list.ids);
    fprintf(stderr, "[ASSETS] block models baked (%d of %d referenced)\n", (int) bakedCount, (int) list.count);
    return 1;
}

LIBMATTI_MC_ModelManager *LIBMATTI_MC_VanillaAssetLoader_GetModelManager(void)
{
    return modelManager;
}

LIBMATTI_MC_MultiPackResourceManager *LIBMATTI_MC_VanillaAssetLoader_GetResourceManager(void)
{
    return resourceManager;
}

size_t LIBMATTI_MC_VanillaAssetLoader_PackEntryCount(void)
{
    return packEntryCount;
}
