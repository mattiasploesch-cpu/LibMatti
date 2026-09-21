//
// Port of java.lang.ModuleLayer.
//

#include "libmatti/java/lang/ModuleLayer.h"

#include "libmatti/java/lang/module/ResolvedModule.h"

#include <stdlib.h>
#include <string.h>

struct LIBMATTI_JL_ModuleLayer
{
    char *name;
    LIBMATTI_JL_Configuration *configuration;
    LIBMATTI_JL_ModuleLayer **parents;
    size_t parentCount;
    // Java: the module -> ClassLoader mapping established by defineModules
    char **moduleNames;
    void **loaders;
    size_t moduleCount;
};

struct LIBMATTI_JL_ModuleLayerController
{
    LIBMATTI_JL_ModuleLayer *layer;
};

static LIBMATTI_JL_ModuleLayer *bootLayer = NULL;

// Java: static ModuleLayer boot()
LIBMATTI_JL_ModuleLayer *LIBMATTI_JL_ModuleLayer_Boot(void)
{
    if (bootLayer == NULL)
    {
        bootLayer = calloc(1, sizeof(LIBMATTI_JL_ModuleLayer));

        // Java: the boot layer has no name
        bootLayer->name = NULL;

        // Java: the JVM's boot configuration; the C port has no boot modules
        bootLayer->configuration = calloc(1, sizeof(LIBMATTI_JL_Configuration));
    }

    return bootLayer;
}

// Java: String name()
const char *LIBMATTI_JL_ModuleLayer_Name(const LIBMATTI_JL_ModuleLayer *layer)
{
    return layer->name;
}

// Java: Configuration configuration()
LIBMATTI_JL_Configuration *LIBMATTI_JL_ModuleLayer_Configuration(const LIBMATTI_JL_ModuleLayer *layer)
{
    return layer->configuration;
}

// Java: List<ModuleLayer> parents()
LIBMATTI_JL_ModuleLayer **LIBMATTI_JL_ModuleLayer_Parents(const LIBMATTI_JL_ModuleLayer *layer, size_t *count)
{
    *count = layer->parentCount;
    return layer->parents;
}

// Java: static ModuleLayer.Controller defineModules(Configuration cf, List<ModuleLayer> parents,
//                                                   Function<String, ClassLoader> clf)
LIBMATTI_JL_ModuleLayerController *LIBMATTI_JL_ModuleLayer_DefineModules(
    LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parents, size_t parentCount,
    void *(*classLoaderFor)(const char *moduleName, void *userdata), void *userdata)
{
    LIBMATTI_JL_ModuleLayer *layer = calloc(1, sizeof(LIBMATTI_JL_ModuleLayer));
    if (layer == NULL) return NULL;

    layer->configuration = configuration;

    if (parentCount > 0)
    {
        layer->parents = calloc(parentCount, sizeof(*layer->parents));
        for (size_t i = 0; i < parentCount; i++) layer->parents[i] = parents[i];
        layer->parentCount = parentCount;
    }

    // Java: every module of the configuration is mapped to clf.apply(moduleName)
    size_t moduleCount = 0;
    LIBMATTI_JL_ResolvedModule **modules = LIBMATTI_JL_Configuration_Modules(configuration, &moduleCount);

    if (moduleCount > 0)
    {
        layer->moduleNames = calloc(moduleCount, sizeof(char *));
        layer->loaders = calloc(moduleCount, sizeof(void *));

        for (size_t i = 0; i < moduleCount; i++)
        {
            const char *name = LIBMATTI_JL_ResolvedModule_Name(modules[i]);
            layer->moduleNames[i] = strdup(name);
            layer->loaders[i] = classLoaderFor != NULL ? classLoaderFor(name, userdata) : NULL;
        }
        layer->moduleCount = moduleCount;
    }

    LIBMATTI_JL_ModuleLayerController *controller = calloc(1, sizeof(LIBMATTI_JL_ModuleLayerController));
    controller->layer = layer;

    return controller;
}

// Java: ModuleLayer.Controller.layer()
LIBMATTI_JL_ModuleLayer *LIBMATTI_JL_ModuleLayerController_Layer(const LIBMATTI_JL_ModuleLayerController *controller)
{
    return controller->layer;
}

// releases what defineModules allocated (the configuration is owned by the caller)
void LIBMATTI_JL_ModuleLayer_Free(LIBMATTI_JL_ModuleLayer *layer)
{
    if (layer == NULL) return;

    free(layer->name);
    free(layer->parents);

    for (size_t i = 0; i < layer->moduleCount; i++) free(layer->moduleNames[i]);
    free(layer->moduleNames);
    free(layer->loaders);

    free(layer);
}

void LIBMATTI_JL_ModuleLayerController_Free(LIBMATTI_JL_ModuleLayerController *controller)
{
    free(controller);
}
