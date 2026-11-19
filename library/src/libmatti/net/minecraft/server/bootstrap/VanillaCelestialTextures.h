





#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLACELESTIALTEXTURES_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLACELESTIALTEXTURES_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: the CELESTIALS atlas stitch over environment/sun and the eight
// environment/moon/<phase> sprites. Returns the atlas with the sprites
// registered, NULL when the stitch or upload fails.
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_VanillaCelestialTextures_Bootstrap(int maxTextureSize);

#ifdef __cplusplus
}
#endif

#endif
