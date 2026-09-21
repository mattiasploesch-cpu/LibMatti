// Port of net.minecraft.client.renderer.texture.MissingTextureAtlasSprite -
// the SpriteContents.create() factory the SpriteSourceList appends.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_MISSINGTEXTUREATLASSPRITE_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_MISSINGTEXTUREATLASSPRITE_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static SpriteContents create() - the 16x16 checkerboard as a
// SpriteContents named "minecraft:missingno"
LIBMATTI_MC_SpriteContents *LIBMATTI_MC_MissingTextureAtlasSprite_Create(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_MISSINGTEXTUREATLASSPRITE_H
