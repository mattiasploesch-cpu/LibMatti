// Port of net.minecraft.world.level.block.SoundType.
// Java stores one SoundEvent per action (break/step/place/hit/fall); the port keeps the
// SoundEvents constant names and resolves them lazily through the SoundEvents port.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCK_SOUNDTYPE_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCK_SOUNDTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class SoundType
typedef struct LIBMATTI_MC_SoundType
{
    // Java: public final float volume / pitch
    float volume;
    float pitch;
    // Java: private final SoundEvent breakSound, stepSound, placeSound, hitSound, fallSound
    const char *breakSound;
    const char *stepSound;
    const char *placeSound;
    const char *hitSound;
    const char *fallSound;
} LIBMATTI_MC_SoundType;

// Java: public float getVolume() / getPitch()
float LIBMATTI_MC_SoundType_GetVolume(const LIBMATTI_MC_SoundType *type);
float LIBMATTI_MC_SoundType_GetPitch(const LIBMATTI_MC_SoundType *type);
// Java: public SoundEvent getBreakSound() / getStepSound() / getPlaceSound() / getHitSound() / getFallSound()
const char *LIBMATTI_MC_SoundType_GetBreakSound(const LIBMATTI_MC_SoundType *type);
const char *LIBMATTI_MC_SoundType_GetStepSound(const LIBMATTI_MC_SoundType *type);
const char *LIBMATTI_MC_SoundType_GetPlaceSound(const LIBMATTI_MC_SoundType *type);
const char *LIBMATTI_MC_SoundType_GetHitSound(const LIBMATTI_MC_SoundType *type);
const char *LIBMATTI_MC_SoundType_GetFallSound(const LIBMATTI_MC_SoundType *type);

// One accessor per Java constant, in declaration order.
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_EMPTY(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WOOD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_GRAVEL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_GRASS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LILY_PAD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_STONE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_METAL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_GLASS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WOOL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SAND(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SNOW(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_POWDER_SNOW(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LADDER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_ANVIL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SLIME_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_HONEY_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WET_GRASS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CORAL_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BAMBOO(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BAMBOO_SAPLING(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCAFFOLDING(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SWEET_BERRY_BUSH(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CROP(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_HARD_CROP(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_VINE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_WART(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LANTERN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_STEM(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NYLIUM(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_FUNGUS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_ROOTS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SHROOMLIGHT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WEEPING_VINES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_TWISTING_VINES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SOUL_SAND(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SOUL_SOIL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BASALT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WART_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHERRACK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_BRICKS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_SPROUTS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_ORE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BONE_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHERITE_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_ANCIENT_DEBRIS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LODESTONE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHAIN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_GOLD_ORE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_GILDED_BLACKSTONE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CANDLE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_AMETHYST(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_AMETHYST_CLUSTER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SMALL_AMETHYST_BUD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MEDIUM_AMETHYST_BUD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LARGE_AMETHYST_BUD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_TUFF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_TUFF_BRICKS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_POLISHED_TUFF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CALCITE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DRIPSTONE_BLOCK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_POINTED_DRIPSTONE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_COPPER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_COPPER_BULB(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_COPPER_GRATE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_COPPER_GOLEM_STATUE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CAVE_VINES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SPORE_BLOSSOM(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CACTUS_FLOWER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_AZALEA(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_FLOWERING_AZALEA(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MOSS_CARPET(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_PINK_PETALS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_LEAF_LITTER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MOSS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BIG_DRIPLEAF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SMALL_DRIPLEAF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_ROOTED_DIRT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_HANGING_ROOTS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_AZALEA_LEAVES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCULK_SENSOR(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCULK_CATALYST(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCULK(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCULK_VEIN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SCULK_SHRIEKER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_GLOW_LICHEN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DEEPSLATE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DEEPSLATE_BRICKS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DEEPSLATE_TILES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_POLISHED_DEEPSLATE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_FROGLIGHT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_FROGSPAWN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MANGROVE_ROOTS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MUDDY_MANGROVE_ROOTS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MUD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_MUD_BRICKS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_PACKED_MUD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_HANGING_SIGN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_WOOD_HANGING_SIGN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BAMBOO_WOOD_HANGING_SIGN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_BAMBOO_WOOD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_NETHER_WOOD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHERRY_WOOD(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHERRY_SAPLING(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHERRY_LEAVES(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHERRY_WOOD_HANGING_SIGN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CHISELED_BOOKSHELF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SHELF(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SUSPICIOUS_SAND(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SUSPICIOUS_GRAVEL(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DECORATED_POT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DECORATED_POT_CRACKED(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_TRIAL_SPAWNER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SPONGE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_WET_SPONGE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_VAULT(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_CREAKING_HEART(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_HEAVY_CORE(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_COBWEB(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_SPAWNER(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_RESIN(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_RESIN_BRICKS(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_IRON(void);
const LIBMATTI_MC_SoundType *LIBMATTI_MC_SoundType_DRIED_GHAST(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCK_SOUNDTYPE_H
