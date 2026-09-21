// Port of cpw.mods.modlauncher.TransformationServicesHandler.

#include "libmatti/cpw/modlauncher/TransformationServicesHandler.h"

#include "Launcher.h"
#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/cpw/modlauncher/serviceapi/ITransformerDiscoveryService.h"
#include "libmatti/cpw/modlauncher/util/ServiceLoaderUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void log_service_error(const char *message, void *userdata)
{
    (void)userdata;
    LIBMATTI_ML_Logger_Fatal(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Encountered serious error loading transformation service, expect problems: {}", message);
}

LIBMATTI_ML_TransformationServicesHandler *LIBMATTI_ML_TransformationServicesHandler_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_ModuleLayerHandler *layerHandler)
{
    LIBMATTI_ML_TransformationServicesHandler *handler = calloc(1,
                                                                sizeof(LIBMATTI_ML_TransformationServicesHandler));
    handler->transformStore = transformStore;
    handler->layerHandler = layerHandler;
    return handler;
}

void LIBMATTI_ML_TransformationServicesHandler_Free(LIBMATTI_ML_TransformationServicesHandler *handler)
{
    if (handler == NULL) return;
    for (size_t i = 0; i < handler->count; i++)
    {
        free(handler->names[i]);
        LIBMATTI_ML_TransformationServiceDecorator_Free(handler->services[i]);
    }
    free(handler->names);
    free(handler->services);
    free(handler);
}

LIBMATTI_ML_TransformationServiceDecorator *service_for(LIBMATTI_ML_TransformationServicesHandler *handler,
                                                        const char *name)
{
    for (size_t i = 0; i < handler->count; i++)
        if (strcmp(handler->names[i], name) == 0) return handler->services[i];
    return NULL;
}

// Java: this.serviceLookup.values().forEach(s -> s.onLoad(environment, serviceLookup.keySet()))
static void load_transformation_services(LIBMATTI_ML_TransformationServicesHandler *handler,
                                         LIBMATTI_ML_Environment *environment)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Transformation services loading");

    for (size_t i = 0; i < handler->count; i++)
        LIBMATTI_ML_TransformationServiceDecorator_OnLoad(handler->services[i], environment, handler->names,
                                                          handler->count);
}

// Java: if (serviceLookup.values().stream().filter(d -> !d.isValid()).count() > 0) { ... throw new InvalidLauncherSetupException(...) }
static void validate_transformation_services(LIBMATTI_ML_TransformationServicesHandler *handler)
{
    size_t invalidCount = 0;
    for (size_t i = 0; i < handler->count; i++)
        if (!LIBMATTI_ML_TransformationServiceDecorator_IsValid(handler->services[i])) invalidCount++;

    if (invalidCount == 0) return;

    LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Found {} services that failed to load : [{}]", "");
    // Java: throw new InvalidLauncherSetupException("Invalid Services found " + names)
}

// Java: private void computeArgumentsForServices(OptionParser parser)
//           service.arguments((a, b) -> parser.accepts(service.name() + "." + a, b))
typedef struct
{
    LIBMATTI_JOPT_OptionParser *parser;
    const char *serviceName;
} ArgumentBuilderContext;

static LIBMATTI_JOPT_OptionSpec *argument_builder(const char *name, const char *description, void *userdata)
{
    ArgumentBuilderContext *context = userdata;

    size_t length = strlen(context->serviceName) + strlen(name) + 2;
    char *optionName = malloc(length);
    sprintf(optionName, "%s.%s", context->serviceName, name);
    LIBMATTI_JOPT_OptionSpec *spec = LIBMATTI_JOPT_OptionParser_Accepts(context->parser, optionName, description);
    free(optionName);

    return spec;
}

static void compute_arguments_for_services(LIBMATTI_JOPT_OptionParser *parser, void *userdata)
{
    LIBMATTI_ML_TransformationServicesHandler *handler = userdata;

    // Java: serviceLookup.values().stream().map(TransformationServiceDecorator::getService)
    //           .forEach(service -> service.arguments((a, b) -> parser.accepts(service.name() + "." + a, b)))
    for (size_t i = 0; i < handler->count; i++)
    {
        LIBMATTI_MLA_ITransformationService *service =
            LIBMATTI_ML_TransformationServiceDecorator_GetService(handler->services[i]);
        ArgumentBuilderContext context = {parser, LIBMATTI_MLA_ITransformationService_Name(service)};
        LIBMATTI_MLA_ITransformationService_Arguments(service, argument_builder, &context);
    }
}

// Java: private void offerArgumentResultsToServices(OptionSet optionSet, BiFunction<...> resultHandler)
static void offer_argument_results_to_services(
    LIBMATTI_JOPT_OptionSet *optionSet,
    LIBMATTI_MLA_ITransformationService_OptionResult *(*resultHandler)(const char *serviceName,
                                                                       LIBMATTI_JOPT_OptionSet *set, void *userdata),
    void *userdata)
{
    LIBMATTI_ML_TransformationServicesHandler *handler = userdata;

    for (size_t i = 0; i < handler->count; i++)
    {
        LIBMATTI_MLA_ITransformationService *service =
            LIBMATTI_ML_TransformationServiceDecorator_GetService(handler->services[i]);
        LIBMATTI_MLA_ITransformationService_OptionResult *result =
            resultHandler(LIBMATTI_MLA_ITransformationService_Name(service), optionSet, NULL);
        LIBMATTI_MLA_ITransformationService_ArgumentValues(service, result);
        LIBMATTI_ML_ArgumentHandler_FreeOptionResults(result);
    }
}

// Java: private void processArguments(ArgumentHandler argumentHandler, Environment environment)
static void process_arguments(LIBMATTI_ML_TransformationServicesHandler *handler,
                              LIBMATTI_ML_ArgumentHandler *argumentHandler, LIBMATTI_ML_Environment *environment)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Configuring option handling for services");

    LIBMATTI_ML_ArgumentHandler_ProcessArguments(argumentHandler, environment, compute_arguments_for_services, handler,
                                                 offer_argument_results_to_services, handler);
}

// Java: serviceLookup.values().forEach(s -> s.onInitialize(environment))
static void initialise_transformation_services(LIBMATTI_ML_TransformationServicesHandler *handler,
                                               LIBMATTI_ML_Environment *environment)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Transformation services initializing");

    for (size_t i = 0; i < handler->count; i++)
        LIBMATTI_ML_TransformationServiceDecorator_OnInitialize(handler->services[i], environment);
}

// Java: serviceLookup.values().stream().map(s -> s.runScan(environment)).mapMulti(Iterable::forEach).toList()
static LIBMATTI_MLA_ITransformationService_Resource *run_scanning_transformation_services(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_Environment *environment, size_t *count)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Transformation services begin scanning");

    LIBMATTI_MLA_ITransformationService_Resource *result = NULL;
    size_t resultCount = 0;

    for (size_t i = 0; i < handler->count; i++)
    {
        size_t resourceCount = 0;
        LIBMATTI_MLA_ITransformationService_Resource *resources =
            LIBMATTI_ML_TransformationServiceDecorator_RunScan(handler->services[i], environment, &resourceCount);

        for (size_t r = 0; r < resourceCount; r++)
        {
            result = realloc(result, sizeof(*result) * (resultCount + 1));
            result[resultCount++] = resources[r];
        }
        // the resource list belongs to the service (Java: the returned List)
    }

    *count = resultCount;
    return result;
}

LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServicesHandler_InitializeTransformationServices(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_ArgumentHandler *argumentHandler,
    LIBMATTI_ML_Environment *environment, size_t *count)
{
    load_transformation_services(handler, environment);
    validate_transformation_services(handler);
    process_arguments(handler, argumentHandler, environment);
    initialise_transformation_services(handler, environment);
    return run_scanning_transformation_services(handler, environment, count);
}

// Java: (cf, parents) -> new TransformingClassLoader(transformStore, pluginHandler, environment, cf, parents)
typedef struct
{
    LIBMATTI_ML_TransformStore *transformStore;
    LIBMATTI_ML_LaunchPluginHandler *pluginHandler;
    LIBMATTI_ML_Environment *environment;
} TransformingClassLoaderContext;

static LIBMATTI_CL_ModuleClassLoader *transforming_class_loader_supplier(LIBMATTI_JL_Configuration *configuration,
                                                                         LIBMATTI_JL_ModuleLayer **parents,
                                                                         size_t parentCount, void *userdata)
{
    TransformingClassLoaderContext *context = userdata;
    return (LIBMATTI_CL_ModuleClassLoader *)LIBMATTI_ML_TransformingClassLoader_NewWithEnvironment(
        context->transformStore, context->pluginHandler, context->environment, configuration, parents, parentCount);
}

// Java: li -> li.cl().setFallbackClassLoader(layerInfo.cl())
typedef struct
{
    LIBMATTI_CL_ModuleClassLoader *fallback;
} FallbackSetterContext;

static void set_fallback_class_loader(LIBMATTI_ML_ModuleLayerHandler_LayerInfo *layerInfo, void *userdata)
{
    FallbackSetterContext *context = userdata;
    LIBMATTI_CL_ModuleClassLoader_SetFallbackClassLoader(layerInfo->cl,
                                                         (LIBMATTI_CL_ClassLoader *)context->fallback);
}

LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformationServicesHandler_BuildTransformingClassLoader(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_ML_ModuleLayerHandler *layerHandler)
{
    // Java: final var layerInfo = layerHandler.buildLayer(Layer.GAME, (cf, parents) -> new TransformingClassLoader(...))
    TransformingClassLoaderContext context = {handler->transformStore, pluginHandler, environment};
    LIBMATTI_ML_ModuleLayerHandler_LayerInfo layerInfo = LIBMATTI_ML_ModuleLayerHandler_BuildLayerWith(
        layerHandler, LIBMATTI_MLA_LAYER_GAME, transforming_class_loader_supplier, &context);

    // Java: layerHandler.updateLayer(Layer.PLUGIN, li -> li.cl().setFallbackClassLoader(layerInfo.cl()))
    FallbackSetterContext fallback = {layerInfo.cl};
    LIBMATTI_ML_ModuleLayerHandler_UpdateLayer(layerHandler, LIBMATTI_MLA_LAYER_PLUGIN, set_fallback_class_loader,
                                               &fallback);

    return (LIBMATTI_ML_TransformingClassLoader *)layerInfo.cl;
}

void LIBMATTI_ML_TransformationServicesHandler_InitialiseServiceTransformers(
    LIBMATTI_ML_TransformationServicesHandler *handler)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Transformation services loading transformers");

    // Java: serviceLookup.values().forEach(s -> s.gatherTransformers(transformStore))
    for (size_t i = 0; i < handler->count; i++)
        LIBMATTI_ML_TransformationServiceDecorator_GatherTransformers(handler->services[i], handler->transformStore);
}

void LIBMATTI_ML_TransformationServicesHandler_DiscoverServices(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_ML_ArgumentHandler_DiscoveryData discoveryData)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Discovering transformation services");

    // Java: var bootLayer = layerHandler.getLayer(IModuleLayerManager.Layer.BOOT).orElseThrow()
    LIBMATTI_JL_ModuleLayer *bootLayer = LIBMATTI_ML_ModuleLayerHandler_GetLayer(handler->layerHandler,
                                                                                 LIBMATTI_MLA_LAYER_BOOT);
    if (bootLayer == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "No BOOT layer available, cannot discover transformation services");
        return;
    }

    // Java: var earlyDiscoveryServices = ServiceLoaderUtils.streamServiceLoader(
    //       () -> ServiceLoader.load(bootLayer, ITransformerDiscoveryService.class), ...).toList()
    size_t discoveryCount = 0;
    void **discoveryServices = LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
        bootLayer, "cpw.mods.modlauncher.serviceapi.ITransformerDiscoveryService", log_service_error, NULL,
        &discoveryCount);

    // Java: additionalPaths = earlyDiscoveryServices.stream()
    //           .map(s -> s.candidates(gameDir, launchTarget)).<NamedPath>mapMulti(Iterable::forEach).toList()
    LIBMATTI_MLA_NamedPath *additionalPaths = NULL;
    size_t additionalPathCount = 0;

    for (size_t i = 0; i < discoveryCount; i++)
    {
        size_t candidateCount = 0;
        LIBMATTI_MLA_NamedPath *candidates = LIBMATTI_MLS_ITransformerDiscoveryService_Candidates(
            discoveryServices[i], discoveryData.gameDir, discoveryData.launchTarget, &candidateCount);

        for (size_t c = 0; c < candidateCount; c++)
        {
            additionalPaths = realloc(additionalPaths, sizeof(*additionalPaths) * (additionalPathCount + 1));
            additionalPaths[additionalPathCount++] = candidates[c];
        }
        free(candidates);
    }

    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Found additional transformation services from discovery services: {}", "");

    // Java: additionalPaths.forEach(np -> layerHandler.addToLayer(Layer.SERVICE, np))
    for (size_t i = 0; i < additionalPathCount; i++)
        LIBMATTI_ML_ModuleLayerHandler_AddNamedPathToLayer(handler->layerHandler, LIBMATTI_MLA_LAYER_SERVICE,
                                                           &additionalPaths[i]);

    // Java: var serviceLayer = layerHandler.buildLayer(Layer.SERVICE)
    LIBMATTI_ML_ModuleLayerHandler_LayerInfo serviceLayer = LIBMATTI_ML_ModuleLayerHandler_BuildLayer(
        handler->layerHandler, LIBMATTI_MLA_LAYER_SERVICE);

    // Java: earlyDiscoveryServices.forEach(s -> s.earlyInitialization(launchTarget, arguments))
    for (size_t i = 0; i < discoveryCount; i++)
        LIBMATTI_MLS_ITransformerDiscoveryService_EarlyInitialization(discoveryServices[i],
                                                                      discoveryData.launchTarget,
                                                                      (int)discoveryData.argumentCount,
                                                                      discoveryData.arguments);
    free(discoveryServices);

    // Java: serviceLookup = ServiceLoaderUtils.streamServiceLoader(
    //       () -> ServiceLoader.load(serviceLayer.layer(), ITransformationService.class), ...)
    //       .collect(Collectors.toMap(ITransformationService::name, TransformationServiceDecorator::new))
    size_t serviceCount = 0;
    void **services = LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
        serviceLayer.layer, "cpw.mods.modlauncher.api.ITransformationService", log_service_error, NULL, &serviceCount);

    for (size_t i = 0; i < serviceCount; i++)
    {
        LIBMATTI_MLA_ITransformationService *service = services[i];

        handler->names = realloc(handler->names, sizeof(*handler->names) * (handler->count + 1));
        handler->services = realloc(handler->services, sizeof(*handler->services) * (handler->count + 1));
        handler->names[handler->count] = strdup(LIBMATTI_MLA_ITransformationService_Name(service));
        handler->services[handler->count] = LIBMATTI_ML_TransformationServiceDecorator_New(service);
        handler->count++;
    }
    free(services);

    // Java: var modlist = serviceLookup.entrySet().stream().map(e -> Map.of("name", ..., "type", "TRANSFORMATIONSERVICE",
    //              "file", ServiceLoaderUtils.fileNameFor(e.getValue().getClass()))).toList()
    //       Launcher.INSTANCE.environment().getProperty(MODLIST).ifPresent(ml -> ml.addAll(modlist))
    if (LIBMATTI_ML_Launcher_INSTANCE != NULL)
    {
        LIBMATTI_MLA_ModList *mods = LIBMATTI_MLA_IEnvironment_GetProperty(
            (LIBMATTI_MLA_IEnvironment *)LIBMATTI_ML_Launcher_INSTANCE->environment,
            LIBMATTI_MLA_IEnvironment_Keys_ModList());

        if (mods != NULL)
        {
            for (size_t i = 0; i < handler->count; i++)
            {
                char *file = LIBMATTI_MLU_ServiceLoaderUtils_FileNameFor(handler->names[i]);
                LIBMATTI_MLA_ModList_Add(mods, handler->names[i], "TRANSFORMATIONSERVICE", file);
                free(file);
            }
        }
    }

    // Java: () -> String.join(",", serviceLookup.keySet())
    size_t namesLength = 1;
    for (size_t i = 0; i < handler->count; i++) namesLength += strlen(handler->names[i]) + 1;
    char *names = malloc(namesLength);
    names[0] = '\0';
    for (size_t i = 0; i < handler->count; i++)
    {
        if (i > 0) strcat(names, ",");
        strcat(names, handler->names[i]);
    }

    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Found transformer services : [{}]", names);
    free(names);
}

LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_ML_TransformationServicesHandler_TriggerScanCompletion(
    LIBMATTI_ML_TransformationServicesHandler *handler, LIBMATTI_MLA_IModuleLayerManager *moduleLayerManager,
    size_t *count)
{
    // Java: serviceLookup.values().stream().map(tsd -> tsd.onCompleteScan(moduleLayerManager))
    //           .<Resource>mapMulti(Iterable::forEach).toList()
    LIBMATTI_MLA_ITransformationService_Resource *result = NULL;
    size_t resultCount = 0;

    for (size_t i = 0; i < handler->count; i++)
    {
        size_t resourceCount = 0;
        LIBMATTI_MLA_ITransformationService_Resource *resources =
            LIBMATTI_ML_TransformationServiceDecorator_OnCompleteScan(handler->services[i], moduleLayerManager,
                                                                      &resourceCount);

        for (size_t r = 0; r < resourceCount; r++)
        {
            result = realloc(result, sizeof(*result) * (resultCount + 1));
            result[resultCount++] = resources[r];
        }
        // the resource list belongs to the service (Java: the returned List)
    }

    *count = resultCount;
    return result;
}
