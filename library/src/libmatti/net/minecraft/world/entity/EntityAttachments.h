// Port of net.minecraft.world.entity.EntityAttachment (the enum constants) and
// net.minecraft.world.entity.EntityAttachments (the per-attachment Vec3 lists
// with the fallback points Java derives from the dimensions).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_ENTITYATTACHMENTS_H
#define MATTICRAFT_MC_WORLD_ENTITY_ENTITYATTACHMENTS_H

#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum EntityAttachment - the fallback rule per constant
typedef enum LIBMATTI_MC_EntityAttachment
{
    // Java: PASSENGER(Fallback.AT_HEIGHT) - one point at the entity top
    LIBMATTI_MC_EntityAttachment_PASSENGER,
    // Java: VEHICLE(Fallback.AT_FEET) - the origin
    LIBMATTI_MC_EntityAttachment_VEHICLE,
    // Java: NAME_TAG(Fallback.AT_HEIGHT)
    LIBMATTI_MC_EntityAttachment_NAME_TAG,
    // Java: WARDEN_CHEST(Fallback.AT_CENTER)
    LIBMATTI_MC_EntityAttachment_WARDEN_CHEST,
    LIBMATTI_MC_EntityAttachment_COUNT,
} LIBMATTI_MC_EntityAttachment;

// Java: EntityAttachments - the fallback lists materialise at build time, the
// port keeps a fixed-size Vec3 pool per attachment kind
typedef struct LIBMATTI_MC_EntityAttachments
{
    // Java: private final Map<EntityAttachment, List<Vec3>> attachments - the
    // port keeps one flat Vec3 array per kind (points[kind] is the array)
    LIBMATTI_MC_Vec3 *points[LIBMATTI_MC_EntityAttachment_COUNT];
    size_t pointCount[LIBMATTI_MC_EntityAttachment_COUNT];
    size_t pointCapacity[LIBMATTI_MC_EntityAttachment_COUNT];
} LIBMATTI_MC_EntityAttachments;

// Java: public static EntityAttachments createDefault(float width, float height)
// - no explicit points, every kind takes its fallback (AT_FEET/AT_HEIGHT/AT_CENTER)
LIBMATTI_MC_EntityAttachments *LIBMATTI_MC_EntityAttachments_CreateDefault(float width, float height);

// Java: public static Builder builder()
typedef struct LIBMATTI_MC_EntityAttachmentsBuilder LIBMATTI_MC_EntityAttachmentsBuilder;
LIBMATTI_MC_EntityAttachmentsBuilder *LIBMATTI_MC_EntityAttachments_Builder(void);
// Java: public Builder attach(EntityAttachment, float x, float y, float z)
LIBMATTI_MC_EntityAttachmentsBuilder *LIBMATTI_MC_EntityAttachmentsBuilder_Attach(
        LIBMATTI_MC_EntityAttachmentsBuilder *builder, LIBMATTI_MC_EntityAttachment kind,
        float x, float y, float z);
// Java: public EntityAttachments build(float width, float height) - fills the
// kinds without explicit points with their fallback lists
LIBMATTI_MC_EntityAttachments *LIBMATTI_MC_EntityAttachmentsBuilder_Build(
        LIBMATTI_MC_EntityAttachmentsBuilder *builder, float width, float height);

void LIBMATTI_MC_EntityAttachments_Free(LIBMATTI_MC_EntityAttachments *attachments);

// Java: public Vec3 getNullable(EntityAttachment, int index, float yRot) - the
// point rotated around Y by -yRot; NULL on out of range
LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetNullable(const LIBMATTI_MC_EntityAttachments *attachments,
                                                            LIBMATTI_MC_EntityAttachment kind, int index, float yRot);
// Java: public Vec3 get(EntityAttachment, int, float) - aborts when absent (Java throws)
LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_Get(const LIBMATTI_MC_EntityAttachments *attachments,
                                                    LIBMATTI_MC_EntityAttachment kind, int index, float yRot);
// Java: public Vec3 getAverage(EntityAttachment) - the mean over the points
LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetAverage(const LIBMATTI_MC_EntityAttachments *attachments,
                                                           LIBMATTI_MC_EntityAttachment kind);
// Java: public Vec3 getClamped(EntityAttachment, int, float) - the index clamps
// into the list length
LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetClamped(const LIBMATTI_MC_EntityAttachments *attachments,
                                                           LIBMATTI_MC_EntityAttachment kind, int index, float yRot);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_ENTITYATTACHMENTS_H
