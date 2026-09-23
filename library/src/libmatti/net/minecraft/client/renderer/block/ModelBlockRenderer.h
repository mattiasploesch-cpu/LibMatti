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

// Creates a renderer that borrows colors for resolving tinted quads.
LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_ModelBlockRenderer_New(struct LIBMATTI_MC_BlockColors *colors);

// Frees the renderer without freeing the borrowed color registry. NULL is allowed.
void LIBMATTI_MC_ModelBlockRenderer_Free(LIBMATTI_MC_ModelBlockRenderer *renderer);

// Renders the model with per-vertex ambient-occlusion brightness and packed
// light. A nonzero checkSides culls faces hidden by solid neighbors. The sink
// receives four calls per rendered quad, with model-local positions and RGB
// components already multiplied by tint and brightness.
void LIBMATTI_MC_ModelBlockRenderer_TesselateWithAO(
    LIBMATTI_MC_ModelBlockRenderer *renderer,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_QuadCollection *model,
    const struct LIBMATTI_MC_BlockState *state,
    const struct LIBMATTI_MC_BlockPos *pos,
    int checkSides,
    // Called per corner (0..3); vertexPos is model-local, RGB includes tint and
    // brightness, lightmap is packed, and quad supplies the source metadata.
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b,
                 int lightmap),
    void *sinkUserdata);

// Renders the same visible quads with one directional brightness and lightmap
// value per quad. The sink contract matches TesselateWithAO.
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

// Returns the current port's packed lightmap value: block light is zero and sky
// light is 15 when pos can see the sky, otherwise zero.
int LIBMATTI_MC_ModelBlockRenderer_GetLightColor(
    const struct LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_BlockPos *pos);

// Returns 0.2 for a full occluding block and 1.0 otherwise. level and pos do
// not affect the current proxy calculation.
float LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(
    const struct LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODELBLOCKRENDERER_H
