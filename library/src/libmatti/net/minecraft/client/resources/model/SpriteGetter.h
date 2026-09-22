// Port of net.minecraft.client.resources.model.SpriteGetter (the atlas side of
// the bake). Java resolves the TextureAtlasSprite and reads getU/getV per
// corner; the port hands the sprite's atlas rect out as (u0, v0, u1, v1) - the
// exact numbers FaceBakery folds into the quad UVs.

#ifndef MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_SPRITEGETTER_H
#define MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_SPRITEGETTER_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: TextureAtlasSprite getSprite(Identifier) - through the atlas, with
// the missing-sprite fallback when the id is unknown. textureId is a
// "block/stone"-style path; the port builds the Identifier ("minecraft" ns).
int LIBMATTI_MC_SpriteGetter_SpriteRect(const LIBMATTI_MC_TextureAtlas *atlas, const char *textureId,
                                        float uvRect[4]);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_SPRITEGETTER_H
