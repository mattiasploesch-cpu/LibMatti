// Port of net.minecraft.client.renderer.block.ModelBlockRenderer - the
// tesselate path: face culling, the flat/AO brightness per vertex, the
// non-cubic face weights and the tint resolution through BlockColors.
//
// Java's VertexConsumer sink becomes the port's vertex callback: the renderer
// emits (pos, color, lightmap) per corner and the caller packs them into the
// layer buffer (the putBulkData slice the section path needs).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODELBLOCKRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODELBLOCKRENDERER_H

#include "libmatti/net/minecraft/client/renderer/block/model/FaceBakery.h"
#include "libmatti/net/minecraft/core/Direction.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_BlockAndTintGetter;
struct LIBMATTI_MC_Level;
struct LIBMATTI_MC_BlockPos;
struct LIBMATTI_MC_BlockState;
struct LIBMATTI_MC_BlockColors;
struct LIBMATTI_MC_QuadCollection;

// Java: ModelBlockRenderer.SizeInfo - the 12 face-shape slots (COUNT = 12).
enum
{
    LIBMATTI_MC_ModelBlockRenderer_SIZEINFO_COUNT = 12
};

// Java: CommonRenderStorage - the per-block scratch state the render paths carry.
typedef struct LIBMATTI_MC_ModelBlockRenderer_Storage
{
    // Java: faceCubic / facePartial (from calculateShape)
    int faceCubic;
    int facePartial;
    // Java: final float[] brightness = new float[4]
    float brightness[4];
    // Java: final int[] lightmap = new int[4]
    int lightmap[4];
    // Java: int tintCacheIndex / tintCacheValue
    int tintCacheIndex;
    int tintCacheValue;
} LIBMATTI_MC_ModelBlockRenderer_Storage;

// Java: public ModelBlockRenderer(BlockColors)
typedef struct LIBMATTI_MC_ModelBlockRenderer
{
    struct LIBMATTI_MC_BlockColors *blockColors;
} LIBMATTI_MC_ModelBlockRenderer;

LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_ModelBlockRenderer_New(struct LIBMATTI_MC_BlockColors *colors);
void LIBMATTI_MC_ModelBlockRenderer_Free(LIBMATTI_MC_ModelBlockRenderer *renderer);

// Java: public void tesselateWithAO(BlockAndTintGetter, List<BlockModelPart>,
// BlockState, BlockPos, PoseStack, VertexConsumer, boolean checkSides, int seed) -
// the model's quads render with the per-vertex AO brightness; the emitted
// corners arrive through the sink (outPos/outColor/outLight per corner).
void LIBMATTI_MC_ModelBlockRenderer_TesselateWithAO(
    LIBMATTI_MC_ModelBlockRenderer *renderer,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_QuadCollection *model,
    const struct LIBMATTI_MC_BlockState *state,
    const struct LIBMATTI_MC_BlockPos *pos,
    int checkSides,
    // the vertex sink: called per corner (corner 0..3, pos in world space,
    // color as ARGB, light the packed lightmap; quad = the metadata)
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b,
                 int lightmap),
    void *sinkUserdata);

// Java: public void tesselateWithoutAO(...) - the flat shading path.
void LIBMATTI_MC_ModelBlockRenderer_TesselateWithoutAO(
    LIBMATTI_MC_ModelBlockRenderer *renderer,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_QuadCollection *model,
    const struct LIBMATTI_MC_BlockState *state,
    const struct LIBMATTI_MC_BlockPos *pos,
    int checkSides,
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b,
                 int lightmap),
    void *sinkUserdata);

// Java: the static LevelRenderer.getLightColor slice the AO storage needs -
// packed (block, sky) through the port's light model (sky = column scan).
int LIBMATTI_MC_ModelBlockRenderer_GetLightColor(
    const struct LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_BlockPos *pos);

// Java: BlockBehaviour.BlockStateBase.getShadeBrightness - the 0.2F/1.0F rule.
float LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(
    const struct LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODELBLOCKRENDERER_H
