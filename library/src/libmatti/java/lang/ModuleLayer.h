//
// Port of java.lang.ModuleLayer.
// Java creates layers from a java.lang.module.Configuration via
// ModuleLayer.defineModules, which is JVM machinery (class loading into a
// layer); the C port keeps the structure (name, configuration, parents) and the
// module-to-classloader mapping.
//

#ifndef MATTICRAFT_MODULELAYER_H
#define MATTICRAFT_MODULELAYER_H

#include "libmatti/java/lang/module/Configuration.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_ModuleLayer LIBMATTI_JL_ModuleLayer;
typedef struct LIBMATTI_JL_ModuleLayerController LIBMATTI_JL_ModuleLayerController;

// Java: static ModuleLayer boot()
LIBMATTI_JL_ModuleLayer *LIBMATTI_JL_ModuleLayer_Boot(void);
// Java: String name()
const char *LIBMATTI_JL_ModuleLayer_Name(const LIBMATTI_JL_ModuleLayer *layer);
// Java: Configuration configuration()
LIBMATTI_JL_Configuration *LIBMATTI_JL_ModuleLayer_Configuration(const LIBMATTI_JL_ModuleLayer *layer);
// Java: List<ModuleLayer> parents()
LIBMATTI_JL_ModuleLayer **LIBMATTI_JL_ModuleLayer_Parents(const LIBMATTI_JL_ModuleLayer *layer, size_t *count);

// Java: static ModuleLayer.Controller defineModules(Configuration cf, List<ModuleLayer> parents, Function<String, ClassLoader> clf)
// The class loader is an opaque handle (the C port's ModuleClassLoader).
LIBMATTI_JL_ModuleLayerController *LIBMATTI_JL_ModuleLayer_DefineModules(
    LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parents, size_t parentCount,
    void *(*classLoaderFor)(const char *moduleName, void *userdata), void *userdata);
// Java: ModuleLayer.Controller.layer()
LIBMATTI_JL_ModuleLayer *LIBMATTI_JL_ModuleLayerController_Layer(const LIBMATTI_JL_ModuleLayerController *controller);

// Java has no teardown for layers; the C port releases what DefineModules allocated.
void LIBMATTI_JL_ModuleLayer_Free(LIBMATTI_JL_ModuleLayer *layer);
void LIBMATTI_JL_ModuleLayerController_Free(LIBMATTI_JL_ModuleLayerController *controller);

#endif //MATTICRAFT_MODULELAYER_H
