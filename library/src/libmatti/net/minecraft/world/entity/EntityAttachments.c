// Port of net.minecraft.world.entity.EntityAttachment(s) (implementation).

#include "libmatti/net/minecraft/world/entity/EntityAttachments.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: EntityAttachment.Fallback - AT_FEET = origin, AT_HEIGHT = top centre,
// AT_CENTER = middle centre
static void fallback_points(LIBMATTI_MC_EntityAttachment kind, float width, float height,
                            LIBMATTI_MC_Vec3 **out, size_t *outCount)
{
    (void) width;
    switch (kind)
    {
        case LIBMATTI_MC_EntityAttachment_PASSENGER:
        case LIBMATTI_MC_EntityAttachment_NAME_TAG:
            // AT_HEIGHT: one point at (0, height, 0)
            out[0] = LIBMATTI_MC_Vec3_New(0.0, (double) height, 0.0);
            *outCount = 1;
            break;
        case LIBMATTI_MC_EntityAttachment_WARDEN_CHEST:
            // AT_CENTER: one point at (0, height / 2, 0)
            out[0] = LIBMATTI_MC_Vec3_New(0.0, (double) (height / 2.0f), 0.0);
            *outCount = 1;
            break;
        case LIBMATTI_MC_EntityAttachment_VEHICLE:
        default:
            // AT_FEET: the zero point
            out[0] = LIBMATTI_MC_Vec3_New(0.0, 0.0, 0.0);
            *outCount = 1;
            break;
    }
}

LIBMATTI_MC_EntityAttachments *LIBMATTI_MC_EntityAttachments_CreateDefault(float width, float height)
{
    LIBMATTI_MC_EntityAttachmentsBuilder *builder = LIBMATTI_MC_EntityAttachments_Builder();
    if (builder == NULL)
        return NULL;
    return LIBMATTI_MC_EntityAttachmentsBuilder_Build(builder, width, height);
}

struct LIBMATTI_MC_EntityAttachmentsBuilder
{
    // per kind: one Vec3 array each (the struct hands the arrays over)
    LIBMATTI_MC_Vec3 *points[LIBMATTI_MC_EntityAttachment_COUNT];
    size_t pointCount[LIBMATTI_MC_EntityAttachment_COUNT];
    size_t pointCapacity[LIBMATTI_MC_EntityAttachment_COUNT];
};

LIBMATTI_MC_EntityAttachmentsBuilder *LIBMATTI_MC_EntityAttachments_Builder(void)
{
    return calloc(1, sizeof(LIBMATTI_MC_EntityAttachmentsBuilder));
}

static void builder_attach(LIBMATTI_MC_EntityAttachmentsBuilder *builder, LIBMATTI_MC_EntityAttachment kind, LIBMATTI_MC_Vec3 *point)
{
    if (builder->pointCount[kind] >= builder->pointCapacity[kind])
    {
        size_t next = builder->pointCapacity[kind] > 0 ? builder->pointCapacity[kind] * 2 : 2;
        LIBMATTI_MC_Vec3 *grown = realloc(builder->points[kind], next * sizeof(LIBMATTI_MC_Vec3));
        if (grown == NULL)
        {
            free(point);
            return;
        }
        builder->points[kind] = grown;
        builder->pointCapacity[kind] = next;
    }
    builder->points[kind][builder->pointCount[kind]++] = *point;
    free(point);
}

LIBMATTI_MC_EntityAttachmentsBuilder *LIBMATTI_MC_EntityAttachmentsBuilder_Attach(
        LIBMATTI_MC_EntityAttachmentsBuilder *builder, LIBMATTI_MC_EntityAttachment kind,
        float x, float y, float z)
{
    if (builder == NULL)
        return NULL;
    builder_attach(builder, kind, LIBMATTI_MC_Vec3_New(x, y, z));
    return builder;
}

LIBMATTI_MC_EntityAttachments *LIBMATTI_MC_EntityAttachmentsBuilder_Build(
        LIBMATTI_MC_EntityAttachmentsBuilder *builder, float width, float height)
{
    LIBMATTI_MC_EntityAttachments *attachments = calloc(1, sizeof(LIBMATTI_MC_EntityAttachments));
    if (attachments == NULL)
    {
        free(builder);
        return NULL;
    }
    if (builder != NULL)
    {
        for (int kind = 0; kind < LIBMATTI_MC_EntityAttachment_COUNT; kind++)
        {
            if (builder->pointCount[kind] > 0)
            {
                attachments->points[kind] = builder->points[kind];
                attachments->pointCount[kind] = builder->pointCount[kind];
                attachments->pointCapacity[kind] = builder->pointCapacity[kind];
            }
        }
        free(builder);
    }
    // Java: the kinds without explicit points take their fallback lists
    for (int kind = 0; kind < LIBMATTI_MC_EntityAttachment_COUNT; kind++)
    {
        if (attachments->pointCount[kind] > 0)
            continue;
        LIBMATTI_MC_Vec3 *fallback[1] = {NULL};
        size_t fallbackCount = 0;
        fallback_points((LIBMATTI_MC_EntityAttachment) kind, width, height, fallback, &fallbackCount);
        for (size_t i = 0; i < fallbackCount; i++)
        {
            if (attachments->pointCount[kind] >= attachments->pointCapacity[kind])
            {
                size_t next = attachments->pointCapacity[kind] > 0 ? attachments->pointCapacity[kind] * 2 : 2;
                LIBMATTI_MC_Vec3 *grown = realloc(attachments->points[kind], next * sizeof(LIBMATTI_MC_Vec3));
                if (grown == NULL)
                    continue;
                attachments->points[kind] = grown;
                attachments->pointCapacity[kind] = next;
            }
            attachments->points[kind][attachments->pointCount[kind]++] = *fallback[i];
            free(fallback[i]);
            fallback[i] = NULL;
        }
    }
    return attachments;
}

void LIBMATTI_MC_EntityAttachments_Free(LIBMATTI_MC_EntityAttachments *attachments)
{
    if (attachments == NULL)
        return;
    for (int kind = 0; kind < LIBMATTI_MC_EntityAttachment_COUNT; kind++)
        free(attachments->points[kind]);
    free(attachments);
}

// Java: private static Vec3 transformPoint(Vec3, float yRot) - the point turned
// around Y by -yRot degrees (Vec3.yRot)
static LIBMATTI_MC_Vec3 *transform_point(const LIBMATTI_MC_Vec3 *point, float yRot)
{
    float rad = -yRot * ((float) 3.141592653589793 / 180.0f);
    float s = (float) sin(rad);
    float c = (float) cos(rad);
    double x = point->x * (double) c + point->z * (double) s;
    double z = point->z * (double) c - point->x * (double) s;
    return LIBMATTI_MC_Vec3_New(x, point->y, z);
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetNullable(const LIBMATTI_MC_EntityAttachments *attachments,
                                                            LIBMATTI_MC_EntityAttachment kind, int index, float yRot)
{
    if (attachments == NULL || index < 0 || (size_t) index >= attachments->pointCount[kind])
        return NULL;
    return transform_point(&attachments->points[kind][index], yRot);
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_Get(const LIBMATTI_MC_EntityAttachments *attachments,
                                                    LIBMATTI_MC_EntityAttachment kind, int index, float yRot)
{
    LIBMATTI_MC_Vec3 *point = LIBMATTI_MC_EntityAttachments_GetNullable(attachments, kind, index, yRot);
    if (point == NULL)
    {
        fprintf(stderr, "IllegalStateException: Had no attachment point of type: %d for index: %d\n", (int) kind, index);
        abort();
    }
    return point;
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetAverage(const LIBMATTI_MC_EntityAttachments *attachments,
                                                           LIBMATTI_MC_EntityAttachment kind)
{
    if (attachments == NULL || attachments->pointCount[kind] == 0)
    {
        fprintf(stderr, "IllegalStateException: No attachment points of type: %d\n", (int) kind);
        abort();
    }
    double x = 0.0, y = 0.0, z = 0.0;
    for (size_t i = 0; i < attachments->pointCount[kind]; i++)
    {
        const LIBMATTI_MC_Vec3 *p = &attachments->points[kind][i];
        x += p->x;
        y += p->y;
        z += p->z;
    }
    double n = (double) attachments->pointCount[kind];
    return LIBMATTI_MC_Vec3_New(x / n, y / n, z / n);
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_EntityAttachments_GetClamped(const LIBMATTI_MC_EntityAttachments *attachments,
                                                           LIBMATTI_MC_EntityAttachment kind, int index, float yRot)
{
    if (attachments == NULL || attachments->pointCount[kind] == 0)
    {
        fprintf(stderr, "IllegalStateException: Had no attachment points of type: %d\n", (int) kind);
        abort();
    }
    int clamped = LIBMATTI_MC_Mth_ClampI(index, 0, (int) attachments->pointCount[kind] - 1);
    return transform_point(&attachments->points[kind][clamped], yRot);
}
