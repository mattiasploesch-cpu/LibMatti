// Port of net.minecraft.client.renderer.block.model.FaceBakery (the slice the
// section compiler needs) plus the FaceInfo corner table: the four cube-corner
// positions per face in Java's winding order and the default per-face UVs.
// Positions are in the block-model 0..16 space; the compiler divides by 16.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_MODELBLOCKRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_MODELBLOCKRENDERER_H

#include "libmatti/net/minecraft/core/Direction.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: FaceInfo - the four (extent, extent, extent) corners per face. The
// extents are 0 = MIN, 1 = MAX; the axis order per corner is (x, y, z).
extern const unsigned char LIBMATTI_MC_FaceInfo_CORNERS[6][4][3];

// Java: FaceBakery.defaultFaceUV(from, to, facing) - the raw 0..16 UV bounds.
// The result is {minU, minV, maxU, maxV} in the UV space.
void LIBMATTI_MC_FaceBakery_DefaultFaceUV(float fromX, float fromY, float fromZ,
                                          float toX, float toY, float toZ,
                                          LIBMATTI_MC_Direction facing, float out[4]);

// Java: BlockElementFace.UVs.getVertexU/getVertexV with Quadrant rotation 0 -
// the per-corner UV in the 0..1 space: corners 0/1 use minU, corners 2/3 use
// maxU; corners 1/2 use maxV, corners 0/3 use minV.
float LIBMATTI_MC_BlockElementFace_GetCornerU(const float uv[4], int corner);
float LIBMATTI_MC_BlockElementFace_GetCornerV(const float uv[4], int corner);

// Java: the render-side quad type the section compiler packs. Positions are
// the final block-space corners, uv the atlas coordinates and light the packed
// UV2 pair the BLOCK format carries.
typedef struct LIBMATTI_MC_ModelQuad
{
    float pos[4][3];
    float uv[4][2];
    int light;
} LIBMATTI_MC_ModelQuad;

// Java: FaceBakery.bakeQuad for the unrotated vanilla cube face with the
// default element UV bounds (0..16): positions are the corners translated by
// the block offset, UVs go through sprite.getU/getV - the caller passes the
// sprite's atlas rect {u0, v0, u1, v1} in the final 0..1 space and the corner
// selection picks min/max exactly like the default bounds do.
void LIBMATTI_MC_ModelBlockRenderer_BakeCubeFace(LIBMATTI_MC_Direction facing,
                                                 float blockX, float blockY, float blockZ,
                                                 const float spriteUv[4],
                                                 LIBMATTI_MC_ModelQuad *outQuad);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_MODELBLOCKRENDERER_H
