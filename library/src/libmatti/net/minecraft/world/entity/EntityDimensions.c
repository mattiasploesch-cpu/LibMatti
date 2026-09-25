// Port of net.minecraft.world.entity.EntityDimensions (implementation).

#include "libmatti/net/minecraft/world/entity/EntityDimensions.h"

#include <stdlib.h>

float LIBMATTI_MC_EntityDimensions_DefaultEyeHeight(float height)
{
    // Java: private static float defaultEyeHeight(float p_331315_) - height * 0.85F
    return height * 0.85f;
}

static LIBMATTI_MC_EntityDimensions *dimensions_new(float width, float height, float eyeHeight,
                                                    LIBMATTI_MC_EntityAttachments *attachments, bool fixed)
{
    LIBMATTI_MC_EntityDimensions *dimensions = calloc(1, sizeof(LIBMATTI_MC_EntityDimensions));
    if (dimensions == NULL)
    {
        LIBMATTI_MC_EntityAttachments_Free(attachments);
        return NULL;
    }
    dimensions->width = width;
    dimensions->height = height;
    dimensions->eyeHeight = eyeHeight;
    dimensions->attachments = attachments;
    dimensions->fixed = fixed;
    return dimensions;
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Scalable(float width, float height)
{
    return dimensions_new(width, height, LIBMATTI_MC_EntityDimensions_DefaultEyeHeight(height),
                          LIBMATTI_MC_EntityAttachments_CreateDefault(width, height), false);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Fixed(float width, float height)
{
    return dimensions_new(width, height, LIBMATTI_MC_EntityDimensions_DefaultEyeHeight(height),
                          LIBMATTI_MC_EntityAttachments_CreateDefault(width, height), true);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_New(float width, float height, float eyeHeight,
                                                               LIBMATTI_MC_EntityAttachments *attachments, bool fixed)
{
    return dimensions_new(width, height, eyeHeight, attachments, fixed);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_WithEyeHeight(const LIBMATTI_MC_EntityDimensions *dimensions, float eyeHeight)
{
    if (dimensions == NULL)
        return NULL;
    // Java: the record copy takes the same attachments over - the port clones the
    // ownership by moving it into a fresh attachments default (the record never
    // shares into a free path, so the copy rebuilds the attachments)
    return dimensions_new(dimensions->width, dimensions->height, eyeHeight,
                          LIBMATTI_MC_EntityAttachments_CreateDefault(dimensions->width, dimensions->height),
                          dimensions->fixed);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_WithAttachments(const LIBMATTI_MC_EntityDimensions *dimensions,
                                                                           LIBMATTI_MC_EntityAttachmentsBuilder *builder)
{
    if (dimensions == NULL)
        return NULL;
    LIBMATTI_MC_EntityAttachments *attachments = LIBMATTI_MC_EntityAttachmentsBuilder_Build(builder,
                                                                                             dimensions->width,
                                                                                             dimensions->height);
    return dimensions_new(dimensions->width, dimensions->height, dimensions->eyeHeight, attachments, dimensions->fixed);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Scale(const LIBMATTI_MC_EntityDimensions *dimensions, float factor)
{
    return LIBMATTI_MC_EntityDimensions_ScaleXY(dimensions, factor, factor);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_ScaleXY(const LIBMATTI_MC_EntityDimensions *dimensions,
                                                                   float scaleX, float scaleY)
{
    if (dimensions == NULL)
        return NULL;
    // Java: fixed dimensions (or the identity scale) return this
    if (dimensions->fixed || (scaleX == 1.0f && scaleY == 1.0f))
        return LIBMATTI_MC_EntityDimensions_New(dimensions->width, dimensions->height, dimensions->eyeHeight,
                                                LIBMATTI_MC_EntityAttachments_CreateDefault(dimensions->width, dimensions->height),
                                                dimensions->fixed);
    return dimensions_new(dimensions->width * scaleX, dimensions->height * scaleY, dimensions->eyeHeight * scaleY,
                          LIBMATTI_MC_EntityAttachments_CreateDefault(dimensions->width * scaleX, dimensions->height * scaleY),
                          false);
}

LIBMATTI_MC_AABB *LIBMATTI_MC_EntityDimensions_MakeBoundingBoxAt(const LIBMATTI_MC_EntityDimensions *dimensions,
                                                                 double x, double y, double z)
{
    if (dimensions == NULL)
        return NULL;
    // Java: half = width / 2; new AABB(x - half, y, z - half, x + half, y + height, z + half)
    float half = dimensions->width / 2.0f;
    return LIBMATTI_MC_AABB_New(x - half, y, z - half, x + half, y + dimensions->height, z + half);
}

LIBMATTI_MC_AABB *LIBMATTI_MC_EntityDimensions_MakeBoundingBox(const LIBMATTI_MC_EntityDimensions *dimensions, const LIBMATTI_MC_Vec3 *pos)
{
    if (pos == NULL)
        return NULL;
    return LIBMATTI_MC_EntityDimensions_MakeBoundingBoxAt(dimensions, pos->x, pos->y, pos->z);
}

void LIBMATTI_MC_EntityDimensions_Free(LIBMATTI_MC_EntityDimensions *dimensions)
{
    if (dimensions == NULL)
        return;
    LIBMATTI_MC_EntityAttachments_Free(dimensions->attachments);
    free(dimensions);
}
