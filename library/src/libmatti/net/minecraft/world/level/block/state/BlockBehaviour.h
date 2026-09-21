// Port of net.minecraft.world.level.block.state.BlockBehaviour - the Properties part
// (the StateArgumentPredicate/StatePredicate functional interfaces and every
// Blocks.java-facing builder method with Java's exact defaults).
// The behaviour dispatch (getShape, tick, ...) stays with the game port; the port
// carries the data model that the registry objects and BlockStates need.

#ifndef MATTICRAFT_MC_BLOCKSTATE_BLOCKBEHAVIOUR_H
#define MATTICRAFT_MC_BLOCKSTATE_BLOCKBEHAVIOUR_H

#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/level/block/SoundType.h"
#include "libmatti/net/minecraft/world/level/block/state/properties/NoteBlockInstrument.h"
#include "libmatti/net/minecraft/world/level/material/MapColor.h"
#include "libmatti/net/minecraft/world/level/material/PushReaction.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_StateHolder;

// Java: BlockBehaviour.StatePredicate - (state, getter, pos) -> bool
typedef int (*LIBMATTI_MC_BlockBehaviour_StatePredicate)(const struct LIBMATTI_MC_StateHolder *state,
                                                         const void *getter, void *pos);
// Java: BlockBehaviour.StateArgumentPredicate<T> - (state, getter, pos, argument)
typedef int (*LIBMATTI_MC_BlockBehaviour_StateArgumentPredicate)(const struct LIBMATTI_MC_StateHolder *state,
                                                                 const void *getter, void *pos, void *argument);

// Java: public static class BlockBehaviour.Properties
typedef struct LIBMATTI_MC_BlockBehaviour_Properties
{
    // Java: Function<BlockState, MapColor> mapColor = state -> MapColor.NONE;
    const LIBMATTI_MC_MapColor *mapColor;
    // Java: boolean hasCollision = true
    int hasCollision;
    // Java: SoundType soundType = SoundType.STONE
    const LIBMATTI_MC_SoundType *soundType;
    // Java: ToIntFunction<BlockState> lightEmission = state -> 0
    int lightEmission;
    // Java: float explosionResistance / destroyTime
    float explosionResistance;
    float destroyTime;
    // Java: boolean requiresCorrectToolForDrops
    int requiresCorrectToolForDrops;
    // Java: boolean isRandomlyTicking
    int isRandomlyTicking;
    // Java: float friction = 0.6F / speedFactor = 1.0F / jumpFactor = 1.0F
    float friction;
    float speedFactor;
    float jumpFactor;
    // Java: private @Nullable ResourceKey<Block> id - set through setId
    LIBMATTI_MC_ResourceKey *id;
    // Java: the loot table key derived from the id (blocks/<path>) or noLootTable
    int hasLootTable;
    // Java: private DependantName<Block, String> descriptionId - "block.<ns>.<path>"
    char *descriptionId;
    // Java: boolean canOcclude = true
    int canOcclude;
    // Java: boolean isAir / ignitedByLava / liquid / forceSolidOff / forceSolidOn
    int isAir;
    int ignitedByLava;
    int liquid;
    int forceSolidOff;
    int forceSolidOn;
    // Java: PushReaction pushReaction = PushReaction.NORMAL
    LIBMATTI_MC_PushReaction pushReaction;
    // Java: boolean spawnTerrainParticles = true
    int spawnTerrainParticles;
    // Java: NoteBlockInstrument instrument = NoteBlockInstrument.HARP
    LIBMATTI_MC_NoteBlockInstrument instrument;
    // Java: boolean replaceable
    int replaceable;
    // Java: BlockBehaviour.StateArgumentPredicate<EntityType<?>> isValidSpawn = (state, getter, pos, type) ->
    //       state.isFaceSturdy(getter, pos, Direction.UP) && state.getLightEmission() < 14
    LIBMATTI_MC_BlockBehaviour_StateArgumentPredicate isValidSpawn;
    // Java: BlockBehaviour.StatePredicate isRedstoneConductor = (state, getter, pos) ->
    //       state.isCollisionShapeFullBlock(getter, pos)
    LIBMATTI_MC_BlockBehaviour_StatePredicate isRedstoneConductor;
    // Java: BlockBehaviour.StatePredicate isSuffocating / isViewBlocking = (state, getter, pos) ->
    //       state.blocksMotion() && state.isCollisionShapeFullBlock(getter, pos)
    LIBMATTI_MC_BlockBehaviour_StatePredicate isSuffocating;
    LIBMATTI_MC_BlockBehaviour_StatePredicate isViewBlocking;
    // Java: BlockBehaviour.StatePredicate hasPostProcess = (state, getter, pos) -> false
    LIBMATTI_MC_BlockBehaviour_StatePredicate hasPostProcess;
    // Java: BlockBehaviour.StatePredicate emissiveRendering = (state, getter, pos) -> false
    LIBMATTI_MC_BlockBehaviour_StatePredicate emissiveRendering;
    // Java: boolean dynamicShape
    int dynamicShape;
} LIBMATTI_MC_BlockBehaviour_Properties;

// Java: private Properties() + public static Properties of()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Of(void);
// Java: public static Properties ofLegacyCopy(BlockBehaviour)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_OfLegacyCopy(void *block);
// Java: public static Properties ofFullCopy(BlockBehaviour)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_OfFullCopy(void *block);

// Java: public Properties mapColor(MapColor)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_MapColor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, const LIBMATTI_MC_MapColor *color);
// Java: public Properties noCollision() - hasCollision = false, canOcclude = false
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoCollision(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties noOcclusion()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoOcclusion(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties friction(float) / speedFactor(float) / jumpFactor(float)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Friction(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float friction);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_SpeedFactor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float factor);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_JumpFactor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float factor);
// Java: public Properties sound(SoundType)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Sound(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, const LIBMATTI_MC_SoundType *soundType);
// Java: public Properties lightLevel(ToIntFunction<BlockState>) - the port takes the value
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_LightLevel(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, int level);
// Java: public Properties strength(float, float) / strength(float) / instabreak()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Strength(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float destroyTime, float explosionResistance);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_StrengthSingle(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float strength);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Instabreak(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties randomTicks() / dynamicShape()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_RandomTicks(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_DynamicShape(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties noLootTable()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoLootTable(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties ignitedByLava() / liquid() / forceSolidOn() / forceSolidOff()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IgnitedByLava(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Liquid(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ForceSolidOn(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ForceSolidOff(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties pushReaction(PushReaction)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_PushReaction(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_PushReaction reaction);
// Java: public Properties air()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Air(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: the four predicate setters
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsValidSpawn(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StateArgumentPredicate predicate);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsRedstoneConductor(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsSuffocating(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_IsViewBlocking(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_HasPostProcess(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_EmissiveRendering(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_BlockBehaviour_StatePredicate predicate);
// Java: public Properties requiresCorrectToolForDrops()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_RequiresCorrectToolForDrops(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties destroyTime(float) / explosionResistance(float) - max(0, ...)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_DestroyTime(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float destroyTime);
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_ExplosionResistance(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, float resistance);
// Java: public Properties noTerrainParticles()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_NoTerrainParticles(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: public Properties instrument(NoteBlockInstrument)
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Instrument(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_NoteBlockInstrument instrument);
// Java: public Properties replaceable()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_Replaceable(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: protected Properties setId(ResourceKey<Block>) - every register() call goes through here
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_BlockBehaviour_Properties_SetId(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_ResourceKey *id);
// Java: protected Optional<ResourceKey<LootTable>> effectiveDrops() - the port reports
// whether the block has a loot table (blocks/<path> derived or none)
int LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDrops(const LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: protected String effectiveDescription() - "block.<ns>.<path>" from the id
const char *LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDescription(
        LIBMATTI_MC_BlockBehaviour_Properties *properties);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_BLOCKBEHAVIOUR_H
