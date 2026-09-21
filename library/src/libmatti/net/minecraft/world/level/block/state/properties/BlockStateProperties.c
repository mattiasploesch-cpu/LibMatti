// Port of net.minecraft.world.level.block.state.properties.BlockStateProperties.

#include "libmatti/net/minecraft/world/level/block/state/properties/BlockStateProperties.h"

#include <stdlib.h>
#include <string.h>

// Java: the properties are created once in the static initialiser; the port caches
// them in file-scope statics. Every accessor returns the same pointer on repeat calls.
// Java: BooleanProperty.create("attached") - the property name is the lower-case
// form of the constant; the port lowercases the stringified constant at creation
static char *lowercase_name(const char *upper)
{
    char *copy = strdup(upper);
    for (char *p = copy; *p != '\0'; p++)
        if (*p >= 'A' && *p <= 'Z')
            *p = (char) (*p - 'A' + 'a');
    return copy;
}

#define BOOL(name)                                                         \
    static LIBMATTI_MC_Property *p_##name;                                 \
    const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_##name(void) \
    {                                                                      \
        if (p_##name == NULL)                                              \
            p_##name = LIBMATTI_MC_BooleanProperty_Create(lowercase_name(#name)); \
        return p_##name;                                                   \
    }

#define INT(name, min, max)                                                \
    static LIBMATTI_MC_Property *p_##name;                                 \
    const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_##name(void) \
    {                                                                      \
        if (p_##name == NULL)                                              \
            p_##name = LIBMATTI_MC_IntegerProperty_Create(lowercase_name(#name), min, max); \
        return p_##name;                                                   \
    }

#define ENUM(name, ... )                                                   \
    static const char *const v_##name[] = {__VA_ARGS__};                   \
    static LIBMATTI_MC_Property *p_##name;                                 \
    const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_##name(void) \
    {                                                                      \
        if (p_##name == NULL)                                              \
            p_##name = LIBMATTI_MC_EnumProperty_Create(                    \
                    lowercase_name(#name), v_##name,                       \
                    (int) (sizeof(v_##name) / sizeof(v_##name[0])));       \
        return p_##name;                                                   \
    }

#define ENUM_NAME(name, prop_name, ... )                                   \
    static const char *const v_##name[] = {__VA_ARGS__};                   \
    static LIBMATTI_MC_Property *p_##name;                                 \
    const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_##name(void) \
    {                                                                      \
        if (p_##name == NULL)                                              \
            p_##name = LIBMATTI_MC_EnumProperty_Create(                    \
                    prop_name, v_##name,                                   \
                    (int) (sizeof(v_##name) / sizeof(v_##name[0])));       \
        return p_##name;                                                   \
    }

// ---------------------------------------------------------------- booleans
BOOL(ATTACHED)
BOOL(BERRIES)
BOOL(BLOOM)
BOOL(BOTTOM)
BOOL(CAN_SUMMON)
BOOL(CONDITIONAL)
BOOL(DISARMED)
BOOL(DRAG)
BOOL(ENABLED)
BOOL(EXTENDED)
BOOL(EYE)
BOOL(FALLING)
BOOL(HANGING)
BOOL(HAS_BOTTLE_0)
BOOL(HAS_BOTTLE_1)
BOOL(HAS_BOTTLE_2)
BOOL(HAS_RECORD)
BOOL(HAS_BOOK)
BOOL(INVERTED)
BOOL(IN_WALL)
BOOL(LIT)
BOOL(LOCKED)
BOOL(NATURAL)
BOOL(OCCUPIED)
BOOL(OPEN)
BOOL(PERSISTENT)
BOOL(POWERED)
BOOL(SHORT)
BOOL(SHRIEKING)
BOOL(SIGNAL_FIRE)
BOOL(SNOWY)
BOOL(TIP)
BOOL(TRIGGERED)
BOOL(UNSTABLE)
BOOL(WATERLOGGED)
// Java: UP / DOWN / NORTH / EAST / SOUTH / WEST
BOOL(UP)
BOOL(DOWN)
BOOL(NORTH)
BOOL(EAST)
BOOL(SOUTH)
BOOL(WEST)

// ------------------------------------------------------------------- enums
// Java: Direction.Axis serial names
ENUM_NAME(HORIZONTAL_AXIS, "axis", "x", "z")
ENUM_NAME(AXIS, "axis", "x", "y", "z")
ENUM_NAME(FACING, "facing", "north", "east", "south", "west", "up", "down")
// Java: FACING_HOPPER - every direction except up
ENUM_NAME(FACING_HOPPER, "facing", "down", "north", "east", "south", "west")
// Java: HORIZONTAL_FACING - Plane.HORIZONTAL
ENUM_NAME(HORIZONTAL_FACING, "facing", "north", "east", "south", "west")
// Java: FrontAndTop
ENUM(ORIENTATION, "down_east", "down_north", "down_south", "down_west", "up_east", "up_north",
     "up_south", "up_west", "west_up", "east_up", "north_up", "south_up")
// Java: AttachFace
ENUM(ATTACH_FACE, "floor", "wall", "ceiling")
// Java: BellAttachType
ENUM(BELL_ATTACHMENT, "floor", "ceiling", "single_wall", "double_wall")
// Java: WallSide
ENUM(EAST_WALL, "none", "low", "tall")
ENUM(NORTH_WALL, "none", "low", "tall")
ENUM(SOUTH_WALL, "none", "low", "tall")
ENUM(WEST_WALL, "none", "low", "tall")
// Java: RedstoneSide
ENUM_NAME(EAST_REDSTONE, "east", "up", "side", "none")
ENUM_NAME(NORTH_REDSTONE, "north", "up", "side", "none")
ENUM_NAME(SOUTH_REDSTONE, "south", "up", "side", "none")
ENUM_NAME(WEST_REDSTONE, "west", "up", "side", "none")
// Java: DoubleBlockHalf (UPPER, LOWER - no explicit names, so the lowercase enum names)
ENUM_NAME(DOUBLE_BLOCK_HALF, "half", "upper", "lower")
// Java: Half
ENUM(HALF, "top", "bottom")
// Java: RailShape
ENUM(RAIL_SHAPE, "north_south", "east_west", "ascending_east", "ascending_west", "ascending_north",
     "ascending_south", "south_east", "south_west", "north_west", "north_east")
// Java: RAIL_SHAPE_STRAIGHT - no curves
ENUM_NAME(RAIL_SHAPE_STRAIGHT, "shape", "north_south", "east_west", "ascending_east",
          "ascending_west", "ascending_north", "ascending_south")
// Java: BedPart
ENUM(BED_PART, "head", "foot")
// Java: ChestType
ENUM(CHEST_TYPE, "single", "left", "right")
// Java: ComparatorMode
ENUM(MODE_COMPARATOR, "compare", "subtract")
// Java: DoorHingeSide
ENUM(DOOR_HINGE, "left", "right")
// Java: NoteBlockInstrument
ENUM(NOTEBLOCK_INSTRUMENT,
     "harp", "basedrum", "snare", "hat", "bass", "flute", "bell", "guitar", "chime", "xylophone",
     "iron_xylophone", "cow_bell", "didgeridoo", "bit", "banjo", "pling", "zombie", "skeleton",
     "creeper", "dragon", "wither_skeleton", "piglin", "custom_head")
// Java: PistonType
ENUM_NAME(PISTON_TYPE, "type", "normal", "sticky")
// Java: SlabType
ENUM_NAME(SLAB_TYPE, "type", "top", "bottom", "double")
// Java: StairsShape
ENUM(STAIRS_SHAPE, "straight", "inner_left", "inner_right", "outer_left", "outer_right")
// Java: StructureMode
ENUM(STRUCTUREBLOCK_MODE, "save", "load", "corner", "data")
// Java: BambooLeaves
ENUM(BAMBOO_LEAVES, "none", "small", "large")
// Java: Tilt
ENUM(TILT, "none", "unstable", "partial", "full")
// Java: VERTICAL_DIRECTION - Direction.UP, Direction.DOWN
ENUM_NAME(VERTICAL_DIRECTION, "vertical_direction", "up", "down")
// Java: DripstoneThickness
ENUM(DRIPSTONE_THICKNESS, "tip_merge", "tip", "frustum", "middle", "base")
// Java: SculkSensorPhase
ENUM(SCULK_SENSOR_PHASE, "inactive", "active", "cooldown")
// Java: TrialSpawnerState
ENUM(TRIAL_SPAWNER_STATE, "inactive", "waiting_for_players", "active", "waiting_for_reward_ejection",
     "ejecting_reward", "cooldown")
// Java: VaultState
ENUM(VAULT_STATE, "inactive", "active", "unlocking", "ejecting")
// Java: CreakingHeartState
ENUM(CREAKING_HEART_STATE, "uprooted", "dormant", "awake")
// Java: TestBlockMode
ENUM(TEST_BLOCK_MODE, "start", "log", "fail", "accept")
// Java: CopperGolemStatueBlock.Pose
ENUM(COPPER_GOLEM_POSE, "standing", "sitting", "running", "star")

// ---------------------------------------------------------------- integers
INT(FLOWER_AMOUNT, 1, 4)
INT(SEGMENT_AMOUNT, 1, 4)
INT(AGE_1, 0, 1)
INT(AGE_2, 0, 2)
INT(AGE_3, 0, 3)
INT(AGE_4, 0, 4)
INT(AGE_5, 0, 5)
INT(AGE_7, 0, 7)
INT(AGE_15, 0, 15)
INT(AGE_25, 0, 25)
INT(BITES, 0, 6)
INT(CANDLES, 1, 4)
INT(DELAY, 1, 4)
INT(DISTANCE, 1, 7)
INT(EGGS, 1, 4)
INT(HATCH, 0, 2)
INT(LAYERS, 1, 8)
INT(LEVEL_CAULDRON, 1, 3)
INT(LEVEL_COMPOSTER, 0, 8)
INT(LEVEL_FLOWING, 1, 8)
INT(LEVEL_HONEY, 0, 5)
INT(LEVEL, 0, 15)
INT(MOISTURE, 0, 7)
INT(NOTE, 0, 24)
INT(PICKLES, 1, 4)
INT(POWER, 0, 15)
INT(STAGE, 0, 1)
INT(STABILITY_DISTANCE, 0, 7)
INT(RESPAWN_ANCHOR_CHARGES, 0, 4)
INT(DRIED_GHAST_HYDRATION_LEVELS, 0, 3)
INT(ROTATION_16, 0, 15)
INT(DUSTED, 0, 3)

// ---------------------------------------------------------------- booleans (II)
static LIBMATTI_MC_Property *p_SLOT_0_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_0_OCCUPIED(void)
{
    if (p_SLOT_0_OCCUPIED_BOOL == NULL)
        p_SLOT_0_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_0_occupied");
    return p_SLOT_0_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_SLOT_1_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_1_OCCUPIED(void)
{
    if (p_SLOT_1_OCCUPIED_BOOL == NULL)
        p_SLOT_1_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_1_occupied");
    return p_SLOT_1_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_SLOT_2_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_2_OCCUPIED(void)
{
    if (p_SLOT_2_OCCUPIED_BOOL == NULL)
        p_SLOT_2_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_2_occupied");
    return p_SLOT_2_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_SLOT_3_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_3_OCCUPIED(void)
{
    if (p_SLOT_3_OCCUPIED_BOOL == NULL)
        p_SLOT_3_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_3_occupied");
    return p_SLOT_3_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_SLOT_4_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_4_OCCUPIED(void)
{
    if (p_SLOT_4_OCCUPIED_BOOL == NULL)
        p_SLOT_4_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_4_occupied");
    return p_SLOT_4_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_SLOT_5_OCCUPIED_BOOL;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_SLOT_5_OCCUPIED(void)
{
    if (p_SLOT_5_OCCUPIED_BOOL == NULL)
        p_SLOT_5_OCCUPIED_BOOL = LIBMATTI_MC_BooleanProperty_Create("slot_5_occupied");
    return p_SLOT_5_OCCUPIED_BOOL;
}
static LIBMATTI_MC_Property *p_CRACKED;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CRACKED(void)
{
    if (p_CRACKED == NULL)
        p_CRACKED = LIBMATTI_MC_BooleanProperty_Create("cracked");
    return p_CRACKED;
}
static LIBMATTI_MC_Property *p_CRAFTING;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_CRAFTING(void)
{
    if (p_CRAFTING == NULL)
        p_CRAFTING = LIBMATTI_MC_BooleanProperty_Create("crafting");
    return p_CRAFTING;
}
static LIBMATTI_MC_Property *p_OMINOUS;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_OMINOUS(void)
{
    if (p_OMINOUS == NULL)
        p_OMINOUS = LIBMATTI_MC_BooleanProperty_Create("ominous");
    return p_OMINOUS;
}
static LIBMATTI_MC_Property *p_MAP;
const LIBMATTI_MC_Property *LIBMATTI_MC_BlockStateProperties_MAP(void)
{
    if (p_MAP == NULL)
        p_MAP = LIBMATTI_MC_BooleanProperty_Create("map");
    return p_MAP;
}
