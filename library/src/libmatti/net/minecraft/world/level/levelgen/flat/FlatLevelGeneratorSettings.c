// Port of net.minecraft.world.level.levelgen.flat.FlatLevelGeneratorSettings:
// the layer info list expands into the per-Y state list Java's updateLayers
// builds (the fillFromNoise pass indexes it directly).

#include "libmatti/net/minecraft/world/level/levelgen/flat/FlatLevelGeneratorSettings.h"

#include <stdlib.h>

void LIBMATTI_MC_FlatLevelGeneratorSettings_Init(LIBMATTI_MC_FlatLevelGeneratorSettings *settings,
                                                 const LIBMATTI_MC_FlatLayerInfo *layers, int layerCount,
                                                 LIBMATTI_MC_Biome *biome)
{
    settings->layerCount = 0;
    settings->biome = biome != NULL ? biome : LIBMATTI_MC_Biomes_Plains();
    settings->decoration = false;
    settings->addLakes = false;
    settings->layersPerY = NULL;
    settings->layersPerYCount = 0;
    for (int i = 0; i < layerCount && settings->layerCount < LIBMATTI_MC_FlatLevelGeneratorSettings_MAX_LAYERS; i++)
        settings->layers[settings->layerCount++] = layers[i];
    LIBMATTI_MC_FlatLevelGeneratorSettings_UpdateLayers(settings);
}

void LIBMATTI_MC_FlatLevelGeneratorSettings_SetDecoration(LIBMATTI_MC_FlatLevelGeneratorSettings *settings)
{
    settings->decoration = true;
}

void LIBMATTI_MC_FlatLevelGeneratorSettings_SetAddLakes(LIBMATTI_MC_FlatLevelGeneratorSettings *settings)
{
    settings->addLakes = true;
}

void LIBMATTI_MC_FlatLevelGeneratorSettings_UpdateLayers(LIBMATTI_MC_FlatLevelGeneratorSettings *settings)
{
    free(settings->layersPerY);
    settings->layersPerY = NULL;
    settings->layersPerYCount = 0;

    int total = 0;
    for (int i = 0; i < settings->layerCount; i++)
        total += settings->layers[i].height;
    if (total <= 0)
        return;

    settings->layersPerY = malloc(sizeof(LIBMATTI_MC_BlockState *) * (size_t) total);
    if (settings->layersPerY == NULL)
        return;
    int index = 0;
    for (int i = 0; i < settings->layerCount; i++)
    {
        for (int h = 0; h < settings->layers[i].height && index < total; h++)
            settings->layersPerY[index++] = settings->layers[i].blockState;
    }
    settings->layersPerYCount = total;
}

void LIBMATTI_MC_FlatLevelGeneratorSettings_Free(LIBMATTI_MC_FlatLevelGeneratorSettings *settings)
{
    if (settings == NULL)
        return;
    free(settings->layersPerY);
    settings->layersPerY = NULL;
    settings->layersPerYCount = 0;
}
