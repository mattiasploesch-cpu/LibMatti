// Port of net.minecraft.client.renderer.block.model.BlockElementFace (implementation).

#include "libmatti/net/minecraft/client/renderer/block/model/BlockElementFace.h"

// Java: Quadrant.rotateVertexIndex(vertexIndex) - the 90-degree UV rotation
// steps remap the corner index around the quad (0 keeps the order).
static int rotate_vertex_index(int rotation, int vertexIndex)
{
    return (vertexIndex + rotation) & 3;
}

// Java: public float getVertexU(int index)
float LIBMATTI_MC_BlockElementFace_UVs_GetVertexU(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int index)
{
    // Java: return index != 0 && index != 1 ? this.maxU : this.minU;
    return (index != 0 && index != 1) ? uvs->maxU : uvs->minU;
}

// Java: public float getVertexV(int index)
float LIBMATTI_MC_BlockElementFace_UVs_GetVertexV(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int index)
{
    // Java: return index != 0 && index != 3 ? this.maxV : this.minV;
    return (index != 0 && index != 3) ? uvs->maxV : uvs->minV;
}

// Java: public static float getU(UVs, Quadrant, int vertexIndex)
float LIBMATTI_MC_BlockElementFace_GetU(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int rotation, int vertexIndex)
{
    return LIBMATTI_MC_BlockElementFace_UVs_GetVertexU(uvs, rotate_vertex_index(rotation, vertexIndex)) / 16.0f;
}

// Java: public static float getV(UVs, Quadrant, int vertexIndex)
float LIBMATTI_MC_BlockElementFace_GetV(const LIBMATTI_MC_BlockElementFace_UVs *uvs, int rotation, int vertexIndex)
{
    return LIBMATTI_MC_BlockElementFace_UVs_GetVertexV(uvs, rotate_vertex_index(rotation, vertexIndex)) / 16.0f;
}
