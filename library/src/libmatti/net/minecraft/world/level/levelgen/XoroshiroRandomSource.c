// Port of net.minecraft.world.level.levelgen.XoroshiroRandomSource.
// The draw semantics are the Java folds 1:1: nextInt takes the low half,
// nextInt(bound) runs the unsigned-multiply rejection, the float/double units
// are the top bits times the exact Java constants.

#include "libmatti/net/minecraft/world/level/levelgen/XoroshiroRandomSource.h"

#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/levelgen/RandomSupport.h"

void LIBMATTI_MC_XoroshiroRandomSource_Init(LIBMATTI_MC_XoroshiroRandomSource *random, int64_t seed)
{
    int64_t lo, hi;
    LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bit(seed, &lo, &hi);
    LIBMATTI_MC_Xoroshiro128PlusPlus_Init(&random->randomNumberGenerator, lo, hi);
}

void LIBMATTI_MC_XoroshiroRandomSource_Init128(LIBMATTI_MC_XoroshiroRandomSource *random, int64_t seedLo, int64_t seedHi)
{
    LIBMATTI_MC_Xoroshiro128PlusPlus_Init(&random->randomNumberGenerator, seedLo, seedHi);
}

int32_t LIBMATTI_MC_XoroshiroRandomSource_NextInt(LIBMATTI_MC_XoroshiroRandomSource *random)
{
    return (int32_t) LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
}

int32_t LIBMATTI_MC_XoroshiroRandomSource_NextIntBounded(LIBMATTI_MC_XoroshiroRandomSource *random, int32_t bound)
{
    // Java: the Integer.toUnsignedLong(nextInt()) * bound fold with the
    // rejection on the low-product remainder
    if (bound <= 0)
        return 0;
    uint32_t i = (uint32_t) LIBMATTI_MC_XoroshiroRandomSource_NextInt(random);
    uint64_t j = (uint64_t) i * (uint64_t) bound;
    uint32_t k = (uint32_t) (j & 0xFFFFFFFFULL);
    if (k < (uint32_t) bound)
    {
        uint32_t l = (uint32_t) ((~(uint32_t) bound + 1U) % (uint32_t) bound);
        while (k < l)
        {
            i = (uint32_t) LIBMATTI_MC_XoroshiroRandomSource_NextInt(random);
            j = (uint64_t) i * (uint64_t) bound;
            k = (uint32_t) (j & 0xFFFFFFFFULL);
        }
    }
    uint64_t i1 = j >> 32;
    return (int32_t) i1;
}

int64_t LIBMATTI_MC_XoroshiroRandomSource_NextLong(LIBMATTI_MC_XoroshiroRandomSource *random)
{
    return LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
}

bool LIBMATTI_MC_XoroshiroRandomSource_NextBoolean(LIBMATTI_MC_XoroshiroRandomSource *random)
{
    return (LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator) & 1LL) != 0LL;
}

float LIBMATTI_MC_XoroshiroRandomSource_NextFloat(LIBMATTI_MC_XoroshiroRandomSource *random)
{
    // Java: nextBits(24) * 5.9604645E-8F - the unsigned top-24 shift
    uint64_t bits = (uint64_t) LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator) >> (64 - 24);
    return (float) bits * 5.9604645E-8F;
}

double LIBMATTI_MC_XoroshiroRandomSource_NextDouble(LIBMATTI_MC_XoroshiroRandomSource *random)
{
    // Java: nextBits(53) * 1.110223E-16 (the literal is the 2^-53 double unit)
    uint64_t bits = (uint64_t) LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator) >> (64 - 53);
    return (double) bits * 1.110223E-16;
}

void LIBMATTI_MC_XoroshiroRandomSource_ConsumeCount(LIBMATTI_MC_XoroshiroRandomSource *random, int count)
{
    for (int i = 0; i < count; i++)
        LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
}

void LIBMATTI_MC_XoroshiroRandomSource_Fork(LIBMATTI_MC_XoroshiroRandomSource *random,
                                            LIBMATTI_MC_XoroshiroRandomSource *out)
{
    int64_t lo = LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
    int64_t hi = LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
    LIBMATTI_MC_Xoroshiro128PlusPlus_Init(&out->randomNumberGenerator, lo, hi);
}

void LIBMATTI_MC_XoroshiroRandomSource_ForkPositional(LIBMATTI_MC_XoroshiroRandomSource *random,
                                                      LIBMATTI_MC_XoroshiroPositionalRandomFactory *outFactory)
{
    outFactory->seedLo = LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
    outFactory->seedHi = LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(&random->randomNumberGenerator);
}

void LIBMATTI_MC_XoroshiroPositionalRandomFactory_FromHashOf(
    const LIBMATTI_MC_XoroshiroPositionalRandomFactory *factory, const char *name,
    LIBMATTI_MC_XoroshiroRandomSource *out)
{
    int64_t lo, hi;
    LIBMATTI_MC_RandomSupport_SeedFromHashOf(name, &lo, &hi);
    LIBMATTI_MC_Xoroshiro128PlusPlus_Init(&out->randomNumberGenerator, lo ^ factory->seedLo,
                                          hi ^ factory->seedHi);
}

void LIBMATTI_MC_XoroshiroPositionalRandomFactory_At(
    const LIBMATTI_MC_XoroshiroPositionalRandomFactory *factory, int x, int y, int z,
    LIBMATTI_MC_XoroshiroRandomSource *out)
{
    int64_t i = LIBMATTI_MC_Mth_GetSeed(x, y, z);
    int64_t j = i ^ factory->seedLo;
    LIBMATTI_MC_Xoroshiro128PlusPlus_Init(&out->randomNumberGenerator, j, factory->seedHi);
}
