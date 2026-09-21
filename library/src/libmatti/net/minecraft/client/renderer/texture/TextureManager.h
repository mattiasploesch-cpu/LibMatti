// Port of net.minecraft.client.renderer.texture.TextureManager (the
// Identifier -> AbstractTexture registry with the load-on-demand path).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREMANAGER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREMANAGER_H

#include "libmatti/java/util/HashMap.h"
#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class TextureManager implements PreparableReloadListener, AutoCloseable
typedef struct LIBMATTI_MC_TextureManager
{
    // Java: private final Map<Identifier, AbstractTexture> byPath - keyed by
    // the "namespace:path" string (the port's key format)
    LIBMATTI_JU_HashMap *byPath;
    // Java: private final Set<TickableTexture> tickableTextures
    LIBMATTI_MC_AbstractTexture **tickableTextures;
    size_t tickableCount;
    size_t tickableCapacity;
    // Java: private final ResourceManager resourceManager
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager;
} LIBMATTI_MC_TextureManager;

// Java: public TextureManager(ResourceManager) - registers the missing texture
LIBMATTI_MC_TextureManager *LIBMATTI_MC_TextureManager_New(const LIBMATTI_MC_MultiPackResourceManager *resourceManager);
void LIBMATTI_MC_TextureManager_Free(LIBMATTI_MC_TextureManager *manager);

// Java: public void register(Identifier, AbstractTexture)
void LIBMATTI_MC_TextureManager_Register(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location,
                                         LIBMATTI_MC_AbstractTexture *texture);
// Java: public void registerAndLoad(Identifier, ReloadableTexture)
void LIBMATTI_MC_TextureManager_RegisterAndLoad(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location,
                                                LIBMATTI_MC_AbstractTexture *texture);
// Java: public AbstractTexture getTexture(Identifier) - loads SimpleTextures on demand
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_TextureManager_GetTexture(LIBMATTI_MC_TextureManager *manager,
                                                                   const LIBMATTI_MC_Identifier *location);
// Java: public void tick()
void LIBMATTI_MC_TextureManager_Tick(LIBMATTI_MC_TextureManager *manager);
// Java: public void release(Identifier)
void LIBMATTI_MC_TextureManager_Release(LIBMATTI_MC_TextureManager *manager, const LIBMATTI_MC_Identifier *location);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREMANAGER_H
