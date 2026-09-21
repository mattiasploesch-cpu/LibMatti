// Ports of net.neoforged.fml.loading.mixin.FMLClassProvider,
// net.neoforged.fml.loading.mixin.FMLMixinFeatureValidator and
// net.neoforged.fml.loading.mixin.FMLMixinAdviceProvider.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSPROVIDER_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSPROVIDER_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: class FMLClassProvider implements IClassProvider
LIBMATTI_SP_IClassProvider *LIBMATTI_FML_FMLClassProvider_Instance(void);
// Java: class FMLMixinFeatureValidator implements IFeatureValidator
LIBMATTI_SP_IFeatureValidator *LIBMATTI_FML_FMLMixinFeatureValidator_Instance(void);
// Java: class FMLMixinAdviceProvider implements IAdviceProvider
LIBMATTI_SP_IAdviceProvider *LIBMATTI_FML_FMLMixinAdviceProvider_Instance(void);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSPROVIDER_H
