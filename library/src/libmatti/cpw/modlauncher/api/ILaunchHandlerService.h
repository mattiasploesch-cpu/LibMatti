// Port of cpw.mods.modlauncher.api.ILaunchHandlerService.

#ifndef MATTICRAFT_MODLAUNCHER_ILAUNCHHANDLERSERVICE_H
#define MATTICRAFT_MODLAUNCHER_ILAUNCHHANDLERSERVICE_H

#include "libmatti/cpw/modlauncher/api/NamedPath.h"
#include "libmatti/cpw/modlauncher/api/ServiceRunner.h"
#include "libmatti/java/lang/ModuleLayer.h"

#include <stddef.h>

// Java: interface ILaunchHandlerService
typedef struct LIBMATTI_MLA_ILaunchHandlerService
{
    // Java: String name()
    const char *(*name)(struct LIBMATTI_MLA_ILaunchHandlerService *self);
    // Java: ServiceRunner launchService(String[] arguments, ModuleLayer gameLayer)
    LIBMATTI_MLA_ServiceRunner *(*launchService)(struct LIBMATTI_MLA_ILaunchHandlerService *self, int argc,
                                                char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer);
    // Java: default NamedPath[] getPaths() { return new NamedPath[0]; }
    LIBMATTI_MLA_NamedPath *(*getPaths)(struct LIBMATTI_MLA_ILaunchHandlerService *self, size_t *count);
} LIBMATTI_MLA_ILaunchHandlerService;

// Java: String name()
const char *LIBMATTI_MLA_ILaunchHandlerService_Name(const LIBMATTI_MLA_ILaunchHandlerService *service);
// Java: ServiceRunner launchService(String[] arguments, ModuleLayer gameLayer)
LIBMATTI_MLA_ServiceRunner *LIBMATTI_MLA_ILaunchHandlerService_LaunchService(LIBMATTI_MLA_ILaunchHandlerService *service,
                                                                             int argc, char *argv[],
                                                                             LIBMATTI_JL_ModuleLayer *gameLayer);
// Java: default NamedPath[] getPaths() { return new NamedPath[0]; }
LIBMATTI_MLA_NamedPath *LIBMATTI_MLA_ILaunchHandlerService_GetPaths(LIBMATTI_MLA_ILaunchHandlerService *service,
                                                                    size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_ILAUNCHHANDLERSERVICE_H
