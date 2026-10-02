// Port of net.minecraft.world.level.levelgen.synth.ImprovedNoise.
// The construction draws xo/yo/zo (three doubles) then runs the Fisher-Yates
// over 256 entries with nextInt(256 - k) - the exact draw order matters for
// the parity. The evaluation is Java's sampleAndLerp with the smoothstep
// folded on the d4 input (the legacy xYCoherent path with zeroed yOffsets).

#include "libmatti/net/minecraft/world/level/levelgen/synth/ImprovedNoise.h"

#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/levelgen/synth/SimplexNoise.h"

void LIBMATTI_MC_ImprovedNoise_Init(LIBMATTI_MC_ImprovedNoise *noise, LIBMATTI_MC_XoroshiroRandomSource *random)
{
    noise->xo = LIBMATTI_MC_XoroshiroRandomSource_NextDouble(random) * 256.0;
    noise->yo = LIBMATTI_MC_XoroshiroRandomSource_NextDouble(random) * 256.0;
    noise->zo = LIBMATTI_MC_XoroshiroRandomSource_NextDouble(random) * 256.0;

    for (int i = 0; i < 256; i++)
        noise->p[i] = (uint8_t) i;
    for (int k = 0; k < 256; k++)
    {
        int j = LIBMATTI_MC_XoroshiroRandomSource_NextIntBounded(random, 256 - k);
        uint8_t b0 = noise->p[k];
        noise->p[k] = noise->p[k + j];
        noise->p[k + j] = b0;
    }
}

// Java: private int p(int) - the byte read unsigned (the & 0xFF)
static inline int perm(const LIBMATTI_MC_ImprovedNoise *noise, int index)
{
    return noise->p[index & 0xFF] & 0xFF;
}

// Java: gradDot(int, double, double, double) through the SimplexNoise table
static inline double grad_dot(int hash, double x, double y, double z)
{
    return LIBMATTI_MC_SimplexNoise_Dot(LIBMATTI_MC_SimplexNoise_GRADIENT[hash & 15], x, y, z);
}

static double sample_and_lerp(const LIBMATTI_MC_ImprovedNoise *noise, int i, int j, int k,
                              double dx, double dy, double dz, double dyOrig)
{
    int a = perm(noise, i);
    int b = perm(noise, i + 1);
    int aa = perm(noise, a + j);
    int ab = perm(noise, a + j + 1);
    int ba = perm(noise, b + j);
    int bb = perm(noise, b + j + 1);

    double d0 = grad_dot(perm(noise, aa + k), dx, dy, dz);
    double d1 = grad_dot(perm(noise, ba + k), dx - 1.0, dy, dz);
    double d2 = grad_dot(perm(noise, ab + k), dx, dy - 1.0, dz);
    double d3 = grad_dot(perm(noise, bb + k), dx - 1.0, dy - 1.0, dz);
    double d4 = grad_dot(perm(noise, aa + k + 1), dx, dy, dz - 1.0);
    double d5 = grad_dot(perm(noise, ba + k + 1), dx - 1.0, dy, dz - 1.0);
    double d6 = grad_dot(perm(noise, ab + k + 1), dx, dy - 1.0, dz - 1.0);
    double d7 = grad_dot(perm(noise, bb + k + 1), dx - 1.0, dy - 1.0, dz - 1.0);

    // Java: Mth.smoothstep on the fractional parts - the y lerp rides d4 (the
    // pre-snap fraction), exactly the deprecated noise(x, y, z, 0, 0) path
    double tX = LIBMATTI_MC_Mth_Smoothstep(dx);
    double tY = LIBMATTI_MC_Mth_Smoothstep(dyOrig);
    double tZ = LIBMATTI_MC_Mth_Smoothstep(dz);
    return LIBMATTI_MC_Mth_Lerp3(tX, tY, tZ, d0, d1, d2, d3, d4, d5, d6, d7);
}

double LIBMATTI_MC_ImprovedNoise_Noise(LIBMATTI_MC_ImprovedNoise *noise, double x, double y, double z)
{
    // Java: noise(p, q, r, 0.0, 0.0) - the yCoherent snap disabled
    double d0 = x + noise->xo;
    double d1 = y + noise->yo;
    double d2 = z + noise->zo;
    int i = LIBMATTI_MC_Mth_FloorD(d0);
    int j = LIBMATTI_MC_Mth_FloorD(d1);
    int k = LIBMATTI_MC_Mth_FloorD(d2);
    return sample_and_lerp(noise, i, j, k, d0 - (double) i, d1 - (double) j, d2 - (double) k, d1 - (double) j);
}
