// Port of net.minecraft.client.renderer.texture.SpriteContents and
// net.minecraft.client.renderer.texture.TextureAtlasSprite. The animation
// frames (AnimatedTexture/AnimationState) stay game-port content for now: the
// port keeps the first frame and the frame metadata (the animation tick loop
// needs the RenderPipelines sprite-blit pass, which the GL skeleton has not).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLASSPRITE_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLASSPRITE_H

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"
#include "libmatti/net/minecraft/client/resources/metadata/TextureMetadataSections.h"
#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class SpriteContents implements Stitcher.Entry, AutoCloseable
typedef struct LIBMATTI_MC_SpriteContents
{
    // Java: final Identifier name; final int width, height
    LIBMATTI_MC_Identifier *name;
    int width;  // the frame size (SpriteLoader.FrameSize)
    int height;
    // Java: private final NativeImage originalImage
    LIBMATTI_B3D_NativeImage *originalImage;
    // Java: NativeImage[] byMipLevel
    LIBMATTI_B3D_NativeImage **byMipLevel;
    size_t mipLevelCount;
    // Java: the animation metadata (the port keeps the section for the frame count)
    int isAnimated;   // more than one frame in the source image
    int frameCount;
    int defaultFrameTime;
    int interpolatedFrames;
    // Java: private final MipmapStrategy / alphaCutoffBias
    int mipmapStrategy;
    float alphaCutoffBias;
} LIBMATTI_MC_SpriteContents;

// Java: SpriteContents(name, FrameSize, NativeImage, Optional<AnimationMetadataSection>, ...)
// Takes ownership of the image.
LIBMATTI_MC_SpriteContents *LIBMATTI_MC_SpriteContents_New(LIBMATTI_MC_Identifier *name, LIBMATTI_MC_FrameSize frameSize,
                                                           LIBMATTI_B3D_NativeImage *image,
                                                           LIBMATTI_MC_AnimationMetadataSection *animation);
// Java: public void increaseMipLevel(int) - MipmapGenerator.generateMipLevels
void LIBMATTI_MC_SpriteContents_IncreaseMipLevel(LIBMATTI_MC_SpriteContents *contents, int mipLevel);
void LIBMATTI_MC_SpriteContents_Free(LIBMATTI_MC_SpriteContents *contents);

// Java: public class TextureAtlasSprite
typedef struct LIBMATTI_MC_TextureAtlasSprite
{
    // Java: private final SpriteContents contents; atlas, x, y, width, height
    LIBMATTI_MC_SpriteContents *contents;
    LIBMATTI_MC_Identifier *atlas;
    int x;
    int y;
    int width;  // the atlas region width (clipped)
    int height;
} LIBMATTI_MC_TextureAtlasSprite;

LIBMATTI_MC_TextureAtlasSprite *LIBMATTI_MC_TextureAtlasSprite_New(LIBMATTI_MC_Identifier *atlas,
                                                                   LIBMATTI_MC_SpriteContents *contents,
                                                                   int atlasWidth, int atlasHeight,
                                                                   int x, int y, int padding);
// Java: TextureAtlasSprite.getX() / getY()
int LIBMATTI_MC_TextureAtlasSprite_GetX(const LIBMATTI_MC_TextureAtlasSprite *sprite);
int LIBMATTI_MC_TextureAtlasSprite_GetY(const LIBMATTI_MC_TextureAtlasSprite *sprite);
void LIBMATTI_MC_TextureAtlasSprite_Free(LIBMATTI_MC_TextureAtlasSprite *sprite);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_TEXTUREATLASSPRITE_H
