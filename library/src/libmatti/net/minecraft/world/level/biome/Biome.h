// Port of the net.minecraft.world.level.biome.Biome identity (P7.2).
//
// The biome model is the registry identity only: an interned
// "minecraft:<path>" id (the full Biome climate/effects model lands with the
// generation-settings port). Interning makes pointer equality meaningful and
// the serializable-chunk palette writes ride the id.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BIOME_BIOME_H
#define MATTICRAFT_MC_WORLD_LEVEL_BIOME_BIOME_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: the Holder<Biome> identity - the port interns by id string
typedef struct LIBMATTI_MC_Biome
{
    const char *id; // "minecraft:<path>"
} LIBMATTI_MC_Biome;

// Java: Registry.register(Biomes, "minecraft:<id>") - the interned instance
// (the same id returns the same pointer; the table grows for the session)
LIBMATTI_MC_Biome *LIBMATTI_MC_Biome_Of(const char *id);
// Java: Biomes.PLAINS / the other vanilla constants the port names so far
LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Plains(void);
LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Forest(void);
LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Desert(void);
// the id accessor (never NULL)
const char *LIBMATTI_MC_Biome_GetId(const LIBMATTI_MC_Biome *biome);
// pointer equality after interning, with the NULL == NULL tolerance
bool LIBMATTI_MC_Biome_Equals(const LIBMATTI_MC_Biome *a, const LIBMATTI_MC_Biome *b);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BIOME_BIOME_H
