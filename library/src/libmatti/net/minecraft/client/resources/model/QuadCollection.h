// Port of net.minecraft.client.resources.model.QuadCollection (plus the
// Builder) - the bake output: unculled quads plus one bucket per cullface
// direction. Java keeps two lists per direction; the C port stores flat
// arrays with counts.

#ifndef MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_QUADCOLLECTION_H
#define MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_QUADCOLLECTION_H

#include "libmatti/net/minecraft/client/renderer/block/model/FaceBakery.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: QuadCollection.Builder - the flat quad storage (unculled + per-direction).
typedef struct LIBMATTI_MC_QuadCollection
{
    // Java: List<BakedQuad> unculledFaces
    size_t unculledCount;
    LIBMATTI_MC_BakedQuad *unculledFaces;
    // Java: Map<Direction, List<BakedQuad>> culledFaces - indexed by ordinal.
    size_t culledCounts[6];
    LIBMATTI_MC_BakedQuad *culledFaces[6];
} LIBMATTI_MC_QuadCollection;

// Java: QuadCollection.Builder.addUnculledFace(BakedQuad) - copies the quad in.
void LIBMATTI_MC_QuadCollection_AddUnculledFace(LIBMATTI_MC_QuadCollection *collection,
                                                const LIBMATTI_MC_BakedQuad *quad);
// Java: QuadCollection.Builder.addCulledFace(Direction, BakedQuad)
void LIBMATTI_MC_QuadCollection_AddCulledFace(LIBMATTI_MC_QuadCollection *collection, LIBMATTI_MC_Direction direction,
                                              const LIBMATTI_MC_BakedQuad *quad);

// Java: List<BakedQuad> getUnculledFaces() / getFaces(Direction)
const LIBMATTI_MC_BakedQuad *LIBMATTI_MC_QuadCollection_GetUnculled(const LIBMATTI_MC_QuadCollection *collection,
                                                                    size_t *outCount);
const LIBMATTI_MC_BakedQuad *LIBMATTI_MC_QuadCollection_GetCulled(const LIBMATTI_MC_QuadCollection *collection,
                                                                  LIBMATTI_MC_Direction direction, size_t *outCount);

void LIBMATTI_MC_QuadCollection_Free(LIBMATTI_MC_QuadCollection *collection);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_QUADCOLLECTION_H
