// Port of cpw.mods.modlauncher.LaunchPluginHandler.

#include "libmatti/cpw/modlauncher/LaunchPluginHandler.h"

#include "Launcher.h"
#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"
#include "libmatti/cpw/modlauncher/util/ServiceLoaderUtils.h"

#include <stdlib.h>
#include <string.h>

// Java: e -> LOGGER.fatal(MODLAUNCHER, "Encountered serious error loading launch plugin service....", e)
static void log_service_error(const char *message, void *userdata)
{
    (void)userdata;
    LIBMATTI_ML_Logger_Fatal(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Encountered serious error loading launch plugin service. Things will not work well: {}",
                             message);
}

LIBMATTI_ML_LaunchPluginHandler *LIBMATTI_ML_LaunchPluginHandler_New(LIBMATTI_ML_ModuleLayerHandler *layerHandler)
{
    LIBMATTI_ML_LaunchPluginHandler *handler = calloc(1, sizeof(LIBMATTI_ML_LaunchPluginHandler));

    // Java: ServiceLoaderUtils.streamServiceLoader(() -> ServiceLoader.load(layerHandler.getLayer(Layer.BOOT)
    //          .orElseThrow(), ILaunchPluginService.class), e -> LOGGER.fatal(...))
    LIBMATTI_JL_ModuleLayer *bootLayer = LIBMATTI_ML_ModuleLayerHandler_GetLayer(layerHandler,
                                                                                 LIBMATTI_MLA_LAYER_BOOT);
    if (bootLayer == NULL)
    {
        // Java: orElseThrow() -> NoSuchElementException
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "No BOOT layer available, cannot load launch plugins");
        return handler;
    }

    size_t count = 0;
    void **plugins = LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
        bootLayer, "cpw.mods.modlauncher.serviceapi.ILaunchPluginService", log_service_error, NULL, &count);

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MLS_ILaunchPluginService *plugin = plugins[i];
        const char *name = LIBMATTI_MLS_ILaunchPluginService_Name(plugin);

        handler->names = realloc(handler->names, sizeof(*handler->names) * (handler->count + 1));
        handler->plugins = realloc(handler->plugins, sizeof(*handler->plugins) * (handler->count + 1));
        handler->names[handler->count] = strdup(name);
        handler->plugins[handler->count] = plugin;
        handler->count++;
    }
    free(plugins);

    // Java: final var modlist = plugins.entrySet().stream().map(e -> Map.of("name", e.getKey(),
    //              "type", "PLUGINSERVICE", "file", ServiceLoaderUtils.fileNameFor(e.getValue().getClass()))).toList()
    if (LIBMATTI_ML_Launcher_INSTANCE != NULL)
    {
        LIBMATTI_MLA_ModList *mods = LIBMATTI_MLA_IEnvironment_GetProperty(
            (LIBMATTI_MLA_IEnvironment *)LIBMATTI_ML_Launcher_INSTANCE->environment,
            LIBMATTI_MLA_IEnvironment_Keys_ModList());

        if (mods != NULL)
        {
            for (size_t i = 0; i < handler->count; i++)
            {
                char *file = LIBMATTI_MLU_ServiceLoaderUtils_FileNameFor(handler->names[i]);
                LIBMATTI_MLA_ModList_Add(mods, handler->names[i], "PLUGINSERVICE", file);
                free(file);
            }
        }
        else
        {
            // Java: () -> { throw new RuntimeException("The MODLIST isn't set, huh?"); }
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                     "The MODLIST isn't set, huh?");
        }
    }

    // Java: () -> String.join(",", this.plugins.keySet())
    size_t namesLength = 1;
    for (size_t i = 0; i < handler->count; i++) namesLength += strlen(handler->names[i]) + 1;
    char *names = malloc(namesLength);
    names[0] = '\0';
    for (size_t i = 0; i < handler->count; i++)
    {
        if (i > 0) strcat(names, ",");
        strcat(names, handler->names[i]);
    }

    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Found launch plugins: [{}]", names);
    free(names);
    return handler;
}

void LIBMATTI_ML_LaunchPluginHandler_Free(LIBMATTI_ML_LaunchPluginHandler *handler)
{
    if (handler == NULL) return;
    for (size_t i = 0; i < handler->count; i++) free(handler->names[i]);
    free(handler->names);
    free(handler->plugins);
    free(handler);
}

LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_ML_LaunchPluginHandler_Get(const LIBMATTI_ML_LaunchPluginHandler *handler,
                                                                       const char *name)
{
    for (size_t i = 0; i < handler->count; i++)
        if (strcmp(handler->names[i], name) == 0) return handler->plugins[i];
    return NULL;
}

// Java: strings -> auditTrail.addPluginCustomAuditTrail(className.getClassName(), plugin, strings)
typedef struct
{
    LIBMATTI_ML_TransformerAuditTrail *auditTrail;
    const char *className;
    LIBMATTI_MLS_ILaunchPluginService *plugin;
} CustomAuditContext;

static void custom_audit_acceptor(char **data, size_t count, void *userdata)
{
    CustomAuditContext *context = userdata;
    LIBMATTI_ML_TransformerAuditTrail_AddPluginCustomAuditTrail(context->auditTrail, context->className, context->plugin,
                                                                data, count);
}

LIBMATTI_ML_LaunchPluginTransformerSet LIBMATTI_ML_LaunchPluginHandler_ComputeLaunchPluginTransformerSet(
    LIBMATTI_ML_LaunchPluginHandler *handler, const LIBMATTI_ASM_Type *className, int isEmpty, const char *reason,
    LIBMATTI_ML_TransformerAuditTrail *auditTrail)
{
    LIBMATTI_ML_LaunchPluginTransformerSet set = {0};

    // Java: Set<ILaunchPluginService> uniqueValues = new HashSet<>()
    LIBMATTI_MLS_ILaunchPluginService **unique = NULL;
    size_t uniqueCount = 0;

    for (size_t i = 0; i < handler->count; i++)
    {
        LIBMATTI_MLS_ILaunchPluginService *plugin = handler->plugins[i];
        int phases = LIBMATTI_MLS_ILaunchPluginService_HandlesClass(plugin, className, isEmpty, reason);

        if ((phases & LIBMATTI_MLS_PHASES_BEFORE) != 0)
        {
            set.before = realloc(set.before, sizeof(*set.before) * (set.beforeCount + 1));
            set.before[set.beforeCount++] = plugin;
        }
        if ((phases & LIBMATTI_MLS_PHASES_AFTER) != 0)
        {
            set.after = realloc(set.after, sizeof(*set.after) * (set.afterCount + 1));
            set.after[set.afterCount++] = plugin;
        }

        if (phases == LIBMATTI_MLS_PHASES_NONE) continue;

        int known = 0;
        for (size_t u = 0; u < uniqueCount; u++) if (unique[u] == plugin) known = 1;
        if (known) continue;

        unique = realloc(unique, sizeof(*unique) * (uniqueCount + 1));
        unique[uniqueCount++] = plugin;

        // Java: plugin.customAuditConsumer(className.getClassName(), strings -> auditTrail.addPluginCustomAuditTrail(...))
        char *classNameString = LIBMATTI_ASM_Type_GetClassName(className);
        CustomAuditContext context = {auditTrail, classNameString, plugin};
        LIBMATTI_MLS_ILaunchPluginService_CustomAuditConsumer(plugin, classNameString, custom_audit_acceptor, &context);
        free(classNameString);
    }
    free(unique);

    // Java: LOGGER.debug(LAUNCHPLUGIN, "LaunchPluginService {}", () -> phaseObjectEnumMap)
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_LAUNCHPLUGIN,
                             "LaunchPluginService {}", "");

    return set;
}

void LIBMATTI_ML_LaunchPluginHandler_OfferScanResultsToPlugins(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                               LIBMATTI_JH_SecureJar **scanResults,
                                                               size_t scanResultCount)
{
    // Java: plugins.forEach((n, p) -> p.addResources(scanResults))
    for (size_t i = 0; i < handler->count; i++)
        LIBMATTI_MLS_ILaunchPluginService_AddResources(handler->plugins[i], scanResults, scanResultCount);
}

int LIBMATTI_ML_LaunchPluginHandler_OfferClassNodeToPlugins(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                            LIBMATTI_MLS_Phase phase,
                                                            LIBMATTI_MLS_ILaunchPluginService **plugins,
                                                            size_t pluginCount, LIBMATTI_ASMT_ClassNode *node,
                                                            const LIBMATTI_ASM_Type *className,
                                                            LIBMATTI_ML_TransformerAuditTrail *auditTrail,
                                                            const char *reason)
{
    (void)handler;
    int flags = 0;

    for (size_t i = 0; i < pluginCount; i++)
    {
        LIBMATTI_MLS_ILaunchPluginService *plugin = plugins[i];

        char *classNameString = LIBMATTI_ASM_Type_GetClassName(className);
        LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_LAUNCHPLUGIN,
                                 "LauncherPluginService {} offering transform {}", 
                                 LIBMATTI_MLS_ILaunchPluginService_Name(plugin), classNameString);

        int pluginFlags = LIBMATTI_MLS_ILaunchPluginService_ProcessClassWithFlags(plugin, phase, node, className, reason);
        if (pluginFlags != LIBMATTI_MLS_COMPUTE_NO_REWRITE)
        {
            LIBMATTI_ML_TransformerAuditTrail_AddPluginAuditTrail(auditTrail, classNameString, plugin, phase);
            LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_LAUNCHPLUGIN,
                                     "LauncherPluginService {} transformed {} with class compute flags {}",
                                     LIBMATTI_MLS_ILaunchPluginService_Name(plugin), classNameString, "");
            flags |= pluginFlags;
        }
        free(classNameString);
    }

    return flags;
}

// Java: s -> transformerLoader.buildTransformedClassNodeFor(s, k)
typedef struct
{
    LIBMATTI_ML_TransformingClassLoader *transformerLoader;
    const char *reason;
} TransformerLoaderAdapter;

static unsigned char *adapter_build_transformed_class_node_for(LIBMATTI_MLS_ITransformerLoader *self,
                                                               const char *className, size_t *length)
{
    TransformerLoaderAdapter *adapter = (TransformerLoaderAdapter *)self;
    return LIBMATTI_ML_TransformingClassLoader_BuildTransformedClassNodeFor(adapter->transformerLoader, className,
                                                                            adapter->reason, length);
}

void LIBMATTI_ML_LaunchPluginHandler_AnnounceLaunch(LIBMATTI_ML_LaunchPluginHandler *handler,
                                                    LIBMATTI_ML_TransformingClassLoader *transformerLoader,
                                                    LIBMATTI_MLA_NamedPath *specialPaths, size_t specialPathCount)
{
    // Java: plugins.forEach((k, p) -> p.initializeLaunch(s -> transformerLoader.buildTransformedClassNodeFor(s, k), specialPaths))
    for (size_t i = 0; i < handler->count; i++)
    {
        TransformerLoaderAdapter adapter = {transformerLoader, handler->names[i]};
        LIBMATTI_MLS_ITransformerLoader loader = {adapter_build_transformed_class_node_for};
        (void)adapter;
        LIBMATTI_MLS_ILaunchPluginService_InitializeLaunch(handler->plugins[i], &loader, specialPaths,
                                                           specialPathCount);
    }
}
