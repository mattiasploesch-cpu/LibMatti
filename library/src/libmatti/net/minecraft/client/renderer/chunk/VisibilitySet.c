// Port of net.minecraft.client.renderer.chunk.VisibilitySet (implementation).
//
// Java packs one visibility bit per ordered direction pair into a long; the
// bit index is (innerIndex(a) * 6 + innerIndex(b)) where innerIndex maps every
// direction to 0..5 (Java's reserveIndex = ordinal - the Direction constants
// are already 0..5 in the port).

#include "libmatti/net/minecraft/client/renderer/chunk/VisibilitySet.h"

#include <stdlib.h>

// Java: private static int innerIndex(Direction) - the constant ordinal 0..5
static int inner_index(LIBMATTI_MC_Direction direction)
{
    return (int) direction;
}

// Java: private static int getFaceIndex(Direction a, Direction b)
int LIBMATTI_MC_VisibilitySet_FaceIndex(LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b)
{
    return inner_index(a) * 6 + inner_index(b);
}

LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_VisibilitySet_New(void)
{
    LIBMATTI_MC_VisibilitySet *set = calloc(1, sizeof(LIBMATTI_MC_VisibilitySet));
    set->visibilityData = 0;
    return set;
}

void LIBMATTI_MC_VisibilitySet_Add(LIBMATTI_MC_VisibilitySet *set, const LIBMATTI_MC_Direction *faces, size_t faceCount)
{
    for (size_t i = 0; i < faceCount; i++)
    {
        // Java: visibilityData |= 1L << getFaceIndex(reserve, reserve)
        // Java adds the pair (d, d) per flooded edge face plus the mirror
        // pairs through the resolution below; the port sets both orders.
        LIBMATTI_MC_Direction face = faces[i];
        for (int other = 0; other < LIBMATTI_MC_Direction_COUNT; other++)
        {
            set->visibilityData |= (uint64_t) 1 << LIBMATTI_MC_VisibilitySet_FaceIndex(face, (LIBMATTI_MC_Direction) other);
        }
    }
}

void LIBMATTI_MC_VisibilitySet_SetAll(LIBMATTI_MC_VisibilitySet *set, int visible)
{
    // Java: visibilityData = visible ? -1L : 0L
    set->visibilityData = visible ? ~(uint64_t) 0 : 0;
}

void LIBMATTI_MC_VisibilitySet_SetDefault(LIBMATTI_MC_VisibilitySet *set)
{
    // Java: setAll(true) - every pair starts visible before the flood fill.
    set->visibilityData = ~(uint64_t) 0;
}

int LIBMATTI_MC_VisibilitySet_VisibilityBetweenFaces(const LIBMATTI_MC_VisibilitySet *set,
                                                     LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b)
{
    return (set->visibilityData >> LIBMATTI_MC_VisibilitySet_FaceIndex(a, b)) & 1;
}

int LIBMATTI_MC_VisibilitySet_BetweenFaces(const LIBMATTI_MC_VisibilitySet *set,
                                           LIBMATTI_MC_Direction a, LIBMATTI_MC_Direction b)
{
    // Java: visibilityBetweenFaces(a, b) && visibilityBetweenFaces(b, a)
    return LIBMATTI_MC_VisibilitySet_VisibilityBetweenFaces(set, a, b)
           && LIBMATTI_MC_VisibilitySet_VisibilityBetweenFaces(set, b, a);
}
