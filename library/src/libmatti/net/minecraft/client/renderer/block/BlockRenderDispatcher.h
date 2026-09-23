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

// Creates a dispatcher with default block colors, ambient occlusion enabled,
// and no model resolver. The dispatcher owns its colors and model renderer.
LIBMATTI_MC_BlockRenderDispatcher *LIBMATTI_MC_BlockRenderDispatcher_New(void);

// Frees the dispatcher and its owned colors and model renderer. NULL is allowed.
void LIBMATTI_MC_BlockRenderDispatcher_Free(LIBMATTI_MC_BlockRenderDispatcher *dispatcher);

// Returns the dispatcher's borrowed model renderer.
LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_BlockRenderDispatcher_GetModelRenderer(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher);

// Resolves the state's block through modelForBlock. Returns NULL when no
// resolver is installed or when the resolver has no model for the block.
const struct LIBMATTI_MC_QuadCollection *LIBMATTI_MC_BlockRenderDispatcher_GetBlockModel(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher,
    const struct LIBMATTI_MC_BlockState *state);

// Resolves and renders the state's model, emitting four model-local vertices
// per rendered quad to sink. A nonzero checkSides culls faces hidden by solid
// neighbors. Ambient occlusion is used when enabled and the block emits no
// light; otherwise flat shading is used. No vertices are emitted when model
// resolution returns NULL.
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
