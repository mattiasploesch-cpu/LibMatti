// Port of net.minecraft.world.level.block.state.properties.BlockStateProperties.
// Every Java static final field becomes one accessor; the properties are created
// lazily on first call and stay alive for the process (Java: static init).
// The Java enum generics collapse to serialized-name lists, exactly like the
// EnumProperty.create(name, Class) calls they replace.

#ifndef MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATEPROPERTIES_H
#define MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATEPROPERTIES_H

#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final BooleanProperty ATTACHED = BooleanProperty.create("attached")
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_ATTACHED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BERRIES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BLOOM(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BOTTOM(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CAN_SUMMON(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CONDITIONAL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DISARMED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DRAG(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_ENABLED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EXTENDED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EYE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_FALLING(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HANGING(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HAS_BOTTLE_0(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HAS_BOTTLE_1(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HAS_BOTTLE_2(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HAS_RECORD(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HAS_BOOK(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_INVERTED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_IN_WALL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LIT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LOCKED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NATURAL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_OPEN(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_PERSISTENT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_POWERED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SHORT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SHRIEKING(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SIGNAL_FIRE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SNOWY(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_TIP(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_TRIGGERED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_UNSTABLE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_WATERLOGGED(void);
// Java: HORIZONTAL_AXIS = EnumProperty.create("axis", Axis.X, Axis.Z)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HORIZONTAL_AXIS(void);
// Java: AXIS = EnumProperty.create("axis", Direction.Axis.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AXIS(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_UP(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DOWN(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NORTH(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EAST(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SOUTH(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_WEST(void);
// Java: FACING = EnumProperty.create("facing", north, east, south, west, up, down)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_FACING(void);
// Java: FACING_HOPPER = EnumProperty.create("facing", f -> f != Direction.UP)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_FACING_HOPPER(void);
// Java: HORIZONTAL_FACING = EnumProperty.create("facing", Plane.HORIZONTAL)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HORIZONTAL_FACING(void);
// Java: FLOWER_AMOUNT / SEGMENT_AMOUNT = IntegerProperty.create(..., 1, 4)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_FLOWER_AMOUNT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SEGMENT_AMOUNT(void);
// Java: ORIENTATION = EnumProperty.create("orientation", FrontAndTop.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_ORIENTATION(void);
// Java: ATTACH_FACE = EnumProperty.create("face", AttachFace.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_ATTACH_FACE(void);
// Java: BELL_ATTACHMENT = EnumProperty.create("attachment", BellAttachType.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BELL_ATTACHMENT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EAST_WALL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NORTH_WALL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SOUTH_WALL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_WEST_WALL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EAST_REDSTONE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NORTH_REDSTONE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SOUTH_REDSTONE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_WEST_REDSTONE(void);
// Java: DOUBLE_BLOCK_HALF = EnumProperty.create("half", DoubleBlockHalf.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DOUBLE_BLOCK_HALF(void);
// Java: HALF = EnumProperty.create("half", Half.class)
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HALF(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_RAIL_SHAPE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_RAIL_SHAPE_STRAIGHT(void);
// Java: MAX_AGE_* constants
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_1 1
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_2 2
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_3 3
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_4 4
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_5 5
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_7 7
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_15 15
#define LIBMATTI_MC_BlockStateProperties_MAX_AGE_25 25
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_1(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_2(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_3(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_4(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_5(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_7(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_15(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_AGE_25(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BITES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CANDLES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DELAY(void);
#define LIBMATTI_MC_BlockStateProperties_MAX_DISTANCE 7
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DISTANCE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_EGGS(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_HATCH(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LAYERS(void);
#define LIBMATTI_MC_BlockStateProperties_MIN_LEVEL 0
#define LIBMATTI_MC_BlockStateProperties_MIN_LEVEL_CAULDRON 1
#define LIBMATTI_MC_BlockStateProperties_MAX_LEVEL_3 3
#define LIBMATTI_MC_BlockStateProperties_MAX_LEVEL_8 8
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LEVEL_CAULDRON(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LEVEL_COMPOSTER(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LEVEL_FLOWING(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LEVEL_HONEY(void);
#define LIBMATTI_MC_BlockStateProperties_MAX_LEVEL_15 15
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_LEVEL(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_MOISTURE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NOTE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_PICKLES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_POWER(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_STAGE(void);
#define LIBMATTI_MC_BlockStateProperties_STABILITY_MAX_DISTANCE 7
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_STABILITY_DISTANCE(void);
#define LIBMATTI_MC_BlockStateProperties_MIN_RESPAWN_ANCHOR_CHARGES 0
#define LIBMATTI_MC_BlockStateProperties_MAX_RESPAWN_ANCHOR_CHARGES 4
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_RESPAWN_ANCHOR_CHARGES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DRIED_GHAST_HYDRATION_LEVELS(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_ROTATION_16(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BED_PART(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CHEST_TYPE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_MODE_COMPARATOR(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DOOR_HINGE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_NOTEBLOCK_INSTRUMENT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_PISTON_TYPE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLAB_TYPE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_STAIRS_SHAPE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_STRUCTUREBLOCK_MODE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_BAMBOO_LEAVES(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_TILT(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_VERTICAL_DIRECTION(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DRIPSTONE_THICKNESS(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SCULK_SENSOR_PHASE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_0_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_1_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_2_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_3_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_4_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_5_OCCUPIED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_DUSTED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CRACKED(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CRAFTING(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_TRIAL_SPAWNER_STATE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_VAULT_STATE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CREAKING_HEART_STATE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_OMINOUS(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_TEST_BLOCK_MODE(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_MAP(void);
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_COPPER_GOLEM_POSE(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATEPROPERTIES_H
