// Port of cpw.mods.modlauncher.LaunchServiceHandlerDecorator.

#ifndef MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLERDECORATOR_H
#define MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLERDECORATOR_H

#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"

// Java: record LaunchServiceHandlerDecorator(ILaunchHandlerService service)
typedef struct
{
    LIBMATTI_MLA_ILaunchHandlerService *service;
} LIBMATTI_ML_LaunchServiceHandlerDecorator;

// Java: public void launch(String[] arguments, ModuleLayer gameLayer) - the port
// returns the game's exit status (the ServiceRunner result).
int LIBMATTI_ML_LaunchServiceHandlerDecorator_Launch(LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator,
                                                     int argc, char *argv[], LIBMATTI_JL_ModuleLayer *gameLayer);
// Java: ILaunchHandlerService service()
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_LaunchServiceHandlerDecorator_Service(
    const LIBMATTI_ML_LaunchServiceHandlerDecorator *decorator);

#endif //MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLERDECORATOR_H
