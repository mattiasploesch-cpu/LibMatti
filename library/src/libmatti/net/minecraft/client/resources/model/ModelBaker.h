// Port of the ModelBaker slice the block-model bake needs (Java:
// net.minecraft.client.resources.model.ModelBaker + SimpleUnbakedGeometry).
// The sprite resolution is a callback: the caller maps a texture id to the
// sprite's atlas rect (u0, v0, u1, v1) - the TextureAtlasSprite.getU/getV
// pass the atlas side owns.

#ifndef MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELBAKER_H
#define MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELBAKER_H

#include "libmatti/net/minecraft/client/renderer/block/model/BlockModel.h"
#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: SpriteGetter.resolveSlot(TextureSlots, String, ModelDebugName) - the
// port resolves the slot through the model first (the "#ref" chase), then maps
// the texture id to the sprite rect. userdata rides along.
typedef int (*LIBMATTI_MC_ModelBaker_SpriteResolver)(void *userdata, const char *textureId, float uvRect[4]);

// Java: SimpleUnbakedGeometry.bake(List<BlockElement>, TextureSlots, ModelBaker,
//           ModelState, ModelDebugName) -
// walks the elements, skips degenerate axes exactly like the Java flag chain,
// bakes every face through FaceBakery and buckets by cullface. Returns 0 when
// a sprite could not resolve (Java throws at this point).
int LIBMATTI_MC_ModelBaker_Bake(const LIBMATTI_MC_BlockModel *model, LIBMATTI_MC_ModelBaker_SpriteResolver resolver,
                                void *userdata, LIBMATTI_MC_QuadCollection *out);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_MODELBAKER_H
