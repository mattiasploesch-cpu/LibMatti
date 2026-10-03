// Port of the SimplexNoise gradient data.

#include "libmatti/net/minecraft/world/level/levelgen/synth/SimplexNoise.h"

const int LIBMATTI_MC_SimplexNoise_GRADIENT[16][3] = {
    {1, 1, 0},  {-1, 1, 0},  {1, -1, 0},  {-1, -1, 0},
    {1, 0, 1},  {-1, 0, 1},  {1, 0, -1},  {-1, 0, -1},
    {0, 1, 1},  {0, -1, 1},  {0, 1, -1},  {0, -1, -1},
    {1, 1, 0},  {0, -1, 1},  {-1, 1, 0},  {0, -1, -1},
};

double LIBMATTI_MC_SimplexNoise_Dot(const int gradient[3], double x, double y, double z)
{
    return gradient[0] * x + gradient[1] * y + gradient[2] * z;
}
