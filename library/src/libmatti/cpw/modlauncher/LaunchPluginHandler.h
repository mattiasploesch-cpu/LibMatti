// Port of cpw.mods.modlauncher.LaunchPluginHandler.

#ifndef MATTICRAFT_MODLAUNCHER_LAUNCHPLUGINHANDLER_H
#define MATTICRAFT_MODLAUNCHER_LAUNCHPLUGINHANDLER_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/cpw/modlauncher/ModuleLayerHandler.h"
#include "libmatti/cpw/modlauncher/TransformerAuditTrail.h"
#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"
#include "libmatti/cpw/modlauncher/serviceapi/ILaunchPluginService.h"

#include <stddef.h>

// Java: EnumMap<ILaunchPluginService.Phase, List<ILaunchPluginService>>
typedef struct
{
    LIBMATTI_MLS_ILaunchPluginService **before;
    size_t beforeCount;
    LIBMATTI_MLS_ILaunchPluginService **after;
    size_t afterCount;
} LIBMATTI_ML_LaunchPluginTransformerSet;

// Java: public class LaunchPluginHandler
typedef struct LIBMATTI_ML_LaunchPluginHandler
{
    // Java: Map<String, ILaunchPluginService> plugins
    char **names;
    LIBMATTI_MLS_ILaunchPluginService **plugins;
    size_t count;
} LIBMATTI_ML_LaunchPluginHandler;

// Java: public LaunchPluginHandler(ModuleLayerHandler layerHandler)
LIBMATTI_ML_LaunchPluginHandler *LIBMATTI_ML_LaunchPluginHandler_New(LIBMATTI_ML_ModuleLayerHandler *layerHandler);
void LIBMATTI_ML_LaunchPluginHandler_Free(LIBMATTI_ML_LaunchPluginHandler *handler);

// Java: public Optional<ILaunchPluginService> get(String name) - NULL means empty
LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_ML_LaunchPluginHandler_Get(const LIBMATTI_ML_LaunchPluginHandler *handler,
                                                                       const char *name);

// Java: public EnumMap<Phase, List<ILaunchPluginService>> computeLaunchPluginTransformerSet(
//             Type className, boolean isEmpty, String reason, TransformerAuditTrail auditTrail)
LIBMATTI_ML_LaunchPluginTransformerSet LIBMATTI_ML_LaunchPluginHandler_ComputeLaunchPluginTransformerSet(
    LIBMATTI_ML_LaunchPluginHandler *handler, const LIBMATTI_ASM_Type *className, int isEmpty, const char *reason,
    LIBMATTI_ML_TransformerAuditTrail *auditTrail);

// Java: void offerScanResultsToPlugins(List<SecureJar> scanResults)
void LIBMATTI_ML_LaunchPluginHandler_OfferScanResultsToPlugins(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                               LIBMATTI_JH_SecureJar **scanResults,
                                                               size_t scanResultCount);

// Java: int offerClassNodeToPlugins(Phase phase, List<ILaunchPluginService> plugins, ClassNode node, Type className,
//                                   TransformerAuditTrail auditTrail, String reason)
int LIBMATTI_ML_LaunchPluginHandler_OfferClassNodeToPlugins(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                            LIBMATTI_MLS_Phase phase,
                                                            LIBMATTI_MLS_ILaunchPluginService **plugins,
                                                            size_t pluginCount, LIBMATTI_ASMT_ClassNode *node,
                                                            const LIBMATTI_ASM_Type *className,
                                                            LIBMATTI_ML_TransformerAuditTrail *auditTrail,
                                                            const char *reason);

// Java: void announceLaunch(TransformingClassLoader transformerLoader, NamedPath[] specialPaths)
void LIBMATTI_ML_LaunchPluginHandler_AnnounceLaunch(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                    LIBMATTI_ML_TransformingClassLoader *transformerLoader,
                                                    LIBMATTI_MLA_NamedPath *specialPaths, size_t specialPathCount);

#endif //MATTICRAFT_MODLAUNCHER_LAUNCHPLUGINHANDLER_H
