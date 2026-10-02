// Port of net.minecraft.world.level.levelgen.synth.PerlinNoise (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_PERLINNOISE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_PERLINNOISE_H

#include "libmatti/net/minecraft/world/level/levelgen/XoroshiroRandomSource.h"
#include "libmatti/net/minecraft/world/level/levelgen/synth/ImprovedNoise.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class PerlinNoise
typedef struct LIBMATTI_MC_PerlinNoise
{
    // Java: private final @Nullable ImprovedNoise[] noiseLevels
    LIBMATTI_MC_ImprovedNoise **noiseLevels;
    int noiseLevelCount;
    int firstOctave;
    // Java: private final DoubleList amplitudes
    double *amplitudes;
    int amplitudeCount;
    double lowestFreqValueFactor;
    double lowestFreqInputFactor;
} LIBMATTI_MC_PerlinNoise;

// Java: public static PerlinNoise create(RandomSource, int firstOctave,
// DoubleList amplitudes) - the modern path (forkPositional + "octave_<n>"
// hashes); zero amplitudes skip the octave draw like the Java null entries.
// Returns malloc'd noise or NULL on allocation failure.
LIBMATTI_MC_PerlinNoise *LIBMATTI_MC_PerlinNoise_Create(LIBMATTI_MC_XoroshiroRandomSource *random, int firstOctave,
                                                        const double *amplitudes, int amplitudeCount);
// Java: public double getValue(double, double, double)
double LIBMATTI_MC_PerlinNoise_GetValue(const LIBMATTI_MC_PerlinNoise *noise, double x, double y, double z);
// Java: public double maxValue() - the edgeValue(2.0) precomputed bound
double LIBMATTI_MC_PerlinNoise_MaxValue(const LIBMATTI_MC_PerlinNoise *noise);
// Java: public double wrap(double) - the 3.3554432E7 coordinate wrap
double LIBMATTI_MC_PerlinNoise_Wrap(double value);
void LIBMATTI_MC_PerlinNoise_Free(LIBMATTI_MC_PerlinNoise *noise);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_PERLINNOISE_H
