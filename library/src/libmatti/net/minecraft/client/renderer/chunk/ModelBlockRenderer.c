// Port of net.minecraft.client.renderer.FaceInfo + the FaceBakery slice the
// section compiler needs (see ModelBlockRenderer.h).

#include "libmatti/net/minecraft/client/renderer/chunk/ModelBlockRenderer.h"

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"

#include <stddef.h>

// Java: the default element bounds (0, 0) - (16, 16) the caller passes when no
// explicit uv block overrides them. The corner selection over these bounds is
// the identity, so the mapping stays sprite.getU(corner / 16).
static const float g_default_uv[4] = {0.0f, 0.0f, 16.0f, 16.0f};

// Java: FaceInfo.DOWN..EAST - the corner table in the enum's declaration order
// (DOWN, UP, NORTH, SOUTH, WEST, EAST), each face listing its four corners in
// the winding Java bakes and draws. Extents: 0 = MIN, 1 = MAX; per corner
// (x, y, z).
const unsigned char LIBMATTI_MC_FaceInfo_CORNERS[6][4][3] = {
    // DOWN: (MIN_X, MIN_Y, MAX_Z), (MIN_X, MIN_Y, MIN_Z), (MAX_X, MIN_Y, MIN_Z), (MAX_X, MIN_Y, MAX_Z)
    {{0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1}},
    // UP: (MIN_X, MAX_Y, MIN_Z), (MIN_X, MAX_Y, MAX_Z), (MAX_X, MAX_Y, MAX_Z), (MAX_X, MAX_Y, MIN_Z)
    {{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},
    // NORTH: (MAX_X, MAX_Y, MIN_Z), (MAX_X, MIN_Y, MIN_Z), (MIN_X, MIN_Y, MIN_Z), (MIN_X, MAX_Y, MIN_Z)
    {{1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}},
    // SOUTH: (MIN_X, MAX_Y, MAX_Z), (MIN_X, MIN_Y, MAX_Z), (MAX_X, MIN_Y, MAX_Z), (MAX_X, MAX_Y, MAX_Z)
    {{0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}},
    // WEST: (MIN_X, MAX_Y, MIN_Z), (MIN_X, MIN_Y, MIN_Z), (MIN_X, MIN_Y, MAX_Z), (MIN_X, MAX_Y, MAX_Z)
    {{0, 1, 0}, {0, 0, 0}, {0, 0, 1}, {0, 1, 1}},
    // EAST: (MAX_X, MAX_Y, MAX_Z), (MAX_X, MIN_Y, MAX_Z), (MAX_X, MIN_Y, MIN_Z), (MAX_X, MAX_Y, MIN_Z)
    {{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}},
};

// Java: FaceBakery.defaultFaceUV(from, to, facing).
void LIBMATTI_MC_FaceBakery_DefaultFaceUV(float fromX, float fromY, float fromZ,
                                          float toX, float toY, float toZ,
                                          LIBMATTI_MC_Direction facing, float out[4])
{
    switch (facing)
    {
    case LIBMATTI_MC_Direction_DOWN:
        out[0] = fromX;
        out[1] = 16.0f - toZ;
        out[2] = toX;
        out[3] = 16.0f - fromZ;
        break;
    case LIBMATTI_MC_Direction_UP:
        out[0] = fromX;
        out[1] = fromZ;
        out[2] = toX;
        out[3] = toZ;
        break;
    case LIBMATTI_MC_Direction_NORTH:
        out[0] = 16.0f - toX;
        out[1] = 16.0f - toY;
        out[2] = 16.0f - fromX;
        out[3] = 16.0f - fromY;
        break;
    case LIBMATTI_MC_Direction_SOUTH:
        out[0] = fromX;
        out[1] = 16.0f - toY;
        out[2] = toX;
        out[3] = 16.0f - fromY;
        break;
    case LIBMATTI_MC_Direction_WEST:
        out[0] = fromZ;
        out[1] = 16.0f - toY;
        out[2] = toZ;
        out[3] = 16.0f - fromY;
        break;
    case LIBMATTI_MC_Direction_EAST:
        out[0] = 16.0f - toZ;
        out[1] = 16.0f - toY;
        out[2] = 16.0f - fromZ;
        out[3] = 16.0f - fromY;
        break;
    }
}

// Java: BlockElementFace.UVs.getVertexU (corners 2/3 -> maxU) and getVertexV
// (corners 1/2 -> maxV); Quadrant.rotateVertexIndex is the identity for the
// unrotated faces the compiler bakes.
float LIBMATTI_MC_BlockElementFace_GetCornerU(const float uv[4], int corner)
{
    return (corner == 0 || corner == 1) ? uv[0] : uv[2];
}

float LIBMATTI_MC_BlockElementFace_GetCornerV(const float uv[4], int corner)
{
    return (corner == 1 || corner == 2) ? uv[3] : uv[1];
}

// Java: bakeVertex - position from the corner table (translated by the block
// offset), UV through sprite.getU/getV: the caller passes the sprite's atlas
// rect (u0, v0, u1, v1) and the default 0..16 element bounds make the corner
// selection pick u0/v0 or u1/v1 - exactly the identity mapping.
void LIBMATTI_MC_ModelBlockRenderer_BakeCubeFace(LIBMATTI_MC_Direction facing,
                                                 float blockX, float blockY, float blockZ,
                                                 const float spriteUv[4],
                                                 LIBMATTI_MC_ModelQuad *outQuad)
{
    if (facing < 0 || facing > 5)
        return;
    for (int corner = 0; corner < 4; corner++)
    {
        const unsigned char *extent = LIBMATTI_MC_FaceInfo_CORNERS[facing][corner];
        outQuad->pos[corner][0] = blockX + (float) extent[0];
        outQuad->pos[corner][1] = blockY + (float) extent[1];
        outQuad->pos[corner][2] = blockZ + (float) extent[2];

        float elementU = LIBMATTI_MC_BlockElementFace_GetCornerU(g_default_uv, corner) / 16.0f;
        float elementV = LIBMATTI_MC_BlockElementFace_GetCornerV(g_default_uv, corner) / 16.0f;
        // Java: sprite.getU(f) = u0 + (u1 - u0) * f, same for getV.
        outQuad->uv[corner][0] = spriteUv[0] + (spriteUv[2] - spriteUv[0]) * elementU;
        outQuad->uv[corner][1] = spriteUv[1] + (spriteUv[3] - spriteUv[1]) * elementV;
    }
    outQuad->light = 0;
}
