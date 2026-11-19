





#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINCLASSPROCESSOR_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINCLASSPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: public FMLMixinClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinClassProcessor_New(void);
// Java: public FMLMixinGeneratingClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinGeneratingClassProcessor_New(void);

#endif
