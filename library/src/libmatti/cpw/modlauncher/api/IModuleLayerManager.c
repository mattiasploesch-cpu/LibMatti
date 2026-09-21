//
// Port of the Layer enum members of cpw.mods.modlauncher.api.IModuleLayerManager.
//

#include "IModuleLayerManager.h"

// Java: enum Layer { BOOT(), SERVICE(BOOT), PLUGIN(BOOT), GAME(PLUGIN, SERVICE) }
static const LIBMATTI_MLA_Layer BOOT_PARENTS[] = {};
static const LIBMATTI_MLA_Layer SERVICE_PARENTS[] = {LIBMATTI_MLA_LAYER_BOOT};
static const LIBMATTI_MLA_Layer PLUGIN_PARENTS[] = {LIBMATTI_MLA_LAYER_BOOT};
static const LIBMATTI_MLA_Layer GAME_PARENTS[] = {LIBMATTI_MLA_LAYER_PLUGIN, LIBMATTI_MLA_LAYER_SERVICE};

// Java: public Layer[] getParent()
const LIBMATTI_MLA_Layer *LIBMATTI_MLA_Layer_GetParent(LIBMATTI_MLA_Layer layer, size_t *count)
{
    switch (layer)
    {
        case LIBMATTI_MLA_LAYER_SERVICE:
            *count = 1;
            return SERVICE_PARENTS;
        case LIBMATTI_MLA_LAYER_PLUGIN:
            *count = 1;
            return PLUGIN_PARENTS;
        case LIBMATTI_MLA_LAYER_GAME:
            *count = 2;
            return GAME_PARENTS;
        case LIBMATTI_MLA_LAYER_BOOT:
        default:
            *count = 0;
            return BOOT_PARENTS;
    }
}

// Java: the enum constant name (Layer.name())
const char *LIBMATTI_MLA_Layer_Name(LIBMATTI_MLA_Layer layer)
{
    switch (layer)
    {
        case LIBMATTI_MLA_LAYER_SERVICE:
            return "SERVICE";
        case LIBMATTI_MLA_LAYER_PLUGIN:
            return "PLUGIN";
        case LIBMATTI_MLA_LAYER_GAME:
            return "GAME";
        case LIBMATTI_MLA_LAYER_BOOT:
        default:
            return "BOOT";
    }
}
