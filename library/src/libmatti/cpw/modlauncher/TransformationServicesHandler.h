// Port of cpw.mods.modlauncher.TransformationServicesHandler.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICESHANDLER_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICESHANDLER_H

#include "libmatti/cpw/modlauncher/ArgumentHandler.h"
#include "libmatti/cpw/modlauncher/Environment.h"
#include "libmatti/cpw/modlauncher/LaunchPluginHandler.h"
#include "libmatti/cpw/modlauncher/ModuleLayerHandler.h"
#include "libmatti/cpw/modlauncher/TransformStore.h"
#include "libmatti/cpw/modlauncher/TransformationServiceDecorator.h"
#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"

#include <stddef.h>

// Java: class TransformationServicesHandler
typedef struct LIBMATTI_ML_TransformationServicesHandler
{
    // Java: Map<String, TransformationServiceDecorator> serviceLookup
    char **names;
    LIBMATTI_ML_TransformationServiceDecorator **services;
    size_t count;
    LIBMATTI_ML_TransformStore *transformStore;
    LIBMATTI_ML_ModuleLayerHandler *layerHandler;
} LIBMATTI_ML_TransformationServicesHandler;

// Java: TransformationServicesHandler(TransformStore transformStore, ModuleLayerHandler layerHandler)
LIBMATTI_ML_TransformationServicesHandler *LIBMATTI_ML_TransformationServicesHandler_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_ModuleLayerHandler *layerHandler);
void LIBMATTI_ML_TransformationServicesHandler_Free(LIBMATTI_ML_TransformationServicesHandler *handler);

// Java: List<ITransformationService.Resource> initializeTransformationServices(ArgumentHandler, Environment)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServicesHandler_InitializeTransformationServices(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_ArgumentHandler *argumentHandler,
    LIBMATTI_ML_Environment *environment, size_t *count);

// Java: TransformingClassLoader buildTransformingClassLoader(LaunchPluginHandler, Environment, ModuleLayerHandler)
LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformationServicesHandler_BuildTransformingClassLoader(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_ML_ModuleLayerHandler *layerHandler);

// Java: void initialiseServiceTransformers()
void LIBMATTI_ML_TransformationServicesHandler_InitialiseServiceTransformers(
    LIBMATTI_ML_TransformationServicesHandler *handler);

// Java: void discoverServices(ArgumentHandler.DiscoveryData discoveryData)
void LIBMATTI_ML_TransformationServicesHandler_DiscoverServices(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_ArgumentHandler_DiscoveryData discoveryData);

// Java: public List<Resource> triggerScanCompletion(IModuleLayerManager moduleLayerManager)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServicesHandler_TriggerScanCompletion(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_MLA_IModuleLayerManager *moduleLayerManager,
    size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICESHANDLER_H
