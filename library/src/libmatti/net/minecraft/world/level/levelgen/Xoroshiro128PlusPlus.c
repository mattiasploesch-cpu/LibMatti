// Port of net.minecraft.world.level.levelgen.Xoroshiro128PlusPlus.
// The step is the Java arithmetic 1:1 (the 64-bit rotates via the helper, the
// additions and xors wrap).

#include "libmatti/net/minecraft/world/level/levelgen/Xoroshiro128PlusPlus.h"

#include "libmatti/net/minecraft/world/level/levelgen/RandomSupport.h"

static inline uint64_t rotl(uint64_t value, int bits)
{
    return (value << bits) | (value >> (64 - bits));
}

void LIBMATTI_MC_Xoroshiro128PlusPlus_Init(LIBMATTI_MC_Xoroshiro128PlusPlus *random, int64_t seedLo, int64_t seedHi)
{
    random->seedLo = seedLo;
    random->seedHi = seedHi;
    if (((uint64_t) random->seedLo | (uint64_t) random->seedHi) == 0ULL)
    {
        random->seedLo = LIBMATTI_MC_RandomSupport_GOLDEN_RATIO_64;
        random->seedHi = LIBMATTI_MC_RandomSupport_SILVER_RATIO_64;
    }
}

int64_t LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(LIBMATTI_MC_Xoroshiro128PlusPlus *random)
{
    uint64_t i = (uint64_t) random->seedLo;
    uint64_t j = (uint64_t) random->seedHi;
    // Java: long k = Long.rotateLeft(i + j, 17) + i
    int64_t k = (int64_t) (rotl(i + j, 17) + i);
    j ^= i;
    random->seedLo = (int64_t) (rotl(i, 49) ^ j ^ (j << 21));
    random->seedHi = (int64_t) rotl(j, 28);
    return k;
}
