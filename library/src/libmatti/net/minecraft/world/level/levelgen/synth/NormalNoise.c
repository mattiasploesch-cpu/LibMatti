// Port of net.minecraft.world.level.levelgen.synth.NormalNoise: two PerlinNoise
// stacks over the shared parameters, the sum folded through the
// 1.0181268882175227 input factor and the expected-deviation value factor.

#include "libmatti/net/minecraft/world/level/levelgen/synth/NormalNoise.h"

#include <stdlib.h>

// Java: private static final double INPUT_FACTOR = 1.0181268882175227
#define INPUT_FACTOR 1.0181268882175227

static double expected_deviation(int spread)
{
    // Java: 0.1 * (1.0 + 1.0 / (p + 1))
    return 0.1 * (1.0 + 1.0 / (double) (spread + 1));
}

LIBMATTI_MC_NormalNoise *LIBMATTI_MC_NormalNoise_Create(LIBMATTI_MC_XoroshiroRandomSource *random, int firstOctave,
                                                        const double *amplitudes, int amplitudeCount)
{
    LIBMATTI_MC_NormalNoise *noise = calloc(1, sizeof(LIBMATTI_MC_NormalNoise));
    if (noise == NULL)
        return NULL;
    noise->first = LIBMATTI_MC_PerlinNoise_Create(random, firstOctave, amplitudes, amplitudeCount);
    noise->second = LIBMATTI_MC_PerlinNoise_Create(random, firstOctave, amplitudes, amplitudeCount);
    if (noise->first == NULL || noise->second == NULL)
    {
        LIBMATTI_MC_NormalNoise_Free(noise);
        return NULL;
    }

    // Java: the min/max nonzero amplitude spread drives the value factor
    int j = 0x7FFFFFFF;
    int k = 0x80000000;
    for (int i = 0; i < amplitudeCount; i++)
    {
        if (amplitudes[i] != 0.0)
        {
            if (i < j)
                j = i;
            if (i > k)
                k = i;
        }
    }
    noise->valueFactor = 0.16666666666666666 / expected_deviation(k - j);
    noise->maxValue = (LIBMATTI_MC_PerlinNoise_MaxValue(noise->first) + LIBMATTI_MC_PerlinNoise_MaxValue(noise->second))
                      * noise->valueFactor;
    return noise;
}

double LIBMATTI_MC_NormalNoise_GetValue(const LIBMATTI_MC_NormalNoise *noise, double x, double y, double z)
{
    double d0 = x * INPUT_FACTOR;
    double d1 = y * INPUT_FACTOR;
    double d2 = z * INPUT_FACTOR;
    return (LIBMATTI_MC_PerlinNoise_GetValue(noise->first, x, y, z)
            + LIBMATTI_MC_PerlinNoise_GetValue(noise->second, d0, d1, d2))
           * noise->valueFactor;
}

double LIBMATTI_MC_NormalNoise_MaxValue(const LIBMATTI_MC_NormalNoise *noise)
{
    return noise->maxValue;
}

void LIBMATTI_MC_NormalNoise_Free(LIBMATTI_MC_NormalNoise *noise)
{
    if (noise == NULL)
        return;
    LIBMATTI_MC_PerlinNoise_Free(noise->first);
    LIBMATTI_MC_PerlinNoise_Free(noise->second);
    free(noise);
}
