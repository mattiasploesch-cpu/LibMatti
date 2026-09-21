// Port of net.minecraft.client.renderer.chunk.VisGraph - the 16x16x16
// visibility flood fill over the opaque positions of one section.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISGRAPH_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISGRAPH_H

#include "libmatti/net/minecraft/client/renderer/chunk/VisibilitySet.h"
#include "libmatti/net/minecraft/core/BlockPos.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class VisGraph - the empty/opaque bookkeeping is a 4096 bit set
typedef struct LIBMATTI_MC_VisGraph LIBMATTI_MC_VisGraph;

// Java: public VisGraph()
LIBMATTI_MC_VisGraph *LIBMATTI_MC_VisGraph_New(void);
void LIBMATTI_MC_VisGraph_Free(LIBMATTI_MC_VisGraph *graph);

// Java: public void setOpaque(BlockPos)
void LIBMATTI_MC_VisGraph_SetOpaque(LIBMATTI_MC_VisGraph *graph, const LIBMATTI_MC_BlockPos *pos);

// Java: public VisibilitySet resolve()
LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_VisGraph_Resolve(LIBMATTI_MC_VisGraph *graph);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISGRAPH_H
