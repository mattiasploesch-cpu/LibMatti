// Port of net.minecraft.world.level.block.entity.BlockEntityType.
// Generated from the vendor Java sources (BlockEntityType.java): one entry per
// register(...) with the exact valid-block set.

#include "libmatti/net/minecraft/world/level/block/entity/BlockEntityType.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_MC_BlockEntityType TYPES[49];
static int initialized;

// the BlockEntitySupplier side: every vanilla type creates the base BlockEntity
// (the typed data subclasses are game-port content)
static void *base_factory(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state);

static const void *validBlocks_FURNACE[1];

static void fill_valid_blocks_FURNACE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"FURNACE"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CHEST[9];

static void fill_valid_blocks_CHEST(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CHEST", "COPPER_CHEST", "EXPOSED_COPPER_CHEST", "WEATHERED_COPPER_CHEST", "OXIDIZED_COPPER_CHEST", "WAXED_COPPER_CHEST", "WAXED_EXPOSED_COPPER_CHEST", "WAXED_WEATHERED_COPPER_CHEST", "WAXED_OXIDIZED_COPPER_CHEST"};
    for (size_t i = 0; i < count && i < 9; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_TRAPPED_CHEST[1];

static void fill_valid_blocks_TRAPPED_CHEST(const void **out, size_t count)
{
    static const char *const NAMES[] = {"TRAPPED_CHEST"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_ENDER_CHEST[1];

static void fill_valid_blocks_ENDER_CHEST(const void **out, size_t count)
{
    static const char *const NAMES[] = {"ENDER_CHEST"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_JUKEBOX[1];

static void fill_valid_blocks_JUKEBOX(const void **out, size_t count)
{
    static const char *const NAMES[] = {"JUKEBOX"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_DISPENSER[1];

static void fill_valid_blocks_DISPENSER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"DISPENSER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_DROPPER[1];

static void fill_valid_blocks_DROPPER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"DROPPER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SIGN[24];

static void fill_valid_blocks_SIGN(const void **out, size_t count)
{
    static const char *const NAMES[] = {"OAK_SIGN", "SPRUCE_SIGN", "BIRCH_SIGN", "ACACIA_SIGN", "CHERRY_SIGN", "JUNGLE_SIGN", "DARK_OAK_SIGN", "PALE_OAK_SIGN", "OAK_WALL_SIGN", "SPRUCE_WALL_SIGN", "BIRCH_WALL_SIGN", "ACACIA_WALL_SIGN", "CHERRY_WALL_SIGN", "JUNGLE_WALL_SIGN", "DARK_OAK_WALL_SIGN", "PALE_OAK_WALL_SIGN", "CRIMSON_SIGN", "CRIMSON_WALL_SIGN", "WARPED_SIGN", "WARPED_WALL_SIGN", "MANGROVE_SIGN", "MANGROVE_WALL_SIGN", "BAMBOO_SIGN", "BAMBOO_WALL_SIGN"};
    for (size_t i = 0; i < count && i < 24; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_HANGING_SIGN[24];

static void fill_valid_blocks_HANGING_SIGN(const void **out, size_t count)
{
    static const char *const NAMES[] = {"OAK_HANGING_SIGN", "SPRUCE_HANGING_SIGN", "BIRCH_HANGING_SIGN", "ACACIA_HANGING_SIGN", "CHERRY_HANGING_SIGN", "JUNGLE_HANGING_SIGN", "DARK_OAK_HANGING_SIGN", "PALE_OAK_HANGING_SIGN", "CRIMSON_HANGING_SIGN", "WARPED_HANGING_SIGN", "MANGROVE_HANGING_SIGN", "BAMBOO_HANGING_SIGN", "OAK_WALL_HANGING_SIGN", "SPRUCE_WALL_HANGING_SIGN", "BIRCH_WALL_HANGING_SIGN", "ACACIA_WALL_HANGING_SIGN", "CHERRY_WALL_HANGING_SIGN", "JUNGLE_WALL_HANGING_SIGN", "DARK_OAK_WALL_HANGING_SIGN", "PALE_OAK_WALL_HANGING_SIGN", "CRIMSON_WALL_HANGING_SIGN", "WARPED_WALL_HANGING_SIGN", "MANGROVE_WALL_HANGING_SIGN", "BAMBOO_WALL_HANGING_SIGN"};
    for (size_t i = 0; i < count && i < 24; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_MOB_SPAWNER[1];

static void fill_valid_blocks_MOB_SPAWNER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SPAWNER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CREAKING_HEART[1];

static void fill_valid_blocks_CREAKING_HEART(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CREAKING_HEART"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_PISTON[1];

static void fill_valid_blocks_PISTON(const void **out, size_t count)
{
    static const char *const NAMES[] = {"MOVING_PISTON"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BREWING_STAND[1];

static void fill_valid_blocks_BREWING_STAND(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BREWING_STAND"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_ENCHANTING_TABLE[1];

static void fill_valid_blocks_ENCHANTING_TABLE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"ENCHANTING_TABLE"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_END_PORTAL[1];

static void fill_valid_blocks_END_PORTAL(const void **out, size_t count)
{
    static const char *const NAMES[] = {"END_PORTAL"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BEACON[1];

static void fill_valid_blocks_BEACON(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BEACON"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SKULL[14];

static void fill_valid_blocks_SKULL(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SKELETON_SKULL", "SKELETON_WALL_SKULL", "CREEPER_HEAD", "CREEPER_WALL_HEAD", "DRAGON_HEAD", "DRAGON_WALL_HEAD", "ZOMBIE_HEAD", "ZOMBIE_WALL_HEAD", "WITHER_SKELETON_SKULL", "WITHER_SKELETON_WALL_SKULL", "PLAYER_HEAD", "PLAYER_WALL_HEAD", "PIGLIN_HEAD", "PIGLIN_WALL_HEAD"};
    for (size_t i = 0; i < count && i < 14; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_DAYLIGHT_DETECTOR[1];

static void fill_valid_blocks_DAYLIGHT_DETECTOR(const void **out, size_t count)
{
    static const char *const NAMES[] = {"DAYLIGHT_DETECTOR"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_HOPPER[1];

static void fill_valid_blocks_HOPPER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"HOPPER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_COMPARATOR[1];

static void fill_valid_blocks_COMPARATOR(const void **out, size_t count)
{
    static const char *const NAMES[] = {"COMPARATOR"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BANNER[32];

static void fill_valid_blocks_BANNER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"WHITE_BANNER", "ORANGE_BANNER", "MAGENTA_BANNER", "LIGHT_BLUE_BANNER", "YELLOW_BANNER", "LIME_BANNER", "PINK_BANNER", "GRAY_BANNER", "LIGHT_GRAY_BANNER", "CYAN_BANNER", "PURPLE_BANNER", "BLUE_BANNER", "BROWN_BANNER", "GREEN_BANNER", "RED_BANNER", "BLACK_BANNER", "WHITE_WALL_BANNER", "ORANGE_WALL_BANNER", "MAGENTA_WALL_BANNER", "LIGHT_BLUE_WALL_BANNER", "YELLOW_WALL_BANNER", "LIME_WALL_BANNER", "PINK_WALL_BANNER", "GRAY_WALL_BANNER", "LIGHT_GRAY_WALL_BANNER", "CYAN_WALL_BANNER", "PURPLE_WALL_BANNER", "BLUE_WALL_BANNER", "BROWN_WALL_BANNER", "GREEN_WALL_BANNER", "RED_WALL_BANNER", "BLACK_WALL_BANNER"};
    for (size_t i = 0; i < count && i < 32; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_STRUCTURE_BLOCK[1];

static void fill_valid_blocks_STRUCTURE_BLOCK(const void **out, size_t count)
{
    static const char *const NAMES[] = {"STRUCTURE_BLOCK"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_END_GATEWAY[1];

static void fill_valid_blocks_END_GATEWAY(const void **out, size_t count)
{
    static const char *const NAMES[] = {"END_GATEWAY"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_COMMAND_BLOCK[3];

static void fill_valid_blocks_COMMAND_BLOCK(const void **out, size_t count)
{
    static const char *const NAMES[] = {"COMMAND_BLOCK", "CHAIN_COMMAND_BLOCK", "REPEATING_COMMAND_BLOCK"};
    for (size_t i = 0; i < count && i < 3; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SHULKER_BOX[17];

static void fill_valid_blocks_SHULKER_BOX(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SHULKER_BOX", "BLACK_SHULKER_BOX", "BLUE_SHULKER_BOX", "BROWN_SHULKER_BOX", "CYAN_SHULKER_BOX", "GRAY_SHULKER_BOX", "GREEN_SHULKER_BOX", "LIGHT_BLUE_SHULKER_BOX", "LIGHT_GRAY_SHULKER_BOX", "LIME_SHULKER_BOX", "MAGENTA_SHULKER_BOX", "ORANGE_SHULKER_BOX", "PINK_SHULKER_BOX", "PURPLE_SHULKER_BOX", "RED_SHULKER_BOX", "WHITE_SHULKER_BOX", "YELLOW_SHULKER_BOX"};
    for (size_t i = 0; i < count && i < 17; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BED[16];

static void fill_valid_blocks_BED(const void **out, size_t count)
{
    static const char *const NAMES[] = {"RED_BED", "BLACK_BED", "BLUE_BED", "BROWN_BED", "CYAN_BED", "GRAY_BED", "GREEN_BED", "LIGHT_BLUE_BED", "LIGHT_GRAY_BED", "LIME_BED", "MAGENTA_BED", "ORANGE_BED", "PINK_BED", "PURPLE_BED", "WHITE_BED", "YELLOW_BED"};
    for (size_t i = 0; i < count && i < 16; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CONDUIT[1];

static void fill_valid_blocks_CONDUIT(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CONDUIT"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BARREL[1];

static void fill_valid_blocks_BARREL(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BARREL"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SMOKER[1];

static void fill_valid_blocks_SMOKER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SMOKER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BLAST_FURNACE[1];

static void fill_valid_blocks_BLAST_FURNACE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BLAST_FURNACE"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_LECTERN[1];

static void fill_valid_blocks_LECTERN(const void **out, size_t count)
{
    static const char *const NAMES[] = {"LECTERN"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BELL[1];

static void fill_valid_blocks_BELL(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BELL"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_JIGSAW[1];

static void fill_valid_blocks_JIGSAW(const void **out, size_t count)
{
    static const char *const NAMES[] = {"JIGSAW"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CAMPFIRE[2];

static void fill_valid_blocks_CAMPFIRE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CAMPFIRE", "SOUL_CAMPFIRE"};
    for (size_t i = 0; i < count && i < 2; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BEEHIVE[2];

static void fill_valid_blocks_BEEHIVE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"BEE_NEST", "BEEHIVE"};
    for (size_t i = 0; i < count && i < 2; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SCULK_SENSOR[1];

static void fill_valid_blocks_SCULK_SENSOR(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SCULK_SENSOR"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CALIBRATED_SCULK_SENSOR[1];

static void fill_valid_blocks_CALIBRATED_SCULK_SENSOR(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CALIBRATED_SCULK_SENSOR"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SCULK_CATALYST[1];

static void fill_valid_blocks_SCULK_CATALYST(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SCULK_CATALYST"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SCULK_SHRIEKER[1];

static void fill_valid_blocks_SCULK_SHRIEKER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SCULK_SHRIEKER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CHISELED_BOOKSHELF[1];

static void fill_valid_blocks_CHISELED_BOOKSHELF(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CHISELED_BOOKSHELF"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_SHELF[12];

static void fill_valid_blocks_SHELF(const void **out, size_t count)
{
    static const char *const NAMES[] = {"ACACIA_SHELF", "BAMBOO_SHELF", "BIRCH_SHELF", "CHERRY_SHELF", "CRIMSON_SHELF", "DARK_OAK_SHELF", "JUNGLE_SHELF", "MANGROVE_SHELF", "OAK_SHELF", "PALE_OAK_SHELF", "SPRUCE_SHELF", "WARPED_SHELF"};
    for (size_t i = 0; i < count && i < 12; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_BRUSHABLE_BLOCK[2];

static void fill_valid_blocks_BRUSHABLE_BLOCK(const void **out, size_t count)
{
    static const char *const NAMES[] = {"SUSPICIOUS_SAND", "SUSPICIOUS_GRAVEL"};
    for (size_t i = 0; i < count && i < 2; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_DECORATED_POT[1];

static void fill_valid_blocks_DECORATED_POT(const void **out, size_t count)
{
    static const char *const NAMES[] = {"DECORATED_POT"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_CRAFTER[1];

static void fill_valid_blocks_CRAFTER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"CRAFTER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_TRIAL_SPAWNER[1];

static void fill_valid_blocks_TRIAL_SPAWNER(const void **out, size_t count)
{
    static const char *const NAMES[] = {"TRIAL_SPAWNER"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_VAULT[1];

static void fill_valid_blocks_VAULT(const void **out, size_t count)
{
    static const char *const NAMES[] = {"VAULT"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_TEST_BLOCK[1];

static void fill_valid_blocks_TEST_BLOCK(const void **out, size_t count)
{
    static const char *const NAMES[] = {"TEST_BLOCK"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_TEST_INSTANCE_BLOCK[1];

static void fill_valid_blocks_TEST_INSTANCE_BLOCK(const void **out, size_t count)
{
    static const char *const NAMES[] = {"TEST_INSTANCE_BLOCK"};
    for (size_t i = 0; i < count && i < 1; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static const void *validBlocks_COPPER_GOLEM_STATUE[8];

static void fill_valid_blocks_COPPER_GOLEM_STATUE(const void **out, size_t count)
{
    static const char *const NAMES[] = {"COPPER_GOLEM_STATUE", "EXPOSED_COPPER_GOLEM_STATUE", "WEATHERED_COPPER_GOLEM_STATUE", "OXIDIZED_COPPER_GOLEM_STATUE", "WAXED_COPPER_GOLEM_STATUE", "WAXED_EXPOSED_COPPER_GOLEM_STATUE", "WAXED_WEATHERED_COPPER_GOLEM_STATUE", "WAXED_OXIDIZED_COPPER_GOLEM_STATUE"};
    for (size_t i = 0; i < count && i < 8; i++)
        out[i] = (const void *) LIBMATTI_MC_VanillaBlocks_GetByName(NAMES[i]);
}

static void initialize(void)
{
    if (initialized)
        return;
    initialized = 1;
    fill_valid_blocks_FURNACE(validBlocks_FURNACE, 1);
    TYPES[0].id = "furnace";
    TYPES[0].factory = base_factory;
    TYPES[0].validBlocks = validBlocks_FURNACE;
    TYPES[0].validBlockCount = 1;
    fill_valid_blocks_CHEST(validBlocks_CHEST, 9);
    TYPES[1].id = "chest";
    TYPES[1].factory = base_factory;
    TYPES[1].validBlocks = validBlocks_CHEST;
    TYPES[1].validBlockCount = 9;
    fill_valid_blocks_TRAPPED_CHEST(validBlocks_TRAPPED_CHEST, 1);
    TYPES[2].id = "trapped_chest";
    TYPES[2].factory = base_factory;
    TYPES[2].validBlocks = validBlocks_TRAPPED_CHEST;
    TYPES[2].validBlockCount = 1;
    fill_valid_blocks_ENDER_CHEST(validBlocks_ENDER_CHEST, 1);
    TYPES[3].id = "ender_chest";
    TYPES[3].factory = base_factory;
    TYPES[3].validBlocks = validBlocks_ENDER_CHEST;
    TYPES[3].validBlockCount = 1;
    fill_valid_blocks_JUKEBOX(validBlocks_JUKEBOX, 1);
    TYPES[4].id = "jukebox";
    TYPES[4].factory = base_factory;
    TYPES[4].validBlocks = validBlocks_JUKEBOX;
    TYPES[4].validBlockCount = 1;
    fill_valid_blocks_DISPENSER(validBlocks_DISPENSER, 1);
    TYPES[5].id = "dispenser";
    TYPES[5].factory = base_factory;
    TYPES[5].validBlocks = validBlocks_DISPENSER;
    TYPES[5].validBlockCount = 1;
    fill_valid_blocks_DROPPER(validBlocks_DROPPER, 1);
    TYPES[6].id = "dropper";
    TYPES[6].factory = base_factory;
    TYPES[6].validBlocks = validBlocks_DROPPER;
    TYPES[6].validBlockCount = 1;
    fill_valid_blocks_SIGN(validBlocks_SIGN, 24);
    TYPES[7].id = "sign";
    TYPES[7].factory = base_factory;
    TYPES[7].validBlocks = validBlocks_SIGN;
    TYPES[7].validBlockCount = 24;
    fill_valid_blocks_HANGING_SIGN(validBlocks_HANGING_SIGN, 24);
    TYPES[8].id = "hanging_sign";
    TYPES[8].factory = base_factory;
    TYPES[8].validBlocks = validBlocks_HANGING_SIGN;
    TYPES[8].validBlockCount = 24;
    fill_valid_blocks_MOB_SPAWNER(validBlocks_MOB_SPAWNER, 1);
    TYPES[9].id = "mob_spawner";
    TYPES[9].factory = base_factory;
    TYPES[9].validBlocks = validBlocks_MOB_SPAWNER;
    TYPES[9].validBlockCount = 1;
    fill_valid_blocks_CREAKING_HEART(validBlocks_CREAKING_HEART, 1);
    TYPES[10].id = "creaking_heart";
    TYPES[10].factory = base_factory;
    TYPES[10].validBlocks = validBlocks_CREAKING_HEART;
    TYPES[10].validBlockCount = 1;
    fill_valid_blocks_PISTON(validBlocks_PISTON, 1);
    TYPES[11].id = "piston";
    TYPES[11].factory = base_factory;
    TYPES[11].validBlocks = validBlocks_PISTON;
    TYPES[11].validBlockCount = 1;
    fill_valid_blocks_BREWING_STAND(validBlocks_BREWING_STAND, 1);
    TYPES[12].id = "brewing_stand";
    TYPES[12].factory = base_factory;
    TYPES[12].validBlocks = validBlocks_BREWING_STAND;
    TYPES[12].validBlockCount = 1;
    fill_valid_blocks_ENCHANTING_TABLE(validBlocks_ENCHANTING_TABLE, 1);
    TYPES[13].id = "enchanting_table";
    TYPES[13].factory = base_factory;
    TYPES[13].validBlocks = validBlocks_ENCHANTING_TABLE;
    TYPES[13].validBlockCount = 1;
    fill_valid_blocks_END_PORTAL(validBlocks_END_PORTAL, 1);
    TYPES[14].id = "end_portal";
    TYPES[14].factory = base_factory;
    TYPES[14].validBlocks = validBlocks_END_PORTAL;
    TYPES[14].validBlockCount = 1;
    fill_valid_blocks_BEACON(validBlocks_BEACON, 1);
    TYPES[15].id = "beacon";
    TYPES[15].factory = base_factory;
    TYPES[15].validBlocks = validBlocks_BEACON;
    TYPES[15].validBlockCount = 1;
    fill_valid_blocks_SKULL(validBlocks_SKULL, 14);
    TYPES[16].id = "skull";
    TYPES[16].factory = base_factory;
    TYPES[16].validBlocks = validBlocks_SKULL;
    TYPES[16].validBlockCount = 14;
    fill_valid_blocks_DAYLIGHT_DETECTOR(validBlocks_DAYLIGHT_DETECTOR, 1);
    TYPES[17].id = "daylight_detector";
    TYPES[17].factory = base_factory;
    TYPES[17].validBlocks = validBlocks_DAYLIGHT_DETECTOR;
    TYPES[17].validBlockCount = 1;
    fill_valid_blocks_HOPPER(validBlocks_HOPPER, 1);
    TYPES[18].id = "hopper";
    TYPES[18].factory = base_factory;
    TYPES[18].validBlocks = validBlocks_HOPPER;
    TYPES[18].validBlockCount = 1;
    fill_valid_blocks_COMPARATOR(validBlocks_COMPARATOR, 1);
    TYPES[19].id = "comparator";
    TYPES[19].factory = base_factory;
    TYPES[19].validBlocks = validBlocks_COMPARATOR;
    TYPES[19].validBlockCount = 1;
    fill_valid_blocks_BANNER(validBlocks_BANNER, 32);
    TYPES[20].id = "banner";
    TYPES[20].factory = base_factory;
    TYPES[20].validBlocks = validBlocks_BANNER;
    TYPES[20].validBlockCount = 32;
    fill_valid_blocks_STRUCTURE_BLOCK(validBlocks_STRUCTURE_BLOCK, 1);
    TYPES[21].id = "structure_block";
    TYPES[21].factory = base_factory;
    TYPES[21].validBlocks = validBlocks_STRUCTURE_BLOCK;
    TYPES[21].validBlockCount = 1;
    fill_valid_blocks_END_GATEWAY(validBlocks_END_GATEWAY, 1);
    TYPES[22].id = "end_gateway";
    TYPES[22].factory = base_factory;
    TYPES[22].validBlocks = validBlocks_END_GATEWAY;
    TYPES[22].validBlockCount = 1;
    fill_valid_blocks_COMMAND_BLOCK(validBlocks_COMMAND_BLOCK, 3);
    TYPES[23].id = "command_block";
    TYPES[23].factory = base_factory;
    TYPES[23].validBlocks = validBlocks_COMMAND_BLOCK;
    TYPES[23].validBlockCount = 3;
    fill_valid_blocks_SHULKER_BOX(validBlocks_SHULKER_BOX, 17);
    TYPES[24].id = "shulker_box";
    TYPES[24].factory = base_factory;
    TYPES[24].validBlocks = validBlocks_SHULKER_BOX;
    TYPES[24].validBlockCount = 17;
    fill_valid_blocks_BED(validBlocks_BED, 16);
    TYPES[25].id = "bed";
    TYPES[25].factory = base_factory;
    TYPES[25].validBlocks = validBlocks_BED;
    TYPES[25].validBlockCount = 16;
    fill_valid_blocks_CONDUIT(validBlocks_CONDUIT, 1);
    TYPES[26].id = "conduit";
    TYPES[26].factory = base_factory;
    TYPES[26].validBlocks = validBlocks_CONDUIT;
    TYPES[26].validBlockCount = 1;
    fill_valid_blocks_BARREL(validBlocks_BARREL, 1);
    TYPES[27].id = "barrel";
    TYPES[27].factory = base_factory;
    TYPES[27].validBlocks = validBlocks_BARREL;
    TYPES[27].validBlockCount = 1;
    fill_valid_blocks_SMOKER(validBlocks_SMOKER, 1);
    TYPES[28].id = "smoker";
    TYPES[28].factory = base_factory;
    TYPES[28].validBlocks = validBlocks_SMOKER;
    TYPES[28].validBlockCount = 1;
    fill_valid_blocks_BLAST_FURNACE(validBlocks_BLAST_FURNACE, 1);
    TYPES[29].id = "blast_furnace";
    TYPES[29].factory = base_factory;
    TYPES[29].validBlocks = validBlocks_BLAST_FURNACE;
    TYPES[29].validBlockCount = 1;
    fill_valid_blocks_LECTERN(validBlocks_LECTERN, 1);
    TYPES[30].id = "lectern";
    TYPES[30].factory = base_factory;
    TYPES[30].validBlocks = validBlocks_LECTERN;
    TYPES[30].validBlockCount = 1;
    fill_valid_blocks_BELL(validBlocks_BELL, 1);
    TYPES[31].id = "bell";
    TYPES[31].factory = base_factory;
    TYPES[31].validBlocks = validBlocks_BELL;
    TYPES[31].validBlockCount = 1;
    fill_valid_blocks_JIGSAW(validBlocks_JIGSAW, 1);
    TYPES[32].id = "jigsaw";
    TYPES[32].factory = base_factory;
    TYPES[32].validBlocks = validBlocks_JIGSAW;
    TYPES[32].validBlockCount = 1;
    fill_valid_blocks_CAMPFIRE(validBlocks_CAMPFIRE, 2);
    TYPES[33].id = "campfire";
    TYPES[33].factory = base_factory;
    TYPES[33].validBlocks = validBlocks_CAMPFIRE;
    TYPES[33].validBlockCount = 2;
    fill_valid_blocks_BEEHIVE(validBlocks_BEEHIVE, 2);
    TYPES[34].id = "beehive";
    TYPES[34].factory = base_factory;
    TYPES[34].validBlocks = validBlocks_BEEHIVE;
    TYPES[34].validBlockCount = 2;
    fill_valid_blocks_SCULK_SENSOR(validBlocks_SCULK_SENSOR, 1);
    TYPES[35].id = "sculk_sensor";
    TYPES[35].factory = base_factory;
    TYPES[35].validBlocks = validBlocks_SCULK_SENSOR;
    TYPES[35].validBlockCount = 1;
    fill_valid_blocks_CALIBRATED_SCULK_SENSOR(validBlocks_CALIBRATED_SCULK_SENSOR, 1);
    TYPES[36].id = "calibrated_sculk_sensor";
    TYPES[36].factory = base_factory;
    TYPES[36].validBlocks = validBlocks_CALIBRATED_SCULK_SENSOR;
    TYPES[36].validBlockCount = 1;
    fill_valid_blocks_SCULK_CATALYST(validBlocks_SCULK_CATALYST, 1);
    TYPES[37].id = "sculk_catalyst";
    TYPES[37].factory = base_factory;
    TYPES[37].validBlocks = validBlocks_SCULK_CATALYST;
    TYPES[37].validBlockCount = 1;
    fill_valid_blocks_SCULK_SHRIEKER(validBlocks_SCULK_SHRIEKER, 1);
    TYPES[38].id = "sculk_shrieker";
    TYPES[38].factory = base_factory;
    TYPES[38].validBlocks = validBlocks_SCULK_SHRIEKER;
    TYPES[38].validBlockCount = 1;
    fill_valid_blocks_CHISELED_BOOKSHELF(validBlocks_CHISELED_BOOKSHELF, 1);
    TYPES[39].id = "chiseled_bookshelf";
    TYPES[39].factory = base_factory;
    TYPES[39].validBlocks = validBlocks_CHISELED_BOOKSHELF;
    TYPES[39].validBlockCount = 1;
    fill_valid_blocks_SHELF(validBlocks_SHELF, 12);
    TYPES[40].id = "shelf";
    TYPES[40].factory = base_factory;
    TYPES[40].validBlocks = validBlocks_SHELF;
    TYPES[40].validBlockCount = 12;
    fill_valid_blocks_BRUSHABLE_BLOCK(validBlocks_BRUSHABLE_BLOCK, 2);
    TYPES[41].id = "brushable_block";
    TYPES[41].factory = base_factory;
    TYPES[41].validBlocks = validBlocks_BRUSHABLE_BLOCK;
    TYPES[41].validBlockCount = 2;
    fill_valid_blocks_DECORATED_POT(validBlocks_DECORATED_POT, 1);
    TYPES[42].id = "decorated_pot";
    TYPES[42].factory = base_factory;
    TYPES[42].validBlocks = validBlocks_DECORATED_POT;
    TYPES[42].validBlockCount = 1;
    fill_valid_blocks_CRAFTER(validBlocks_CRAFTER, 1);
    TYPES[43].id = "crafter";
    TYPES[43].factory = base_factory;
    TYPES[43].validBlocks = validBlocks_CRAFTER;
    TYPES[43].validBlockCount = 1;
    fill_valid_blocks_TRIAL_SPAWNER(validBlocks_TRIAL_SPAWNER, 1);
    TYPES[44].id = "trial_spawner";
    TYPES[44].factory = base_factory;
    TYPES[44].validBlocks = validBlocks_TRIAL_SPAWNER;
    TYPES[44].validBlockCount = 1;
    fill_valid_blocks_VAULT(validBlocks_VAULT, 1);
    TYPES[45].id = "vault";
    TYPES[45].factory = base_factory;
    TYPES[45].validBlocks = validBlocks_VAULT;
    TYPES[45].validBlockCount = 1;
    fill_valid_blocks_TEST_BLOCK(validBlocks_TEST_BLOCK, 1);
    TYPES[46].id = "test_block";
    TYPES[46].factory = base_factory;
    TYPES[46].validBlocks = validBlocks_TEST_BLOCK;
    TYPES[46].validBlockCount = 1;
    fill_valid_blocks_TEST_INSTANCE_BLOCK(validBlocks_TEST_INSTANCE_BLOCK, 1);
    TYPES[47].id = "test_instance_block";
    TYPES[47].factory = base_factory;
    TYPES[47].validBlocks = validBlocks_TEST_INSTANCE_BLOCK;
    TYPES[47].validBlockCount = 1;
    fill_valid_blocks_COPPER_GOLEM_STATUE(validBlocks_COPPER_GOLEM_STATUE, 8);
    TYPES[48].id = "copper_golem_statue";
    TYPES[48].factory = base_factory;
    TYPES[48].validBlocks = validBlocks_COPPER_GOLEM_STATUE;
    TYPES[48].validBlockCount = 8;
}

bool LIBMATTI_MC_BlockEntityType_IsValid(const LIBMATTI_MC_BlockEntityType *type, LIBMATTI_MC_BlockState *state)
{
    // Java: this.validBlocks.contains(state.getBlock())
    const void *block = LIBMATTI_MC_BlockState_GetBlock(state);
    for (size_t i = 0; i < type->validBlockCount; i++)
    {
        if (type->validBlocks[i] == block)
            return true;
    }
    return false;
}

void *LIBMATTI_MC_BlockEntityType_Create(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state)
{
    return type->factory(type, pos, state);
}

const char *LIBMATTI_MC_BlockEntityType_GetId(const LIBMATTI_MC_BlockEntityType *type)
{
    return type->id;
}

static void *base_factory(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state)
{
    return LIBMATTI_MC_BlockEntity_New(type, pos, state);
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_FURNACE(void)
{
    initialize();
    return &TYPES[0];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CHEST(void)
{
    initialize();
    return &TYPES[1];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TRAPPED_CHEST(void)
{
    initialize();
    return &TYPES[2];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_ENDER_CHEST(void)
{
    initialize();
    return &TYPES[3];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_JUKEBOX(void)
{
    initialize();
    return &TYPES[4];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DISPENSER(void)
{
    initialize();
    return &TYPES[5];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DROPPER(void)
{
    initialize();
    return &TYPES[6];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SIGN(void)
{
    initialize();
    return &TYPES[7];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_HANGING_SIGN(void)
{
    initialize();
    return &TYPES[8];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_MOB_SPAWNER(void)
{
    initialize();
    return &TYPES[9];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CREAKING_HEART(void)
{
    initialize();
    return &TYPES[10];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_PISTON(void)
{
    initialize();
    return &TYPES[11];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BREWING_STAND(void)
{
    initialize();
    return &TYPES[12];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_ENCHANTING_TABLE(void)
{
    initialize();
    return &TYPES[13];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_END_PORTAL(void)
{
    initialize();
    return &TYPES[14];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BEACON(void)
{
    initialize();
    return &TYPES[15];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SKULL(void)
{
    initialize();
    return &TYPES[16];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DAYLIGHT_DETECTOR(void)
{
    initialize();
    return &TYPES[17];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_HOPPER(void)
{
    initialize();
    return &TYPES[18];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COMPARATOR(void)
{
    initialize();
    return &TYPES[19];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BANNER(void)
{
    initialize();
    return &TYPES[20];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_STRUCTURE_BLOCK(void)
{
    initialize();
    return &TYPES[21];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_END_GATEWAY(void)
{
    initialize();
    return &TYPES[22];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COMMAND_BLOCK(void)
{
    initialize();
    return &TYPES[23];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SHULKER_BOX(void)
{
    initialize();
    return &TYPES[24];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BED(void)
{
    initialize();
    return &TYPES[25];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CONDUIT(void)
{
    initialize();
    return &TYPES[26];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BARREL(void)
{
    initialize();
    return &TYPES[27];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SMOKER(void)
{
    initialize();
    return &TYPES[28];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BLAST_FURNACE(void)
{
    initialize();
    return &TYPES[29];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_LECTERN(void)
{
    initialize();
    return &TYPES[30];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BELL(void)
{
    initialize();
    return &TYPES[31];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_JIGSAW(void)
{
    initialize();
    return &TYPES[32];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CAMPFIRE(void)
{
    initialize();
    return &TYPES[33];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BEEHIVE(void)
{
    initialize();
    return &TYPES[34];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_SENSOR(void)
{
    initialize();
    return &TYPES[35];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CALIBRATED_SCULK_SENSOR(void)
{
    initialize();
    return &TYPES[36];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_CATALYST(void)
{
    initialize();
    return &TYPES[37];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_SHRIEKER(void)
{
    initialize();
    return &TYPES[38];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CHISELED_BOOKSHELF(void)
{
    initialize();
    return &TYPES[39];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SHELF(void)
{
    initialize();
    return &TYPES[40];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BRUSHABLE_BLOCK(void)
{
    initialize();
    return &TYPES[41];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DECORATED_POT(void)
{
    initialize();
    return &TYPES[42];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CRAFTER(void)
{
    initialize();
    return &TYPES[43];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TRIAL_SPAWNER(void)
{
    initialize();
    return &TYPES[44];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_VAULT(void)
{
    initialize();
    return &TYPES[45];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TEST_BLOCK(void)
{
    initialize();
    return &TYPES[46];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TEST_INSTANCE_BLOCK(void)
{
    initialize();
    return &TYPES[47];
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COPPER_GOLEM_STATUE(void)
{
    initialize();
    return &TYPES[48];
}

// the by-index access LevelChunk uses for the hasBlockEntity/create scan over all
// vanilla types (Java: instanceof EntityBlock on the block)
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_AccessIndex(int index)
{
    if (index < 0 || index >= 49)
        return NULL;
    initialize();
    return &TYPES[index];
}
