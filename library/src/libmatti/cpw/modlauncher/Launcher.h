//
// Port of cpw.mods.modlauncher.Launcher.
//

#ifndef MATTICRAFT_MODLAUNCHER_LAUNCHER_H
#define MATTICRAFT_MODLAUNCHER_LAUNCHER_H

#include "libmatti/cpw/modlauncher/ArgumentHandler.h"
#include "libmatti/cpw/modlauncher/Environment.h"
#include "libmatti/cpw/modlauncher/LaunchPluginHandler.h"
#include "libmatti/cpw/modlauncher/LaunchServiceHandler.h"
#include "libmatti/cpw/modlauncher/ModuleLayerHandler.h"
#include "libmatti/cpw/modlauncher/TransformStore.h"
#include "libmatti/cpw/modlauncher/TransformationServicesHandler.h"
#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"
#include "libmatti/cpw/modlauncher/api/IModuleLayerManager.h"
#include "libmatti/cpw/modlauncher/api/TypesafeMap.h"

struct LIBMATTI_ML_Launcher;

// Java: public class Launcher
typedef struct LIBMATTI_ML_Launcher
{
    LIBMATTI_MLA_TypesafeMap *blackboard;
    LIBMATTI_ML_TransformationServicesHandler *transformationServicesHandler;
    LIBMATTI_ML_Environment *environment;
    LIBMATTI_ML_TransformStore *transformStore;
    LIBMATTI_ML_ArgumentHandler *argumentHandler;
    LIBMATTI_ML_LaunchServiceHandler *launchService;
    LIBMATTI_ML_LaunchPluginHandler *launchPlugins;
    LIBMATTI_ML_ModuleLayerHandler *moduleLayerHandler;
    LIBMATTI_ML_TransformingClassLoader *classLoader;
} LIBMATTI_ML_Launcher;

// Java: public static Launcher INSTANCE
extern LIBMATTI_ML_Launcher *LIBMATTI_ML_Launcher_INSTANCE;

// Java: private Launcher()
LIBMATTI_ML_Launcher *LIBMATTI_ML_Launcher_New(void);
void LIBMATTI_ML_Launcher_Free(LIBMATTI_ML_Launcher *launcher);

// Java: public static void main(String... args)
int LIBMATTI_ML_Launcher_Main(int argc, char *argv[]);

// Java: public final TypesafeMap blackboard()
LIBMATTI_MLA_TypesafeMap *LIBMATTI_ML_Launcher_Blackboard(const LIBMATTI_ML_Launcher *launcher);

// Java: private void run(String... args)
// 0 = ran to completion, 1 = the Java method threw (the JVM exits non-zero)
int LIBMATTI_ML_Launcher_Run(LIBMATTI_ML_Launcher *launcher, int argc, char *argv[]);

// Java: public Environment environment()
LIBMATTI_ML_Environment *LIBMATTI_ML_Launcher_Environment(const LIBMATTI_ML_Launcher *launcher);

// Java: Optional<ILaunchPluginService> findLaunchPlugin(String name) - NULL means empty
LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_ML_Launcher_FindLaunchPlugin(LIBMATTI_ML_Launcher *launcher,
                                                                         const char *name);
// Java: Optional<ILaunchHandlerService> findLaunchHandler(String name) - NULL means empty
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_Launcher_FindLaunchHandler(LIBMATTI_ML_Launcher *launcher,
                                                                           const char *name);
// Java: public Optional<IModuleLayerManager> findLayerManager() - NULL means empty
LIBMATTI_MLA_IModuleLayerManager *LIBMATTI_ML_Launcher_FindLayerManager(LIBMATTI_ML_Launcher *launcher);

#endif //MATTICRAFT_MODLAUNCHER_LAUNCHER_H
