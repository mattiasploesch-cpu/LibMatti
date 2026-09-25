// Port of net.minecraft.world.entity.EntityDimensions (the record carrying the
// width/height/eye-height plus the EntityAttachments the bounding box derives).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_ENTITYDIMENSIONS_H
#define MATTICRAFT_MC_WORLD_ENTITY_ENTITYDIMENSIONS_H

#include "libmatti/net/minecraft/world/entity/EntityAttachments.h"
#include "libmatti/net/minecraft/world/phys/AABB.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record EntityDimensions(float width, float height, float eyeHeight,
//                                      EntityAttachments attachments, boolean fixed)
typedef struct LIBMATTI_MC_EntityDimensions
{
    float width;
    float height;
    float eyeHeight;
    LIBMATTI_MC_EntityAttachments *attachments; // owned
    bool fixed;
} LIBMATTI_MC_EntityDimensions;

// Java: private EntityDimensions(width, height, fixed) - the default eye height
// is height * 0.85, the attachments default to createDefault(width, height)
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Scalable(float width, float height);
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Fixed(float width, float height);
// The full constructor (takes the attachments over; the dimensions own it)
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_New(float width, float height, float eyeHeight,
                                                               LIBMATTI_MC_EntityAttachments *attachments, bool fixed);
// Java: public EntityDimensions withEyeHeight(float)
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_WithEyeHeight(const LIBMATTI_MC_EntityDimensions *dimensions, float eyeHeight);
// Java: public EntityDimensions withAttachments(Builder)
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_WithAttachments(const LIBMATTI_MC_EntityDimensions *dimensions,
                                                                           LIBMATTI_MC_EntityAttachmentsBuilder *builder);
// Java: public EntityDimensions scale(float) / scale(float, float) - fixed
// dimensions return the same object
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_Scale(const LIBMATTI_MC_EntityDimensions *dimensions, float factor);
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityDimensions_ScaleXY(const LIBMATTI_MC_EntityDimensions *dimensions, float scaleX, float scaleY);
// Java: public AABB makeBoundingBox(Vec3) / makeBoundingBox(double, double, double)
LIBMATTI_MC_AABB *LIBMATTI_MC_EntityDimensions_MakeBoundingBox(const LIBMATTI_MC_EntityDimensions *dimensions, const LIBMATTI_MC_Vec3 *pos);
LIBMATTI_MC_AABB *LIBMATTI_MC_EntityDimensions_MakeBoundingBoxAt(const LIBMATTI_MC_EntityDimensions *dimensions,
                                                                 double x, double y, double z);
// Java: private static float defaultEyeHeight(float height) - height * 0.85
float LIBMATTI_MC_EntityDimensions_DefaultEyeHeight(float height);
void LIBMATTI_MC_EntityDimensions_Free(LIBMATTI_MC_EntityDimensions *dimensions);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_ENTITYDIMENSIONS_H
