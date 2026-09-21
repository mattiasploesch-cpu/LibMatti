// Port of net.minecraft.world.level.material.MapColor.

#include "libmatti/net/minecraft/world/level/material/MapColor.h"

#include <stddef.h>

// Java: private static final @Nullable MapColor[] MATERIAL_COLORS = new MapColor[64]
#define COLOR_COUNT 64
static const LIBMATTI_MC_MapColor *materialColors[COLOR_COUNT];

const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_Create(int id, int col)
{
    // Java: the constructor stores the singleton in MATERIAL_COLORS; the port keeps
    // a static instance per call site, so repeated creates are shared through the table
    static LIBMATTI_MC_MapColor instances[COLOR_COUNT];
    static int used[COLOR_COUNT];

    if (id < 0 || id >= COLOR_COUNT)
    {
        // Java: throw new IndexOutOfBoundsException("Map colour ID must be between 0 and 63 (inclusive)")
        return LIBMATTI_MC_MapColor_NONE();
    }
    if (!used[id])
    {
        instances[id].id = id;
        instances[id].col = col;
        materialColors[id] = &instances[id];
        used[id] = 1;
    }
    return materialColors[id];
}

// Java: public static MapColor byId(int)
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_ById(int id)
{
    if (id < 0 || id >= COLOR_COUNT)
        return LIBMATTI_MC_MapColor_NONE();
    const LIBMATTI_MC_MapColor *color = materialColors[id];
    // Java: byIdUnsafe falls back to NONE
    return color != NULL ? color : LIBMATTI_MC_MapColor_NONE();
}

// Java: NONE = new MapColor(0, 0) - defined first so the fallback exists
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_NONE(void)
{
    static LIBMATTI_MC_MapColor none = {0, 0};
    if (materialColors[0] == NULL)
        materialColors[0] = &none;
    return &none;
}

// The remaining constants, 1:1 in declaration order (id, col).
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GRASS(void) { return LIBMATTI_MC_MapColor_Create(1, 8368696); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_SAND(void) { return LIBMATTI_MC_MapColor_Create(2, 16247203); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WOOL(void) { return LIBMATTI_MC_MapColor_Create(3, 13092807); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_FIRE(void) { return LIBMATTI_MC_MapColor_Create(4, 16711680); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_ICE(void) { return LIBMATTI_MC_MapColor_Create(5, 10526975); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_METAL(void) { return LIBMATTI_MC_MapColor_Create(6, 10987431); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_PLANT(void) { return LIBMATTI_MC_MapColor_Create(7, 31744); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_SNOW(void) { return LIBMATTI_MC_MapColor_Create(8, 16777215); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CLAY(void) { return LIBMATTI_MC_MapColor_Create(9, 10791096); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DIRT(void) { return LIBMATTI_MC_MapColor_Create(10, 9923917); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_STONE(void) { return LIBMATTI_MC_MapColor_Create(11, 7368816); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WATER(void) { return LIBMATTI_MC_MapColor_Create(12, 4210943); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WOOD(void) { return LIBMATTI_MC_MapColor_Create(13, 9402184); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_QUARTZ(void) { return LIBMATTI_MC_MapColor_Create(14, 16776437); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_ORANGE(void) { return LIBMATTI_MC_MapColor_Create(15, 14188339); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_MAGENTA(void) { return LIBMATTI_MC_MapColor_Create(16, 11685080); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_BLUE(void) { return LIBMATTI_MC_MapColor_Create(17, 6724056); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_YELLOW(void) { return LIBMATTI_MC_MapColor_Create(18, 15066419); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_GREEN(void) { return LIBMATTI_MC_MapColor_Create(19, 8375321); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_PINK(void) { return LIBMATTI_MC_MapColor_Create(20, 15892389); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_GRAY(void) { return LIBMATTI_MC_MapColor_Create(21, 5000268); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_LIGHT_GRAY(void) { return LIBMATTI_MC_MapColor_Create(22, 10066329); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_CYAN(void) { return LIBMATTI_MC_MapColor_Create(23, 5013401); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_PURPLE(void) { return LIBMATTI_MC_MapColor_Create(24, 8339378); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BLUE(void) { return LIBMATTI_MC_MapColor_Create(25, 3361970); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BROWN(void) { return LIBMATTI_MC_MapColor_Create(26, 6704179); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_GREEN(void) { return LIBMATTI_MC_MapColor_Create(27, 6717235); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_RED(void) { return LIBMATTI_MC_MapColor_Create(28, 10040115); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_COLOR_BLACK(void) { return LIBMATTI_MC_MapColor_Create(29, 1644825); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GOLD(void) { return LIBMATTI_MC_MapColor_Create(30, 16445005); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DIAMOND(void) { return LIBMATTI_MC_MapColor_Create(31, 6085589); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_LAPIS(void) { return LIBMATTI_MC_MapColor_Create(32, 4882687); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_EMERALD(void) { return LIBMATTI_MC_MapColor_Create(33, 55610); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_PODZOL(void) { return LIBMATTI_MC_MapColor_Create(34, 8476209); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_NETHER(void) { return LIBMATTI_MC_MapColor_Create(35, 7340544); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_WHITE(void) { return LIBMATTI_MC_MapColor_Create(36, 13742497); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_ORANGE(void) { return LIBMATTI_MC_MapColor_Create(37, 10441252); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_MAGENTA(void) { return LIBMATTI_MC_MapColor_Create(38, 9787244); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_BLUE(void) { return LIBMATTI_MC_MapColor_Create(39, 7367818); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_YELLOW(void) { return LIBMATTI_MC_MapColor_Create(40, 12223780); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_GREEN(void) { return LIBMATTI_MC_MapColor_Create(41, 6780213); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_PINK(void) { return LIBMATTI_MC_MapColor_Create(42, 10505550); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_GRAY(void) { return LIBMATTI_MC_MapColor_Create(43, 3746083); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_LIGHT_GRAY(void) { return LIBMATTI_MC_MapColor_Create(44, 8874850); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_CYAN(void) { return LIBMATTI_MC_MapColor_Create(45, 5725276); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_PURPLE(void) { return LIBMATTI_MC_MapColor_Create(46, 8014168); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BLUE(void) { return LIBMATTI_MC_MapColor_Create(47, 4996700); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BROWN(void) { return LIBMATTI_MC_MapColor_Create(48, 4993571); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_GREEN(void) { return LIBMATTI_MC_MapColor_Create(49, 5001770); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_RED(void) { return LIBMATTI_MC_MapColor_Create(50, 9321518); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_TERRACOTTA_BLACK(void) { return LIBMATTI_MC_MapColor_Create(51, 2430480); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_NYLIUM(void) { return LIBMATTI_MC_MapColor_Create(52, 12398641); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_STEM(void) { return LIBMATTI_MC_MapColor_Create(53, 9715553); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_CRIMSON_HYPHAE(void) { return LIBMATTI_MC_MapColor_Create(54, 6035741); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_NYLIUM(void) { return LIBMATTI_MC_MapColor_Create(55, 1474182); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_STEM(void) { return LIBMATTI_MC_MapColor_Create(56, 3837580); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_HYPHAE(void) { return LIBMATTI_MC_MapColor_Create(57, 5647422); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_WARPED_WART_BLOCK(void) { return LIBMATTI_MC_MapColor_Create(58, 1356933); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_DEEPSLATE(void) { return LIBMATTI_MC_MapColor_Create(59, 6579300); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_RAW_IRON(void) { return LIBMATTI_MC_MapColor_Create(60, 14200723); }
const LIBMATTI_MC_MapColor *LIBMATTI_MC_MapColor_GLOW_LICHEN(void) { return LIBMATTI_MC_MapColor_Create(61, 8365974); }
