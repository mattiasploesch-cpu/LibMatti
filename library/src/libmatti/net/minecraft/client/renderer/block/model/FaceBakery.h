// Port of net.minecraft.client.renderer.block.model.FaceBakery.
// Every formula follows the vendored 1.21.11 source: defaultFaceUV's
// per-direction UV derivation, bakeVertex's coordinate/UV pipeline, the
// calculateFacing normal snap and recalculateWinding.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_FACEBAKERY_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_FACEBAKERY_H

#include "libmatti/net/minecraft/client/renderer/block/model/BlockElement.h"
#include "libmatti/net/minecraft/client/renderer/block/model/BlockElementFace.h"
#include "libmatti/net/minecraft/core/Direction.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: BakedQuad's vertex/UV pairs before the quad record wraps them.
typedef struct LIBMATTI_MC_BakedQuad
{
    // Java: the four vertex positions in block space (already /16 scaled).
    float pos[4][3];
    // Java: the four packed UV pairs (the sprite-atlas space, u then v).
    float uv[4][2];
    int tintIndex;
    // Java: the calculated face direction (UP when the snap fails).
    LIBMATTI_MC_Direction direction;
    // Java: boolean shade
    int shade;
} LIBMATTI_MC_BakedQuad;

// Java: static BlockElementFace.UVs defaultFaceUV(Vector3fc from, Vector3fc to, Direction) -
// the per-direction UV derivation over the 0..16 model space.
void LIBMATTI_MC_FaceBakery_DefaultFaceUV(const float from[3], const float to[3], LIBMATTI_MC_Direction direction,
                                          LIBMATTI_MC_BlockElementFace_UVs *out);

// Java: public static BakedQuad bakeQuad(Vector3fc from, Vector3fc to, BlockElementFace,
//           sprite getter context, Direction, ModelState, @Nullable BlockElementRotation,
//           boolean shade, int tintIndex) -
// the port takes the sprite rect directly (the atlas sprite's u0..u1/v0..v1 in
// the atlas space) plus the element's shade flag; the ModelState part rides on
// the vanilla identity state until the model transformations are ported.
void LIBMATTI_MC_FaceBakery_BakeQuad(const float from[3], const float to[3],
                                     const LIBMATTI_MC_BlockElementFace *face,
                                     const float spriteUvRect[4], LIBMATTI_MC_Direction direction,
                                     const LIBMATTI_MC_BlockElementRotation *rotation,
                                     int tintIndex, int shade, LIBMATTI_MC_BakedQuad *out);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_FACEBAKERY_H
