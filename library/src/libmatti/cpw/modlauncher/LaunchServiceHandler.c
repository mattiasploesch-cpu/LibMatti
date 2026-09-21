// Port of cpw.mods.modlauncher.LaunchServiceHandler.

#include "libmatti/cpw/modlauncher/LaunchServiceHandler.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/cpw/modlauncher/util/ServiceLoaderUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: sce -> LOGGER.fatal("Encountered serious error loading transformation service, expect problems", sce)
static void log_service_error(const char *message, void *userdata)
{
    (void)userdata;
    LIBMATTI_ML_Logger_Fatal(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Encountered serious error loading transformation service, expect problems: {}", message);
}

LIBMATTI_ML_LaunchServiceHandler *LIBMATTI_ML_LaunchServiceHandler_New(LIBMATTI_ML_ModuleLayerHandler *layerHandler)
{
    LIBMATTI_ML_LaunchServiceHandler *handler = calloc(1, sizeof(LIBMATTI_ML_LaunchServiceHandler));

    // Java: ServiceLoaderUtils.streamServiceLoader(() -> ServiceLoader.load(
    //          layerHandler.getLayer(Layer.BOOT).orElseThrow(), ILaunchHandlerService.class), ...)
    //          .collect(Collectors.toMap(ILaunchHandlerService::name, LaunchServiceHandlerDecorator::new))
    LIBMATTI_JL_ModuleLayer *bootLayer = LIBMATTI_ML_ModuleLayerHandler_GetLayer(layerHandler,
                                                                                 LIBMATTI_MLA_LAYER_BOOT);
    if (bootLayer == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "No BOOT layer available, cannot load launch services");
        return handler;
    }

    size_t count = 0;
    void **services = LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
        bootLayer, "cpw.mods.modlauncher.api.ILaunchHandlerService", log_service_error, NULL, &count);

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MLA_ILaunchHandlerService *service = services[i];

        handler->names = realloc(handler->names, sizeof(*handler->names) * (handler->count + 1));
        handler->decorators = realloc(handler->decorators, sizeof(*handler->decorators) * (handler->count + 1));
        handler->names[handler->count] = strdup(LIBMATTI_MLA_ILaunchHandlerService_Name(service));
        handler->decorators[handler->count].service = service;
        handler->count++;
    }
    free(services);

    // Java: () -> String.join(",", launchHandlerLookup.keySet())
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
                             "Found launch services [{}]", names);
    free(names);
    return handler;
}

void LIBMATTI_ML_LaunchServiceHandler_Free(LIBMATTI_ML_LaunchServiceHandler *handler)
{
    if (handler == NULL) return;
    for (size_t i = 0; i < handler->count; i++) free(handler->names[i]);
    free(handler->names);
    free(handler->decorators);
    free(handler);
}

static LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator_for(LIBMATTI_ML_LaunchServiceHandler *handler,
                                                               const char *name)
{
    // Java: HashMap.get(null) / containsKey(null) - an absent target is not found
    if (name == NULL) return NULL;

    for (size_t i = 0; i < handler->count; i++)
        if (strcmp(handler->names[i], name) == 0) return &handler->decorators[i];
    return NULL;
}

LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_LaunchServiceHandler_FindLaunchHandler(
    LIBMATTI_ML_LaunchServiceHandler *handler, const char *name)
{
    LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator = decorator_for(handler, name);
    // Java: Optional.ofNullable(lookup.getOrDefault(name, null)).map(LaunchServiceHandlerDecorator::service)
    return decorator != NULL ? LIBMATTI_ML_LaunchServiceHandlerDecorator_Service(decorator) : NULL;
}

// Java: static List<String> hideAccessToken(String[] arguments)
char **LIBMATTI_ML_LaunchServiceHandler_HideAccessToken(int argc, char *argv[], size_t *count)
{
    // Java: "❄❄❄❄❄❄❄❄"
    static const char *SNOWFLAKES = "\xE2\x9D\x84\xE2\x9D\x84\xE2\x9D\x84\xE2\x9D\x84"
                                   "\xE2\x9D\x84\xE2\x9D\x84\xE2\x9D\x84\xE2\x9D\x84";

    char **output = malloc(sizeof(*output) * (size_t)(argc > 0 ? argc : 1));
    size_t outputCount = 0;

    for (int i = 0; i < argc; i++)
    {
        if (i > 0 && strcmp(argv[i - 1], "--accessToken") == 0) output[outputCount++] = strdup(SNOWFLAKES);
        else output[outputCount++] = strdup(argv[i]);
    }

    *count = outputCount;
    return output;
}

// Java: private void launch(String target, String[] arguments, ModuleLayer gameLayer,
//                           TransformingClassLoader classLoader, LaunchPluginHandler launchPluginHandler)
// The port returns the game's exit status.
static int launch_target(LIBMATTI_ML_LaunchServiceHandler *handler, const char *target, int argc, char *argv[],
                         LIBMATTI_JL_ModuleLayer *gameLayer, LIBMATTI_ML_TransformingClassLoader *classLoader,
                         LIBMATTI_ML_LaunchPluginHandler *launchPluginHandler)
{
    LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator = decorator_for(handler, target);

    if (decorator == NULL)
    {
        // Java: launchHandlerLookup.get(target) -> NPE
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Cannot find launch target {}, unable to launch", target);
        return 1;
    }

    // Java: final NamedPath[] paths = launchServiceHandlerDecorator.service().getPaths()
    //       launchPluginHandler.announceLaunch(classLoader, paths)
    size_t specialPathCount = 0;
    LIBMATTI_MLA_NamedPath *paths = LIBMATTI_MLA_ILaunchHandlerService_GetPaths(decorator->service,
                                                                                &specialPathCount);
    LIBMATTI_ML_LaunchPluginHandler_AnnounceLaunch(launchPluginHandler, classLoader, paths, specialPathCount);

    size_t hiddenCount = 0;
    char **hidden = LIBMATTI_ML_LaunchServiceHandler_HideAccessToken(argc, argv, &hiddenCount);
    // Java: List.toString() -> "[a, b]"
    size_t length = 3;
    for (size_t i = 0; i < hiddenCount; i++) length += strlen(hidden[i]) + 2;
    char *arguments = malloc(length);
    arguments[0] = '[';
    arguments[1] = '\0';
    for (size_t i = 0; i < hiddenCount; i++)
    {
        if (i > 0) strcat(arguments, ", ");
        strcat(arguments, hidden[i]);
    }
    strcat(arguments, "]");

    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "Launching target '{}' with arguments {}", target, arguments);

    for (size_t i = 0; i < hiddenCount; i++) free(hidden[i]);
    free(hidden);
    free(arguments);

    return LIBMATTI_ML_LaunchServiceHandlerDecorator_Launch(decorator, argc, argv, gameLayer);
}

// Java: public void launch(ArgumentHandler argumentHandler, ModuleLayer gameLayer,
//                          TransformingClassLoader classLoader, LaunchPluginHandler launchPluginHandler)
// The port returns the game's exit status.
int LIBMATTI_ML_LaunchServiceHandler_Launch(LIBMATTI_ML_LaunchServiceHandler *handler,
                                            LIBMATTI_ML_ArgumentHandler *argumentHandler,
                                            LIBMATTI_JL_ModuleLayer *gameLayer,
                                            LIBMATTI_ML_TransformingClassLoader *classLoader,
                                            LIBMATTI_ML_LaunchPluginHandler *launchPluginHandler)
{
    // Java: String launchTarget = argumentHandler.getLaunchTarget(); String[] args = argumentHandler.buildArgumentList()
    const char *launchTarget = LIBMATTI_ML_ArgumentHandler_GetLaunchTarget(argumentHandler);
    size_t argCount = 0;
    char **args = LIBMATTI_ML_ArgumentHandler_BuildArgumentList(argumentHandler, &argCount);

    int exitCode = launch_target(handler, launchTarget, (int)argCount, args, gameLayer, classLoader,
                                 launchPluginHandler);
    for (size_t i = 0; i < argCount; i++) free(args[i]);
    free(args);
    return exitCode;
}

int LIBMATTI_ML_LaunchServiceHandler_ValidateLaunchTarget(LIBMATTI_ML_LaunchServiceHandler *handler,
                                                          LIBMATTI_ML_ArgumentHandler *argumentHandler)
{
    const char *launchTarget = LIBMATTI_ML_ArgumentHandler_GetLaunchTarget(argumentHandler);

    if (decorator_for(handler, launchTarget) == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Cannot find launch target {}, unable to launch",
                                 launchTarget != NULL ? launchTarget : "");
        // Java: throw new RuntimeException("Cannot find launch target")
        return 0;
    }

    return 1;
}
