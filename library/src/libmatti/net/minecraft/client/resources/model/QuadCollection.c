// Port of net.minecraft.client.resources.model.QuadCollection (implementation).

#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"

#include <stdlib.h>
#include <string.h>

void LIBMATTI_MC_QuadCollection_AddUnculledFace(LIBMATTI_MC_QuadCollection *collection,
                                                const LIBMATTI_MC_BakedQuad *quad)
{
    if (collection == NULL || quad == NULL)
        return;
    LIBMATTI_MC_BakedQuad *grown =
        realloc(collection->unculledFaces, (collection->unculledCount + 1) * sizeof(LIBMATTI_MC_BakedQuad));
    if (grown == NULL)
        return;
    collection->unculledFaces = grown;
    collection->unculledFaces[collection->unculledCount++] = *quad;
}

void LIBMATTI_MC_QuadCollection_AddCulledFace(LIBMATTI_MC_QuadCollection *collection, LIBMATTI_MC_Direction direction,
                                              const LIBMATTI_MC_BakedQuad *quad)
{
    if (collection == NULL || quad == NULL || direction < 0 || direction > 5)
        return;
    size_t count = collection->culledCounts[direction];
    LIBMATTI_MC_BakedQuad *grown = realloc(collection->culledFaces[direction], (count + 1) * sizeof(LIBMATTI_MC_BakedQuad));
    if (grown == NULL)
        return;
    collection->culledFaces[direction] = grown;
    collection->culledFaces[direction][count] = *quad;
    collection->culledCounts[direction] = count + 1;
}

const LIBMATTI_MC_BakedQuad *LIBMATTI_MC_QuadCollection_GetUnculled(const LIBMATTI_MC_QuadCollection *collection,
                                                                    size_t *outCount)
{
    if (outCount != NULL)
        *outCount = collection != NULL ? collection->unculledCount : 0;
    return collection != NULL ? collection->unculledFaces : NULL;
}

const LIBMATTI_MC_BakedQuad *LIBMATTI_MC_QuadCollection_GetCulled(const LIBMATTI_MC_QuadCollection *collection,
                                                                  LIBMATTI_MC_Direction direction, size_t *outCount)
{
    if (outCount != NULL)
        *outCount = (collection != NULL && direction >= 0 && direction <= 5) ? collection->culledCounts[direction] : 0;
    return (collection != NULL && direction >= 0 && direction <= 5) ? collection->culledFaces[direction] : NULL;
}

void LIBMATTI_MC_QuadCollection_Free(LIBMATTI_MC_QuadCollection *collection)
{
    if (collection == NULL)
        return;
    free(collection->unculledFaces);
    for (int i = 0; i < 6; i++)
        free(collection->culledFaces[i]);
    memset(collection, 0, sizeof(*collection));
}
