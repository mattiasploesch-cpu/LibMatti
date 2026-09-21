#include "libmatti/net/neoforged/fml/startup/NeoForgeLaunchHandler.h"

#include "libmatti/net/neoforged/fml/startup/Client.h"

#include <stdlib.h>
#include <stdint.h>

// Java: public String name() { return "neoforge"; }
static const char *neoforge_name(LIBMATTI_MLA_ILaunchHandlerService *self)
{
    (void)self;
    return "neoforge";
}

// Java: the ServiceRunner returned by launchService(arguments, gameLayer)
typedef struct
{
    LIBMATTI_MLA_ServiceRunner base;
    int argc;
    char **argv;
} NeoForgeServiceRunner;

// Java: public Class<?> run(ClassLoader loader) { Client.main(arguments); return null; }
// The port's Client_Main returns the game's exit status; the runner carries it as
// the pointer value (Java discards the Class<?> result, the port observes it).
static void *neoforge_service_runner_run(LIBMATTI_MLA_ServiceRunner *self, void *classLoader)
{
    NeoForgeServiceRunner *runner = (NeoForgeServiceRunner *)self;
    (void)classLoader;

    return (void *) (intptr_t) LIBMATTI_FML_Client_Main(runner->argc, runner->argv);
}

// Java: public ServiceRunner launchService(String[] arguments, ModuleLayer gameLayer)
static LIBMATTI_MLA_ServiceRunner *neoforge_launch_service(LIBMATTI_MLA_ILaunchHandlerService *self, int argc,
                                                           char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer)
{
    (void)self;

    NeoForgeServiceRunner *runner = calloc(1, sizeof(NeoForgeServiceRunner));
    runner->base.run = neoforge_service_runner_run;
    runner->argc = argc;
    runner->argv = argv;
    (void)gameLayer;
    return &runner->base;
}

LIBMATTI_FML_NeoForgeLaunchHandler *LIBMATTI_FML_NeoForgeLaunchHandler_New(void)
{
    LIBMATTI_FML_NeoForgeLaunchHandler *handler = calloc(1, sizeof(LIBMATTI_FML_NeoForgeLaunchHandler));
    handler->base.name = neoforge_name;
    handler->base.launchService = neoforge_launch_service;
    // Java: getPaths() is not overridden -> the interface default returns an empty array
    handler->base.getPaths = NULL;
    return handler;
}

void LIBMATTI_FML_NeoForgeLaunchHandler_Free(LIBMATTI_FML_NeoForgeLaunchHandler *handler)
{
    free(handler);
}
