// Port of java.util.Random (the 48-bit LCG, bit-compatible with Java).

#ifndef MATTICRAFT_JAVA_UTIL_RANDOM_H
#define MATTICRAFT_JAVA_UTIL_RANDOM_H

#include <stddef.h>
#include <stdint.h>

typedef struct LIBMATTI_JU_Random
{
    uint64_t seed; // Java: private long seed (48 bits used)
    int haveNextNextGaussian;
    double nextNextGaussian;
} LIBMATTI_JU_Random;

// Java: public Random() - seeded from nanoTime
LIBMATTI_JU_Random *LIBMATTI_JU_Random_New(void);
// Java: public Random(long seed)
LIBMATTI_JU_Random *LIBMATTI_JU_Random_NewSeeded(uint64_t seed);
void LIBMATTI_JU_Random_Free(LIBMATTI_JU_Random *random);

// Java: public void setSeed(long seed)
void LIBMATTI_JU_Random_SetSeed(LIBMATTI_JU_Random *random, uint64_t seed);

// Java: public int nextInt()
int LIBMATTI_JU_Random_NextInt(LIBMATTI_JU_Random *random);
// Java: public int nextInt(int bound)
int LIBMATTI_JU_Random_NextIntBound(LIBMATTI_JU_Random *random, int bound);
// Java: public long nextLong()
long LIBMATTI_JU_Random_NextLong(LIBMATTI_JU_Random *random);
// Java: public boolean nextBoolean()
int LIBMATTI_JU_Random_NextBoolean(LIBMATTI_JU_Random *random);
// Java: public float nextFloat()
float LIBMATTI_JU_Random_NextFloat(LIBMATTI_JU_Random *random);
// Java: public double nextDouble()
double LIBMATTI_JU_Random_NextDouble(LIBMATTI_JU_Random *random);
// Java: public double nextGaussian()
double LIBMATTI_JU_Random_NextGaussian(LIBMATTI_JU_Random *random);

#endif //MATTICRAFT_JAVA_UTIL_RANDOM_H
