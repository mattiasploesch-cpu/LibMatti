// Port of net.neoforged.fml.loading.mixin.FMLMixinGlobalProperties.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINGLOBALPROPERTIES_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINGLOBALPROPERTIES_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: public class FMLMixinGlobalProperties implements IGlobalPropertyService
LIBMATTI_SP_IGlobalPropertyService *LIBMATTI_FML_FMLMixinGlobalProperties_Instance(void);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINGLOBALPROPERTIES_H
