// Port of net.minecraft.world.entity.Entity (the base-class core the game
// needs before the P5 player/mob work: position + rotation, the dimensions-
// derived bounding box, the entity id + UUID, the command tags, the base tick
// bookkeeping and the NBT surface Java's saveWithoutId/load write).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_ENTITY_H
#define MATTICRAFT_MC_WORLD_ENTITY_ENTITY_H

#include "libmatti/java/util/UUID.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/world/entity/EntityDimensions.h"
#include "libmatti/net/minecraft/world/entity/EntityType.h"
#include "libmatti/net/minecraft/world/phys/AABB.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Level;

// Java: public static enum Entity.RemovalReason
typedef enum LIBMATTI_MC_EntityRemovalReason
{
    LIBMATTI_MC_EntityRemovalReason_KILLED,
    LIBMATTI_MC_EntityRemovalReason_DISCARDED,
    LIBMATTI_MC_EntityRemovalReason_UNLOADED_TO_CHUNK,
    LIBMATTI_MC_EntityRemovalReason_UNLOADED_WITH_PLAYER,
    LIBMATTI_MC_EntityRemovalReason_CHANGED_DIMENSION,
} LIBMATTI_MC_EntityRemovalReason;

// Java: public abstract class Entity
typedef struct LIBMATTI_MC_Entity
{
    // Java: private final EntityType<?> type
    const LIBMATTI_MC_EntityType *type;
    // Java: protected Level level (NULL until the entity is added to a level)
    struct LIBMATTI_MC_Level *level;
    // Java: private int id = ENTITY_COUNTER.incrementAndGet()
    int id;
    // Java: private UUID uuid
    LIBMATTI_JU_UUID uuid;
    // Java: private Vec3 position - the port keeps the flat triple like Camera
    double x;
    double y;
    double z;
    // Java: private Vec3 lastKnownPosition (the interpolation anchor)
    bool hasLastKnownPosition;
    double lastX;
    double lastY;
    double lastZ;
    // Java: private Vec3 deltaMovement = Vec3.ZERO
    double dx;
    double dy;
    double dz;
    // Java: private float yRot / xRot (+ the old rotations xRotO/yRotO)
    float yRot;
    float xRot;
    float yRotO;
    float xRotO;
    // Java: private float fallDistance
    float fallDistance;
    // Java: private AABB bb - derived from the dimensions + position
    LIBMATTI_MC_AABB *bb;
    // Java: private boolean onGround
    bool onGround;
    // Java: private boolean invulnerable
    bool invulnerable;
    // Java: private int remainingFireTicks / portalCooldown
    int remainingFireTicks;
    int portalCooldown;
    // Java: private boolean removed + the RemovalReason
    bool removed;
    LIBMATTI_MC_EntityRemovalReason removalReason;
    // Java: private final ArraySetSorted<String> tags (the /tag command set)
    char **tags;
    int tagCount;
    int tagCapacity;
    // Java: protected int airSupply = getMaxAirSupply() - 300 by default
    int airSupply;
    // Java: private boolean noPhysics / noCulling / hasVisualFire ...
    bool noPhysics;
    bool glowing;
    bool silent;
    bool noGravity;
    // Java: the first tick flag (EntitiesStatefulSection)
    bool firstTick;
} LIBMATTI_MC_Entity;

// Java: protected Entity(EntityType<?>, Level) - the subclass entry; sets the
// id + uuid, positions at origin with the type's dimensions
LIBMATTI_MC_Entity *LIBMATTI_MC_Entity_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
// The subclass tail: the concrete type calls this after embedding the base
void LIBMATTI_MC_Entity_Init(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
// The base free (frees the bbox + tags; the subclass frees its own fields)
void LIBMATTI_MC_Entity_Free(LIBMATTI_MC_Entity *entity);

// Java: public final int getId() / public final EntityType<?> getType()
int LIBMATTI_MC_Entity_GetId(const LIBMATTI_MC_Entity *entity);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_Entity_GetType(const LIBMATTI_MC_Entity *entity);
// Java: public UUID getUUID() / getStringUUID()
LIBMATTI_JU_UUID LIBMATTI_MC_Entity_GetUUID(const LIBMATTI_MC_Entity *entity);
const char *LIBMATTI_MC_Entity_GetStringUUID(const LIBMATTI_MC_Entity *entity, char *out, size_t outSize);
// Java: public void setUUID(UUID)
void LIBMATTI_MC_Entity_SetUUID(LIBMATTI_MC_Entity *entity, LIBMATTI_JU_UUID uuid);

// Java: public final double getX()/getY()/getZ() (through position)
double LIBMATTI_MC_Entity_GetX(const LIBMATTI_MC_Entity *entity);
double LIBMATTI_MC_Entity_GetY(const LIBMATTI_MC_Entity *entity);
double LIBMATTI_MC_Entity_GetZ(const LIBMATTI_MC_Entity *entity);
// Java: public Vec3 position() - a fresh heap Vec3 like Java's records
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Entity_Position(const LIBMATTI_MC_Entity *entity);
// Java: public BlockPos blockPosition()
LIBMATTI_MC_BlockPos LIBMATTI_MC_Entity_BlockPosition(const LIBMATTI_MC_Entity *entity);

// Java: public void setPos(double, double, double) - raw position + bbox refresh
void LIBMATTI_MC_Entity_SetPos(LIBMATTI_MC_Entity *entity, double x, double y, double z);
void LIBMATTI_MC_Entity_SetPosVec(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *pos);
// Java: protected final void setPosRaw(double, double, double) - no bbox refresh
void LIBMATTI_MC_Entity_SetPosRaw(LIBMATTI_MC_Entity *entity, double x, double y, double z);

// Java: public float getYRot()/getXRot() / protected void setRot(float, float)
float LIBMATTI_MC_Entity_GetYRot(const LIBMATTI_MC_Entity *entity);
float LIBMATTI_MC_Entity_GetXRot(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetRot(LIBMATTI_MC_Entity *entity, float yRot, float xRot);
void LIBMATTI_MC_Entity_SetYRot(LIBMATTI_MC_Entity *entity, float yRot);
void LIBMATTI_MC_Entity_SetXRot(LIBMATTI_MC_Entity *entity, float xRot);
// Java: public void setOldPosAndRot()
void LIBMATTI_MC_Entity_SetOldPosAndRot(LIBMATTI_MC_Entity *entity);
// Java: public float getYHeadRot() - the base entity mirrors yRot
float LIBMATTI_MC_Entity_GetYHeadRot(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetYHeadRot(LIBMATTI_MC_Entity *entity, float yHeadRot);
// Java: public void turn(double yaw, double pitch) - the mouse-look path
// (0.15 sensitivity, pitch clamped to +-90 like Java)
void LIBMATTI_MC_Entity_Turn(LIBMATTI_MC_Entity *entity, double yaw, double pitch);

// Java: public AABB getBoundingBox() / public final void setBoundingBox(AABB)
const LIBMATTI_MC_AABB *LIBMATTI_MC_Entity_GetBoundingBox(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetBoundingBox(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_AABB *bb);
// Java: protected AABB makeBoundingBox() - dimensions.makeBoundingBox(position)
LIBMATTI_MC_AABB *LIBMATTI_MC_Entity_MakeBoundingBox(const LIBMATTI_MC_Entity *entity);
// Java: public EntityDimensions getDimensions() / refreshDimensions()
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_Entity_GetDimensions(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_RefreshDimensions(LIBMATTI_MC_Entity *entity);
// Java: public float getEyeHeight()
float LIBMATTI_MC_Entity_GetEyeHeight(const LIBMATTI_MC_Entity *entity);
// Java: public final Vec3 getEyePosition()
LIBMATTI_MC_Vec3 *LIBMATTI_MC_Entity_GetEyePosition(const LIBMATTI_MC_Entity *entity);

// Java: public Vec3 getDeltaMovement() / setDeltaMovement(Vec3) - the port
// reads into the caller's out-parameter (the Vec3 port is heap-returning)
void LIBMATTI_MC_Entity_GetDeltaMovement(const LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Vec3 *out);
void LIBMATTI_MC_Entity_SetDeltaMovement(LIBMATTI_MC_Entity *entity, const LIBMATTI_MC_Vec3 *movement);
// Java: public boolean onGround() / setOnGround(boolean)
bool LIBMATTI_MC_Entity_OnGround(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetOnGround(LIBMATTI_MC_Entity *entity, bool onGround);
// Java: public double getFallDistance() / setFallDistance(double)
float LIBMATTI_MC_Entity_GetFallDistance(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetFallDistance(LIBMATTI_MC_Entity *entity, float fallDistance);

// Java: public boolean isInvulnerable() / setInvulnerable(boolean)
bool LIBMATTI_MC_Entity_IsInvulnerable(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetInvulnerable(LIBMATTI_MC_Entity *entity, bool invulnerable);
// Java: public boolean isNoGravity() / setNoGravity(boolean)
bool LIBMATTI_MC_Entity_IsNoGravity(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetNoGravity(LIBMATTI_MC_Entity *entity, bool noGravity);
// Java: public boolean isSilent() / isVisualFire / isGlowing
bool LIBMATTI_MC_Entity_IsSilent(const LIBMATTI_MC_Entity *entity);
bool LIBMATTI_MC_Entity_IsGlowing(const LIBMATTI_MC_Entity *entity);

// Java: public Level level() / public void setLevel(Level)
struct LIBMATTI_MC_Level *LIBMATTI_MC_Entity_Level(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetLevel(LIBMATTI_MC_Entity *entity, struct LIBMATTI_MC_Level *level);
// Java: public boolean isRemoved() / setRemoved(RemovalReason) / discard() / kill()
bool LIBMATTI_MC_Entity_IsRemoved(const LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_SetRemoved(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_EntityRemovalReason reason);
void LIBMATTI_MC_Entity_Discard(LIBMATTI_MC_Entity *entity);
void LIBMATTI_MC_Entity_Kill(LIBMATTI_MC_Entity *entity);

// Java: the tag surface (/tag command): addTag/removeTag/getTags
bool LIBMATTI_MC_Entity_AddTag(LIBMATTI_MC_Entity *entity, const char *tag);
bool LIBMATTI_MC_Entity_RemoveTag(LIBMATTI_MC_Entity *entity, const char *tag);
const char *const *LIBMATTI_MC_Entity_GetTags(const LIBMATTI_MC_Entity *entity, int *outCount);

// Java: public void tick() { baseTick(); } - the port calls the base tick
// directly (the tick vtable lands with the P5 entity tick loop)
void LIBMATTI_MC_Entity_Tick(LIBMATTI_MC_Entity *entity);
// Java: public void baseTick() - the movement-agnostic per-tick bookkeeping
void LIBMATTI_MC_Entity_BaseTick(LIBMATTI_MC_Entity *entity);
// Java: private void computeSpeed() - the walk-speed bookkeeping
void LIBMATTI_MC_Entity_ComputeSpeed(LIBMATTI_MC_Entity *entity);

// Java: public void saveWithoutId(ValueOutput) / public void load(ValueInput) -
// the port writes/reads the vanilla NBT keys over CompoundTag
void LIBMATTI_MC_Entity_SaveWithoutId(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Nbt_CompoundTag *tag);
void LIBMATTI_MC_Entity_Load(LIBMATTI_MC_Entity *entity, LIBMATTI_MC_Nbt_CompoundTag *tag);

// Java: getEntityCounter - the shared id source
int LIBMATTI_MC_Entity_NextEntityCounter(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_ENTITY_H
