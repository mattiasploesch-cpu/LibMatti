// Port of net.minecraft.world.level.levelgen.synth.NormalNoise (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_NORMALNOISE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_NORMALNOISE_H

#include "libmatti/net/minecraft/world/level/levelgen/XoroshiroRandomSource.h"
#include "libmatti/net/minecraft/world/level/levelgen/synth/PerlinNoise.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class NormalNoise
typedef struct LIBMATTI_MC_NormalNoise
{
    LIBMATTI_MC_PerlinNoise *first;
    LIBMATTI_MC_PerlinNoise *second;
    // Java: private final double valueFactor / maxValue
    double valueFactor;
    double maxValue;
} LIBMATTI_MC_NormalNoise;

// Java: public static NormalNoise create(RandomSource, int firstOctave,
// double... amplitudes) - two independent PerlinNoise over the same parameters
LIBMATTI_MC_NormalNoise *LIBMATTI_MC_NormalNoise_Create(LIBMATTI_MC_XoroshiroRandomSource *random, int firstOctave,
                                                        const double *amplitudes, int amplitudeCount);
// Java: public double getValue(double, double, double) - the INPUT_FACTOR fold
double LIBMATTI_MC_NormalNoise_GetValue(const LIBMATTI_MC_NormalNoise *noise, double x, double y, double z);
double LIBMATTI_MC_NormalNoise_MaxValue(const LIBMATTI_MC_NormalNoise *noise);
void LIBMATTI_MC_NormalNoise_Free(LIBMATTI_MC_NormalNoise *noise);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_NORMALNOISE_H
