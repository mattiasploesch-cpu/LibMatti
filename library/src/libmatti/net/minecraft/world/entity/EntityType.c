// Port of net.minecraft.world.entity.EntityType (implementation).

#include "libmatti/net/minecraft/world/entity/EntityType.h"

#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/minecraft/core/Registry.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/core/registries/Registries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/entity/Entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Builder (Java: public static class Builder<T extends Entity>)
// ---------------------------------------------------------------------------

struct LIBMATTI_MC_EntityTypeBuilder
{
    LIBMATTI_MC_EntityFactory factory;
    LIBMATTI_MC_MobCategory category;
    const void *const *immuneTo;
    size_t immuneToCount;
    bool serialize;
    bool summon;
    bool fireImmune;
    bool canSpawnFarFromPlayer;
    int clientTrackingRange;
    int updateInterval;
    LIBMATTI_MC_EntityDimensions *dimensions;
    float spawnDimensionsScale;
    bool hasLootTable;
    bool allowedInPeaceful;
};

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_EntityFactory factory, LIBMATTI_MC_MobCategory category)
{
    LIBMATTI_MC_EntityTypeBuilder *builder = calloc(1, sizeof(LIBMATTI_MC_EntityTypeBuilder));
    if (builder == NULL)
        return NULL;
    builder->factory = factory;
    builder->category = category;
    // Java: serialize = true, summon = true, clientTrackingRange = 5,
    // updateInterval = 3, dimensions = scalable(0.6, 1.8), spawnDimensionsScale = 1
    builder->serialize = true;
    builder->summon = true;
    builder->canSpawnFarFromPlayer = category == LIBMATTI_MC_MobCategory_CREATURE || category == LIBMATTI_MC_MobCategory_MISC;
    builder->clientTrackingRange = 5;
    builder->updateInterval = 3;
    builder->dimensions = LIBMATTI_MC_EntityDimensions_Scalable(0.6f, 1.8f);
    builder->spawnDimensionsScale = 1.0f;
    builder->hasLootTable = true;
    builder->allowedInPeaceful = true;
    return builder;
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_CreateNothing(LIBMATTI_MC_MobCategory category)
{
    return LIBMATTI_MC_EntityTypeBuilder_Of(NULL, category);
}

static LIBMATTI_MC_EntityTypeBuilder *self(LIBMATTI_MC_EntityTypeBuilder *builder)
{
    return builder;
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder *builder, float width, float height)
{
    if (builder == NULL)
        return NULL;
    LIBMATTI_MC_EntityDimensions_Free(builder->dimensions);
    // Java: this.dimensions = EntityDimensions.scalable(width, height) (the eye
    // height defaults through the dimensions again)
    builder->dimensions = LIBMATTI_MC_EntityDimensions_Scalable(width, height);
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_EyeHeight(LIBMATTI_MC_EntityTypeBuilder *builder, float eyeHeight)
{
    if (builder == NULL)
        return NULL;
    LIBMATTI_MC_EntityDimensions *updated = LIBMATTI_MC_EntityDimensions_WithEyeHeight(builder->dimensions, eyeHeight);
    LIBMATTI_MC_EntityDimensions_Free(builder->dimensions);
    builder->dimensions = updated;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoLootTable(LIBMATTI_MC_EntityTypeBuilder *builder)
{
    if (builder == NULL)
        return NULL;
    builder->hasLootTable = false;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoSummon(LIBMATTI_MC_EntityTypeBuilder *builder)
{
    if (builder == NULL)
        return NULL;
    builder->summon = false;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_NoSave(LIBMATTI_MC_EntityTypeBuilder *builder)
{
    if (builder == NULL)
        return NULL;
    builder->serialize = false;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_FireImmune(LIBMATTI_MC_EntityTypeBuilder *builder)
{
    if (builder == NULL)
        return NULL;
    builder->fireImmune = true;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_ImmuneTo(LIBMATTI_MC_EntityTypeBuilder *builder, const void *const *blocks, size_t count)
{
    if (builder == NULL)
        return NULL;
    builder->immuneTo = blocks;
    builder->immuneToCount = count;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_ClientTrackingRange(LIBMATTI_MC_EntityTypeBuilder *builder, int range)
{
    if (builder == NULL)
        return NULL;
    builder->clientTrackingRange = range;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_UpdateInterval(LIBMATTI_MC_EntityTypeBuilder *builder, int interval)
{
    if (builder == NULL)
        return NULL;
    builder->updateInterval = interval;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_SpawnDimensionsScale(LIBMATTI_MC_EntityTypeBuilder *builder, float scale)
{
    if (builder == NULL)
        return NULL;
    builder->spawnDimensionsScale = scale;
    return self(builder);
}

LIBMATTI_MC_EntityTypeBuilder *LIBMATTI_MC_EntityTypeBuilder_AllowedInPeaceful(LIBMATTI_MC_EntityTypeBuilder *builder, bool allowed)
{
    if (builder == NULL)
        return NULL;
    builder->allowedInPeaceful = allowed;
    return self(builder);
}

LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityTypeBuilder_Build(LIBMATTI_MC_EntityTypeBuilder *builder, const char *id)
{
    if (builder == NULL || id == NULL)
        return NULL;
    LIBMATTI_MC_EntityType *type = calloc(1, sizeof(LIBMATTI_MC_EntityType));
    if (type == NULL)
    {
        LIBMATTI_MC_EntityDimensions_Free(builder->dimensions);
        free(builder);
        return NULL;
    }
    type->factory = builder->factory;
    type->category = builder->category;
    type->serialize = builder->serialize;
    type->summon = builder->summon;
    type->fireImmune = builder->fireImmune;
    type->canSpawnFarFromPlayer = builder->canSpawnFarFromPlayer;
    type->immuneTo = builder->immuneTo;
    type->immuneToCount = builder->immuneToCount;
    type->dimensions = builder->dimensions;
    type->spawnDimensionsScale = builder->spawnDimensionsScale;
    type->clientTrackingRange = builder->clientTrackingRange;
    type->updateInterval = builder->updateInterval;
    type->allowedInPeaceful = builder->allowedInPeaceful;
    // Java: the id + description derive from the resource key
    type->id = strdup(id);
    {
        // Java: Util.makeDescriptionId("entity", identifier) - "entity.<ns>.<path>"
        LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(id);
        if (identifier != NULL)
        {
            const char *ns = LIBMATTI_MC_Identifier_GetNamespace(identifier);
            const char *path = LIBMATTI_MC_Identifier_GetPath(identifier);
            size_t len = strlen("entity.") + strlen(ns) + 1 + strlen(path) + 1;
            type->descriptionId = malloc(len);
            if (type->descriptionId != NULL)
                snprintf(type->descriptionId, len, "entity.%s.%s", ns, path);
            LIBMATTI_MC_Identifier_Free(identifier);
        }
    }
    free(builder);
    return type;
}

// ---------------------------------------------------------------------------
// EntityType surface
// ---------------------------------------------------------------------------

struct LIBMATTI_MC_Entity *LIBMATTI_MC_EntityType_Create(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level)
{
    if (type == NULL || type->factory == NULL)
        return NULL;
    return type->factory(type, level);
}

struct LIBMATTI_MC_Entity *LIBMATTI_MC_EntityType_CreateAt(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level,
                                                           const LIBMATTI_MC_BlockPos *pos)
{
    struct LIBMATTI_MC_Entity *entity = LIBMATTI_MC_EntityType_Create(type, level);
    if (entity == NULL)
        return NULL;
    // Java: moveTo(x, y, z, 0, 0) on the fresh entity
    LIBMATTI_MC_Entity_SetPos(entity, (double) pos->base.x + 0.5, (double) pos->base.y, (double) pos->base.z + 0.5);
    return entity;
}

LIBMATTI_MC_EntityDimensions *LIBMATTI_MC_EntityType_GetDimensions(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->dimensions : NULL;
}

float LIBMATTI_MC_EntityType_GetWidth(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->dimensions != NULL ? type->dimensions->width : 0.0f;
}

float LIBMATTI_MC_EntityType_GetHeight(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->dimensions != NULL ? type->dimensions->height : 0.0f;
}

LIBMATTI_MC_MobCategory LIBMATTI_MC_EntityType_GetCategory(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->category : LIBMATTI_MC_MobCategory_MISC;
}

bool LIBMATTI_MC_EntityType_CanSerialize(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->serialize;
}

bool LIBMATTI_MC_EntityType_CanSummon(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->summon;
}

bool LIBMATTI_MC_EntityType_IsAllowedInPeaceful(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->allowedInPeaceful;
}

bool LIBMATTI_MC_EntityType_FireImmune(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL && type->fireImmune;
}

int LIBMATTI_MC_EntityType_ClientTrackingRange(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->clientTrackingRange : 0;
}

int LIBMATTI_MC_EntityType_UpdateInterval(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->updateInterval : 0;
}

bool LIBMATTI_MC_EntityType_TrackDeltas(const LIBMATTI_MC_EntityType *type)
{
    // Java: trackDeltas - everything except a small known set syncs deltas; the
    // port names PLAYER/LLAMA_SPIT/WITHER/BAT/ITEM_FRAME/... by id
    if (type == NULL || type->id == NULL)
        return false;
    static const char *const NO_DELTAS[] = {
        "minecraft:player", "minecraft:llama_spit", "minecraft:wither", "minecraft:bat",
        "minecraft:item_frame", "minecraft:glow_item_frame", "minecraft:painting",
        "minecraft:item_display", "minecraft:interaction", "minecraft:evoker_fangs",
    };
    for (size_t i = 0; i < sizeof(NO_DELTAS) / sizeof(NO_DELTAS[0]); i++)
    {
        if (strcmp(type->id, NO_DELTAS[i]) == 0)
            return false;
    }
    return true;
}

const char *LIBMATTI_MC_EntityType_GetDescriptionId(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->descriptionId : NULL;
}

const char *LIBMATTI_MC_EntityType_GetId(const LIBMATTI_MC_EntityType *type)
{
    return type != NULL ? type->id : NULL;
}

void LIBMATTI_MC_EntityType_Free(LIBMATTI_MC_EntityType *type)
{
    if (type == NULL)
        return;
    LIBMATTI_MC_EntityDimensions_Free(type->dimensions);
    free(type->descriptionId);
    free(type->id);
    free(type);
}

// Java: private static <T> EntityType<T> register(String, Builder) - through
// BuiltInRegistries.ENTITY_TYPE (ResourceKey.create(Registries.ENTITY_TYPE, id))
LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_Register(const char *id, LIBMATTI_MC_EntityTypeBuilder *builder)
{
    LIBMATTI_MC_EntityType *type = LIBMATTI_MC_EntityTypeBuilder_Build(builder, id);
    if (type == NULL)
        return NULL;
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_ENTITY_TYPE();
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(id);
    if (identifier == NULL)
    {
        LIBMATTI_MC_EntityType_Free(type);
        return NULL;
    }
    LIBMATTI_MC_Registry_RegisterWithIdentifier(registry, identifier, type);
    LIBMATTI_MC_Identifier_Free(identifier);
    return type;
}

const LIBMATTI_MC_Identifier *LIBMATTI_MC_EntityType_GetKey(const LIBMATTI_MC_EntityType *type)
{
    if (type == NULL)
        return NULL;
    return LIBMATTI_MC_MappedRegistry_GetKey(LIBMATTI_MC_BuiltInRegistries_ENTITY_TYPE(), type);
}

LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ByString(const char *name)
{
    if (name == NULL)
        return NULL;
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_ENTITY_TYPE();
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(name);
    if (identifier == NULL)
        return NULL;
    LIBMATTI_MC_EntityType *type = (LIBMATTI_MC_EntityType *) LIBMATTI_MC_Registry_GetOptional(registry, identifier);
    LIBMATTI_MC_Identifier_Free(identifier);
    return type;
}
