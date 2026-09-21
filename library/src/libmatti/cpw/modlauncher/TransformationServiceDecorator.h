// Port of cpw.mods.modlauncher.TransformationServiceDecorator.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICEDECORATOR_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICEDECORATOR_H

#include "libmatti/cpw/modlauncher/Environment.h"
#include "libmatti/cpw/modlauncher/TransformStore.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"

#include <stddef.h>

// Java: public class TransformationServiceDecorator
typedef struct LIBMATTI_ML_TransformationServiceDecorator
{
    LIBMATTI_MLA_ITransformationService *service;
    int isValid;
} LIBMATTI_ML_TransformationServiceDecorator;

// Java: public TransformationServiceDecorator(ITransformationService service)
LIBMATTI_ML_TransformationServiceDecorator *LIBMATTI_ML_TransformationServiceDecorator_New(
    LIBMATTI_MLA_ITransformationService *service);
void LIBMATTI_ML_TransformationServiceDecorator_Free(LIBMATTI_ML_TransformationServiceDecorator *decorator);

// Java: void onLoad(IEnvironment env, Set<String> otherServices)
void LIBMATTI_ML_TransformationServiceDecorator_OnLoad(LIBMATTI_ML_TransformationServiceDecorator *decorator,
                                                       LIBMATTI_ML_Environment *env, char **otherServices,
                                                       size_t otherServiceCount);
// Java: boolean isValid()
int LIBMATTI_ML_TransformationServiceDecorator_IsValid(const LIBMATTI_ML_TransformationServiceDecorator *decorator);
// Java: void onInitialize(IEnvironment environment)
void LIBMATTI_ML_TransformationServiceDecorator_OnInitialize(LIBMATTI_ML_TransformationServiceDecorator *decorator,
                                                             LIBMATTI_ML_Environment *environment);
// Java: public void gatherTransformers(TransformStore transformStore)
void LIBMATTI_ML_TransformationServiceDecorator_GatherTransformers(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_ML_TransformStore *transformStore);
// Java: ITransformationService getService()
LIBMATTI_MLA_ITransformationService *LIBMATTI_ML_TransformationServiceDecorator_GetService(
    const LIBMATTI_ML_TransformationServiceDecorator *decorator);
// Java: List<ITransformationService.Resource> runScan(Environment environment)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServiceDecorator_RunScan(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_ML_Environment *environment, size_t *count);
// Java: public List<ITransformationService.Resource> onCompleteScan(IModuleLayerManager moduleLayerManager)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServiceDecorator_OnCompleteScan(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_MLA_IModuleLayerManager *moduleLayerManager,
    size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMATIONSERVICEDECORATOR_H
