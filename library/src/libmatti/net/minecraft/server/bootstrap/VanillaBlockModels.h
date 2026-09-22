// Port of the ModelBakery's vanilla block model bootstrap slice: the demo
// level's models (stone, dirt) baked against the atlas. Java reads them from
// the resource pack; the port bakes the embedded VanillaModels JSONs.

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

#endif //MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKMODELS_H
