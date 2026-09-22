// Port of net.minecraft.client.resources.model.SimpleUnbakedGeometry.bake
// (implementation) over the ModelBaker surface.

#include "libmatti/net/minecraft/client/resources/model/ModelBaker.h"

#include "libmatti/net/minecraft/core/Direction.h"

#include <stdlib.h>
#include <string.h>

// Java: the degenerate-axis flag chain in SimpleUnbakedGeometry.bake -
// boolean flag (Y), flag1 (X), flag2 (Z) - a face renders only when its axis
// has extent (a flat element has no faces on the flat axes).
static int axis_has_extent(const LIBMATTI_MC_BlockElement *element, LIBMATTI_MC_Direction direction)
{
    float from[3] = {element->fromX, element->fromY, element->fromZ};
    float to[3] = {element->toX, element->toY, element->toZ};

    int flagX = from[0] != to[0];
    int flagY = from[1] != to[1];
    int flagZ = from[2] != to[2];

    switch (LIBMATTI_MC_Direction_GetAxis(direction))
    {
        case LIBMATTI_MC_Direction_Axis_X: return flagX;
        case LIBMATTI_MC_Direction_Axis_Y: return flagY;
        default: return flagZ;
    }
}

// Java: SimpleUnbakedGeometry.bake
int LIBMATTI_MC_ModelBaker_Bake(const LIBMATTI_MC_BlockModel *model, LIBMATTI_MC_ModelBaker_SpriteResolver resolver,
                                void *userdata, LIBMATTI_MC_QuadCollection *out)
{
    if (model == NULL || resolver == NULL || out == NULL)
        return 0;
    if (!model->elementsPresent)
        return 1; // Java: an inherit-only model bakes to the empty collection.

    memset(out, 0, sizeof(*out));
    for (size_t e = 0; e < model->elementCount; e++)
    {
        const LIBMATTI_MC_BlockElement *element = &model->elements[e];
        for (int d = 0; d < 6; d++)
        {
            LIBMATTI_MC_Direction direction = (LIBMATTI_MC_Direction) d;
            const LIBMATTI_MC_BlockElementFace *face = element->faces[d];
            if (face == NULL)
                continue;
            // Java: the axis-extent gate
            if (!axis_has_extent(element, direction))
                continue;

            // Java: SpriteGetter.resolveSlot - resolve "#ref" chains first.
            char *textureId = LIBMATTI_MC_BlockModel_ResolveTexture(model, face->texture);
            if (textureId == NULL)
                return 0; // Java: the missing-sprite throw.
            float uvRect[4];
            int resolved = resolver(userdata, textureId, uvRect);
            free(textureId);
            if (!resolved)
                return 0;

            float from[3] = {element->fromX, element->fromY, element->fromZ};
            float to[3] = {element->toX, element->toY, element->toZ};
            LIBMATTI_MC_BakedQuad quad;
            LIBMATTI_MC_FaceBakery_BakeQuad(from, to, face, uvRect, direction, element->rotation,
                                            face->tintIndex, element->shade, &quad);

            // Java: cullForDirection == null -> unculled, else the culled bucket.
            if (face->cullForDirection == NULL)
                LIBMATTI_MC_QuadCollection_AddUnculledFace(out, &quad);
            else
                LIBMATTI_MC_QuadCollection_AddCulledFace(out, *face->cullForDirection, &quad);
        }
    }
    return 1;
}
