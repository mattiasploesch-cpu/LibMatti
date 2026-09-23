// Port of net.minecraft.world.level.GrassColor and net.minecraft.world.level.FoliageColor.
// Both classes are pure color-table holders; the pixel data lives in the client's
// texture files, the port embeds the 256-entry defaults like the vanilla
// grass.png/foliage.png textures resolve to in the absence of a resource pack.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_GRASSCOLOR_H
#define MATTICRAFT_MC_WORLD_LEVEL_GRASSCOLOR_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int[] pixels (GrassColor) / FoliageColor
extern const int LIBMATTI_MC_GrassColor_pixels[256];
extern const int LIBMATTI_MC_FoliageColor_pixels[256];

// Returns the embedded grass table's center sample at index 128.
int LIBMATTI_MC_GrassColor_GetDefaultColor(void);

// Returns the embedded foliage table's center sample at index 128.
int LIBMATTI_MC_FoliageColor_GetDefaultColor(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_GRASSCOLOR_H
