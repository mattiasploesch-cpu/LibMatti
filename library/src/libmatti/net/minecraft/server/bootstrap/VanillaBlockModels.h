




#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKMODELS_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKMODELS_H

#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: ModelBakery's loadBlockModels over the vanilla ids - bakes
// block/stone and block/dirt into the manager (idempotent like the registry).
// Returns 0 when the atlas cannot resolve a sprite.
int LIBMATTI_MC_VanillaBlockModels_Bootstrap(LIBMATTI_MC_ModelManager *manager);

#ifdef __cplusplus
}
#endif

#endif
