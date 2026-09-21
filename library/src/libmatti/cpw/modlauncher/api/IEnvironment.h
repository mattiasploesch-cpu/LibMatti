//
// Port of cpw.mods.modlauncher.api.IEnvironment.
//

#ifndef MATTICRAFT_MODLAUNCHER_IENVIRONMENT_H
#define MATTICRAFT_MODLAUNCHER_IENVIRONMENT_H

#include "libmatti/cpw/modlauncher/api/IModuleLayerManager.h"
#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"
#include "libmatti/cpw/modlauncher/api/TypesafeMap.h"
#include "libmatti/cpw/modlauncher/serviceapi/ILaunchPluginService.h"

#include <stddef.h>

// Java: interface IEnvironment
typedef struct LIBMATTI_MLA_IEnvironment LIBMATTI_MLA_IEnvironment;

// Java: Optional<T> getProperty(TypesafeMap.Key<T> key) - NULL means empty
void *LIBMATTI_MLA_IEnvironment_GetProperty(LIBMATTI_MLA_IEnvironment *environment, LIBMATTI_MLA_Key *key);
// Java: T computePropertyIfAbsent(Key<T>, Function)
void *LIBMATTI_MLA_IEnvironment_ComputePropertyIfAbsent(LIBMATTI_MLA_IEnvironment *environment, LIBMATTI_MLA_Key *key,
                                                        void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                                        void *userdata);
// Java: Optional<ILaunchPluginService> findLaunchPlugin(String name)
LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_MLA_IEnvironment_FindLaunchPlugin(LIBMATTI_MLA_IEnvironment *environment,
                                                                             const char *name);
// Java: Optional<ILaunchHandlerService> findLaunchHandler(String name)
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_MLA_IEnvironment_FindLaunchHandler(LIBMATTI_MLA_IEnvironment *environment,
                                                                               const char *name);
// Java: Optional<IModuleLayerManager> findModuleLayerManager()
LIBMATTI_MLA_IModuleLayerManager *LIBMATTI_MLA_IEnvironment_FindModuleLayerManager(LIBMATTI_MLA_IEnvironment *environment);

// Java: Key<List<Map<String,String>>> MODLIST - the C model of List<Map<String,String>>
// (entries carry the "name"/"type"/"file" keys used by ModLauncher)
typedef struct LIBMATTI_MLA_ModList LIBMATTI_MLA_ModList;
// Java: new ArrayList<>()
LIBMATTI_MLA_ModList *LIBMATTI_MLA_ModList_New(void);
// Java: list.add(Map.of("name", ...))
void LIBMATTI_MLA_ModList_Add(LIBMATTI_MLA_ModList *list, const char *name, const char *type, const char *file);
size_t LIBMATTI_MLA_ModList_Count(const LIBMATTI_MLA_ModList *list);

// Java: final class Keys { static final Supplier<Key<...>> ... = buildKey(...); }
// buildKey uses a KeyBuilder, so the keys are lazily created once a TypesafeMap
// owned by IEnvironment exists (TypesafeMap.KeyBuilder.get()).
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_Version(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_GameDir(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_AssetsDir(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_Uuid(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_LaunchTarget(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_AuditTrail(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_ModList(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_MlSpecVersion(void);
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_MlImplVersion(void);

#endif //MATTICRAFT_MODLAUNCHER_IENVIRONMENT_H
