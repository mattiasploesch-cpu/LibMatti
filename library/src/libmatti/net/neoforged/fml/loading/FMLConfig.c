#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"

#include "libmatti/com/electronwill/nightconfig/core/Config.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlWriter.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Runtime.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogUtils.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

typedef enum
{
    VALUE_TYPE_BOOLEAN,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_STRING,
    VALUE_TYPE_LIST
} ValueType;

// Java: the enum constant's entry, defaultValue, comment and entryFunction
typedef struct
{
    const char *name;
    const char *entry;
    ValueType type;
    int boolValue;
    int intValue;
    const char *stringValue;
    int maxThreadsFunction;
    const char *comment;
} Descriptor;

static const Descriptor DESCRIPTORS[LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT] = {
    {"DISABLE_CONFIG_WATCHER", "disableConfigWatcher", VALUE_TYPE_BOOLEAN, 0, 0, NULL, 0,
     "Disables File Watcher. Used to automatically update config if its file has been modified."},
    {"EARLY_WINDOW_CONTROL", "earlyWindowControl", VALUE_TYPE_BOOLEAN, 1, 0, NULL, 0,
     "Shows an early loading screen for mod loading which improves the user experience with early feedback about mod loading."},
    {"MAX_THREADS", "maxThreads", VALUE_TYPE_INTEGER, 0, -1, NULL, 1,
     "Max threads for early initialization parallelism,  -1 is based on processor count"},
    {"VERSION_CHECK", "versionCheck", VALUE_TYPE_BOOLEAN, 1, 0, NULL, 0,
     "Enable NeoForge global version checking"},
    {"DEBUG_OPENGL", "debugOpenGl", VALUE_TYPE_BOOLEAN, 0, 0, NULL, 0,
     "Enable synchronous OpenGL debug output and object labeling"},
    {"DEFAULT_CONFIG_PATH", "defaultConfigPath", VALUE_TYPE_STRING, 0, 0, "defaultconfigs", 0,
     "Default config path for servers"},
    {"DISABLE_OPTIMIZED_DFU", "disableOptimizedDFU", VALUE_TYPE_BOOLEAN, 1, 0, NULL, 0,
     "Disables Optimized DFU client-side - already disabled on servers"},
    {"EARLY_WINDOW_PROVIDER", "earlyWindowProvider", VALUE_TYPE_STRING, 0, 0, "fmlearlywindow", 0, "Early window provider"},
    {"EARLY_WINDOW_WIDTH", "earlyWindowWidth", VALUE_TYPE_INTEGER, 0, 854, NULL, 0, "Early window width"},
    {"EARLY_WINDOW_HEIGHT", "earlyWindowHeight", VALUE_TYPE_INTEGER, 0, 480, NULL, 0, "Early window height"},
    {"EARLY_WINDOW_MAXIMIZED", "earlyWindowMaximized", VALUE_TYPE_BOOLEAN, 0, 0, NULL, 0,
     "Early window starts maximized"},
    {"EARLY_LOADING_SCREEN_THEME", "earlyLoadingScreenTheme", VALUE_TYPE_STRING, 0, 0, "", 0,
     "Force a given theme-id to be used for the early loading screen"},
};

// Java: private CommentedConfig configData
// com.electronwill.nightconfig.core.CommentedConfig is external; the C port keeps the loaded values here.
typedef struct
{
    int boolValue;
    int intValue;
    char *stringValue;
} ConfigEntry;

// Java: configData is null until load() ran
static ConfigEntry *configData = NULL;

// Java: private static Map<String, List<DependencyOverride>> dependencyOverrides = Map.of();
// The port keeps a flat override array plus the parallel target-mod-id array; the pair
// forms the Map<String, List<DependencyOverride>> of the original.
static LIBMATTI_FML_FMLConfig_DependencyOverride **dependencyOverrides = NULL;
static char **dependencyOverrideTargets = NULL;
static size_t dependencyOverrideModCount = 0;

// Java: private static Object maxThreads(Object value)
static int max_threads(int value)
{
    // Java: int val = (Integer) value; if (val <= 0) return Runtime.getRuntime().availableProcessors(); else return val;
    return value <= 0 ? LIBMATTI_JL_Runtime_AvailableProcessors() : value;
}

const char *const *LIBMATTI_FML_FMLConfig_ConfigValue_Names(size_t *count)
{
    static const char *const names[LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT] = {
        "DISABLE_CONFIG_WATCHER", "EARLY_WINDOW_CONTROL", "MAX_THREADS", "VERSION_CHECK", "DEBUG_OPENGL",
        "DEFAULT_CONFIG_PATH", "DISABLE_OPTIMIZED_DFU", "EARLY_WINDOW_PROVIDER", "EARLY_WINDOW_WIDTH",
        "EARLY_WINDOW_HEIGHT", "EARLY_WINDOW_MAXIMIZED", "EARLY_LOADING_SCREEN_THEME"
    };

    *count = LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT;
    return names;
}

const char *LIBMATTI_FML_FMLConfig_ConfigValue_Entry(LIBMATTI_FML_FMLConfig_ConfigValue value)
{
    return DESCRIPTORS[value].entry;
}

const char *LIBMATTI_FML_FMLConfig_ConfigValue_Comment(LIBMATTI_FML_FMLConfig_ConfigValue value)
{
    return DESCRIPTORS[value].comment;
}

// Java: public String getMessage()
char *LIBMATTI_FML_FMLConfig_DependencyOverride_GetMessage(const LIBMATTI_FML_FMLConfig_DependencyOverride *override)
{
    // Java: (remove ? "softening dependency constraints against" : "adding explicit AFTER ordering against") + " '" + modId + "'"
    const char *prefix = override->remove ? "softening dependency constraints against"
                                          : "adding explicit AFTER ordering against";
    size_t length = strlen(prefix) + strlen(override->modId) + 4;
    char *message = malloc(length);
    snprintf(message, length, "%s '%s'", prefix, override->modId);
    return message;
}

void LIBMATTI_FML_FMLConfig_DependencyOverride_Free(LIBMATTI_FML_FMLConfig_DependencyOverride *override)
{
    if (override == NULL) return;

    free(override->modId);
    free(override);
}

// Java: private void saveConfig(Path configFile) { new TomlWriter().write(this.configData, configFile, WritingMode.REPLACE); }
static void save_config(const char *configFile)
{
    LIBMATTI_NC_Config *config = LIBMATTI_NC_Config_New();
    for (int i = 0; i < LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT; i++)
    {
        const char *path[] = {DESCRIPTORS[i].entry};
        LIBMATTI_NC_Value value;
        switch (DESCRIPTORS[i].type)
        {
            case VALUE_TYPE_BOOLEAN:
                value = LIBMATTI_NC_Value_OfBoolean(configData[i].boolValue);
                break;
            case VALUE_TYPE_INTEGER:
                value = LIBMATTI_NC_Value_OfInt(configData[i].intValue);
                break;
            default:
                value = LIBMATTI_NC_Value_OfString(configData[i].stringValue != NULL ? configData[i].stringValue : "");
                break;
        }
        LIBMATTI_NC_Config_Set(config, path, 1, &value);
        // Config_Set copies the value into the config; the temporary must not be freed here
        // (freeing the string leaves a dangling pointer inside the config and aborts in
        // LIBMATTI_NC_Config_Free). The config owns the copy and releases it.
    }

    LIBMATTI_NC_TomlWriter *writer = LIBMATTI_NC_TomlWriter_New();
    char *serialized = LIBMATTI_NC_TomlWriter_WriteToString(writer, config);
    LIBMATTI_NC_TomlWriter_Free(writer);
    if (serialized != NULL)
        LIBMATTI_JNF_Files_WriteString(configFile, serialized);
    free(serialized);
    LIBMATTI_NC_Config_Free(config);
}

// Java: private void loadFrom(Path configFile)
static void load_from(const char *configFile)
{
    // Java: this.configData = CommentedConfig.of(LinkedHashMap::new, TomlFormat.instance());
    free(configData);
    configData = calloc(LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT, sizeof(ConfigEntry));

    // Java: configSpec.correct(this.configData) first fills the defaults, then the file's
    //       values replace them (ParsingMode.REPLACE); the port mirrors that order.
    for (int i = 0; i < LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT; i++)
    {
        configData[i].boolValue = DESCRIPTORS[i].boolValue;
        configData[i].intValue = DESCRIPTORS[i].intValue;
        configData[i].stringValue = DESCRIPTORS[i].stringValue != NULL ? strdup(DESCRIPTORS[i].stringValue) : NULL;
    }

    LIBMATTI_NC_Config *loaded = NULL;
    if (LIBMATTI_JNF_Files_Exists(configFile))
    {
        // Java: new TomlParser().parse(configStream, configData, ParsingMode.REPLACE);
        size_t length = 0;
        unsigned char *content = LIBMATTI_JNF_Files_ReadAllBytes(configFile, &length);
        if (content != NULL)
        {
            LIBMATTI_NC_TomlParser *parser = LIBMATTI_NC_TomlParser_New();
            loaded = LIBMATTI_NC_TomlParser_Parse(parser, (const char *) content, length);
            LIBMATTI_NC_TomlParser_Free(parser);
            free(content);
        }
        if (loaded == NULL)
        {
            // Java: a broken config aborts the load with the parser's exception
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Failed to parse the FML config {}",
                                     configFile);
        }
    }

    if (loaded != NULL)
    {
        // Java: configSpec.isCorrect(configData) corrections happen implicitly: unknown keys
        //       are ignored and absent keys keep the defaults set above.
        for (int i = 0; i < LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT; i++)
        {
            LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_GetKey(loaded, DESCRIPTORS[i].entry);
            if (value == NULL) continue;
            switch (DESCRIPTORS[i].type)
            {
                case VALUE_TYPE_BOOLEAN:
                    if (value->type == LIBMATTI_NC_VALUE_BOOLEAN) configData[i].boolValue = value->boolean;
                    break;
                case VALUE_TYPE_INTEGER:
                    if (value->type == LIBMATTI_NC_VALUE_INT) configData[i].intValue = value->integer;
                    break;
                default:
                    if (value->type == LIBMATTI_NC_VALUE_STRING)
                    {
                        free(configData[i].stringValue);
                        configData[i].stringValue = strdup(value->string);
                    }
                    break;
            }
        }
        LIBMATTI_NC_Config_Free(loaded);
    }

    // Java: if (!configSpec.isCorrect(configData)) { ... saveConfig(configFile); }
    save_config(configFile);
}
void LIBMATTI_FML_FMLConfig_Load(void)
{
    // Java: Path configFile = FMLPaths.FMLCONFIG.get(); INSTANCE.loadFrom(configFile);
    const char *configFile = LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_FMLCONFIG);
    load_from(configFile);

    // Java: if (LOGGER.isTraceEnabled(CORE)) { LOGGER.trace(CORE, "Loaded FML config from {}", ...); for (ConfigValue cv : values()) LOGGER.trace(CORE, "FMLConfig {} is {}", cv.entry, cv.getConfigValue(INSTANCE.configData)); }
    LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Loaded FML config from {}", configFile);
    for (int i = 0; i < LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT; i++)
    {
        // Java: getConfigValue returns the typed value; the port renders each type like
        //       Java's String.valueOf so the trace shows the real config, not null.
        char rendered[32];
        const char *renderedValue;
        switch (DESCRIPTORS[i].type)
        {
            case VALUE_TYPE_BOOLEAN:
                renderedValue = configData[i].boolValue ? "true" : "false";
                break;
            case VALUE_TYPE_INTEGER:
                snprintf(rendered, sizeof(rendered), "%d", configData[i].intValue);
                renderedValue = rendered;
                break;
            default:
                renderedValue = configData[i].stringValue != NULL ? configData[i].stringValue : "null";
                break;
        }
        LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "FMLConfig {} is {}", DESCRIPTORS[i].entry,
                                 renderedValue);
    }

    // Java: FMLPaths.getOrCreateGameRelativePath(Paths.get(FMLConfig.getConfigValue(ConfigValue.DEFAULT_CONFIG_PATH)));
    char *defaultConfigPath =
        LIBMATTI_FML_FMLPaths_GetOrCreateGameRelativePath(LIBMATTI_FML_FMLConfig_GetConfigValue(
            LIBMATTI_FML_FMLConfig_DEFAULT_CONFIG_PATH));
    free(defaultConfigPath);

    // Java: var overridesObject = INSTANCE.configData.get("dependencyOverrides");
    //       if (overridesObject instanceof Config overrides) overrides.valueMap().forEach((key, value) -> {
    //           var list = (List<?>) value; ... dependencyOverrides.put(key, list.stream().map(...).toList()); });
    // The TOML form is: [dependencyOverrides.<modId>]
    //                    remove = ["dep1", "dep2"]
    //                    order  = ["dep3"]
    free(dependencyOverrides);
    dependencyOverrides = NULL;
    for (size_t i = 0; i < dependencyOverrideModCount; i++)
        free(dependencyOverrideTargets[i]);
    free(dependencyOverrideTargets);
    dependencyOverrideTargets = NULL;
    dependencyOverrideModCount = 0;
    LIBMATTI_NC_Config *overridesConfig = LIBMATTI_NC_Config_New();
    {
        size_t overrideLength = 0;
        unsigned char *overrideContent =
            LIBMATTI_JNF_Files_ReadAllBytes(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_FMLCONFIG), &overrideLength);
        if (overrideContent != NULL)
        {
            LIBMATTI_NC_TomlParser *overrideParser = LIBMATTI_NC_TomlParser_New();
            LIBMATTI_NC_Config *overrideRoot =
                LIBMATTI_NC_TomlParser_Parse(overrideParser, (const char *) overrideContent, overrideLength);
            LIBMATTI_NC_TomlParser_Free(overrideParser);
            free(overrideContent);
            if (overrideRoot != NULL)
            {
                LIBMATTI_NC_Value *overridesValue = LIBMATTI_NC_Config_GetKey(overrideRoot, "dependencyOverrides");
                if (overridesValue != NULL && overridesValue->type == LIBMATTI_NC_VALUE_CONFIG)
                {
                    // shallow-copy the override tables so the root config can be freed
                    LIBMATTI_NC_Config *overrides = overridesValue->config;
                    for (size_t i = 0; i < overrides->count; i++)
                    {
                        const char *path[] = {overrides->keys[i]};
                        LIBMATTI_NC_Config_Set(overridesConfig, path, 1, &overrides->values[i]);
                    }
                }
                LIBMATTI_NC_Config_Free(overrideRoot);
            }
        }
    }

    size_t overrideEntryCount = 0;
    LIBMATTI_NC_Config_ValueMap(overridesConfig, &overrideEntryCount);
    if (overrideEntryCount > 0)
    {
        // Java: the map maps a target mod id to its override list; the port stores a parallel
        //       target-name array next to the flat override array so getOverrides can group.
        LIBMATTI_FML_FMLConfig_DependencyOverride **flattened = NULL;
        char **targets = NULL;
        size_t flattenedCount = 0;
        for (size_t i = 0; i < overrideEntryCount; i++)
        {
            const char *targetModId = LIBMATTI_NC_Config_KeyAt(overridesConfig, i);
            LIBMATTI_NC_Value *listValue = LIBMATTI_NC_Config_GetKey(overridesConfig, targetModId);
            if (listValue == NULL || listValue->type != LIBMATTI_NC_VALUE_CONFIG) continue;

            LIBMATTI_NC_Value *removeList = LIBMATTI_NC_Config_GetKey(listValue->config, "remove");
            LIBMATTI_NC_Value *orderList = LIBMATTI_NC_Config_GetKey(listValue->config, "order");
            if (removeList != NULL && removeList->type == LIBMATTI_NC_VALUE_LIST)
            {
                for (size_t r = 0; r < removeList->listCount; r++)
                {
                    if (removeList->list[r].type != LIBMATTI_NC_VALUE_STRING) continue;
                    flattened = realloc(flattened, sizeof(*flattened) * (flattenedCount + 1));
                    targets = realloc(targets, sizeof(*targets) * (flattenedCount + 1));
                    LIBMATTI_FML_FMLConfig_DependencyOverride *override =
                        calloc(1, sizeof(LIBMATTI_FML_FMLConfig_DependencyOverride));
                    override->modId = strdup(removeList->list[r].string);
                    override->remove = 1;
                    flattened[flattenedCount] = override;
                    targets[flattenedCount] = strdup(targetModId);
                    flattenedCount++;
                }
            }
            if (orderList != NULL && orderList->type == LIBMATTI_NC_VALUE_LIST)
            {
                for (size_t o = 0; o < orderList->listCount; o++)
                {
                    if (orderList->list[o].type != LIBMATTI_NC_VALUE_STRING) continue;
                    flattened = realloc(flattened, sizeof(*flattened) * (flattenedCount + 1));
                    targets = realloc(targets, sizeof(*targets) * (flattenedCount + 1));
                    LIBMATTI_FML_FMLConfig_DependencyOverride *override =
                        calloc(1, sizeof(LIBMATTI_FML_FMLConfig_DependencyOverride));
                    override->modId = strdup(orderList->list[o].string);
                    override->remove = 0;
                    flattened[flattenedCount] = override;
                    targets[flattenedCount] = strdup(targetModId);
                    flattenedCount++;
                }
            }
        }
        if (flattenedCount > 0)
        {
            dependencyOverrides = flattened;
            dependencyOverrideTargets = targets;
            dependencyOverrideModCount = flattenedCount;
        }
        else
        {
            free(flattened);
            free(targets);
        }
    }
    LIBMATTI_NC_Config_Free(overridesConfig);
}

// Java: public static String getConfigValue(ConfigValue v)
const char *LIBMATTI_FML_FMLConfig_GetConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value)
{
    // Java: return (T) this.entryFunction.apply(config != null ? config.get(this.entry) : this.defaultValue);
    if (configData == NULL) return DESCRIPTORS[value].stringValue;
    return configData[value].stringValue;
}

// Java: public static boolean getBoolConfigValue(ConfigValue v)
int LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value)
{
    if (configData == NULL) return DESCRIPTORS[value].boolValue;
    return configData[value].boolValue;
}

// Java: public static int getIntConfigValue(ConfigValue v)
int LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value)
{
    if (configData == NULL) return max_threads(DESCRIPTORS[value].intValue);
    return max_threads(configData[value].intValue);
}

// Java: public static <A> List<A> getListConfigValue(ConfigValue v)
char **LIBMATTI_FML_FMLConfig_GetListConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value, size_t *count)
{
    // Java: none of the ConfigValue constants is a List in this version, so getConfigValue is cast instead.
    (void) value;
    *count = 0;
    return NULL;
}

// Java: public static <T> void updateConfig(ConfigValue v, T value)
void LIBMATTI_FML_FMLConfig_UpdateConfigBool(LIBMATTI_FML_FMLConfig_ConfigValue value, int newValue)
{
    // Java: if (INSTANCE.configData != null) { v.setConfigValue(INSTANCE.configData, value); INSTANCE.saveConfig(FMLPaths.FMLCONFIG.get()); }
    if (configData == NULL) return;

    configData[value].boolValue = newValue;
    save_config(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_FMLCONFIG));
}

void LIBMATTI_FML_FMLConfig_UpdateConfigInt(LIBMATTI_FML_FMLConfig_ConfigValue value, int newValue)
{
    if (configData == NULL) return;

    configData[value].intValue = newValue;
    save_config(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_FMLCONFIG));
}

void LIBMATTI_FML_FMLConfig_UpdateConfigString(LIBMATTI_FML_FMLConfig_ConfigValue value, const char *newValue)
{
    if (configData == NULL) return;

    free(configData[value].stringValue);
    configData[value].stringValue = strdup(newValue);
    save_config(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_FMLCONFIG));
}

// Java: public static String defaultConfigPath()
const char *LIBMATTI_FML_FMLConfig_DefaultConfigPath(void)
{
    return LIBMATTI_FML_FMLConfig_GetConfigValue(LIBMATTI_FML_FMLConfig_DEFAULT_CONFIG_PATH);
}

// Java: @Unmodifiable public static List<DependencyOverride> getOverrides(String modId)
// Returns the internal array of the entries whose target is modId, do not free.
LIBMATTI_FML_FMLConfig_DependencyOverride **LIBMATTI_FML_FMLConfig_GetOverrides(const char *modId, size_t *count)
{
    // Java: var ov = dependencyOverrides.get(modId); if (ov == null) return List.of(); return ov;
    *count = 0;
    if (modId == NULL) return NULL;
    static LIBMATTI_FML_FMLConfig_DependencyOverride **group = NULL;
    static size_t groupCapacity = 0;
    size_t groupCount = 0;
    for (size_t i = 0; i < dependencyOverrideModCount; i++)
    {
        if (strcmp(dependencyOverrideTargets[i], modId) != 0) continue;
        if (groupCount == groupCapacity)
        {
            groupCapacity = groupCapacity == 0 ? 4 : groupCapacity * 2;
            group = realloc(group, sizeof(*group) * groupCapacity);
        }
        group[groupCount++] = dependencyOverrides[i];
    }
    if (groupCount == 0) return NULL;
    *count = groupCount;
    return group;
}
