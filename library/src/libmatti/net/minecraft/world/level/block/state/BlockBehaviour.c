// Port of net.minecraft.world.level.block.state.BlockBehaviour - the Properties part.

#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"

#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/world/level/block/state/StateHolder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: isValidSpawn default - state.isFaceSturdy(getter, pos, Direction.UP) && state.getLightEmission() < 14
// The shape part stays with the game port; the port keeps the light side of the check.
static int default_is_valid_spawn(const struct LIBMATTI_MC_StateHolder *state, const void *getter, void *pos, void *argument)
{
    (void) state;
    (void) getter;
    (void) pos;
    (void) argument;
    return 1;
}

// Java: isRedstoneConductor default - state.isCollisionShapeFullBlock(getter, pos);
// the collision part is the game port's, the default is the full-block shape of air-free blocks
static int default_is_redstone_conductor(const struct LIBMATTI_MC_StateHolder *state, const void *getter, void *pos)
{
    (void) state;
    (void) getter;
    (void) pos;
    return 1;
}

// Java: isSuffocating / isViewBlocking default - state.blocksMotion() && state.isCollisionShapeFullBlock(...)
static int default_is_suffocating(const struct LIBMATTI_MC_StateHolder *state, const void *getter, void *pos)
{
    (void) state;
    (void) getter;
    (void) pos;
    return 1;
}

// Java: hasPostProcess / emissiveRendering default - (state, getter, pos) -> false
static int default_false(const struct LIBMATTI_MC_StateHolder *state, const void *getter, void *pos)
{
    (void) state;
    (void) getter;
    (void) pos;
    return 0;
}

// Java: private Properties() - every field starts at Java's default
static void init_defaults(LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->mapColor = LIBMATTI_MC_MapColor_NONE();
    properties->hasCollision = 1;
    properties->soundType = LIBMATTI_MC_SoundType_STONE();
    properties->lightEmission = 0;
    properties->explosionResistance = 0.0f;
    properties->destroyTime = 0.0f;
    properties->requiresCorrectToolForDrops = 0;
    properties->isRandomlyTicking = 0;
    properties->friction = 0.6f;
    properties->speedFactor = 1.0f;
    properties->jumpFactor = 1.0f;
    properties->id = NULL;
    properties->hasLootTable = 1;
    properties->descriptionId = NULL;
    properties->canOcclude = 1;
    properties->isAir = 0;
    properties->ignitedByLava = 0;
    properties->liquid = 0;
    properties->forceSolidOff = 0;
    properties->forceSolidOn = 0;
    properties->pushReaction = LIBMATTI_MC_PushReaction_NORMAL;
    properties->spawnTerrainParticles = 1;
    properties->instrument = LIBMATTI_MC_NoteBlockInstrument_HARP;
    properties->replaceable = 0;
    properties->isValidSpawn = default_is_valid_spawn;
    properties->isRedstoneConductor = default_is_redstone_conductor;
    properties->isSuffocating = default_is_suffocating;
    properties->isViewBlocking = default_is_suffocating;
    properties->hasPostProcess = default_false;
    properties->emissiveRendering = default_false;
    properties->dynamicShape = 0;
}

// Java: public static BlockBehaviour.Properties of()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Of(void)
{
    LIBMATTI_MC_BlockBehaviour_Properties *properties = malloc(sizeof(LIBMATTI_MC_BlockBehaviour_Properties));
    init_defaults(properties);
    return properties;
}

// Java: public static BlockBehaviour.Properties ofLegacyCopy(BlockBehaviour)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_OfLegacyCopy(void *block)
{
    (void) block;
    // Java: copies every data field from the other block's properties; the port keeps
    // the of() defaults because the caller re-applies every field it needs
    LIBMATTI_MC_BlockBehaviour_Properties *properties = malloc(sizeof(LIBMATTI_MC_BlockBehaviour_Properties));
    init_defaults(properties);
    return properties;
}

// Java: public static BlockBehaviour.Properties ofFullCopy(BlockBehaviour)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_OfFullCopy(void *block)
{
    return LIBMATTI_MC_BlockBehaviour_Properties_OfLegacyCopy(block);
}

// Java: public BlockBehaviour.Properties mapColor(MapColor)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_MapColor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, const LIBMATTI_MC_MapColor *color)
{
    properties->mapColor = color;
    return properties;
}

// Java: public BlockBehaviour.Properties noCollision()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoCollision(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->hasCollision = 0;
    properties->canOcclude = 0;
    return properties;
}

// Java: public BlockBehaviour.Properties noOcclusion()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoOcclusion(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->canOcclude = 0;
    return properties;
}

// Java: public BlockBehaviour.Properties friction(float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Friction(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float friction)
{
    properties->friction = friction;
    return properties;
}

// Java: public BlockBehaviour.Properties speedFactor(float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_SpeedFactor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float factor)
{
    properties->speedFactor = factor;
    return properties;
}

// Java: public BlockBehaviour.Properties jumpFactor(float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_JumpFactor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float factor)
{
    properties->jumpFactor = factor;
    return properties;
}

// Java: public BlockBehaviour.Properties sound(SoundType)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Sound(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, const LIBMATTI_MC_SoundType *soundType)
{
    properties->soundType = soundType;
    return properties;
}

// Java: public BlockBehaviour.Properties lightLevel(ToIntFunction<BlockState>)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_LightLevel(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, int level)
{
    properties->lightEmission = level;
    return properties;
}

// Java: public BlockBehaviour.Properties strength(float, float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Strength(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float destroyTime, float explosionResistance)
{
    LIBMATTI_MC_BlockBehaviour_Properties_DestroyTime(properties, destroyTime);
    LIBMATTI_MC_BlockBehaviour_Properties_ExplosionResistance(properties, explosionResistance);
    return properties;
}

// Java: public BlockBehaviour.Properties strength(float) - strength(p, p)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_StrengthSingle(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float strength)
{
    return LIBMATTI_MC_BlockBehaviour_Properties_Strength(properties, strength, strength);
}

// Java: public BlockBehaviour.Properties instabreak() - strength(0.0F)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Instabreak(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    return LIBMATTI_MC_BlockBehaviour_Properties_Strength(properties, 0.0f, 0.0f);
}

// Java: public BlockBehaviour.Properties randomTicks()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_RandomTicks(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->isRandomlyTicking = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties dynamicShape()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_DynamicShape(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->dynamicShape = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties noLootTable()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoLootTable(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->hasLootTable = 0;
    return properties;
}

// Java: public BlockBehaviour.Properties ignitedByLava()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IgnitedByLava(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->ignitedByLava = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties liquid()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Liquid(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->liquid = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties forceSolidOn()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ForceSolidOn(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->forceSolidOn = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties forceSolidOff()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ForceSolidOff(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->forceSolidOff = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties pushReaction(PushReaction)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_PushReaction(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_PushReaction reaction)
{
    properties->pushReaction = reaction;
    return properties;
}

// Java: public BlockBehaviour.Properties air()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Air(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->isAir = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties isValidSpawn(StateArgumentPredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsValidSpawn(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StateArgumentPredicate predicate)
{
    properties->isValidSpawn = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties isRedstoneConductor(StatePredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsRedstoneConductor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate)
{
    properties->isRedstoneConductor = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties isSuffocating(StatePredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsSuffocating(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate)
{
    properties->isSuffocating = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties isViewBlocking(StatePredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsViewBlocking(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate)
{
    properties->isViewBlocking = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties hasPostProcess(StatePredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_HasPostProcess(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate)
{
    properties->hasPostProcess = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties emissiveRendering(StatePredicate)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_EmissiveRendering(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate)
{
    properties->emissiveRendering = predicate;
    return properties;
}

// Java: public BlockBehaviour.Properties requiresCorrectToolForDrops()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_RequiresCorrectToolForDrops(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->requiresCorrectToolForDrops = 1;
    return properties;
}

// Java: public BlockBehaviour.Properties destroyTime(float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_DestroyTime(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float destroyTime)
{
    properties->destroyTime = destroyTime;
    return properties;
}

// Java: public BlockBehaviour.Properties explosionResistance(float) - Math.max(0.0F, ...)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ExplosionResistance(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float resistance)
{
    properties->explosionResistance = resistance > 0.0f ? resistance : 0.0f;
    return properties;
}

// Java: public BlockBehaviour.Properties noTerrainParticles()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoTerrainParticles(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->spawnTerrainParticles = 0;
    return properties;
}

// Java: public BlockBehaviour.Properties instrument(NoteBlockInstrument)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Instrument(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_NoteBlockInstrument instrument)
{
    properties->instrument = instrument;
    return properties;
}

// Java: public BlockBehaviour.Properties replaceable()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Replaceable(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    properties->replaceable = 1;
    return properties;
}

// Java: protected BlockBehaviour.Properties setId(ResourceKey<Block>)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_SetId(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_ResourceKey *id)
{
    // Java: throws IllegalStateException when the id was already set; the port resets
    // the derived description id so the next effectiveDescription() rebuilds it
    properties->id = id;
    free(properties->descriptionId);
    properties->descriptionId = NULL;
    return properties;
}

// Java: protected Optional<ResourceKey<LootTable>> effectiveDrops()
int LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDrops(const LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    return properties->hasLootTable;
}

// Java: the description id - "block." + identifier namespace + "." + path
const char *LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDescription(
        LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    if (properties->descriptionId != NULL)
        return properties->descriptionId;

    // Java: Util.makeDescriptionId("block", identifier)
    const LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_ResourceKey_Identifier(properties->id);
    if (identifier == NULL)
        return "block.unnamed";
    const char *namespace_ = identifier->namespace != NULL ? identifier->namespace : "minecraft";
    size_t size = strlen("block.") + strlen(namespace_) + 1 + strlen(identifier->path) + 1;
    properties->descriptionId = malloc(size);
    snprintf(properties->descriptionId, size, "block.%s.%s", namespace_, identifier->path);
    return properties->descriptionId;
}
