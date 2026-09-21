//
// Port of cpw.mods.modlauncher.Environment.
//

#include "Environment.h"

#include "Launcher.h"

#include <stdlib.h>

// Java: package-private Environment(Launcher launcher)
LIBMATTI_ML_Environment *LIBMATTI_ML_Environment_New(struct LIBMATTI_ML_Launcher *launcher)
{
    LIBMATTI_ML_Environment *environment = calloc(1, sizeof(LIBMATTI_ML_Environment));
    if (environment == NULL) return NULL;

    // Java: environment = new TypesafeMap(IEnvironment.class)
    environment->environment = LIBMATTI_MLA_TypesafeMap_NewOwned("cpw.mods.modlauncher.api.IEnvironment");
    environment->launcher = launcher;

    return environment;
}

// Java: public final <T> Optional<T> getProperty(TypesafeMap.Key<T> key)
void *LIBMATTI_MLA_IEnvironment_GetProperty(LIBMATTI_MLA_IEnvironment *environment, LIBMATTI_MLA_Key *key)
{
    // Java: return environment.get(key)
    return LIBMATTI_MLA_TypesafeMap_Get(((LIBMATTI_ML_Environment *)environment)->environment, key);
}

// Java: public <T> T computePropertyIfAbsent(TypesafeMap.Key<T> key, Function<...> valueFunction)
void *LIBMATTI_MLA_IEnvironment_ComputePropertyIfAbsent(LIBMATTI_MLA_IEnvironment *environment, LIBMATTI_MLA_Key *key,
                                                        void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                                        void *userdata)
{
    // Java: return environment.computeIfAbsent(key, valueFunction)
    return LIBMATTI_MLA_TypesafeMap_ComputeIfAbsent(((LIBMATTI_ML_Environment *)environment)->environment, key,
                                                    valueFunction, userdata);
}

// Java: public Optional<ILaunchPluginService> findLaunchPlugin(final String name)
LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_MLA_IEnvironment_FindLaunchPlugin(LIBMATTI_MLA_IEnvironment *environment,
                                                                             const char *name)
{
    // Java: return launcher.findLaunchPlugin(name)
    return LIBMATTI_ML_Launcher_FindLaunchPlugin(((LIBMATTI_ML_Environment *)environment)->launcher, name);
}

// Java: public Optional<ILaunchHandlerService> findLaunchHandler(final String name)
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_MLA_IEnvironment_FindLaunchHandler(LIBMATTI_MLA_IEnvironment *environment,
                                                                               const char *name)
{
    // Java: return launcher.findLaunchHandler(name)
    return LIBMATTI_ML_Launcher_FindLaunchHandler(((LIBMATTI_ML_Environment *)environment)->launcher, name);
}

// Java: public Optional<IModuleLayerManager> findModuleLayerManager()
LIBMATTI_MLA_IModuleLayerManager *LIBMATTI_MLA_IEnvironment_FindModuleLayerManager(LIBMATTI_MLA_IEnvironment *environment)
{
    // Java: return launcher.findLayerManager()
    return LIBMATTI_ML_Launcher_FindLayerManager(((LIBMATTI_ML_Environment *)environment)->launcher);
}

// Java: Environment has no other public members
void *LIBMATTI_ML_Environment_ComputePropertyIfAbsent(LIBMATTI_ML_Environment *environment, LIBMATTI_MLA_Key *key,
                                                      void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                                      void *userdata)
{
    return LIBMATTI_MLA_IEnvironment_ComputePropertyIfAbsent((LIBMATTI_MLA_IEnvironment *)environment, key,
                                                             valueFunction, userdata);
}
