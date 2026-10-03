// Port of net.minecraft.world.level.levelgen.Xoroshiro128PlusPlus (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRO128PLUSPLUS_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRO128PLUSPLUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class Xoroshiro128PlusPlus
typedef struct LIBMATTI_MC_Xoroshiro128PlusPlus
{
    int64_t seedLo;
    int64_t seedHi;
} LIBMATTI_MC_Xoroshiro128PlusPlus;

// Java: the ctor - the all-zero state falls back to the golden/silver pair
void LIBMATTI_MC_Xoroshiro128PlusPlus_Init(LIBMATTI_MC_Xoroshiro128PlusPlus *random, int64_t seedLo, int64_t seedHi);
// Java: public long nextLong() - the state step + the rotated sum output
int64_t LIBMATTI_MC_Xoroshiro128PlusPlus_NextLong(LIBMATTI_MC_Xoroshiro128PlusPlus *random);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_XOROSHIRO128PLUSPLUS_H
