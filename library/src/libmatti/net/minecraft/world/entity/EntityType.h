// Port of net.minecraft.world.entity.EntityType. Java models the type as the
// registry object carrying the factory, the MobCategory, the sync/tracking
// settings and the EntityDimensions; the C port keeps the same fields plus the
// Builder chain and the registry helpers (register through
// BuiltInRegistries.ENTITY_TYPE like Java's static block).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_ENTITYTYPE_H
#define MATTICRAFT_MC_WORLD_ENTITY_ENTITYTYPE_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/world/entity/EntityDimensions.h"
#include "libmatti/net/minecraft/world/entity/MobCategory.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Level;
struct LIBMATTI_MC_Entity;
struct LIBMATTI_MC_EntityType;
typedef struct LIBMATTI_MC_EntityType LIBMATTI_MC_EntityType;

// Java: interface EntityType.EntityFactory<T> - (EntityType, Level) -> Entity
typedef struct LIBMATTI_MC_Entity *(*LIBMATTI_MC_EntityFactory)(
        const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);

// Java: public class EntityType<T extends Entity>
typedef struct LIBMATTI_MC_EntityType
{
    // Java: private final EntityType.EntityFactory<T> factory
    LIBMATTI_MC_EntityFactory factory;
    // Java: private final MobCategory category
    LIBMATTI_MC_MobCategory category;
    // Java: private final boolean serialize / summon / fireImmune / canSpawnFarFromPlayer
    bool serialize;
    bool summon;
    bool fireImmune;
    bool canSpawnFarFromPlayer;
    // Java: private final ImmutableSet<Block> immuneTo - NULL = empty set
    const void *const *immuneTo;
    size_t immuneToCount;
    // Java: private final EntityDimensions dimensions
    LIBMATTI_MC_EntityDimensions *dimensions;
    // Java: private final float spawnDimensionsScale
    float spawnDimensionsScale;
    // Java: private final int clientTrackingRange / updateInterval
    int clientTrackingRange;
    int updateInterval;
    // Java: private final String descriptionId - "entity.<ns>.<path>"
    char *descriptionId;
    // Java: private final Optional<ResourceKey<LootTable>> lootTable - NULL = empty
    void *lootTable;
    // Java: private final boolean allowedInPeaceful
    bool allowedInPeaceful;
    // the registered id (the registry key path; the registry lookup derives from it)
    char *id;
} LIBMATTI_MC_EntityType;

// Java: public static class Builder<T extends Entity>
typedef struct LIBMATTI_MC_EntityTypeBuilder LIBMATTI_MC_EntityTypeBuilder;

// Java: public static <T> Builder<T> of(EntityFactory, MobCategory)
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_EntityFactory factory, LIBMATTI_MC_MobCategory category);
// Java: public static <T> Builder<T> createNothing(MobCategory) - a NULL factory
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_CreateNothing(LIBMATTI_MC_MobCategory category);
// The builder chain (each returns the builder, Java-style)
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder *builder, float width, float height);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_EyeHeight(LIBMATTI_MC_EntityTypeBuilder *builder, float eyeHeight);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoLootTable(LIBMATTI_MC_EntityTypeBuilder *builder);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoSummon(LIBMATTI_MC_EntityTypeBuilder *builder);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoSave(LIBMATTI_MC_EntityTypeBuilder *builder);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_FireImmune(LIBMATTI_MC_EntityTypeBuilder *builder);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_ImmuneTo(LIBMATTI_MC_EntityTypeBuilder *builder, const void *const *blocks, size_t count);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_ClientTrackingRange(LIBMATTI_MC_EntityTypeBuilder *builder, int range);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_UpdateInterval(LIBMATTI_MC_EntityTypeBuilder *builder, int interval);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_SpawnDimensionsScale(LIBMATTI_MC_EntityTypeBuilder *builder, float scale);
LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_AllowedInPeaceful(LIBMATTI_MC_EntityTypeBuilder *builder, bool allowed);
// Java: public EntityType<T> build(ResourceKey) - the key supplies the id/description
LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityTypeBuilder_Build(LIBMATTI_MC_EntityTypeBuilder *builder, const char *id);

// Java: public T create(Level) - through the factory (NULL factory = NULL)
struct LIBMATTI_MC_Entity *LIBMATTI_MC_EntityType_Create(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
// Java: public T create(ServerLevel, SpawnReason, BlockPos) - positioned spawn
struct LIBMATTI_MC_Entity *LIBMATTI_MC_EntityType_CreateAt(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level,
                                                           const LIBMATTI_MC_BlockPos *pos);
// Java: public EntityDimensions getDimensions() / float getWidth()/getHeight()
LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityType_GetDimensions(const LIBMATTI_MC_EntityType *type);
float LIBMATTI_MC_EntityType_GetWidth(const LIBMATTI_MC_EntityType *type);
float LIBMATTI_MC_EntityType_GetHeight(const LIBMATTI_MC_EntityType *type);
// Java: public MobCategory getCategory()
LIBMATTI_MC_MobCategory LIBMATTI_MC_EntityType_GetCategory(const LIBMATTI_MC_EntityType *type);
// Java: public boolean canSerialize() / canSummon() / isAllowedInPeaceful()
bool LIBMATTI_MC_EntityType_CanSerialize(const LIBMATTI_MC_EntityType *type);
bool LIBMATTI_MC_EntityType_CanSummon(const LIBMATTI_MC_EntityType *type);
bool LIBMATTI_MC_EntityType_IsAllowedInPeaceful(const LIBMATTI_MC_EntityType *type);
// Java: public boolean fireImmune()
bool LIBMATTI_MC_EntityType_FireImmune(const LIBMATTI_MC_EntityType *type);
// Java: public int clientTrackingRange() / updateInterval() / trackDeltas()
int LIBMATTI_MC_EntityType_ClientTrackingRange(const LIBMATTI_MC_EntityType *type);
int LIBMATTI_MC_EntityType_UpdateInterval(const LIBMATTI_MC_EntityType *type);
bool LIBMATTI_MC_EntityType_TrackDeltas(const LIBMATTI_MC_EntityType *type);
// Java: public String getDescriptionId()
const char *LIBMATTI_MC_EntityType_GetDescriptionId(const LIBMATTI_MC_EntityType *type);
// the registered id path (the port keeps it on the type)
const char *LIBMATTI_MC_EntityType_GetId(const LIBMATTI_MC_EntityType *type);
void LIBMATTI_MC_EntityType_Free(LIBMATTI_MC_EntityType *type);

// Java: private static <T> EntityType<T> register(String, Builder) - through
// BuiltInRegistries.ENTITY_TYPE (ResourceKey.create(Registries.ENTITY_TYPE, ...))
LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_Register(const char *id, LIBMATTI_MC_EntityTypeBuilder *builder);
// Java: public static Identifier getKey(EntityType) - NULL when unregistered
const LIBMATTI_MC_Identifier *LIBMATTI_MC_EntityType_GetKey(const LIBMATTI_MC_EntityType *type);
// Java: public static Optional<EntityType<?>> byString(String) - the registry lookup
LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ByString(const char *name);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_ENTITYTYPE_H
