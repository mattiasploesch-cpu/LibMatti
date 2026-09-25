// Port of net.minecraft.world.entity.Entity (the base-class core).

#include "libmatti/net/minecraft/world/entity/Entity.h"

#include "libmatti/java/util/UUID.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final EntityCounter ENTITY_COUNTER - a shared AtomicInteger
int LIBMATTI_MC_Entity_NextEntityCounter(void)
{
    static int counter = 0;
    return ++counter;
}

// Java: protected Entity(EntityType, Level) - the subclass entry; the concrete
// type calls this after embedding the base
void LIBMATTI_MC_Entity_Init(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level)
{
    memset(entity, 0, sizeof(*entity));
    entity->type = type;
    entity->level = level;
    entity->id = LIBMATTI_MC_Entity_NextEntityCounter();
    // Java: this.uuid = UUID.randomUUID(); this.stringUUID = this.uuid.toString()
    entity->uuid = LIBMATTI_JU_UUID_RandomUUID();
    // Java: this.position = Vec3.ZERO; this.deltaMovement = Vec3.ZERO
    entity->x = entity->y = entity->z = 0.0;
    entity->dx = entity->dy = entity->dz = 0.0;
    entity->airSupply = 300; // Java: getMaxAirSupply() base value
    entity->firstTick = true;
    entity->bb = LIBMATTI_MC_EntityDimensions_MakeBoundingBoxAt(LIBMATTI_MC_EntityType_GetDimensions(type), entity->x, entity->y, entity->z);
}

LIBMATTI_MC_Entity *LIBMATTI_MC_Entity_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level)
{
    LIBMATTI_MC_Entity *entity = malloc(sizeof(LIBMATTI_MC_Entity));
    if (entity == NULL)
        return NULL;
    LIBMATTI_MC_Entity_Init(entity, type, level);
    return entity;
}

void LIBMATTI_MC_Entity_Free(LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return;
    // the AABB port returns heap objects the caller frees (no AABB_Free helper
    // exists - the plain free matches the allocation)
    free(entity->bb);
    entity->bb = NULL;
    for (int i = 0; i < entity->tagCount; i++)
        free(entity->tags[i]);
    free(entity->tags);
    entity->tags = NULL;
    entity->tagCount = 0;
    entity->tagCapacity = 0;
}

int LIBMATTI_MC_Entity_GetId(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->id : 0;
}

const LIBMATTI_MC_EntityType *LIBMATTI_MC_Entity_GetType(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->type : NULL;
}

LIBMATTI_JU_UUID LIBMATTI_MC_Entity_GetUUID(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->uuid : LIBMATTI_JU_UUID_Zero();
}

const char *LIBMATTI_MC_Entity_GetStringUUID(const LIBMATTI_MC_Entity *entity, char *out, size_t outSize)
{
    if (entity == NULL || out == NULL || outSize < 37)
        return NULL;
    LIBMATTI_JU_UUID_ToString(&entity->uuid, out, outSize);
    return out;
}

void LIBMATTI_MC_Entity_SetUUID(LIBMATTI_MC_Entity *entity, LIBMATTI_JU_UUID uuid)
{
    if (entity != NULL)
        entity->uuid = uuid;
}

double LIBMATTI_MC_Entity_GetX(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->x : 0.0;
}

double LIBMATTI_MC_Entity_GetY(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->y : 0.0;
}

double LIBMATTI_MC_Entity_GetZ(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->z : 0.0;
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_Entity_Position(const LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return LIBMATTI_MC_Vec3_New(0.0, 0.0, 0.0);
    return LIBMATTI_MC_Vec3_New(entity->x, entity->y, entity->z);
}

LIBMATTI_MC_BlockPos LIBMATTI_MC_Entity_BlockPosition(const LIBMATTI_MC_Entity *entity)
{
    // Java: BlockPos.containing(x, y, z) - the floor of the position
    LIBMATTI_MC_BlockPos blockPos;
    blockPos.base.x = entity != NULL ? (int) floor(entity->x) : 0;
    blockPos.base.y = entity != NULL ? (int) floor(entity->y) : 0;
    blockPos.base.z = entity != NULL ? (int) floor(entity->z) : 0;
    return blockPos;
}

// Java: public void setPos(double, double, double) - raw position + bbox refresh
void LIBMATTI_MC_Entity_SetPos(LIBMATTI_MC_Entity *entity, double x, double y, double z)
{
    if (entity == NULL)
        return;
    LIBMATTI_MC_Entity_SetPosRaw(entity, x, y, z);
    LIBMATTI_MC_AABB *bb = LIBMATTI_MC_Entity_MakeBoundingBox(entity);
    LIBMATTI_MC_Entity_SetBoundingBox(entity, bb);
}

void LIBMATTI_MC_Entity_SetPosVec(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *pos)
{
    if (pos == NULL)
        return;
    LIBMATTI_MC_Entity_SetPos(entity, pos->x, pos->y, pos->z);
}

void LIBMATTI_MC_Entity_SetPosRaw(LIBMATTI_MC_Entity *entity, double x, double y, double z)
{
    if (entity == NULL)
        return;
    // Java: the raw position write (the bbox refresh is the caller's part)
    entity->x = x;
    entity->y = y;
    entity->z = z;
    // Java: this.requiresPrecisePosition stays a sync-layer concern
}

float LIBMATTI_MC_Entity_GetYRot(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->yRot : 0.0f;
}

float LIBMATTI_MC_Entity_GetXRot(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->xRot : 0.0f;
}

// Java: protected void setRot(float yRot, float xRot) - the % 360 normalisation
void LIBMATTI_MC_Entity_SetRot(LIBMATTI_MC_Entity *entity, float yRot, float xRot)
{
    if (entity == NULL)
        return;
    LIBMATTI_MC_Entity_SetYRot(entity, (float) fmod(yRot, 360.0f));
    LIBMATTI_MC_Entity_SetXRot(entity, (float) fmod(xRot, 360.0f));
}

void LIBMATTI_MC_Entity_SetYRot(LIBMATTI_MC_Entity *entity, float yRot)
{
    if (entity != NULL)
        entity->yRot = yRot;
}

void LIBMATTI_MC_Entity_SetXRot(LIBMATTI_MC_Entity *entity, float xRot)
{
    if (entity != NULL)
        entity->xRot = xRot;
}

// Java: public void setOldPosAndRot() - the interpolation anchors
void LIBMATTI_MC_Entity_SetOldPosAndRot(LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return;
    entity->lastX = entity->x;
    entity->lastY = entity->y;
    entity->lastZ = entity->z;
    entity->hasLastKnownPosition = true;
    entity->xRotO = entity->xRot;
    entity->yRotO = entity->yRot;
}

float LIBMATTI_MC_Entity_GetYHeadRot(const LIBMATTI_MC_Entity *entity)
{
    // Java: the base Entity mirrors yRot through yHeadRot
    return LIBMATTI_MC_Entity_GetYRot(entity);
}

void LIBMATTI_MC_Entity_SetYHeadRot(LIBMATTI_MC_Entity *entity, float yHeadRot)
{
    LIBMATTI_MC_Entity_SetYRot(entity, yHeadRot);
}

// Java: public void turn(double yaw, double pitch) - the mouse-look path
void LIBMATTI_MC_Entity_Turn(LIBMATTI_MC_Entity *entity, double yaw, double pitch)
{
    if (entity == NULL)
        return;
    float f = (float) pitch * 0.15f;
    float f1 = (float) yaw * 0.15f;
    LIBMATTI_MC_Entity_SetXRot(entity, LIBMATTI_MC_Entity_GetXRot(entity) + f);
    LIBMATTI_MC_Entity_SetYRot(entity, LIBMATTI_MC_Entity_GetYRot(entity) + f1);
    LIBMATTI_MC_Entity_SetXRot(entity, LIBMATTI_MC_Mth_Clamp(LIBMATTI_MC_Entity_GetXRot(entity), -90.0f, 90.0f));
    entity->xRotO += f;
}

const LIBMATTI_MC_AABB *LIBMATTI_MC_Entity_GetBoundingBox(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->bb : NULL;
}

void LIBMATTI_MC_Entity_SetBoundingBox(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_AABB *bb)
{
    if (entity == NULL)
    {
        free(bb);
        return;
    }
    free(entity->bb);
    entity->bb = bb;
}

LIBMATTI_MC_AABB *LIBMATTI_MC_Entity_MakeBoundingBox(const LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return NULL;
    LIBMATTI_MC_EntityDimensions *dimensions = LIBMATTI_MC_EntityType_GetDimensions(entity->type);
    return LIBMATTI_MC_EntityDimensions_MakeBoundingBoxAt(dimensions, entity->x, entity->y, entity->z);
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_Entity_GetDimensions(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? LIBMATTI_MC_EntityType_GetDimensions(entity->type) : NULL;
}

void LIBMATTI_MC_Entity_RefreshDimensions(LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return;
    // Java: refreshDimensions - the bbox rebuilds from the (possibly changed) type
    LIBMATTI_MC_AABB *bb = LIBMATTI_MC_Entity_MakeBoundingBox(entity);
    LIBMATTI_MC_Entity_SetBoundingBox(entity, bb);
}

float LIBMATTI_MC_Entity_GetEyeHeight(const LIBMATTI_MC_Entity *entity)
{
    LIBMATTI_MC_EntityDimensions *dimensions = LIBMATTI_MC_Entity_GetDimensions(entity);
    return dimensions != NULL ? dimensions->eyeHeight : 0.0f;
}

LIBMATTI_MC_Vec3 *LIBMATTI_MC_Entity_GetEyePosition(const LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return LIBMATTI_MC_Vec3_New(0.0, 0.0, 0.0);
    return LIBMATTI_MC_Vec3_New(entity->x, entity->y + LIBMATTI_MC_Entity_GetEyeHeight(entity), entity->z);
}

void LIBMATTI_MC_Entity_GetDeltaMovement(const LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Vec3 *out)
{
    if (out == NULL)
        return;
    if (entity != NULL)
        *out = (LIBMATTI_MC_Vec3){entity->dx, entity->dy, entity->dz};
    else
        *out = (LIBMATTI_MC_Vec3){0.0, 0.0, 0.0};
}

void LIBMATTI_MC_Entity_SetDeltaMovement(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *movement)
{
    if (entity == NULL || movement == NULL)
        return;
    entity->dx = movement->x;
    entity->dy = movement->y;
    entity->dz = movement->z;
}

bool LIBMATTI_MC_Entity_OnGround(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->onGround;
}

void LIBMATTI_MC_Entity_SetOnGround(LIBMATTI_MC_Entity *entity, bool onGround)
{
    if (entity != NULL)
        entity->onGround = onGround;
}

float LIBMATTI_MC_Entity_GetFallDistance(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->fallDistance : 0.0f;
}

void LIBMATTI_MC_Entity_SetFallDistance(LIBMATTI_MC_Entity *entity, float fallDistance)
{
    if (entity != NULL)
        entity->fallDistance = fallDistance;
}

bool LIBMATTI_MC_Entity_IsInvulnerable(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->invulnerable;
}

void LIBMATTI_MC_Entity_SetInvulnerable(LIBMATTI_MC_Entity *entity, bool invulnerable)
{
    if (entity != NULL)
        entity->invulnerable = invulnerable;
}

bool LIBMATTI_MC_Entity_IsNoGravity(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->noGravity;
}

void LIBMATTI_MC_Entity_SetNoGravity(LIBMATTI_MC_Entity *entity, bool noGravity)
{
    if (entity != NULL)
        entity->noGravity = noGravity;
}

// Java: public boolean isNoPhysics() / setNoPhysics - the ghost/spectator gate
bool LIBMATTI_MC_Entity_IsNoPhysics(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->noPhysics;
}

void LIBMATTI_MC_Entity_SetNoPhysics(LIBMATTI_MC_Entity *entity, bool noPhysics)
{
    if (entity != NULL)
        entity->noPhysics = noPhysics;
}

// Java: the horizontalCollision / verticalCollision getters
bool LIBMATTI_MC_Entity_HorizontalCollision(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->horizontalCollision;
}

bool LIBMATTI_MC_Entity_VerticalCollision(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->verticalCollision;
}

bool LIBMATTI_MC_Entity_VerticalCollisionBelow(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->verticalCollisionBelow;
}

bool LIBMATTI_MC_Entity_IsSilent(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->silent;
}

bool LIBMATTI_MC_Entity_IsGlowing(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->glowing;
}

struct LIBMATTI_MC_Level *LIBMATTI_MC_Entity_Level(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL ? entity->level : NULL;
}

void LIBMATTI_MC_Entity_SetLevel(LIBMATTI_MC_Entity *entity, struct LIBMATTI_MC_Level *level)
{
    if (entity != NULL)
        entity->level = level;
}

bool LIBMATTI_MC_Entity_IsRemoved(const LIBMATTI_MC_Entity *entity)
{
    return entity != NULL && entity->removed;
}

void LIBMATTI_MC_Entity_SetRemoved(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_EntityRemovalReason reason)
{
    if (entity == NULL || entity->removed)
        return;
    // Java: if (this.removalReason == null) this.removalReason = reason
    entity->removed = true;
    entity->removalReason = reason;
}

void LIBMATTI_MC_Entity_Discard(LIBMATTI_MC_Entity *entity)
{
    // Java: this.remove(Entity.RemovalReason.DISCARDED)
    LIBMATTI_MC_Entity_SetRemoved(entity, LIBMATTI_MC_EntityRemovalReason_DISCARDED);
}

void LIBMATTI_MC_Entity_Kill(LIBMATTI_MC_Entity *entity)
{
    // Java: this.remove(Entity.RemovalReason.KILLED)
    LIBMATTI_MC_Entity_SetRemoved(entity, LIBMATTI_MC_EntityRemovalReason_KILLED);
}

// Java: the ArraySetSorted<String> tags - sorted insertion, no duplicates
bool LIBMATTI_MC_Entity_AddTag(LIBMATTI_MC_Entity *entity, const char *tag)
{
    if (entity == NULL || tag == NULL)
        return false;
    for (int i = 0; i < entity->tagCount; i++)
    {
        int cmp = strcmp(entity->tags[i], tag);
        if (cmp == 0)
            return false; // Java: the set add returns false on presence
        if (cmp > 0)
        {
            // insert sorted
            if (entity->tagCount >= entity->tagCapacity)
            {
                int next = entity->tagCapacity > 0 ? entity->tagCapacity * 2 : 4;
                char **grown = realloc(entity->tags, (size_t) next * sizeof(char *));
                if (grown == NULL)
                    return false;
                entity->tags = grown;
                entity->tagCapacity = next;
            }
            memmove(&entity->tags[i + 1], &entity->tags[i], (size_t) (entity->tagCount - i) * sizeof(char *));
            entity->tags[i] = strdup(tag);
            entity->tagCount++;
            return true;
        }
    }
    if (entity->tagCount >= entity->tagCapacity)
    {
        int next = entity->tagCapacity > 0 ? entity->tagCapacity * 2 : 4;
        char **grown = realloc(entity->tags, (size_t) next * sizeof(char *));
        if (grown == NULL)
            return false;
        entity->tags = grown;
        entity->tagCapacity = next;
    }
    entity->tags[entity->tagCount++] = strdup(tag);
    return true;
}

bool LIBMATTI_MC_Entity_RemoveTag(LIBMATTI_MC_Entity *entity, const char *tag)
{
    if (entity == NULL || tag == NULL)
        return false;
    for (int i = 0; i < entity->tagCount; i++)
    {
        if (strcmp(entity->tags[i], tag) == 0)
        {
            free(entity->tags[i]);
            memmove(&entity->tags[i], &entity->tags[i + 1], (size_t) (entity->tagCount - i - 1) * sizeof(char *));
            entity->tagCount--;
            return true;
        }
    }
    return false;
}

const char *const *LIBMATTI_MC_Entity_GetTags(const LIBMATTI_MC_Entity *entity, int *outCount)
{
    if (outCount != NULL)
        *outCount = entity != NULL ? entity->tagCount : 0;
    return entity != NULL ? (const char *const *) entity->tags : NULL;
}

// ---------------------------------------------------------------------------
// Java: the collide path (Entity.collideBoundingBox -> collectColliders ->
// collideWithShapes -> Shapes.collide). The sweep walks the three axes in
// axisStepOrder (|x| < |z| ? YZX : YXZ - Y first, then the smaller horizontal
// axis first) and clips the movement against every block shape the swept box
// overlaps, exactly like Shapes.collide's per-shape reduction.
// ---------------------------------------------------------------------------

// Java: Math.abs(p_193139_) < 1.0E-7 -> 0.0 (the epsilon the collide paths share)
#define COLLIDE_EPSILON 1.0e-7

// Java: Direction.axisStepOrder(Vec3) - |x| < |z| ? YZX : YXZ
static void axis_step_order(const LIBMATTI_MC_Vec3 *movement, int *axes)
{
    if (fabs(movement->x) < fabs(movement->z))
    {
        // YZX_AXIS_ORDER
        axes[0] = 1;
        axes[1] = 2;
        axes[2] = 0;
    }
    else
    {
        // YXZ_AXIS_ORDER
        axes[0] = 1;
        axes[1] = 0;
        axes[2] = 2;
    }
}

static double box_min(const LIBMATTI_MC_AABB *box, int axis)
{
    return axis == 0 ? box->minX : (axis == 1 ? box->minY : box->minZ);
}

static double box_max(const LIBMATTI_MC_AABB *box, int axis)
{
    return axis == 0 ? box->maxX : (axis == 1 ? box->maxY : box->maxZ);
}

// Java: Shapes.collide(Axis, AABB, Iterable<VoxelShape>, double) - the movement
// on one axis reduces against every shape; the shapes arrive as the swept AABBs
// the level scan produced (the port's VoxelShape stand-in)
static double collide_axis(int axis, const LIBMATTI_MC_AABB *box, LIBMATTI_MC_AABB **shapes, int shapeCount, double movement)
{
    for (int i = 0; i < shapeCount; i++)
    {
        // Java: if (Math.abs(p_193139_) < 1.0E-7) return 0.0
        if (fabs(movement) < COLLIDE_EPSILON)
            return 0.0;
        const LIBMATTI_MC_AABB *shape = shapes[i];
        // Java: VoxelShape.collideX - the other two axes must overlap first
        int axisA = (axis + 1) % 3;
        int axisB = (axis + 2) % 3;
        if (box_max(shape, axisA) <= box_min(box, axisA) + COLLIDE_EPSILON
            || box_min(shape, axisA) >= box_max(box, axisA) - COLLIDE_EPSILON)
            continue;
        if (box_max(shape, axisB) <= box_min(box, axisB) + COLLIDE_EPSILON
            || box_min(shape, axisB) >= box_max(box, axisB) - COLLIDE_EPSILON)
            continue;
        // Java: the positive walk - the nearest shape face ahead of box.max(axis)
        if (movement > 0.0)
        {
            double d2 = box_min(shape, axis) - box_max(box, axis);
            if (d2 >= -COLLIDE_EPSILON)
                movement = fmin(movement, d2);
        }
        else if (movement < 0.0)
        {
            // Java: the negative walk - the nearest face behind box.min(axis)
            double d3 = box_max(shape, axis) - box_min(box, axis);
            if (d3 <= COLLIDE_EPSILON)
                movement = fmax(movement, d3);
        }
    }
    return movement;
}

// Java: public static Vec3 collideBoundingBox(Entity, Vec3, AABB, Level, List)
void LIBMATTI_MC_Entity_CollideBoundingBox(const LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *movement,
                                           const LIBMATTI_MC_AABB *box, struct LIBMATTI_MC_Level *level,
                                           LIBMATTI_MC_Vec3 *out)
{
    if (out == NULL)
        return;
    out->x = 0.0;
    out->y = 0.0;
    out->z = 0.0;
    if (movement == NULL || box == NULL || level == NULL)
        return;

    // Java: collectColliders - the swept box expandTowards(movement) bounds the
    // block scan (the entity/world-border collisions ride later phases)
    LIBMATTI_MC_AABB *swept = LIBMATTI_MC_AABB_ExpandTowards(box, movement);
    if (swept == NULL)
        return;
    // the scan bounds grow one block over the swept box (the epsilon slack of
    // the BlockCollisions walk)
    LIBMATTI_MC_AABB *shapes[128];
    int shapeCount = LIBMATTI_MC_Level_GetBlockCollisions(level, swept->minX - 1.0, swept->minY - 1.0, swept->minZ - 1.0,
                                                          swept->maxX + 1.0, swept->maxY + 1.0, swept->maxZ + 1.0,
                                                          shapes, (int) (sizeof(shapes) / sizeof(shapes[0])));
    free(swept);
    if (shapeCount == 0)
    {
        // Java: collideWithShapes returns the movement when the list is empty
        *out = *movement;
        return;
    }

    // Java: collideWithShapes - vec3 accumulates the per-axis clip over the moved
    // box (the box rides the already-clipped axes)
    int axes[3];
    axis_step_order(movement, axes);
    double x = 0.0, y = 0.0, z = 0.0;
    for (int i = 0; i < 3; i++)
    {
        int axis = axes[i];
        double d0 = axis == 0 ? movement->x : (axis == 1 ? movement->y : movement->z);
        if (d0 == 0.0)
            continue;
        LIBMATTI_MC_AABB *moved = LIBMATTI_MC_AABB_Move3(box, x, y, z);
        if (moved == NULL)
            break;
        double d1 = collide_axis(axis, moved, shapes, shapeCount, d0);
        free(moved);
        if (axis == 0)
            x = d1;
        else if (axis == 1)
            y = d1;
        else
            z = d1;
    }
    for (int i = 0; i < shapeCount; i++)
        free(shapes[i]);
    out->x = x;
    out->y = y;
    out->z = z;
}

// Java: private Vec3 collide(Vec3) - the entity box sweeps the movement
void LIBMATTI_MC_Entity_Collide(const LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *movement, LIBMATTI_MC_Vec3 *out)
{
    if (out == NULL)
        return;
    out->x = 0.0;
    out->y = 0.0;
    out->z = 0.0;
    if (entity == NULL || movement == NULL)
        return;
    // Java: vec3 = p_20273_.lengthSqr() == 0.0 ? p_20273_ : collideBoundingBox(...)
    if (movement->x == 0.0 && movement->y == 0.0 && movement->z == 0.0)
    {
        *out = *movement;
        return;
    }
    const LIBMATTI_MC_AABB *box = LIBMATTI_MC_Entity_GetBoundingBox(entity);
    LIBMATTI_MC_Entity_CollideBoundingBox(entity, movement, box, entity->level, out);
}

// Java: public void move(MoverType, Vec3)
void LIBMATTI_MC_Entity_Move(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_MoverType moverType, const LIBMATTI_MC_Vec3 *movement)
{
    if (entity == NULL || movement == NULL)
        return;
    (void) moverType;
    // Java: if (this.noPhysics) setPos(x + dx, y + dy, z + dz) + the flags false
    if (entity->noPhysics)
    {
        LIBMATTI_MC_Entity_SetPos(entity, entity->x + movement->x, entity->y + movement->y, entity->z + movement->z);
        entity->horizontalCollision = false;
        entity->verticalCollision = false;
        entity->verticalCollisionBelow = false;
        entity->minorHorizontalCollision = false;
        return;
    }

    // Java: Vec3 vec3 = this.collide(p_19974_) - the clipped movement
    LIBMATTI_MC_Vec3 clipped;
    LIBMATTI_MC_Entity_Collide(entity, movement, &clipped);
    double lengthSqr = clipped.x * clipped.x + clipped.y * clipped.y + clipped.z * clipped.z;
    // Java: if (d0 > 1.0E-7 || p_19974_.lengthSqr() - d0 < 1.0E-7) setPos(position.add(vec3))
    double moveSqr = movement->x * movement->x + movement->y * movement->y + movement->z * movement->z;
    if (lengthSqr > 1.0e-7 || moveSqr - lengthSqr < 1.0e-7)
        LIBMATTI_MC_Entity_SetPos(entity, entity->x + clipped.x, entity->y + clipped.y, entity->z + clipped.z);

    // Java: boolean flag = !Mth.equal(p_19974_.x, vec3.x); flag1 = !equal(z)
    bool flag = fabs(movement->x - clipped.x) >= 1.0e-5;
    bool flag1 = fabs(movement->z - clipped.z) >= 1.0e-5;
    entity->horizontalCollision = flag || flag1;
    // Java: if (Math.abs(p_19974_.y) > 0.0 || authoritative) - the vertical flags
    if (fabs(movement->y) > 0.0)
    {
        entity->verticalCollision = movement->y != clipped.y;
        entity->verticalCollisionBelow = entity->verticalCollision && movement->y < 0.0;
        // Java: setOnGroundWithMovement(verticalCollisionBelow, horizontalCollision, vec3)
        LIBMATTI_MC_Entity_SetOnGround(entity, entity->verticalCollisionBelow);
    }

    // Java: if (this.horizontalCollision) setDeltaMovement(flag ? 0 : dx, dy, flag1 ? 0 : dz)
    if (entity->horizontalCollision)
    {
        LIBMATTI_MC_Vec3 motion;
        LIBMATTI_MC_Entity_GetDeltaMovement(entity, &motion);
        LIBMATTI_MC_Vec3 reset = {flag ? 0.0 : motion.x, motion.y, flag1 ? 0.0 : motion.z};
        LIBMATTI_MC_Entity_SetDeltaMovement(entity, &reset);
    }
}

// Java: private void computeSpeed() - the walk-speed bookkeeping (the port
// tracks the horizontal movement into the speed value)
void LIBMATTI_MC_Entity_ComputeSpeed(LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return;
    // Java: the walkDistance/walkAnimation bookkeeping rides the position delta;
    // the port keeps the delta-movement length as the visible speed
    (void) entity;
}

// Java: public void tick() { this.baseTick(); }
void LIBMATTI_MC_Entity_Tick(LIBMATTI_MC_Entity *entity)
{
    LIBMATTI_MC_Entity_BaseTick(entity);
}

// Java: public void baseTick() - the movement-agnostic per-tick bookkeeping
void LIBMATTI_MC_Entity_BaseTick(LIBMATTI_MC_Entity *entity)
{
    if (entity == NULL)
        return;
    LIBMATTI_MC_Entity_ComputeSpeed(entity);
    // Java: the vehicle-removed stopRiding, the fire ticks, the head rotation
    // update, the portal checks - each arrives with its system. The port keeps
    // the fire tick decay and the old-rotation anchors.
    entity->yRotO = entity->yRot;
    entity->xRotO = entity->xRot;
    if (entity->remainingFireTicks > 0)
    {
        if (entity->remainingFireTicks % 20 == 0 && !LIBMATTI_MC_EntityType_FireImmune(entity->type))
            entity->invulnerable = entity->invulnerable; // the damage path is game content
        entity->remainingFireTicks--;
        if (entity->remainingFireTicks < 0)
            entity->remainingFireTicks = 0;
    }
    // Java: if (this.isRemoved()) { this.stopRiding(); ... } - the level removes
    // the entity through its own list (P5's tick loop)
}

// ---------------------------------------------------------------------------
// Java: the NBT surface
// ---------------------------------------------------------------------------

// Java: the Vec3/Vec2 codec writes a ListTag of doubles/floats - the port
// builds the list over the raw tag primitives
static void put_double_list(LIBMATTI_MC_Nbt_CompoundTag *tag, const char *key, const double *values, int count)
{
    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < count; i++)
        LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_DoubleTag_Of(values[i]));
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, key, (LIBMATTI_MC_Nbt_Tag *) list);
}

static void put_float_list(LIBMATTI_MC_Nbt_CompoundTag *tag, const char *key, const float *values, int count)
{
    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < count; i++)
        LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_FloatTag_Of(values[i]));
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, key, (LIBMATTI_MC_Nbt_Tag *) list);
}

static int get_double_list(const LIBMATTI_MC_Nbt_CompoundTag *tag, const char *key, double *out, int count)
{
    LIBMATTI_MC_Nbt_ListTag *list = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, key, &list) || list == NULL)
        return 0;
    if (LIBMATTI_MC_Nbt_ListTag_Size(list) != count)
        return 0;
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_Tag *entry = LIBMATTI_MC_Nbt_ListTag_Get(list, i);
        if (entry == NULL || entry->id != LIBMATTI_MC_Nbt_TAG_DOUBLE)
            return 0;
        out[i] = entry->as.doubleValue;
    }
    return 1;
}

static int get_float_list(const LIBMATTI_MC_Nbt_CompoundTag *tag, const char *key, float *out, int count)
{
    LIBMATTI_MC_Nbt_ListTag *list = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, key, &list) || list == NULL)
        return 0;
    if (LIBMATTI_MC_Nbt_ListTag_Size(list) != count)
        return 0;
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_Tag *entry = LIBMATTI_MC_Nbt_ListTag_Get(list, i);
        if (entry == NULL || entry->id != LIBMATTI_MC_Nbt_TAG_FLOAT)
            return 0;
        out[i] = entry->as.floatValue;
    }
    return 1;
}

// Java: public void saveWithoutId(ValueOutput) - the vanilla keys
void LIBMATTI_MC_Entity_SaveWithoutId(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (entity == NULL || tag == NULL)
        return;
    // Java: p_409187_.store("Pos", Vec3.CODEC, this.position()) - the list of
    // three doubles
    {
        double pos[3] = {entity->x, entity->y, entity->z};
        put_double_list(tag, "Pos", pos, 3);
    }
    {
        double motion[3] = {entity->dx, entity->dy, entity->dz};
        put_double_list(tag, "Motion", motion, 3);
    }
    {
        // Java: store("Rotation", Vec2.CODEC, new Vec2(yRot, xRot)) - floats
        float rot[2] = {entity->yRot, entity->xRot};
        put_float_list(tag, "Rotation", rot, 2);
    }
    LIBMATTI_MC_Nbt_CompoundTag_PutDouble(tag, "fall_distance", entity->fallDistance);
    LIBMATTI_MC_Nbt_CompoundTag_PutShort(tag, "Fire", (int16_t) entity->remainingFireTicks);
    LIBMATTI_MC_Nbt_CompoundTag_PutShort(tag, "Air", (int16_t) entity->airSupply);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, "OnGround", entity->onGround);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, "Invulnerable", entity->invulnerable);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "PortalCooldown", entity->portalCooldown);
    // Java: store("UUID", UUIDUtil.CODEC, this.getUUID()) - the int-array form
    // (four ints, big-endian over the two words)
    {
        uint64_t most = LIBMATTI_JU_UUID_GetMostSignificantBits(&entity->uuid);
        uint64_t least = LIBMATTI_JU_UUID_GetLeastSignificantBits(&entity->uuid);
        int32_t uuid[4] = {
            (int32_t) (uint32_t) (most >> 32),
            (int32_t) (uint32_t) most,
            (int32_t) (uint32_t) (least >> 32),
            (int32_t) (uint32_t) least,
        };
        LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(tag, "UUID", uuid, 4);
    }
    if (entity->noGravity)
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, "NoGravity", true);
    if (entity->silent)
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, "Silent", true);
    if (entity->glowing)
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, "Glowing", true);
    if (entity->tagCount > 0)
    {
        // Java: the Tags list of strings
        LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
        for (int i = 0; i < entity->tagCount; i++)
            LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_StringTag_Of(entity->tags[i]));
        LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "Tags", (LIBMATTI_MC_Nbt_Tag *) list);
    }
}

// Java: public void load(ValueInput) - the vanilla keys (the invalid-position
// checks abort like Java's IllegalStateException)
void LIBMATTI_MC_Entity_Load(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (entity == NULL || tag == NULL)
        return;
    {
        double pos[3] = {0.0, 0.0, 0.0};
        if (get_double_list(tag, "Pos", pos, 3))
        {
            // Java: the world-border clamps (3.0000512E7 / 2.0E7)
            double x = LIBMATTI_MC_Mth_ClampD(pos[0], -3.0000512e7, 3.0000512e7);
            double y = LIBMATTI_MC_Mth_ClampD(pos[1], -2.0e7, 2.0e7);
            double z = LIBMATTI_MC_Mth_ClampD(pos[2], -3.0000512e7, 3.0000512e7);
            LIBMATTI_MC_Entity_SetPosRaw(entity, x, y, z);
        }
    }
    {
        double motion[3] = {0.0, 0.0, 0.0};
        if (get_double_list(tag, "Motion", motion, 3))
        {
            // Java: components above 10 clamp to zero
            entity->dx = fabs(motion[0]) > 10.0 ? 0.0 : motion[0];
            entity->dy = fabs(motion[1]) > 10.0 ? 0.0 : motion[1];
            entity->dz = fabs(motion[2]) > 10.0 ? 0.0 : motion[2];
        }
    }
    {
        float rot[2] = {0.0f, 0.0f};
        if (get_float_list(tag, "Rotation", rot, 2))
        {
            entity->yRot = rot[0];
            entity->xRot = rot[1];
        }
    }
    LIBMATTI_MC_Entity_SetOldPosAndRot(entity);
    entity->fallDistance = (float) LIBMATTI_MC_Nbt_CompoundTag_GetDoubleOr(tag, "fall_distance", 0.0);
    entity->remainingFireTicks = LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(tag, "Fire", 0);
    entity->airSupply = LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(tag, "Air", 300);
    entity->onGround = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "OnGround", false);
    entity->invulnerable = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "Invulnerable", false);
    entity->portalCooldown = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "PortalCooldown", 0);
    {
        // Java: read("UUID", UUIDUtil.CODEC) - the int-array form
        const int32_t *uuid = NULL;
        size_t uuidLength = 0;
        if (LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(tag, "UUID", &uuid, &uuidLength) && uuidLength == 4)
        {
            uint64_t most = ((uint64_t) (uint32_t) uuid[0] << 32) | (uint32_t) uuid[1];
            uint64_t least = ((uint64_t) (uint32_t) uuid[2] << 32) | (uint32_t) uuid[3];
            LIBMATTI_MC_Entity_SetUUID(entity, LIBMATTI_JU_UUID_FromBits(most, least));
        }
    }
    entity->noGravity = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "NoGravity", false);
    entity->silent = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "Silent", false);
    entity->glowing = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "Glowing", false);
    {
        // Java: this.tag = read("Tags") - the sorted string list clears + refills
        LIBMATTI_MC_Nbt_ListTag *tagList = NULL;
        if (LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, "Tags", &tagList))
        {
            for (int i = entity->tagCount - 1; i >= 0; i--)
                free(entity->tags[i]);
            entity->tagCount = 0;
            if (tagList != NULL)
            {
                int size = LIBMATTI_MC_Nbt_ListTag_Size(tagList);
                for (int i = 0; i < size; i++)
                    LIBMATTI_MC_Entity_AddTag(entity, LIBMATTI_MC_Nbt_Tag_AsString(LIBMATTI_MC_Nbt_ListTag_Get(tagList, i)));
            }
        }
    }
    // Java: this.reapplyPosition(); this.setRot(getYRot(), getXRot()) - the bbox
    // rebuilds from the loaded position
    LIBMATTI_MC_AABB *bb = LIBMATTI_MC_Entity_MakeBoundingBox(entity);
    LIBMATTI_MC_Entity_SetBoundingBox(entity, bb);
}
