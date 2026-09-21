// Port of cpw.mods.modlauncher.TestingLaunchHandlerService.

#include "TestingLaunchHandlerService.h"

#include "LogManager.h"
#include "LogMarkers.h"

#include <stdlib.h>
#include <string.h>

// Java: the class loader registry; Class.forName resolves the callable class,
// the MethodHandle resolves its static "supplier" method.
typedef struct
{
    char *className;
    LIBMATTI_ML_TestingLaunchHandlerService_Supplier supplier;
    void *userdata;
} TestingCallable;

static TestingCallable *callables = NULL;
static size_t callableCount = 0;

void LIBMATTI_ML_TestingLaunchHandlerService_RegisterCallable(
    const char *className, LIBMATTI_ML_TestingLaunchHandlerService_Supplier supplier, void *userdata)
{
    callables = realloc(callables, sizeof(*callables) * (callableCount + 1));
    callables[callableCount].className = strdup(className);
    callables[callableCount].supplier = supplier;
    callables[callableCount].userdata = userdata;
    callableCount++;
}

// Java: public String name() { return "testharness"; }
static const char *testing_name(LIBMATTI_MLA_ILaunchHandlerService *self)
{
    (void)self;
    return "testharness";
}

// Java: public ServiceRunner launchService(String[] arguments, ModuleLayer gameLayer)
static LIBMATTI_MLA_ServiceRunner *testing_launch_service(LIBMATTI_MLA_ILaunchHandlerService *self, int argc,
                                                          char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer)
{
    (void)self;
    (void)argc;
    (void)argv;
    (void)gameLayer;

    // Java: System.getProperty("test.harness.callable")
    const char *callableName = getenv("test.harness.callable");

    for (size_t i = 0; callableName != NULL && i < callableCount; i++)
    {
        if (strcmp(callables[i].className, callableName) != 0) continue;
        // Java: (ServiceRunner) handle.invoke()
        return callables[i].supplier(callables[i].userdata);
    }

    // Java: catch (...) { throw new RuntimeException(e); }
    LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Unable to resolve test harness callable {}", callableName != NULL ? callableName : "");
    return NULL;
}

LIBMATTI_ML_TestingLaunchHandlerService *LIBMATTI_ML_TestingLaunchHandlerService_New(void)
{
    LIBMATTI_ML_TestingLaunchHandlerService *service = calloc(1, sizeof(LIBMATTI_ML_TestingLaunchHandlerService));
    service->base.name = testing_name;
    service->base.launchService = testing_launch_service;
    // Java: getPaths() is not overridden -> the interface default returns an empty array
    service->base.getPaths = NULL;
    return service;
}

void LIBMATTI_ML_TestingLaunchHandlerService_Free(LIBMATTI_ML_TestingLaunchHandlerService *service)
{
    free(service);
}
