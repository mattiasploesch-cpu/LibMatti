// Port of net.minecraft.world.level.levelgen.synth.PerlinNoise (the modern
// create path): the octaves seed through the random source's positional
// factory and the "octave_<firstOctave + k>" MD5 hashes, zero-amplitude
// octaves stay NULL without consuming a draw. The evaluation sums the octaves
// with the doubling input frequency and the halving value factor.

#include "libmatti/net/minecraft/world/level/levelgen/synth/PerlinNoise.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double LIBMATTI_MC_PerlinNoise_Wrap(double value)
{
    // Java: p - Mth.lfloor(p / 3.3554432E7 + 0.5) * 3.3554432E7
    return value - LIBMATTI_MC_Mth_LFloor(value / 3.3554432E7 + 0.5) * 3.3554432E7;
}

LIBMATTI_MC_PerlinNoise *LIBMATTI_MC_PerlinNoise_Create(LIBMATTI_MC_XoroshiroRandomSource *random, int firstOctave,
                                                        const double *amplitudes, int amplitudeCount)
{
    LIBMATTI_MC_PerlinNoise *noise = calloc(1, sizeof(LIBMATTI_MC_PerlinNoise));
    if (noise == NULL)
        return NULL;
    noise->firstOctave = firstOctave;
    noise->amplitudeCount = amplitudeCount;
    noise->noiseLevelCount = amplitudeCount;
    noise->amplitudes = malloc(sizeof(double) * (size_t) (amplitudeCount > 0 ? amplitudeCount : 1));
    noise->noiseLevels = calloc((size_t) (amplitudeCount > 0 ? amplitudeCount : 1), sizeof(LIBMATTI_MC_ImprovedNoise *));
    if (noise->amplitudes == NULL || noise->noiseLevels == NULL)
    {
        free(noise->amplitudes);
        free(noise->noiseLevels);
        free(noise);
        return NULL;
    }
    memcpy(noise->amplitudes, amplitudes, sizeof(double) * (size_t) amplitudeCount);

    // Java: PositionalRandomFactory positionalrandomfactory = p.forkPositional()
    LIBMATTI_MC_XoroshiroPositionalRandomFactory factory;
    LIBMATTI_MC_XoroshiroRandomSource_ForkPositional(random, &factory);

    int i = amplitudeCount;
    for (int k = 0; k < i; k++)
    {
        if (noise->amplitudes[k] == 0.0)
            continue;
        int l = firstOctave + k;
        char name[32];
        snprintf(name, sizeof(name), "octave_%d", l);
        LIBMATTI_MC_ImprovedNoise *octave = malloc(sizeof(LIBMATTI_MC_ImprovedNoise));
        if (octave == NULL)
        {
            LIBMATTI_MC_PerlinNoise_Free(noise);
            return NULL;
        }
        LIBMATTI_MC_XoroshiroRandomSource octaveRandom;
        LIBMATTI_MC_XoroshiroPositionalRandomFactory_FromHashOf(&factory, name, &octaveRandom);
        LIBMATTI_MC_ImprovedNoise_Init(octave, &octaveRandom);
        noise->noiseLevels[k] = octave;
    }

    // Java: j = -firstOctave (the lowest octave index into the amplitude list)
    int j = -firstOctave;
    noise->lowestFreqInputFactor = pow(2.0, (double) -j);
    noise->lowestFreqValueFactor = pow(2.0, (double) (i - 1)) / (pow(2.0, (double) i) - 1.0);
    return noise;
}

double LIBMATTI_MC_PerlinNoise_GetValue(const LIBMATTI_MC_PerlinNoise *noise, double x, double y, double z)
{
    // Java: the deprecated getValue(x, y, z, 0.0, 0.0, false) body
    double d0 = 0.0;
    double d1 = noise->lowestFreqInputFactor;
    double d2 = noise->lowestFreqValueFactor;

    for (int i = 0; i < noise->noiseLevelCount; i++)
    {
        LIBMATTI_MC_ImprovedNoise *octave = noise->noiseLevels[i];
        if (octave != NULL)
        {
            double d3 = LIBMATTI_MC_ImprovedNoise_Noise(octave, LIBMATTI_MC_PerlinNoise_Wrap(x * d1), y * d1,
                                                        LIBMATTI_MC_PerlinNoise_Wrap(z * d1));
            d0 += noise->amplitudes[i] * d3 * d2;
        }
        d1 *= 2.0;
        d2 /= 2.0;
    }
    return d0;
}

double LIBMATTI_MC_PerlinNoise_MaxValue(const LIBMATTI_MC_PerlinNoise *noise)
{
    // Java: edgeValue(2.0)
    double d0 = 0.0;
    double d1 = noise->lowestFreqValueFactor;
    for (int i = 0; i < noise->noiseLevelCount; i++)
    {
        if (noise->noiseLevels[i] != NULL)
            d0 += noise->amplitudes[i] * 2.0 * d1;
        d1 /= 2.0;
    }
    return d0;
}

void LIBMATTI_MC_PerlinNoise_Free(LIBMATTI_MC_PerlinNoise *noise)
{
    if (noise == NULL)
        return;
    for (int i = 0; i < noise->noiseLevelCount; i++)
        free(noise->noiseLevels[i]);
    free(noise->noiseLevels);
    free(noise->amplitudes);
    free(noise);
}
