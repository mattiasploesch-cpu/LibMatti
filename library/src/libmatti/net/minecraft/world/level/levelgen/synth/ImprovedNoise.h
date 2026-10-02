// Port of net.minecraft.world.level.levelgen.synth.ImprovedNoise (P7.2).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_IMPROVEDNOISE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_IMPROVEDNOISE_H

#include "libmatti/net/minecraft/world/level/levelgen/XoroshiroRandomSource.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public final class ImprovedNoise
typedef struct LIBMATTI_MC_ImprovedNoise
{
    // Java: private final byte[] p - the 256-entry permutation
    uint8_t p[256];
    // Java: public final double xo/yo/zo - the origin offsets drawn from the seed
    double xo;
    double yo;
    double zo;
} LIBMATTI_MC_ImprovedNoise;

// Java: public ImprovedNoise(RandomSource) - the three origin doubles + the
// 256-draw Fisher-Yates shuffle
void LIBMATTI_MC_ImprovedNoise_Init(LIBMATTI_MC_ImprovedNoise *noise, LIBMATTI_MC_XoroshiroRandomSource *random);
// Java: public double noise(double, double, double) - the yOffsets pair zeroed
double LIBMATTI_MC_ImprovedNoise_Noise(LIBMATTI_MC_ImprovedNoise *noise, double x, double y, double z);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_IMPROVEDNOISE_H
