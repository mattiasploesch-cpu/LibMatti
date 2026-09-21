// Port of cpw.mods.modlauncher.TransformationServiceDecorator.

#include "libmatti/cpw/modlauncher/TransformationServiceDecorator.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/cpw/modlauncher/TransformTargetLabel.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ML_TransformationServiceDecorator *LIBMATTI_ML_TransformationServiceDecorator_New(
    LIBMATTI_MLA_ITransformationService *service)
{
    LIBMATTI_ML_TransformationServiceDecorator *decorator = calloc(1,
                                                                   sizeof(LIBMATTI_ML_TransformationServiceDecorator));
    decorator->service = service;
    return decorator;
}

void LIBMATTI_ML_TransformationServiceDecorator_Free(LIBMATTI_ML_TransformationServiceDecorator *decorator)
{
    free(decorator);
}

// Java: void onLoad(IEnvironment env, Set<String> otherServices)
void LIBMATTI_ML_TransformationServiceDecorator_OnLoad(LIBMATTI_ML_TransformationServiceDecorator *decorator,
                                                       LIBMATTI_ML_Environment *env, char **otherServices,
                                                       size_t otherServiceCount)
{
    // Java: try { service.onLoad(env, otherServices); isValid = true; }
    //       catch (IncompatibleEnvironmentException e) { LOGGER.error(...); isValid = false; }
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER, "Loading service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));

    if (LIBMATTI_MLA_ITransformationService_OnLoad(decorator->service, (LIBMATTI_MLA_IEnvironment *)env, otherServices,
                                                   otherServiceCount))
    {
        decorator->isValid = 1;
        LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Loaded service {}", LIBMATTI_MLA_ITransformationService_Name(decorator->service));
    }
    else
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Service failed to load {}", LIBMATTI_MLA_ITransformationService_Name(decorator->service));
        decorator->isValid = 0;
    }
}

int LIBMATTI_ML_TransformationServiceDecorator_IsValid(const LIBMATTI_ML_TransformationServiceDecorator *decorator)
{
    return decorator->isValid;
}

void LIBMATTI_ML_TransformationServiceDecorator_OnInitialize(LIBMATTI_ML_TransformationServiceDecorator *decorator,
                                                             LIBMATTI_ML_Environment *environment)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Initializing transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));
    LIBMATTI_MLA_ITransformationService_Initialize(decorator->service, (LIBMATTI_MLA_IEnvironment *)environment);
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Initialized transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));
}

// Java: public void gatherTransformers(TransformStore transformStore)
void LIBMATTI_ML_TransformationServiceDecorator_GatherTransformers(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_ML_TransformStore *transformStore)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Initializing transformers for transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));

    size_t transformerCount = 0;
    LIBMATTI_MLA_ITransformer **transformers =
        LIBMATTI_MLA_ITransformationService_Transformers(decorator->service, &transformerCount);

    for (size_t i = 0; i < transformerCount; i++)
    {
        LIBMATTI_MLA_ITransformer *xform = transformers[i];

        // Java: final TargetType<?> targetType = xform.getTargetType()
        const LIBMATTI_MLA_TargetType *targetType = LIBMATTI_MLA_ITransformer_GetTargetType(xform);

        size_t targetCount = 0;
        LIBMATTI_MLA_ITransformer_Target *targets = LIBMATTI_MLA_ITransformer_Targets(xform, &targetCount);

        if (targetCount == 0) continue;

        // Java: targets.stream().map(TransformTargetLabel::new).collect(Collectors.groupingBy(TransformTargetLabel::getTargetType))
        LIBMATTI_ML_TransformTargetLabel **labels = malloc(sizeof(*labels) * targetCount);
        for (size_t t = 0; t < targetCount; t++) labels[t] = LIBMATTI_ML_TransformTargetLabel_NewFromTarget(&targets[t]);

        size_t groupCount = 0;
        const LIBMATTI_MLA_TargetType **groupTypes = NULL;
        for (size_t t = 0; t < targetCount; t++)
        {
            const LIBMATTI_MLA_TargetType *labelType = LIBMATTI_ML_TransformTargetLabel_GetTargetType(labels[t]);

            int known = 0;
            for (size_t g = 0; g < groupCount; g++) if (groupTypes[g] == labelType) known = 1;
            if (!known)
            {
                groupTypes = realloc(groupTypes, sizeof(*groupTypes) * (groupCount + 1));
                groupTypes[groupCount++] = labelType;
            }
        }

        // Java: if (targetTypeListMap.keySet().size() > 1 || !targetTypeListMap.containsKey(targetType))
        int hasTargetType = 0;
        for (size_t g = 0; g < groupCount; g++) if (groupTypes[g] == targetType) hasTargetType = 1;

        if (groupCount > 1 || !hasTargetType)
        {
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                     "Invalid target {} for transformer {}", targetType != NULL ? targetType->name : "",
                                     "");
            // Java: throw new IllegalArgumentException("The transformer contains invalid targets")
            for (size_t t = 0; t < targetCount; t++) LIBMATTI_ML_TransformTargetLabel_Free(labels[t]);
            free(labels);
            free(groupTypes);
            continue;
        }

        // Java: targetTypeListMap.values().stream().flatMap(Collection::stream).forEach(target -> transformStore.addTransformer(target, xform, service))
        for (size_t t = 0; t < targetCount; t++)
            LIBMATTI_ML_TransformStore_AddTransformer(transformStore, labels[t], xform, decorator->service);

        free(labels);
        free(groupTypes);
    }

    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Initialized transformers for transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));
}

LIBMATTI_MLA_ITransformationService *LIBMATTI_ML_TransformationServiceDecorator_GetService(
    const LIBMATTI_ML_TransformationServiceDecorator *decorator)
{
    return decorator->service;
}

// Java: List<ITransformationService.Resource> runScan(Environment environment)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServiceDecorator_RunScan(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_ML_Environment *environment, size_t *count)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Beginning scan trigger - transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));

    LIBMATTI_MLA_ITransformationService_Resource *resources =
        LIBMATTI_MLA_ITransformationService_BeginScanning(decorator->service, (LIBMATTI_MLA_IEnvironment *)environment,
                                                          count);

    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "End scan trigger - transformation service {}",
                             LIBMATTI_MLA_ITransformationService_Name(decorator->service));
    return resources;
}

// Java: public List<ITransformationService.Resource> onCompleteScan(IModuleLayerManager moduleLayerManager)
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServiceDecorator_OnCompleteScan(
    LIBMATTI_ML_TransformationServiceDecorator *decorator, LIBMATTI_MLA_IModuleLayerManager *moduleLayerManager,
    size_t *count)
{
    return LIBMATTI_MLA_ITransformationService_CompleteScan(decorator->service, moduleLayerManager, count);
}
