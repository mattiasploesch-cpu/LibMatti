// Port of net.neoforged.fml.loading.FMLConfig.

#ifndef MATTICRAFT_FML_FMLCONFIG_H
#define MATTICRAFT_FML_FMLCONFIG_H

#include <stddef.h>

// Java: public enum ConfigValue
typedef enum
{
    LIBMATTI_FML_FMLConfig_DISABLE_CONFIG_WATCHER,
    LIBMATTI_FML_FMLConfig_EARLY_WINDOW_CONTROL,
    LIBMATTI_FML_FMLConfig_MAX_THREADS,
    LIBMATTI_FML_FMLConfig_VERSION_CHECK,
    LIBMATTI_FML_FMLConfig_DEBUG_OPENGL,
    LIBMATTI_FML_FMLConfig_DEFAULT_CONFIG_PATH,
    LIBMATTI_FML_FMLConfig_DISABLE_OPTIMIZED_DFU,
    LIBMATTI_FML_FMLConfig_EARLY_WINDOW_PROVIDER,
    LIBMATTI_FML_FMLConfig_EARLY_WINDOW_WIDTH,
    LIBMATTI_FML_FMLConfig_EARLY_WINDOW_HEIGHT,
    LIBMATTI_FML_FMLConfig_EARLY_WINDOW_MAXIMIZED,
    LIBMATTI_FML_FMLConfig_EARLY_LOADING_SCREEN_THEME
} LIBMATTI_FML_FMLConfig_ConfigValue;

#define LIBMATTI_FML_FMLCONFIG_CONFIG_VALUE_COUNT 12

// Java: public record DependencyOverride(String modId, boolean remove)
typedef struct
{
    char *modId;
    int remove;
} LIBMATTI_FML_FMLConfig_DependencyOverride;

// Java: public String getMessage()
char *LIBMATTI_FML_FMLConfig_DependencyOverride_GetMessage(const LIBMATTI_FML_FMLConfig_DependencyOverride *override);
void LIBMATTI_FML_FMLConfig_DependencyOverride_Free(LIBMATTI_FML_FMLConfig_DependencyOverride *override);

// Java: static ConfigValue[] values()
const char *const *LIBMATTI_FML_FMLConfig_ConfigValue_Names(size_t *count);
// Java: the enum's `entry` field
const char *LIBMATTI_FML_FMLConfig_ConfigValue_Entry(LIBMATTI_FML_FMLConfig_ConfigValue value);
// Java: the enum's `comment` field
const char *LIBMATTI_FML_FMLConfig_ConfigValue_Comment(LIBMATTI_FML_FMLConfig_ConfigValue value);

// Java: public static void load()
void LIBMATTI_FML_FMLConfig_Load(void);

// Java: public static String getConfigValue(ConfigValue v)
const char *LIBMATTI_FML_FMLConfig_GetConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value);
// Java: public static boolean getBoolConfigValue(ConfigValue v)
int LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value);
// Java: public static int getIntConfigValue(ConfigValue v)
int LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value);
// Java: public static <A> List<A> getListConfigValue(ConfigValue v)
char **LIBMATTI_FML_FMLConfig_GetListConfigValue(LIBMATTI_FML_FMLConfig_ConfigValue value, size_t *count);

// Java: public static <T> void updateConfig(ConfigValue v, T value) - C splits the generic into three setters
void LIBMATTI_FML_FMLConfig_UpdateConfigBool(LIBMATTI_FML_FMLConfig_ConfigValue value, int newValue);
void LIBMATTI_FML_FMLConfig_UpdateConfigInt(LIBMATTI_FML_FMLConfig_ConfigValue value, int newValue);
void LIBMATTI_FML_FMLConfig_UpdateConfigString(LIBMATTI_FML_FMLConfig_ConfigValue value, const char *newValue);

// Java: public static String defaultConfigPath()
const char *LIBMATTI_FML_FMLConfig_DefaultConfigPath(void);

// Java: @Unmodifiable public static List<DependencyOverride> getOverrides(String modId)
// returns the internal array, do not free
LIBMATTI_FML_FMLConfig_DependencyOverride **LIBMATTI_FML_FMLConfig_GetOverrides(const char *modId, size_t *count);

#endif //MATTICRAFT_FML_FMLCONFIG_H
