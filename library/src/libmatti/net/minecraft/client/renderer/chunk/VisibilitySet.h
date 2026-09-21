// Port of net.minecraft.client.renderer.chunk.VisibilitySet - the 6-bit face
// visibility between the two section faces (Direction pairs).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISIBILITYSET_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISIBILITYSET_H

#include "libmatti/net/minecraft/core/Direction.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class VisibilitySet - the bit index is the direction pair
// (INNER_INDEX_ACCESSOR stores the per-direction inner index 0..5).
typedef struct LIBMATTI_MC_VisibilitySet
{
    // Java: private long visibilityData
    uint64_t visibilityData;
} LIBMATTI_MC_VisibilitySet;

// Java: public VisibilitySet()
LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_VisibilitySet_New(void);

// Java: private static int getFaceIndex(Direction a, Direction b) - the pair
// packed into a 6-bit index (a's inner index * 6 + b's inner index... Java
// uses (a.ordinal() * 6 + b.ordinal()) with the reserveIndex convention).
int LIBMATTI_MC_VisibilitySet_FaceIndex(LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b);

// Java: public void add(Set<Direction> faces) - the flood-fill edge set
void LIBMATTI_MC_VisibilitySet_Add(LIBMATTI_MC_VisibilitySet *set, const LIBMATTI_MC_Direction *faces, size_t faceCount);

// Java: public void setAll(boolean visible) - the fully-visible / fully-hidden form
void LIBMATTI_MC_VisibilitySet_SetAll(LIBMATTI_MC_VisibilitySet *set, int visible);

// Java: public void setDefault() - all pairs visible
void LIBMATTI_MC_VisibilitySet_SetDefault(LIBMATTI_MC_VisibilitySet *set);

// Java: public boolean visibilityBetweenFaces(Direction a, Direction b)
int LIBMATTI_MC_VisibilitySet_VisibilityBetweenFaces(const LIBMATTI_MC_VisibilitySet *set,
                                                     LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b);
// Java: public boolean betweenFaces(Direction a, Direction b) - the resolved
// two-face visibility (a -> b and b -> a both set)
int LIBMATTI_MC_VisibilitySet_BetweenFaces(const LIBMATTI_MC_VisibilitySet *set,
                                           LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_VISIBILITYSET_H
