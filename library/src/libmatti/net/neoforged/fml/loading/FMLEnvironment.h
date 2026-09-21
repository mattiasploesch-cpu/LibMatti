// Port of net.neoforged.fml.loading.FMLEnvironment.

#ifndef MATTICRAFT_FML_LOADING_FMLENVIRONMENT_H
#define MATTICRAFT_FML_LOADING_FMLENVIRONMENT_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"

// Java: public static Dist getDist()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLEnvironment_GetDist(void);
// Java: public static boolean isProduction()
int LIBMATTI_FML_FMLEnvironment_IsProduction(void);

#endif //MATTICRAFT_FML_LOADING_FMLENVIRONMENT_H
