// Port of cpw.mods.modlauncher.DefaultLaunchHandlerService.

#include "libmatti/cpw/modlauncher/DefaultLaunchHandlerService.h"

#include "LogManager.h"
#include "LogMarkers.h"

#include <stdlib.h>
#include <string.h>

// Java: public String name() { return "minecraft"; }
static const char *default_name(LIBMATTI_MLA_ILaunchHandlerService *self)
{
    (void)self;
    return "minecraft";
}

// Java: the ServiceRunner returned by launchService(...)
typedef struct
{
    LIBMATTI_MLA_ServiceRunner base;
    int argc;
    char **argv;
    LIBMATTI_JL_ModuleLayer *gameLayer;
} DefaultServiceRunner;

static void *default_service_runner_run(LIBMATTI_MLA_ServiceRunner *self, void *classLoader)
{
    DefaultServiceRunner *runner = (DefaultServiceRunner *)self;

    // Java: final Class<?> mcClass = Class.forName(gameLayer.findModule("minecraft").orElseThrow(),
    //                                                "net.minecraft.client.main.Main");
    //       mcClass.getMethod("main", String[].class).invoke(null, (Object) arguments);
    // Class definition/invocation is JVM-backed (external); the C port reports the target.
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "Launching minecraft target net.minecraft.client.main.Main with {} argument(s)",
                            "");

    (void)classLoader;
    (void)runner;
    return NULL;
}

// Java: public ServiceRunner launchService(String[] arguments, ModuleLayer gameLayer)
static LIBMATTI_MLA_ServiceRunner *default_launch_service(LIBMATTI_MLA_ILaunchHandlerService *self, int argc,
                                                          char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer)
{
    (void)self;
    DefaultServiceRunner *runner = calloc(1, sizeof(DefaultServiceRunner));
    runner->base.run = default_service_runner_run;
    runner->argc = argc;
    runner->argv = argv;
    runner->gameLayer = gameLayer;
    return &runner->base;
}

// Java: public NamedPath[] getPaths()
//           { return new NamedPath[] { new NamedPath("launch", FileSystems.getDefault().getPath(LAUNCH_PATH_STRING)) }; }
static LIBMATTI_MLA_NamedPath *default_get_paths(LIBMATTI_MLA_ILaunchHandlerService *self, size_t *count)
{
    (void)self;

    // Java: private static final String LAUNCH_PATH_STRING = System.getProperty(LAUNCH_PROPERTY)
    const char *launchPath = getenv(LIBMATTI_ML_DefaultLaunchHandlerService_LAUNCH_PROPERTY);
    if (launchPath == NULL)
    {
        *count = 0;
        return NULL;
    }

    LIBMATTI_MLA_NamedPath *paths = calloc(1, sizeof(LIBMATTI_MLA_NamedPath));
    paths[0].name = strdup("launch");
    paths[0].paths = malloc(sizeof(char *));
    paths[0].paths[0] = strdup(launchPath);
    paths[0].pathCount = 1;

    *count = 1;
    return paths;
}

LIBMATTI_ML_DefaultLaunchHandlerService *LIBMATTI_ML_DefaultLaunchHandlerService_New(void)
{
    LIBMATTI_ML_DefaultLaunchHandlerService *service = calloc(1,
                                                              sizeof(LIBMATTI_ML_DefaultLaunchHandlerService));
    service->base.name = default_name;
    service->base.launchService = default_launch_service;
    service->base.getPaths = default_get_paths;
    return service;
}

void LIBMATTI_ML_DefaultLaunchHandlerService_Free(LIBMATTI_ML_DefaultLaunchHandlerService *service)
{
    free(service);
}
