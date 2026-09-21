#include "libmatti/java/util/Random.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

// Java: private static final long multiplier = 0x5DEECE66DL
#define MULTIPLIER 0x5DEECE66ULL
// Java: private static final long addend = 0xBL
#define ADDEND 0xBULL
// Java: private static final long mask = (1L << 48) - 1
#define MASK ((1ULL << 48) - 1)

// Java: private static long seedUniquifier + System.nanoTime()
static uint64_t seed_uniquifier(void)
{
    static uint64_t uniquifier = 8682522807148012ULL;
    uniquifier *= 181783497276652981ULL;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uniquifier ^ (uint64_t) ts.tv_nsec;
}

LIBMATTI_JU_Random *LIBMATTI_JU_Random_New(void)
{
    return LIBMATTI_JU_Random_NewSeeded((uint64_t) seed_uniquifier());
}

LIBMATTI_JU_Random *LIBMATTI_JU_Random_NewSeeded(uint64_t seed)
{
    LIBMATTI_JU_Random *random = calloc(1, sizeof(LIBMATTI_JU_Random));
    LIBMATTI_JU_Random_SetSeed(random, seed);
    return random;
}

void LIBMATTI_JU_Random_Free(LIBMATTI_JU_Random *random)
{
    free(random);
}

// Java: private static long initialScramble(long seed)
static uint64_t initial_scramble(uint64_t seed)
{
    return (seed ^ MULTIPLIER) & MASK;
}

void LIBMATTI_JU_Random_SetSeed(LIBMATTI_JU_Random *random, uint64_t seed)
{
    random->seed = initial_scramble(seed);
    random->haveNextNextGaussian = 0;
}

// Java: protected int next(int bits)
static int32_t next_bits(LIBMATTI_JU_Random *random, int bits)
{
    random->seed = (random->seed * MULTIPLIER + ADDEND) & MASK;
    return (int32_t) (random->seed >> (48 - bits));
}

int LIBMATTI_JU_Random_NextInt(LIBMATTI_JU_Random *random)
{
    return next_bits(random, 32);
}

int LIBMATTI_JU_Random_NextIntBound(LIBMATTI_JU_Random *random, int bound)
{
    // Java: the power-of-two and modulo paths, exactly
    if ((bound & -bound) == bound)
        return (int) ((bound * (long) next_bits(random, 31)) >> 31);

    int bits, val;
    do
    {
        bits = next_bits(random, 31);
        val = bits % bound;
    }
    while (bits - val + (bound - 1) < 0);
    return val;
}

long LIBMATTI_JU_Random_NextLong(LIBMATTI_JU_Random *random)
{
    long high = (long) next_bits(random, 32);
    long low = (long) next_bits(random, 32);
    return (high << 32) + low;
}

int LIBMATTI_JU_Random_NextBoolean(LIBMATTI_JU_Random *random)
{
    return next_bits(random, 1) != 0;
}

float LIBMATTI_JU_Random_NextFloat(LIBMATTI_JU_Random *random)
{
    return (float) next_bits(random, 24) / (float) (1 << 24);
}

double LIBMATTI_JU_Random_NextDouble(LIBMATTI_JU_Random *random)
{
    long high = ((long) next_bits(random, 26)) << 27;
    long low = next_bits(random, 27);
    return (double) (high + low) * (1.0 / (double) (1LL << 53));
}

double LIBMATTI_JU_Random_NextGaussian(LIBMATTI_JU_Random *random)
{
    // Java: the Box-Muller polar method with the cached second value
    if (random->haveNextNextGaussian)
    {
        random->haveNextNextGaussian = 0;
        return random->nextNextGaussian;
    }

    double v1, v2, s;
    do
    {
        v1 = 2.0 * LIBMATTI_JU_Random_NextDouble(random) - 1.0;
        v2 = 2.0 * LIBMATTI_JU_Random_NextDouble(random) - 1.0;
        s = v1 * v1 + v2 * v2;
    }
    while (s >= 1.0 || s == 0.0);

    double multiplier = sqrt(-2.0 * log(s) / s);
    random->nextNextGaussian = v2 * multiplier;
    random->haveNextNextGaussian = 1;
    return v1 * multiplier;
}
