






#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKTEXTURES_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKTEXTURES_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: the atlas stitch over the vanilla block textures - builds the sprite
// sources for block/stone + block/dirt (plus the missing sprite the loader
// appends) and uploads them into the block atlas. Returns the atlas with the
// sprites registered, NULL when the stitch or upload fails.
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_VanillaBlockTextures_Bootstrap(int maxTextureSize);

#ifdef __cplusplus
}
#endif

#endif
