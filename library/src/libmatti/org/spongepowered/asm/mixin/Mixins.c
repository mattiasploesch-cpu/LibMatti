// Port of org.spongepowered.asm.mixin.Mixins.
// "Entry point for registering global mixin resources. Compatibility with pre-0.6 versions is
// maintained via the methods on MixinEnvironment delegating to the methods here."

#include "libmatti/org/spongepowered/asm/mixin/Mixins.h"

#include "libmatti/org/spongepowered/asm/launch/GlobalProperties.h"
#include "libmatti/org/spongepowered/asm/mixin/transformer/Config.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final ILogger logger = MixinService.getService().getLogger("mixin");
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// Java: private static final Keys CONFIGS_KEY = Keys.of(GlobalProperties.Keys.CONFIGS + ".queue");
static LIBMATTI_SP_GlobalProperties_Keys *CONFIGS_KEY(void)
{
    return LIBMATTI_SP_GlobalProperties_Keys_Of("mixin.configs.queue");
}

// Java: public static Set<Config> getConfigs() - the blackboard carries the ConfigSet
static LIBMATTI_SP_ConfigSet *get_configs_set(void)
{
    LIBMATTI_SP_ConfigSet *mixinConfigs = LIBMATTI_SP_GlobalProperties_Get(CONFIGS_KEY());
    if (mixinConfigs == NULL)
    {
        mixinConfigs = calloc(1, sizeof(LIBMATTI_SP_ConfigSet));
        LIBMATTI_SP_GlobalProperties_Put(CONFIGS_KEY(), mixinConfigs);
    }
    return mixinConfigs;
}

// Java: private static final Set<String> errorHandlers = new LinkedHashSet<String>();
static char **errorHandlers = NULL;
static size_t errorHandlerCount = 0;

// Java: private static final Set<String> registeredConfigs = new HashSet<String>();
static char **registeredConfigs = NULL;
static size_t registeredConfigCount = 0;

static int registered_configs_contains(const char *name)
{
    for (size_t i = 0; i < registeredConfigCount; i++)
        if (strcmp(registeredConfigs[i], name) == 0) return 1;

    return 0;
}

static void registered_configs_add(const char *name)
{
    registeredConfigs = realloc(registeredConfigs, sizeof(char *) * (registeredConfigCount + 1));
    registeredConfigs[registeredConfigCount++] = strdup(name);
}

// Java: private static void registerConfiguration(Config config)
static void register_configuration(LIBMATTI_SP_Config *config);

// Java: @SuppressWarnings("deprecation") private static void createConfiguration(String configFile,
//                                            MixinEnvironment fallback, IMixinConfigSource source)
static void create_configuration(const char *configFile, void *fallback, LIBMATTI_SP_ContainerHandle *source)
{
    // Java: Config config = null; try { config = Config.create(configFile, fallback, source); }
    //       catch (Exception ex) { logger.error("Error encountered reading mixin config " + ...); }
    LIBMATTI_SP_Config *config = LIBMATTI_SP_Config_Create(configFile, fallback, source);

    // Java: Mixins.registerConfiguration(config);
    register_configuration(config);
}

// Java: private static void registerConfiguration(Config config)
static void register_configuration(LIBMATTI_SP_Config *config)
{
    if (config == NULL || registered_configs_contains(config->name)) return;

    // Java: MixinEnvironment env = config.getEnvironment(); if (env != null) env.registerConfig(config.getName());
    if (config->environment != NULL)
    {
        LIBMATTI_SP_MixinEnvironment_RegisterConfig(config->environment, config->name);
    }

    // Java: Mixins.getConfigs().add(config);
    LIBMATTI_SP_ConfigSet_Add(get_configs_set(), config);

    // Java: Mixins.registeredConfigs.add(config.getName());
    registered_configs_add(config->name);

    // Java: Config parent = config.getParent(); if (parent != null) Mixins.registerConfiguration(parent);
    if (config->parent != NULL) register_configuration(config->parent);
}

// Java: public static void addConfigurations(String... configFiles)
void LIBMATTI_SP_Mixins_AddConfigurations(const char *const *configFiles, size_t count)
{
    LIBMATTI_SP_Mixins_AddConfigurationsWithSource(configFiles, count, NULL);
}

// Java: public static void addConfigurations(String[] configFiles, IMixinConfigSource source)
void LIBMATTI_SP_Mixins_AddConfigurationsWithSource(const char *const *configFiles, size_t count,
                                                    LIBMATTI_SP_ContainerHandle *source)
{
    // Java: MixinEnvironment fallback = MixinEnvironment.getDefaultEnvironment();
    void *fallback = LIBMATTI_SP_MixinEnvironment_GetDefaultEnvironment();
    for (size_t i = 0; i < count; i++)
        create_configuration(configFiles[i], fallback, source);
}

// Java: public static void addConfiguration(String configFile)
void LIBMATTI_SP_Mixins_AddConfiguration(const char *configFile)
{
    LIBMATTI_SP_Mixins_AddConfigurationWithSource(configFile, NULL);
}

// Java: public static void addConfiguration(String configFile, IMixinConfigSource source)
void LIBMATTI_SP_Mixins_AddConfigurationWithSource(const char *configFile, LIBMATTI_SP_ContainerHandle *source)
{
    // Java: Mixins.createConfiguration(configFile, MixinEnvironment.getDefaultEnvironment(), source);
    create_configuration(configFile, LIBMATTI_SP_MixinEnvironment_GetDefaultEnvironment(), source);
}

// Java: @Deprecated static void addConfiguration(String configFile, MixinEnvironment fallback)
void LIBMATTI_SP_Mixins_AddConfigurationWithFallback(const char *configFile, void *fallback)
{
    create_configuration(configFile, fallback, NULL);
}

// Java: public static int getUnvisitedCount()
int LIBMATTI_SP_Mixins_GetUnvisitedCount(void)
{
    size_t configCount = 0;
    LIBMATTI_SP_Config **configs = LIBMATTI_SP_Mixins_GetConfigs(&configCount);
    int count = 0;
    for (size_t i = 0; i < configCount; i++)
    {
        if (!LIBMATTI_SP_Config_IsVisited(configs[i])) count++;
    }
    return count;
}

// Java: public static Set<Config> getConfigs() - the port hands back the growable item array;
// the array is owned by the blackboard and must not be freed by the caller
LIBMATTI_SP_Config **LIBMATTI_SP_Mixins_GetConfigs(size_t *count)
{
    LIBMATTI_SP_ConfigSet *mixinConfigs = get_configs_set();
    *count = mixinConfigs->count;
    return mixinConfigs->items;
}

// Java: public static void registerErrorHandlerClass(String handlerName)
void LIBMATTI_SP_Mixins_RegisterErrorHandlerClass(const char *handlerName)
{
    if (handlerName == NULL) return;

    for (size_t i = 0; i < errorHandlerCount; i++)
        if (strcmp(errorHandlers[i], handlerName) == 0) return;

    errorHandlers = realloc(errorHandlers, sizeof(char *) * (errorHandlerCount + 1));
    errorHandlers[errorHandlerCount++] = strdup(handlerName);
}

// Java: public static Set<String> getErrorHandlerClasses()
const char *const *LIBMATTI_SP_Mixins_GetErrorHandlerClasses(size_t *count)
{
    *count = errorHandlerCount;
    return (const char *const *) errorHandlers;
}
