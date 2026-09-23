// Port of net.minecraft.client.renderer.block.BlockRenderDispatcher - the
// facade the section compiler and the item/entity renderers call: the model
// shaper (the port's QuadCollection resolver), the ModelBlockRenderer with
// BlockColors and the batched/single render entries.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_BLOCKRENDERDISPATCHER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_BLOCKRENDERDISPATCHER_H

#include "libmatti/net/minecraft/client/renderer/block/ModelBlockRenderer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Block;
struct LIBMATTI_MC_BlockAndTintGetter;
struct LIBMATTI_MC_BlockPos;
struct LIBMATTI_MC_BlockState;
struct LIBMATTI_MC_QuadCollection;

// Java: public class BlockRenderDispatcher
typedef struct LIBMATTI_MC_BlockRenderDispatcher
{
    // Java: private final BlockColors blockColors (the renderer owns them)
    struct LIBMATTI_MC_BlockColors *blockColors;
    // Java: private final ModelBlockRenderer modelRenderer
    LIBMATTI_MC_ModelBlockRenderer *modelRenderer;
    // Java: useAmbientOcclusion() - the Minecraft options flag; the dispatcher
    // carries it because the C port has no global options object yet.
    int ambientOcclusion;
    // The game layer's model resolver (Java: BlockModelShaper.getBlockModel);
    // NULL keeps the hardcoded cube path.
    const struct LIBMATTI_MC_QuadCollection *(*modelForBlock)(void *userdata, const struct LIBMATTI_MC_Block *block);
    void *modelUserdata;
} LIBMATTI_MC_BlockRenderDispatcher;

// Java: the constructor slice - blockColors created through
// BlockColors.createDefault(), the model renderer over them.
LIBMATTI_MC_BlockRenderDispatcher *LIBMATTI_MC_BlockRenderDispatcher_New(void);
void LIBMATTI_MC_BlockRenderDispatcher_Free(LIBMATTI_MC_BlockRenderDispatcher *dispatcher);

// Java: public ModelBlockRenderer getModelRenderer()
LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_BlockRenderDispatcher_GetModelRenderer(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher);

// Java: public BlockStateModel getBlockModel(BlockState) - the port resolves
// through the game layer's model-for-block callback; NULL keeps the caller on
// the hardcoded cube path.
const struct LIBMATTI_MC_QuadCollection *LIBMATTI_MC_BlockRenderDispatcher_GetBlockModel(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher,
    const struct LIBMATTI_MC_BlockState *state);

// Java: public void renderBatched(BlockState, BlockPos, BlockAndTintGetter,
// PoseStack, VertexConsumer, boolean checkSides, List<BlockModelPart>) -
// the model quads through the AO decision; the vertex sink is the
// putBulkData slice the caller packs into its buffer.
void LIBMATTI_MC_BlockRenderDispatcher_RenderBatched(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher,
    const struct LIBMATTI_MC_BlockState *state,
    const struct LIBMATTI_MC_BlockPos *pos,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    int checkSides,
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap),
    void *sinkUserdata);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_BLOCKRENDERDISPATCHER_H
