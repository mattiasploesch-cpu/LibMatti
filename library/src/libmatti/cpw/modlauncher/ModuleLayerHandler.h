//
// Port of cpw.mods.modlauncher.ModuleLayerHandler.
//

#ifndef MATTICRAFT_MODLAUNCHER_MODULELAYERHANDLER_H
#define MATTICRAFT_MODLAUNCHER_MODULELAYERHANDLER_H

#include "libmatti/bsl/sjh/cl/ModuleClassLoader.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/cpw/modlauncher/api/IModuleLayerManager.h"
#include "libmatti/cpw/modlauncher/api/NamedPath.h"

#include <stddef.h>

// Java: record LayerInfo(ModuleLayer layer, ModuleClassLoader cl)
typedef struct
{
    LIBMATTI_JL_ModuleLayer *layer;
    LIBMATTI_CL_ModuleClassLoader *cl;
} LIBMATTI_ML_ModuleLayerHandler_LayerInfo;

// Java: public final class ModuleLayerHandler implements IModuleLayerManager
typedef struct LIBMATTI_ML_ModuleLayerHandler LIBMATTI_ML_ModuleLayerHandler;

// Java: ModuleLayerHandler()
LIBMATTI_ML_ModuleLayerHandler *LIBMATTI_ML_ModuleLayerHandler_New(void);
void LIBMATTI_ML_ModuleLayerHandler_Free(LIBMATTI_ML_ModuleLayerHandler *handler);

// Java: ClassLoader classLoader = getClass().getClassLoader();
// The C port has no ambient class loader, so the host registers the loader that
// ModLauncher itself was loaded with (the stage-1 seam). Unset falls back to the
// Javas else-branch: new ModuleClassLoader("BOOT", ModuleLayer.boot().configuration(), List.of()).
void LIBMATTI_ML_ModuleLayerHandler_SetBootClassLoader(LIBMATTI_CL_ModuleClassLoader *classLoader);

// Java: void addToLayer(Layer layer, SecureJar jar)
void LIBMATTI_ML_ModuleLayerHandler_AddToLayer(LIBMATTI_ML_ModuleLayerHandler *handler,
                                               LIBMATTI_MLA_Layer layer, LIBMATTI_JH_SecureJar *jar);
// Java: void addToLayer(Layer layer, NamedPath namedPath) - a second overload,
// which C expresses through the distinct name
void LIBMATTI_ML_ModuleLayerHandler_AddNamedPathToLayer(LIBMATTI_ML_ModuleLayerHandler *handler,
                                                        LIBMATTI_MLA_Layer layer, LIBMATTI_MLA_NamedPath *namedPath);

// Java: public LayerInfo buildLayer(Layer layer, BiFunction<...> classLoaderSupplier)
LIBMATTI_ML_ModuleLayerHandler_LayerInfo LIBMATTI_ML_ModuleLayerHandler_BuildLayerWith(
    LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer,
    LIBMATTI_CL_ModuleClassLoader *(*classLoaderSupplier)(LIBMATTI_JL_Configuration *configuration,
                                                          LIBMATTI_JL_ModuleLayer **parents, size_t parentCount,
                                                          void *userdata),
    void *userdata);
// Java: public LayerInfo buildLayer(Layer layer)
LIBMATTI_ML_ModuleLayerHandler_LayerInfo LIBMATTI_ML_ModuleLayerHandler_BuildLayer(
    LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer);

// Java: public Optional<ModuleLayer> getLayer(Layer layer) - NULL means empty
LIBMATTI_JL_ModuleLayer *LIBMATTI_ML_ModuleLayerHandler_GetLayer(LIBMATTI_ML_ModuleLayerHandler *handler,
                                                                 LIBMATTI_MLA_Layer layer);
// Java: public void updateLayer(Layer layer, Consumer<LayerInfo> action)
void LIBMATTI_ML_ModuleLayerHandler_UpdateLayer(LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer,
                                                void (*action)(LIBMATTI_ML_ModuleLayerHandler_LayerInfo *layerInfo,
                                                               void *userdata),
                                                void *userdata);

#endif //MATTICRAFT_MODLAUNCHER_MODULELAYERHANDLER_H
