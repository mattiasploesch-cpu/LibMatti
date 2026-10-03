// Port of net.minecraft.world.level.levelgen.flat.FlatLevelGeneratorSettings
// (P7.2) - the superflat recipe: the layer stack (bottom-up), the biome, the
// decoration/lake flags (the features themselves stay later-phase content).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELGENERATORSETTINGS_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELGENERATORSETTINGS_H

#include "libmatti/net/minecraft/world/level/biome/Biome.h"
#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLayerInfo.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBMATTI_MC_FlatLevelGeneratorSettings_MAX_LAYERS 16

// Java: public class FlatLevelGeneratorSettings
typedef struct LIBMATTI_MC_FlatLevelGeneratorSettings
{
    // Java: private final List<FlatLayerInfo> layersInfo
    LIBMATTI_MC_FlatLayerInfo layers[LIBMATTI_MC_FlatLevelGeneratorSettings_MAX_LAYERS];
    int layerCount;
    // Java: private final Holder<Biome> biome
    LIBMATTI_MC_Biome *biome;
    // Java: private boolean decoration / addLakes
    bool decoration;
    bool addLakes;
    // the expanded per-Y block states (Java's updateLayers -> List<BlockState>):
    // layers[y - minY], length = the summed layer heights
    LIBMATTI_MC_BlockState **layersPerY;
    int layersPerYCount;
} LIBMATTI_MC_FlatLevelGeneratorSettings;

// Java: the withBiomeAndLayers shape - the settings over the layer stack
// (the layer states are the blocks' default states)
void LIBMATTI_MC_FlatLevelGeneratorSettings_Init(LIBMATTI_MC_FlatLevelGeneratorSettings *settings,
                                                 const LIBMATTI_MC_FlatLayerInfo *layers, int layerCount,
                                                 LIBMATTI_MC_Biome *biome);
// Java: setDecoration() / setAddLakes()
void LIBMATTI_MC_FlatLevelGeneratorSettings_SetDecoration(LIBMATTI_MC_FlatLevelGeneratorSettings *settings);
void LIBMATTI_MC_FlatLevelGeneratorSettings_SetAddLakes(LIBMATTI_MC_FlatLevelGeneratorSettings *settings);
// Java: private void updateLayers() - the per-Y expansion (re-run after mutation)
void LIBMATTI_MC_FlatLevelGeneratorSettings_UpdateLayers(LIBMATTI_MC_FlatLevelGeneratorSettings *settings);
void LIBMATTI_MC_FlatLevelGeneratorSettings_Free(LIBMATTI_MC_FlatLevelGeneratorSettings *settings);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_FLAT_FLATLEVELGENERATORSETTINGS_H
