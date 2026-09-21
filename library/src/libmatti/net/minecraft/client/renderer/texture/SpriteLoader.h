// Port of net.minecraft.client.renderer.texture.SpriteLoader and its
// Preparations record. The sprite suppliers (SpriteSourceList) feed the
// stitch; the port runs everything synchronously (Java's executor chain).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_SPRITELOADER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_SPRITELOADER_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: record SpriteLoader.Preparations(int width, int height, int mipLevel,
//       TextureAtlasSprite missing, Map<Identifier, TextureAtlasSprite> regions, ...)
typedef struct LIBMATTI_MC_SpriteLoader_Preparations
{
    int width;
    int height;
    int mipLevel;
    LIBMATTI_MC_TextureAtlasSprite *missing;
    // Java: Map<Identifier, TextureAtlasSprite> regions
    LIBMATTI_MC_TextureAtlasSprite **sprites;
    LIBMATTI_MC_Identifier **spriteNames;
    size_t spriteCount;
} LIBMATTI_MC_SpriteLoader_Preparations;

// Java: public class SpriteLoader
typedef struct LIBMATTI_MC_SpriteLoader
{
    LIBMATTI_MC_Identifier *location;
    int maxSupportedTextureSize;
} LIBMATTI_MC_SpriteLoader;

// Java: public static SpriteLoader create(TextureAtlas)
LIBMATTI_MC_SpriteLoader *LIBMATTI_MC_SpriteLoader_Create(const LIBMATTI_MC_Identifier *atlasLocation,
                                                          int maxSupportedTextureSize);
void LIBMATTI_MC_SpriteLoader_Free(LIBMATTI_MC_SpriteLoader *loader);

// Java: public CompletableFuture<Preparations> loadAndStitch(ResourceManager, Identifier, int mipLevel, ...)
// The sprite list comes from the SpriteSourceList run over the atlas
// definition; the port takes the resource manager and atlas location.
LIBMATTI_MC_SpriteLoader_Preparations *LIBMATTI_MC_SpriteLoader_LoadAndStitch(
    LIBMATTI_MC_SpriteLoader *loader, const LIBMATTI_MC_MultiPackResourceManager *resourceManager,
    const LIBMATTI_MC_Identifier *atlasInfo, int mipLevel);
void LIBMATTI_MC_SpriteLoader_Preparations_Free(LIBMATTI_MC_SpriteLoader_Preparations *preparations);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_SPRITELOADER_H
