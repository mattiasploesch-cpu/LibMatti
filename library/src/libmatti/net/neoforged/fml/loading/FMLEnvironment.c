// Port of net.neoforged.fml.loading.FMLEnvironment.

#include "libmatti/net/neoforged/fml/loading/FMLEnvironment.h"

#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"

// Java: public static Dist getDist()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLEnvironment_GetDist(void)
{
    return LIBMATTI_FML_FMLLoader_GetDist(LIBMATTI_FML_FMLLoader_GetCurrent());
}

// Java: public static boolean isProduction()
int LIBMATTI_FML_FMLEnvironment_IsProduction(void)
{
    return LIBMATTI_FML_FMLLoader_IsProduction(LIBMATTI_FML_FMLLoader_GetCurrent());
}
