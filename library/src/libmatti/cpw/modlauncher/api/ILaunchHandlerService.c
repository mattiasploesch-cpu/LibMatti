// Port of cpw.mods.modlauncher.api.ILaunchHandlerService.

#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"

const char *LIBMATTI_MLA_ILaunchHandlerService_Name(const LIBMATTI_MLA_ILaunchHandlerService *service)
{
    if (service == NULL) return "";
    return service->name((LIBMATTI_MLA_ILaunchHandlerService *)service);
}

LIBMATTI_MLA_ServiceRunner *LIBMATTI_MLA_ILaunchHandlerService_LaunchService(LIBMATTI_MLA_ILaunchHandlerService *service,
                                                                             int argc, char *argv[],
                                                                             LIBMATTI_JL_ModuleLayer *gameLayer)
{
    return service->launchService(service, argc, argv, gameLayer);
}

LIBMATTI_MLA_NamedPath *LIBMATTI_MLA_ILaunchHandlerService_GetPaths(LIBMATTI_MLA_ILaunchHandlerService *service,
                                                                    size_t *count)
{
    // Java: default NamedPath[] getPaths() { return new NamedPath[0]; }
    if (service == NULL || service->getPaths == NULL)
    {
        *count = 0;
        return NULL;
    }
    return service->getPaths(service, count);
}
