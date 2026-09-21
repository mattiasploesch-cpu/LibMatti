// Port of net.minecraft.client.renderer.texture.TextureAtlas - the stitched
// texture page with the sprite lookup. The animation UBO/blit path of the Java
// class (uploadInitialContents render passes) is the game-port part; the port
// uploads the first frame of every sprite into the atlas texture directly.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLAS_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLAS_H

#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/minecraft/client/renderer/texture/SpriteLoader.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: LOCATION_BLOCKS - the deprecated constants the ports reference
extern const LIBMATTI_MC_Identifier *const LIBMATTI_MC_TextureAtlas_LOCATION_BLOCKS;

// Java: public class TextureAtlas extends AbstractTexture implements TickableTexture
typedef struct LIBMATTI_MC_TextureAtlas
{
    LIBMATTI_MC_AbstractTexture base; // the texture/view/sampler fields

    LIBMATTI_MC_Identifier *location;
    int maxSupportedTextureSize;
    int width;
    int height;
    int maxMipLevel;

    // Java: private Map<Identifier, TextureAtlasSprite> texturesByName
    LIBMATTI_MC_TextureAtlasSprite **sprites;
    size_t spriteCount;
    // Java: private TextureAtlasSprite missingSprite
    LIBMATTI_MC_TextureAtlasSprite *missingSprite;
} LIBMATTI_MC_TextureAtlas;

// Java: public TextureAtlas(Identifier location)
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_TextureAtlas_New(const LIBMATTI_MC_Identifier *location,
                                                       int maxSupportedTextureSize);
void LIBMATTI_MC_TextureAtlas_Free(LIBMATTI_MC_TextureAtlas *atlas);

// Java: public void upload(SpriteLoader.Preparations)
int LIBMATTI_MC_TextureAtlas_Upload(LIBMATTI_MC_TextureAtlas *atlas, LIBMATTI_MC_SpriteLoader_Preparations *preparations);

// Java: public TextureAtlasSprite getSprite(Identifier) - missing sprite fallback
LIBMATTI_MC_TextureAtlasSprite *LIBMATTI_MC_TextureAtlas_GetSprite(const LIBMATTI_MC_TextureAtlas *atlas,
                                                                   const LIBMATTI_MC_Identifier *spriteId);

// Java: public void clearTextureData() / tick()
void LIBMATTI_MC_TextureAtlas_ClearTextureData(LIBMATTI_MC_TextureAtlas *atlas);
void LIBMATTI_MC_TextureAtlas_Tick(LIBMATTI_MC_TextureAtlas *atlas);

// Accessors for the renderer
int LIBMATTI_MC_TextureAtlas_GetWidth(const LIBMATTI_MC_TextureAtlas *atlas);
int LIBMATTI_MC_TextureAtlas_GetHeight(const LIBMATTI_MC_TextureAtlas *atlas);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLAS_H
