// Port of the SimplexNoise pieces ImprovedNoise rides (P7.2): the 16-entry
// gradient table and the dot product (the simplex evaluation itself is not
// ported yet - no consumer needs it).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_SIMPLEXNOISE_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_SIMPLEXNOISE_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: protected static final int[][] GRADIENT
extern const int LIBMATTI_MC_SimplexNoise_GRADIENT[16][3];

// Java: protected static double dot(int[], double, double, double)
double LIBMATTI_MC_SimplexNoise_Dot(const int gradient[3], double x, double y, double z);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_SIMPLEXNOISE_H
