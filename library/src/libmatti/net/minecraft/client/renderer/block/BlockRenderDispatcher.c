// Port of net.minecraft.client.renderer.block.BlockRenderDispatcher
// (implementation). Java's renderBatched wraps ModelBlockRenderer.tesselateBlock
// in the AO decision (Minecraft.useAmbientOcclusion() && no light emission &&
// the model's useAmbientOcclusion flag) and the CrashReport wrapper; the C
// port keeps the decision, reports through the log.

#include "libmatti/net/minecraft/client/renderer/block/BlockRenderDispatcher.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/client/color/block/BlockColors.h"
#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"

#include <stdlib.h>

LIBMATTI_MC_BlockRenderDispatcher *LIBMATTI_MC_BlockRenderDispatcher_New(void)
{
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher = calloc(1, sizeof(LIBMATTI_MC_BlockRenderDispatcher));
    dispatcher->blockColors = LIBMATTI_MC_BlockColors_CreateDefault();
    dispatcher->modelRenderer = LIBMATTI_MC_ModelBlockRenderer_New(dispatcher->blockColors);
    dispatcher->ambientOcclusion = 1;
    return dispatcher;
}

void LIBMATTI_MC_BlockRenderDispatcher_Free(LIBMATTI_MC_BlockRenderDispatcher *dispatcher)
{
    if (dispatcher == NULL)
        return;
    LIBMATTI_MC_ModelBlockRenderer_Free(dispatcher->modelRenderer);
    LIBMATTI_MC_BlockColors_Free(dispatcher->blockColors);
    free(dispatcher);
}

LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_BlockRenderDispatcher_GetModelRenderer(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher)
{
    return dispatcher->modelRenderer;
}

const LIBMATTI_MC_QuadCollection *LIBMATTI_MC_BlockRenderDispatcher_GetBlockModel(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher,
    const LIBMATTI_MC_BlockState *state)
{
    if (dispatcher->modelForBlock == NULL)
        return NULL;
    return dispatcher->modelForBlock(dispatcher->modelUserdata, LIBMATTI_MC_BlockState_GetBlock(state));
}

void LIBMATTI_MC_BlockRenderDispatcher_RenderBatched(
    LIBMATTI_MC_BlockRenderDispatcher *dispatcher,
    const LIBMATTI_MC_BlockState *state,
    const LIBMATTI_MC_BlockPos *pos,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    int checkSides,
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap),
    void *sinkUserdata)
{
    const LIBMATTI_MC_QuadCollection *model =
        LIBMATTI_MC_BlockRenderDispatcher_GetBlockModel(dispatcher, state);
    if (model == NULL)
        return;

    // Java: tesselateBlock - the AO decision over the options flag, the block's
    // light emission and the model's useAmbientOcclusion flag.
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    int lightEmission = block != NULL && block->properties != NULL ? block->properties->lightEmission : 0;
    int useAO = dispatcher->ambientOcclusion && lightEmission == 0;

    if (useAO)
        LIBMATTI_MC_ModelBlockRenderer_TesselateWithAO(
            dispatcher->modelRenderer, level, model, state, pos, checkSides, sink, sinkUserdata);
    else
        LIBMATTI_MC_ModelBlockRenderer_TesselateWithoutAO(
            dispatcher->modelRenderer, level, model, state, pos, checkSides, sink, sinkUserdata);
}
