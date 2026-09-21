// Implementation of the MissingTextureAtlasSprite SpriteContents factory.

#include "libmatti/net/minecraft/client/renderer/texture/MissingTextureAtlasSprite.h"

#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"

#include <stdlib.h>

LIBMATTI_MC_SpriteContents *LIBMATTI_MC_MissingTextureAtlasSprite_Create(void)
{
    // Java: NativeImage image = generateMissingImage(16, 16);
    //       return new SpriteContents(MISSING_TEXTURE_LOCATION, new FrameSize(16, 16), image);
    LIBMATTI_B3D_NativeImage *image = LIBMATTI_MC_MissingTextureAtlasSprite_GenerateMissingImage();
    LIBMATTI_MC_Identifier *name = LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation();
    LIBMATTI_MC_FrameSize frameSize = {16, 16};
    return LIBMATTI_MC_SpriteContents_New(name, frameSize, image, NULL);
}
