//
// Port of cpw.mods.modlauncher.api.IModuleLayerManager.
//

#ifndef MATTICRAFT_MODLAUNCHER_IMODULELAYERMANAGER_H
#define MATTICRAFT_MODLAUNCHER_IMODULELAYERMANAGER_H

#include "libmatti/java/lang/ModuleLayer.h"

#include <stddef.h>

// Java: enum Layer { BOOT(), SERVICE(BOOT), PLUGIN(BOOT), GAME(PLUGIN, SERVICE) }
typedef enum
{
    LIBMATTI_MLA_LAYER_BOOT = 0,
    LIBMATTI_MLA_LAYER_SERVICE,
    LIBMATTI_MLA_LAYER_PLUGIN,
    LIBMATTI_MLA_LAYER_GAME
} LIBMATTI_MLA_Layer;

// Java: Layer.getParent()
const LIBMATTI_MLA_Layer *LIBMATTI_MLA_Layer_GetParent(LIBMATTI_MLA_Layer layer, size_t *count);
// Java: Layer.name()
const char *LIBMATTI_MLA_Layer_Name(LIBMATTI_MLA_Layer layer);

// Java: interface IModuleLayerManager { Optional<ModuleLayer> getLayer(Layer); }
typedef struct LIBMATTI_MLA_IModuleLayerManager LIBMATTI_MLA_IModuleLayerManager;

// Java: Optional<ModuleLayer> getLayer(Layer layer) - NULL means Optional.empty()
LIBMATTI_JL_ModuleLayer *LIBMATTI_MLA_IModuleLayerManager_GetLayer(LIBMATTI_MLA_IModuleLayerManager *manager,
                                                                   LIBMATTI_MLA_Layer layer);

#endif //MATTICRAFT_MODLAUNCHER_IMODULELAYERMANAGER_H
