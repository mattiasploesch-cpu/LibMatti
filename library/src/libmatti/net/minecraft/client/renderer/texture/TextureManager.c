// Port of net.minecraft.client.renderer.texture.TextureManager.
// Java keys by Identifier with Identifier equality; the port keys the HashMap
// by the "namespace:path" string (the HashMap port is string-keyed).

#include "libmatti/net/minecraft/client/renderer/texture/TextureManager.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/util/HashMap.h"
#include "libmatti/net/neoforged/fml/Logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER
#define LOGGER() LIBMATTI_ML_LogManager_GetLogger()

// The map key for an identifier: "namespace:path"
static char *location_key(const LIBMATTI_MC_Identifier *location)
{
    size_t size = strlen(location->namespace) + 1 + strlen(location->path) + 1;
    char *key = malloc(size);
    snprintf(key, size, "%s:%s", location->namespace, location->path);
    return key;
}

// Java: private void safeClose(Identifier, AbstractTexture)
static void safe_close(LIBMATTI_MC_TextureManager *manager, const char *key, LIBMATTI_MC_AbstractTexture *texture)
{
    (void) manager;
    (void) key;
    // Java: this.tickableTextures.remove(texture)
    for (size_t i = 0; i < manager->tickableCount; i++)
    {
        if (manager->tickableTextures[i] == texture)
        {
            memmove(&manager->tickableTextures[i], &manager->tickableTextures[i + 1],
                    (manager->tickableCount - i - 1) * sizeof(LIBMATTI_MC_AbstractTexture *));
            manager->tickableCount--;
            break;
        }
    }

    // Java: try { texture.close(); } catch (Exception e) { LOGGER.warn(...) }
    LIBMATTI_MC_AbstractTexture_Close(texture);
    free(texture);
}

// Java: public TextureManager(ResourceManager)
LIBMATTI_MC_TextureManager *LIBMATTI_MC_TextureManager_New(const LIBMATTI_MC_MultiPackResourceManager *resourceManager)
{
    LIBMATTI_MC_TextureManager *manager = calloc(1, sizeof(LIBMATTI_MC_TextureManager));
    manager->byPath = LIBMATTI_JU_HashMap_New();
    manager->resourceManager = resourceManager;

    // Java: NativeImage image = MissingTextureAtlasSprite.generateMissingImage();
    //       this.register(MissingTextureAtlasSprite.getLocation(),
    //                     new DynamicTexture(() -> "(intentionally-)Missing Texture", image));
    LIBMATTI_B3D_NativeImage *missing = LIBMATTI_MC_MissingTextureAtlasSprite_GenerateMissingImage();
    LIBMATTI_MC_AbstractTexture *missingTexture = LIBMATTI_MC_DynamicTexture_New(missing);
    LIBMATTI_MC_Identifier *missingLocation = LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation();

    // Java: DynamicTexture uploads in its constructor; the port uploads when a
    // GL context exists (GetTexture/RegisterAndLoad paths re-upload on load).
    LIBMATTI_MC_TextureManager_Register(manager, missingLocation, missingTexture);
    LIBMATTI_MC_Identifier_Free(missingLocation);
    return manager;
}

// Java: public void register(Identifier, AbstractTexture)
void LIBMATTI_MC_TextureManager_Register(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location,
                                         LIBMATTI_MC_AbstractTexture *texture)
{
    char *key = location_key(location);
    LIBMATTI_MC_AbstractTexture *previous = LIBMATTI_JU_HashMap_Put(manager->byPath, key, texture);

    // Java: if (abstracttexture != p_118497_) { close the old one; track tickable }
    if (previous != texture)
    {
        if (previous != NULL)
            safe_close(manager, key, previous);

        if (texture->tick != NULL)
        {
            if (manager->tickableCount == manager->tickableCapacity)
            {
                manager->tickableCapacity = manager->tickableCapacity == 0 ? 8 : manager->tickableCapacity * 2;
                manager->tickableTextures =
                    realloc(manager->tickableTextures, manager->tickableCapacity * sizeof(LIBMATTI_MC_AbstractTexture *));
            }
            manager->tickableTextures[manager->tickableCount++] = texture;
        }
    }
    free(key);
}

// Java: private TextureContents loadContentsSafe(Identifier, ReloadableTexture)
static LIBMATTI_MC_TextureContents *load_contents_safe(LIBMATTI_MC_TextureManager *manager,
                                                       const LIBMATTI_MC_Identifier *location,
                                                       LIBMATTI_MC_AbstractTexture *texture)
{
    LIBMATTI_MC_TextureContents *contents =
        LIBMATTI_MC_ReloadableTexture_LoadContents(texture, manager->resourceManager);
    if (contents == NULL)
    {
        // Java: LOGGER.error("Failed to load texture {} into slot {}", ...)
        char message[512];
        snprintf(message, sizeof(message), "Failed to load texture %s:%s (missing resource)",
                 texture->resourceId->namespace, texture->resourceId->path);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, message);
        return LIBMATTI_MC_TextureContents_CreateMissing();
    }
    return contents;
}

// Java: public void registerAndLoad(Identifier, ReloadableTexture)
void LIBMATTI_MC_TextureManager_RegisterAndLoad(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location,
                                                LIBMATTI_MC_AbstractTexture *texture)
{
    // Java: texture.apply(this.loadContentsSafe(location, texture))
    LIBMATTI_MC_TextureContents *contents = load_contents_safe(manager, location, texture);
    LIBMATTI_MC_ReloadableTexture_Apply(texture, contents);
    LIBMATTI_MC_TextureContents_Free(contents);

    LIBMATTI_MC_TextureManager_Register(manager, location, texture);
}

// Java: public AbstractTexture getTexture(Identifier)
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_TextureManager_GetTexture(LIBMATTI_MC_TextureManager *manager,
                                                                   const LIBMATTI_MC_Identifier *location)
{
    char *key = location_key(location);
    LIBMATTI_MC_AbstractTexture *texture = LIBMATTI_JU_HashMap_Get(manager->byPath, key);
    free(key);

    if (texture != NULL)
        return texture;

    // Java: SimpleTexture + registerAndLoad
    texture = LIBMATTI_MC_SimpleTexture_New(location);
    LIBMATTI_MC_TextureManager_RegisterAndLoad(manager, location, texture);
    return texture;
}

// Java: public void tick()
void LIBMATTI_MC_TextureManager_Tick(LIBMATTI_MC_TextureManager *manager)
{
    for (size_t i = 0; i < manager->tickableCount; i++)
        manager->tickableTextures[i]->tick(manager->tickableTextures[i]);
}

// Java: public void release(Identifier)
void LIBMATTI_MC_TextureManager_Release(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location)
{
    char *key = location_key(location);
    LIBMATTI_MC_AbstractTexture *texture = LIBMATTI_JU_HashMap_Remove(manager->byPath, key);
    if (texture != NULL)
        safe_close(manager, key, texture);
    free(key);
}

// Java: public void close() { this.byPath.forEach(this::safeClose); ... }
void LIBMATTI_MC_TextureManager_Free(LIBMATTI_MC_TextureManager *manager)
{
    if (manager == NULL)
        return;

    // The map keys are owned by the map (KeySet borrows them - do not free
    // the strings); the values are the textures.
    size_t count = 0;
    char **keys = LIBMATTI_JU_HashMap_KeySet(manager->byPath, &count);
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_AbstractTexture *texture = LIBMATTI_JU_HashMap_Get(manager->byPath, keys[i]);
        if (texture != NULL)
        {
            LIBMATTI_MC_AbstractTexture_Close(texture);
            free(texture);
        }
    }
    free(keys);
    LIBMATTI_JU_HashMap_Free(manager->byPath);
    free(manager->tickableTextures);
    free(manager);
}
