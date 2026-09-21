// Port of net.minecraft.world.level.material.MapColor.
// Java registers every colour into a static MATERIAL_COLORS[64] table in the
// constructor; the C port keeps the same table built at first use.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_MATERIAL_MAPCOLOR_H
#define MATTICRAFT_MC_WORLD_LEVEL_MATERIAL_MAPCOLOR_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class MapColor - the port stores the two finals; identity (pointer
// equality) against the constants works like Java
typedef struct LIBMATTI_MC_MapColor
{
    // Java: public final int col
    int col;
    // Java: public final int id
    int id;
} LIBMATTI_MC_MapColor;

// One accessor per Java constant, in declaration order. All pointers stay valid
// for the process lifetime (Java: the static fields).
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_NONE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GRASS(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_SAND(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WOOL(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_FIRE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_ICE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_METAL(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_PLANT(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_SNOW(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CLAY(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DIRT(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_STONE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WATER(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WOOD(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_QUARTZ(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_ORANGE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_MAGENTA(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_BLUE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_YELLOW(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_GREEN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_PINK(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_GRAY(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_GRAY(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_CYAN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_PURPLE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BLUE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BROWN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_GREEN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_RED(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BLACK(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GOLD(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DIAMOND(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_LAPIS(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_EMERALD(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_PODZOL(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_NETHER(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_WHITE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_ORANGE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_MAGENTA(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_BLUE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_YELLOW(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_GREEN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_PINK(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_GRAY(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_GRAY(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_CYAN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_PURPLE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BLUE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BROWN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_GREEN(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_RED(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BLACK(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_NYLIUM(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_STEM(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_HYPHAE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_NYLIUM(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_STEM(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_HYPHAE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_WART_BLOCK(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DEEPSLATE(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_RAW_IRON(void);
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GLOW_LICHEN(void);

// Java: public static MapColor byId(int) - NONE when the slot is unpopulated
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_ById(int id);
// Java: private MapColor(int, int) registered every colour - used by the
// initialiser to fill MATERIAL_COLORS exactly like Java's constructor
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_Create(int id, int col);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_MATERIAL_MAPCOLOR_H
