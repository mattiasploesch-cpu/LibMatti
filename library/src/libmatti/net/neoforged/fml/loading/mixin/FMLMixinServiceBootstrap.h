// Port of net.neoforged.fml.loading.mixin.FMLMixinServiceBootstrap.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICEBOOTSTRAP_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICEBOOTSTRAP_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: public class FMLMixinServiceBootstrap implements IMixinServiceBootstrap
LIBMATTI_SP_IMixinServiceBootstrap *LIBMATTI_FML_FMLMixinServiceBootstrap_Instance(void);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICEBOOTSTRAP_H
