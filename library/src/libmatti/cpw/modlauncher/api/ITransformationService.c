// Port of cpw.mods.modlauncher.api.ITransformationService.

#include "libmatti/cpw/modlauncher/api/ITransformationService.h"

const char *LIBMATTI_MLA_ITransformationService_Name(LIBMATTI_MLA_ITransformationService *service)
{
    return service->name(service);
}

void LIBMATTI_MLA_ITransformationService_Arguments(
    LIBMATTI_MLA_ITransformationService *service,
    LIBMATTI_JOPT_OptionSpec *(*builder)(const char *name, const char *description, void *userdata), void *userdata)
{
    // Java: default void arguments(...) {}
    if (service->arguments != NULL) service->arguments(service, builder, userdata);
}

void LIBMATTI_MLA_ITransformationService_ArgumentValues(LIBMATTI_MLA_ITransformationService *service,
                                                        LIBMATTI_MLA_ITransformationService_OptionResult *option)
{
    // Java: default void argumentValues(OptionResult option) {}
    if (service->argumentValues != NULL) service->argumentValues(service, option);
}

void LIBMATTI_MLA_ITransformationService_Initialize(LIBMATTI_MLA_ITransformationService *service,
                                                    LIBMATTI_MLA_IEnvironment *environment)
{
    service->initialize(service, environment);
}

int LIBMATTI_MLA_ITransformationService_OnLoad(LIBMATTI_MLA_ITransformationService *service,
                                               LIBMATTI_MLA_IEnvironment *env, char **otherServices,
                                               size_t otherServiceCount)
{
    return service->onLoad(service, env, otherServices, otherServiceCount);
}

LIBMATTI_MLA_ITransformer **LIBMATTI_MLA_ITransformationService_Transformers(
    LIBMATTI_MLA_ITransformationService *service, size_t *count)
{
    return service->transformers(service, count);
}

LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_MLA_ITransformationService_BeginScanning(
    LIBMATTI_MLA_ITransformationService *service, LIBMATTI_MLA_IEnvironment *environment, size_t *count)
{
    // Java: default List<Resource> beginScanning(...) { return List.of(); }
    if (service->beginScanning == NULL)
    {
        *count = 0;
        return NULL;
    }
    return service->beginScanning(service, environment, count);
}

LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_MLA_ITransformationService_CompleteScan(
    LIBMATTI_MLA_ITransformationService *service, LIBMATTI_MLA_IModuleLayerManager *layerManager, size_t *count)
{
    // Java: default List<Resource> completeScan(...) { return List.of(); }
    if (service->completeScan == NULL)
    {
        *count = 0;
        return NULL;
    }
    return service->completeScan(service, layerManager, count);
}
