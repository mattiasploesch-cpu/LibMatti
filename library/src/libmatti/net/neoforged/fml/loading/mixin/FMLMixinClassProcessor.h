// Ports of net.neoforged.fml.loading.mixin.FMLMixinClassProcessor and
// net.neoforged.fml.loading.mixin.FMLMixinGeneratingClassProcessor.
// Java's processors weave mixin bytecode; the port's mixin backend is the native hook table,
// so the processors resolve to the M4/M2 collectors behind IMixinTransformer's port.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINCLASSPROCESSOR_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINCLASSPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: public FMLMixinClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinClassProcessor_New(void);
// Java: public FMLMixinGeneratingClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinGeneratingClassProcessor_New(void);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINCLASSPROCESSOR_H
