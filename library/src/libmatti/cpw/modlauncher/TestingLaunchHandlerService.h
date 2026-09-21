// Port of cpw.mods.modlauncher.TestingLaunchHandlerService.

#ifndef MATTICRAFT_MODLAUNCHER_TESTINGLAUNCHHANDLERSERVICE_H
#define MATTICRAFT_MODLAUNCHER_TESTINGLAUNCHHANDLERSERVICE_H

#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"
#include "libmatti/cpw/modlauncher/api/ServiceRunner.h"

// Java: Class<?> callableLaunch = Class.forName(System.getProperty("test.harness.callable"), ...)
//       MethodHandles.lookup().findStatic(callableLaunch, "supplier", MethodType.methodType(ServiceRunner.class))
// C has no class loading: the host registers the "supplier" under the class name
// it would have been loaded by.
typedef LIBMATTI_MLA_ServiceRunner *(*LIBMATTI_ML_TestingLaunchHandlerService_Supplier)(void *userdata);

void LIBMATTI_ML_TestingLaunchHandlerService_RegisterCallable(
    const char *className, LIBMATTI_ML_TestingLaunchHandlerService_Supplier supplier, void *userdata);

// Java: public class TestingLaunchHandlerService implements ILaunchHandlerService
typedef struct
{
    LIBMATTI_MLA_ILaunchHandlerService base;
} LIBMATTI_ML_TestingLaunchHandlerService;

// Java: new TestingLaunchHandlerService()
LIBMATTI_ML_TestingLaunchHandlerService *LIBMATTI_ML_TestingLaunchHandlerService_New(void);
void LIBMATTI_ML_TestingLaunchHandlerService_Free(LIBMATTI_ML_TestingLaunchHandlerService *service);

#endif //MATTICRAFT_MODLAUNCHER_TESTINGLAUNCHHANDLERSERVICE_H
