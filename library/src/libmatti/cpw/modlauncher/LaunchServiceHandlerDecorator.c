// Port of cpw.mods.modlauncher.LaunchServiceHandlerDecorator.

#include "libmatti/cpw/modlauncher/LaunchServiceHandlerDecorator.h"

#include "LogManager.h"
#include "LogMarkers.h"

#include <stdint.h>

int LIBMATTI_ML_LaunchServiceHandlerDecorator_Launch(LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator,
                                                     int argc, char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer)
{
    // Java: try { this.service.launchService(arguments, gameLayer).run(); } catch (Throwable e) { throw new RuntimeException(e); }
    LIBMATTI_MLA_ServiceRunner *runner =
        LIBMATTI_MLA_ILaunchHandlerService_LaunchService(decorator->service, argc, argv, gameLayer);

    if (runner == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Launch service {} returned no ServiceRunner", 
                                 LIBMATTI_MLA_ILaunchHandlerService_Name(decorator->service));
        return 1;
    }

    // Java: the ServiceRunner's Class<?> result is discarded; the port carries the
    // game's exit status in the pointer value.
    return (int) (intptr_t) LIBMATTI_MLA_ServiceRunner_Run(runner, NULL);
}

LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_LaunchServiceHandlerDecorator_Service(
    const LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator)
{
    return decorator->service;
}
