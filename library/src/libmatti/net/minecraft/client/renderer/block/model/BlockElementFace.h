// Port of net.minecraft.client.renderer.block.model.BlockElementFace.
// Java's record becomes a plain struct; the @Nullable UVs stay a present-flag
// (Java: null means "derive the UVs from the element's from/to corners").

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENTFACE_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENTFACE_H

#include "libmatti/net/minecraft/core/Direction.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int NO_TINT = -1
#define LIBMATTI_MC_BlockElementFace_NO_TINT (-1)

// Java: public record UVs(float minU, float minV, float maxU, float maxV)
typedef struct LIBMATTI_MC_BlockElementFace_UVs
{
    float minU;
    float minV;
    float maxU;
    float maxV;
} LIBMATTI_MC_BlockElementFace_UVs;

// Java: public record BlockElementFace(@Nullable Direction cullForDirection,
//           int tintIndex, String texture, @Nullable UVs uvs, Quadrant rotation)
typedef struct LIBMATTI_MC_BlockElementFace
{
    // Java: the face this face culls against (NULL = never culled).
    LIBMATTI_MC_Direction *cullForDirection;
    int tintIndex;
    // Java: the texture slot reference ("#all" etc.) - owned by the struct.
    char *texture;
    // Java: UVs uvs - NULL derives them from the element corners.
    bool uvsPresent;
    LIBMATTI_MC_BlockElementFace_UVs uvs;
    // Java: Quadrant rotation - the 90-degree UV rotation steps (0..3).
    int rotation;
} LIBMATTI_MC_BlockElementFace;

// Java: public static float getU(UVs, Quadrant, int vertexIndex) -
// vertexIndex is pre-rotated by the quadrant and divided by 16 (the texture
// space is 16x16).
float LIBMATTI_MC_BlockElementFace_GetU(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int rotation, int vertexIndex);
// Java: public static float getV(UVs, Quadrant, int vertexIndex)
float LIBMATTI_MC_BlockElementFace_GetV(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int rotation, int vertexIndex);

// Java: public float getVertexU(int index) - 0/1 pick min, 2/3 pick max
float LIBMATTI_MC_BlockElementFace_UVs_GetVertexU(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int index);
// Java: public float getVertexV(int index) - 1/2 pick max, 0/3 pick min
float LIBMATTI_MC_BlockElementFace_UVs_GetVertexV(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int index);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENTFACE_H
