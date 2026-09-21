// Port of cpw.mods.modlauncher.LaunchServiceHandler.

#ifndef MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLER_H
#define MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLER_H

#include "libmatti/cpw/modlauncher/ArgumentHandler.h"
#include "libmatti/cpw/modlauncher/LaunchPluginHandler.h"
#include "libmatti/cpw/modlauncher/LaunchServiceHandlerDecorator.h"
#include "libmatti/cpw/modlauncher/ModuleLayerHandler.h"
#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"
#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"

#include <stddef.h>

// Java: class LaunchServiceHandler
typedef struct LIBMATTI_ML_LaunchServiceHandler
{
    // Java: Map<String, LaunchServiceHandlerDecorator> launchHandlerLookup
    char **names;
    LIBMATTI_ML_LaunchServiceHandlerDecorator *decorators;
    size_t count;
} LIBMATTI_ML_LaunchServiceHandler;

// Java: public LaunchServiceHandler(ModuleLayerHandler layerHandler)
LIBMATTI_ML_LaunchServiceHandler *LIBMATTI_ML_LaunchServiceHandler_New(LIBMATTI_ML_ModuleLayerHandler *layerHandler);
void LIBMATTI_ML_LaunchServiceHandler_Free(LIBMATTI_ML_LaunchServiceHandler *handler);

// Java: public Optional<ILaunchHandlerService> findLaunchHandler(String name) - NULL means empty
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_LaunchServiceHandler_FindLaunchHandler(
    LIBMATTI_ML_LaunchServiceHandler *handler, const char *name);

// Java: static List<String> hideAccessToken(String[] arguments)
char **LIBMATTI_ML_LaunchServiceHandler_HideAccessToken(int argc, char *argv[], size_t *count);// Java: public void launch(ArgumentHandler argumentHandler, ModuleLayer gameLayer,
//                          TransformingClassLoader classLoader, LaunchPluginHandler launchPluginHandler)
// The port returns the game's exit status (the ServiceRunner result).
int LIBMATTI_ML_LaunchServiceHandler_Launch(LIBMATTI_ML_LaunchServiceHandler *handler,
                                            LIBMATTI_ML_ArgumentHandler *argumentHandler,
                                            LIBMATTI_JL_ModuleLayer *gameLayer,
                                            LIBMATTI_ML_TransformingClassLoader *classLoader,
                                            LIBMATTI_ML_LaunchPluginHandler *launchPluginHandler);

// Java: void validateLaunchTarget(ArgumentHandler argumentHandler)
// 1 = the launch target exists, 0 = the Java method threw a RuntimeException
int LIBMATTI_ML_LaunchServiceHandler_ValidateLaunchTarget(LIBMATTI_ML_LaunchServiceHandler *handler,
                                                          LIBMATTI_ML_ArgumentHandler *argumentHandler);

#endif //MATTICRAFT_MODLAUNCHER_LAUNCHSERVICEHANDLER_H
