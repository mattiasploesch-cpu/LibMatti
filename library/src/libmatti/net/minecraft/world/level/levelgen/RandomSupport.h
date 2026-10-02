// Port of net.minecraft.world.level.levelgen.RandomSupport (P7.2).
//
// Java: the seed material - the stafford13 mixer, the 64 -> 128 bit seed
// upgrade (the silver/golden-ratio folds) and the MD5 hash-of-string path the
// positional random factory rides. The port runs the exact integer arithmetic
// so the derived noise seeds match the JVM bit for bit (the parity harness
// pins the vectors).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_RANDOMSUPPORT_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_RANDOMSUPPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final long GOLDEN_RATIO_64 = -7046029254386353131L
#define LIBMATTI_MC_RandomSupport_GOLDEN_RATIO_64 (-7046029254386353131LL)
// Java: public static final long SILVER_RATIO_64 = 7640891576956012809L
#define LIBMATTI_MC_RandomSupport_SILVER_RATIO_64 7640891576956012809LL

// Java: public static long mixStafford13(long)
int64_t LIBMATTI_MC_RandomSupport_MixStafford13(int64_t seed);

// Java: public static Seed128bit upgradeSeedTo128bitUnmixed(long) - the
// seed ^ SILVER_RATIO_64, + GOLDEN_RATIO_64 pair
void LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bitUnmixed(int64_t seed, int64_t *outLo, int64_t *outHi);
// Java: public static Seed128bit upgradeSeedTo128bit(long) - the unmixed pair
// through mixStafford13 on both halves
void LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bit(int64_t seed, int64_t *outLo, int64_t *outHi);
// Java: public static Seed128bit seedFromHashOf(String) - the MD5 digest of the
// UTF-8 string read as two big-endian longs (the Guava hash path)
void LIBMATTI_MC_RandomSupport_SeedFromHashOf(const char *string, int64_t *outLo, int64_t *outHi);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_RANDOMSUPPORT_H
