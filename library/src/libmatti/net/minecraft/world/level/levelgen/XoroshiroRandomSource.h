// Port of net.minecraft.world.level.levelgen.XoroshiroRandomSource (P7.2) -
// the RandomSource the world generation seeds ride, plus the
// XoroshiroPositionalRandomFactory (the fromHashOf octave seeds, the at(x,y,z)
// positional forks).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRORANDOMSOURCE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRORANDOMSOURCE_H

#include "libmatti/net/minecraft/world/level/levelgen/Xoroshiro128PlusPlus.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class XoroshiroRandomSource implements RandomSource
typedef struct LIBMATTI_MC_XoroshiroRandomSource
{
    LIBMATTI_MC_Xoroshiro128PlusPlus randomNumberGenerator;
} LIBMATTI_MC_XoroshiroRandomSource;

// Java: XoroshiroRandomSource(long) - the seed through upgradeSeedTo128bit
void LIBMATTI_MC_XoroshiroRandomSource_Init(LIBMATTI_MC_XoroshiroRandomSource *random, int64_t seed);
// Java: XoroshiroRandomSource(Seed128bit) / (long, long) - the raw pair
void LIBMATTI_MC_XoroshiroRandomSource_Init128(LIBMATTI_MC_XoroshiroRandomSource *random, int64_t seedLo, int64_t seedHi);

// Java: public int nextInt() - the low half of the long
int32_t LIBMATTI_MC_XoroshiroRandomSource_NextInt(LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public int nextInt(int bound) - the unsigned-multiply rejection fold
int32_t LIBMATTI_MC_XoroshiroRandomSource_NextIntBounded(LIBMATTI_MC_XoroshiroRandomSource *random, int32_t bound);
// Java: public long nextLong()
int64_t LIBMATTI_MC_XoroshiroRandomSource_NextLong(LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public boolean nextBoolean() - the low bit
bool LIBMATTI_MC_XoroshiroRandomSource_NextBoolean(LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public float nextFloat() - the top 24 bits times FLOAT_UNIT
float LIBMATTI_MC_XoroshiroRandomSource_NextFloat(LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public double nextDouble() - the top 53 bits times DOUBLE_UNIT
double LIBMATTI_MC_XoroshiroRandomSource_NextDouble(LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public void consumeCount(int) - the discarded draws (the legacy octave skip)
void LIBMATTI_MC_XoroshiroRandomSource_ConsumeCount(LIBMATTI_MC_XoroshiroRandomSource *random, int count);
// Java: public RandomSource fork() - the two fresh longs as the new state
void LIBMATTI_MC_XoroshiroRandomSource_Fork(LIBMATTI_MC_XoroshiroRandomSource *random,
                                            LIBMATTI_MC_XoroshiroRandomSource *out);

// Java: forkPositional() - the factory state is the two drawn longs
typedef struct LIBMATTI_MC_XoroshiroPositionalRandomFactory
{
    int64_t seedLo;
    int64_t seedHi;
} LIBMATTI_MC_XoroshiroPositionalRandomFactory;

void LIBMATTI_MC_XoroshiroRandomSource_ForkPositional(LIBMATTI_MC_XoroshiroRandomSource *random,
                                                      LIBMATTI_MC_XoroshiroPositionalRandomFactory *outFactory);
// Java: XoroshiroPositionalRandomFactory.fromHashOf(String) - the MD5 pair
// xor'd into the factory state
void LIBMATTI_MC_XoroshiroPositionalRandomFactory_FromHashOf(
    const LIBMATTI_MC_XoroshiroPositionalRandomFactory *factory, const char *name,
    LIBMATTI_MC_XoroshiroRandomSource *out);
// Java: XoroshiroPositionalRandomFactory.at(int, int, int) - Mth.getSeed xor'd
// into the factory state
void LIBMATTI_MC_XoroshiroPositionalRandomFactory_At(
    const LIBMATTI_MC_XoroshiroPositionalRandomFactory *factory, int x, int y, int z,
    LIBMATTI_MC_XoroshiroRandomSource *out);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRORANDOMSOURCE_H
