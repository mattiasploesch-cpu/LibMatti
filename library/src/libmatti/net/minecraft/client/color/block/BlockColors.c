// Port of net.minecraft.client.color.block.BlockColors (implementation).
//
// Java: createDefault() registers the per-block color providers; the
// biome-dependent providers (grass/foliage/water through BiomeColors) fall
// back to the default color tables when the level is absent - the biome color
// engine itself is game-port content.

#include "libmatti/net/minecraft/client/color/block/BlockColors.h"

#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/util/ARGB.h"
#include "libmatti/net/minecraft/world/level/GrassColor.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"
#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/material/MapColor.h"

#include <stdlib.h>

// Java: private static final int DEFAULT = -1
#define DEFAULT -1

LIBMATTI_MC_BlockColors *LIBMATTI_MC_BlockColors_New(void)
{
    LIBMATTI_MC_BlockColors *colors = calloc(1, sizeof(LIBMATTI_MC_BlockColors));
    colors->blockColors = LIBMATTI_JL_IdMapper_New();
    return colors;
}

void LIBMATTI_MC_BlockColors_Free(LIBMATTI_MC_BlockColors *colors)
{
    if (colors == NULL)
        return;
    LIBMATTI_JL_IdMapper_Free(colors->blockColors);
    free(colors);
}

// Java: RedStoneWireBlock.getColorForPower(POWER) - the COLORS table formula
// (Util.make fills it once; the port computes the same value on demand).
static int redstone_color_for_power(int power)
{
    if (power < 0)
        power = 0;
    if (power > 15)
        power = 15;
    float f = power / 15.0f;
    float f1 = f * 0.6f + (f > 0.0f ? 0.4f : 0.3f);
    float f2 = f * f * 0.7f - 0.5f;
    if (f2 < 0.0f)
        f2 = 0.0f;
    if (f2 > 1.0f)
        f2 = 1.0f;
    float f3 = f * f * 0.6f - 0.7f;
    if (f3 < 0.0f)
        f3 = 0.0f;
    if (f3 > 1.0f)
        f3 = 1.0f;
    return LIBMATTI_MC_ARGB_ColorFromFloat(1.0f, f1, f2, f3);
}

// Java: p_92616_.getValue(RedStoneWireBlock.POWER) - the property read rides
// on the state (the provider lambda maps it through getColorForPower).
static int redstone_wire_color(const LIBMATTI_MC_BlockState *state,
                               struct LIBMATTI_MC_BlockAndTintGetter *level,
                               const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) level;
    (void) pos;
    (void) tintIndex;
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    const LIBMATTI_MC_Property *power = LIBMATTI_MC_StateDefinition_GetProperty(block->stateDefinition, "power");
    int value = 0;
    if (power != NULL)
        value = LIBMATTI_MC_BlockState_GetValue(state, power).index;
    return redstone_color_for_power(value);
}

// Java: the StemBlock provider - int i = state.getValue(AGE);
// ARGB.color(i * 32, 255 - i * 8, i * 4) (the 3-arg color is opaque).
static int stem_age_color(const LIBMATTI_MC_BlockState *state,
                          struct LIBMATTI_MC_BlockAndTintGetter *level,
                          const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) level;
    (void) pos;
    (void) tintIndex;
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    const LIBMATTI_MC_Property *age = LIBMATTI_MC_StateDefinition_GetProperty(block->stateDefinition, "age");
    int value = 0;
    if (age != NULL)
        value = LIBMATTI_MC_BlockState_GetValue(state, age).index;
    return LIBMATTI_MC_ARGB_Color(255, value * 32, 255 - value * 8, value * 4);
}

// Java: p_276241_ ... the PINK_PETALS/WILDFLOWERS provider - the biome grass
// color for tintIndex != 0, untinted for index 0.
static int pink_petals_color(const LIBMATTI_MC_BlockState *state,
                             struct LIBMATTI_MC_BlockAndTintGetter *level,
                             const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    if (tintIndex != 0)
        return level != NULL && pos != NULL ? LIBMATTI_MC_GrassColor_GetDefaultColor() : LIBMATTI_MC_GrassColor_GetDefaultColor();
    return DEFAULT;
}

// Java: the grass provider - BiomeColors.getAverageGrassColor when the level
// is present, the default table color otherwise (the biome color engine is
// game-port content; the port resolves the default).
static int grass_color(const LIBMATTI_MC_BlockState *state,
                       struct LIBMATTI_MC_BlockAndTintGetter *level,
                       const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) tintIndex;
    return level != NULL && pos != NULL ? LIBMATTI_MC_GrassColor_GetDefaultColor() : LIBMATTI_MC_GrassColor_GetDefaultColor();
}

// Java: the SPRUCE_LEAVES provider - constant -10380959
static int spruce_leaves_color(const LIBMATTI_MC_BlockState *state,
                               struct LIBMATTI_MC_BlockAndTintGetter *level,
                               const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) level;
    (void) pos;
    (void) tintIndex;
    return -10380959;
}

// Java: the BIRCH_LEAVES provider - constant -8345771
static int birch_leaves_color(const LIBMATTI_MC_BlockState *state,
                              struct LIBMATTI_MC_BlockAndTintGetter *level,
                              const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) level;
    (void) pos;
    (void) tintIndex;
    return -8345771;
}

// Java: the oak/jungle/acacia/dark_oak/vine/mangrove provider - the foliage
// table default when the level is absent (-12012264 is the vanilla fallback).
static int foliage_color(const LIBMATTI_MC_BlockState *state,
                         struct LIBMATTI_MC_BlockAndTintGetter *level,
                         const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) tintIndex;
    return level != NULL && pos != NULL ? LIBMATTI_MC_FoliageColor_GetDefaultColor() : -12012264;
}

// Java: the LEAF_LITTER provider - the dry foliage default.
static int dry_foliage_color(const LIBMATTI_MC_BlockState *state,
                             struct LIBMATTI_MC_BlockAndTintGetter *level,
                             const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) level;
    (void) pos;
    (void) tintIndex;
    return -10732494;
}

// Java: the WATER provider - the water table default (-1 before a biome).
static int water_color(const LIBMATTI_MC_BlockState *state,
                       struct LIBMATTI_MC_BlockAndTintGetter *level,
                       const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) level;
    (void) pos;
    (void) tintIndex;
    return DEFAULT;
}

// Java: the SUGAR_CANE provider - the grass provider's constant.
static int sugar_cane_color(const LIBMATTI_MC_BlockState *state,
                            struct LIBMATTI_MC_BlockAndTintGetter *level,
                            const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) level;
    (void) pos;
    (void) tintIndex;
    return DEFAULT;
}

// Java: the LILY_PAD provider - -14647248 in the world, -9321636 as the default.
static int lily_pad_color(const LIBMATTI_MC_BlockState *state,
                          struct LIBMATTI_MC_BlockAndTintGetter *level,
                          const LIBMATTI_MC_BlockPos *pos, int tintIndex)
{
    (void) state;
    (void) tintIndex;
    return level != NULL && pos != NULL ? -14647248 : -9321636;
}

// Java: register(BlockColor, Block...) - the helper over the vararg surface.
static void register_blocks(LIBMATTI_MC_BlockColors *colors, LIBMATTI_MC_BlockColor provider, const char *const *names, int count)
{
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_BLOCK();
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Block *block = LIBMATTI_MC_VanillaBlocks_GetByName(names[i]);
        if (block == NULL)
            continue;
        LIBMATTI_JL_IdMapper_AddMapping(colors->blockColors, (void *) provider,
                                        LIBMATTI_MC_MappedRegistry_GetId(registry, block));
    }
}

LIBMATTI_MC_BlockColors *LIBMATTI_MC_BlockColors_CreateDefault(void)
{
    LIBMATTI_MC_BlockColors *colors = LIBMATTI_MC_BlockColors_New();

    // Java: the LARGE_FERN/TALL_GRASS provider (upper half looks below)
    static const char *const tall_grass[] = {"LARGE_FERN", "TALL_GRASS"};
    register_blocks(colors, grass_color, tall_grass, 2);

    // Java: the GRASS_BLOCK/FERN/SHORT_GRASS/POTTED_FERN/BUSH provider
    static const char *const grass[] = {"GRASS_BLOCK", "FERN", "SHORT_GRASS", "POTTED_FERN", "BUSH"};
    register_blocks(colors, grass_color, grass, 5);

    // Java: the PINK_PETALS/WILDFLOWERS provider
    static const char *const petals[] = {"PINK_PETALS", "WILDFLOWERS"};
    register_blocks(colors, pink_petals_color, petals, 2);

    // Java: the leaves providers
    static const char *const spruce[] = {"SPRUCE_LEAVES"};
    register_blocks(colors, spruce_leaves_color, spruce, 1);
    static const char *const birch[] = {"BIRCH_LEAVES"};
    register_blocks(colors, birch_leaves_color, birch, 1);
    static const char *const foliage[] = {"OAK_LEAVES", "JUNGLE_LEAVES", "ACACIA_LEAVES", "DARK_OAK_LEAVES", "VINE", "MANGROVE_LEAVES"};
    register_blocks(colors, foliage_color, foliage, 6);
    static const char *const leaf_litter[] = {"LEAF_LITTER"};
    register_blocks(colors, dry_foliage_color, leaf_litter, 1);

    // Java: the WATER/BUBBLE_COLUMN/WATER_CAULDRON provider
    static const char *const water[] = {"WATER", "BUBBLE_COLUMN", "WATER_CAULDRON"};
    register_blocks(colors, water_color, water, 3);

    // Java: the REDSTONE_WIRE provider
    static const char *const redstone[] = {"REDSTONE_WIRE"};
    register_blocks(colors, redstone_wire_color, redstone, 1);

    // Java: the SUGAR_CANE provider
    static const char *const cane[] = {"SUGAR_CANE"};
    register_blocks(colors, sugar_cane_color, cane, 1);

    // Java: the stems - ARGB over the AGE property value
    static const char *const melon_stem[] = {"MELON_STEM", "PUMPKIN_STEM"};
    register_blocks(colors, stem_age_color, melon_stem, 2);

    // Java: the LILY_PAD provider
    static const char *const lily[] = {"LILY_PAD"};
    register_blocks(colors, lily_pad_color, lily, 1);

    return colors;
}

int LIBMATTI_MC_BlockColors_GetColor(LIBMATTI_MC_BlockColors *colors,
                                     const LIBMATTI_MC_BlockState *state,
                                     struct LIBMATTI_MC_Level *level,
                                     const LIBMATTI_MC_BlockPos *pos)
{
    LIBMATTI_MC_BlockColor color = LIBMATTI_JL_IdMapper_ById(
        colors->blockColors,
        LIBMATTI_MC_MappedRegistry_GetId(LIBMATTI_MC_BuiltInRegistries_BLOCK(),
                                         LIBMATTI_MC_BlockState_GetBlock(state)));
    if (color != NULL)
        return color(state, NULL, NULL, 0);
    // Java: the MapColor fallback
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    if (block != NULL && block->properties != NULL && block->properties->mapColor != NULL)
        return block->properties->mapColor->col;
    return DEFAULT;
}

int LIBMATTI_MC_BlockColors_GetColorIndexed(LIBMATTI_MC_BlockColors *colors,
                                            const LIBMATTI_MC_BlockState *state,
                                            struct LIBMATTI_MC_BlockAndTintGetter *level,
                                            const LIBMATTI_MC_BlockPos *pos,
                                            int tintIndex)
{
    LIBMATTI_MC_BlockColor color = LIBMATTI_JL_IdMapper_ById(
        colors->blockColors,
        LIBMATTI_MC_MappedRegistry_GetId(LIBMATTI_MC_BuiltInRegistries_BLOCK(),
                                         LIBMATTI_MC_BlockState_GetBlock(state)));
    return color == NULL ? DEFAULT : color(state, level, pos, tintIndex);
}

void LIBMATTI_MC_BlockColors_Register(LIBMATTI_MC_BlockColors *colors,
                                      LIBMATTI_MC_BlockColor provider,
                                      const struct LIBMATTI_MC_Block *const *blocks, int count)
{
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_BLOCK();
    for (int i = 0; i < count; i++)
        LIBMATTI_JL_IdMapper_AddMapping(colors->blockColors, (void *) provider,
                                        LIBMATTI_MC_MappedRegistry_GetId(registry, blocks[i]));
}
