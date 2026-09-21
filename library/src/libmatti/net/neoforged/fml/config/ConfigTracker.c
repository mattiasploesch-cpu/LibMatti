// Port of net.neoforged.fml.config.ConfigTracker.

#include "libmatti/net/neoforged/fml/config/ConfigTracker.h"

#include "libmatti/com/electronwill/nightconfig/core/file/FileWatcher.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlWriter.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/config/LoadedConfig.h"
#include "libmatti/net/neoforged/fml/event/config/ModConfigEvent.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: static final Marker CONFIG = MarkerFactory.getMarker("CONFIG")
static const LIBMATTI_ML_LogMarker CONFIG_MARKER = {"CONFIG"};

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: net.neoforged.fml.config.ConfigWatcher.run() - the FileWatcher change handler:
//       "Config file {} changed, re-loading" + loadConfig(modConfig, path, ModConfigEvent.Reloading::new)
//       under the modConfig lock (Java wraps it around the whole reload on its watcher thread).
static void config_watcher_handler(void *self, void *userdata)
{
    (void) userdata;
    LIBMATTI_FML_ModConfig *modConfig = self;

    pthread_mutex_lock(&modConfig->lock);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Config file {} changed, re-loading", modConfig->fileName);
    // Java: the watcher fires with the path it registered; ModConfig keeps no path of its own,
    //       the reload re-resolves through the loaded config's path.
    const char *path = modConfig->loadedConfig != NULL ? modConfig->loadedConfig->path : NULL;
    if (path != NULL)
        LIBMATTI_FML_ConfigTracker_LoadConfig(modConfig, path, LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_New,
                                              NULL);
    pthread_mutex_unlock(&modConfig->lock);
}

typedef struct
{
    char *fileName;
    LIBMATTI_FML_ModConfig *config;
} FileMapEntry;

typedef struct
{
    char *modId;
    // Java: List<ModConfig> configsByMod (Collections.synchronizedList)
    LIBMATTI_FML_ModConfig **configs;
    size_t configCount;
} ConfigsByModEntry;

struct LIBMATTI_FML_ConfigTracker
{
    // Java: final ConcurrentHashMap<String, ModConfig> fileMap
    FileMapEntry *fileMap;
    size_t fileMapCount;
    // Java: final EnumMap<ModConfig.Type, Set<ModConfig>> configSets (LinkedHashSet, insertion order)
    LIBMATTI_FML_ModConfig *configSets[4][16];
    size_t configSetCounts[4];
    // Java: final ConcurrentHashMap<String, List<ModConfig>> configsByMod
    ConfigsByModEntry *configsByMod;
    size_t configsByModCount;
    // Java: private final Map<String, ReentrantLock> locksByMod
    struct
    {
        char *modId;
        pthread_mutex_t *lock;
    } *locksByMod;
    size_t locksByModCount;
};

// Java: public static final ConfigTracker INSTANCE = new ConfigTracker();
static LIBMATTI_FML_ConfigTracker *instance = NULL;

LIBMATTI_FML_ConfigTracker *LIBMATTI_FML_ConfigTracker_Instance(void)
{
    if (instance == NULL) instance = LIBMATTI_FML_ConfigTracker_New();
    return instance;
}

// Java: public ConfigTracker()
LIBMATTI_FML_ConfigTracker *LIBMATTI_FML_ConfigTracker_New(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_ConfigTracker));
}

void LIBMATTI_FML_ConfigTracker_Free(LIBMATTI_FML_ConfigTracker *configTracker)
{
    if (configTracker == NULL) return;

    for (size_t i = 0; i < configTracker->fileMapCount; i++) free(configTracker->fileMap[i].fileName);
    free(configTracker->fileMap);
    for (size_t i = 0; i < configTracker->configsByModCount; i++)
    {
        free(configTracker->configsByMod[i].modId);
        free(configTracker->configsByMod[i].configs);
    }
    free(configTracker->configsByMod);
    for (size_t i = 0; i < configTracker->locksByModCount; i++)
    {
        free(configTracker->locksByMod[i].modId);
        LIBMATTI_FML_ModConfig_FreeLock(configTracker->locksByMod[i].lock);
    }
    free(configTracker->locksByMod);
    free(configTracker);

    if (instance == configTracker) instance = NULL;
}

// Java: private static final Path defaultConfigPath =
//           FMLPaths.GAMEDIR.get().resolve(FMLConfig.defaultConfigPath());
static char *default_config_path(void)
{
    return LIBMATTI_JNF_Path_Resolve(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_GAMEDIR),
                                     LIBMATTI_FML_FMLConfig_DefaultConfigPath());
}

// Java: private static String defaultConfigName(ModConfig.Type type, String modId) {
//           return String.format(Locale.ROOT, "%s-%s.toml", modId, type.extension()); }
static char *default_config_name(LIBMATTI_FML_ModConfig_Type type, const char *modId)
{
    size_t size = strlen(modId) + 2 + strlen(LIBMATTI_FML_ModConfig_Type_Extension(type)) + 6;
    char *name = malloc(size);
    snprintf(name, size, "%s-%s.toml", modId, LIBMATTI_FML_ModConfig_Type_Extension(type));
    return name;
}

// Java: void trackConfig(ModConfig config)
static void track_config(LIBMATTI_FML_ConfigTracker *configTracker, LIBMATTI_FML_ModConfig *config)
{
    // Java: var previousValue = this.fileMap.putIfAbsent(config.getFileName(), config);
    for (size_t i = 0; i < configTracker->fileMapCount; i++)
    {
        if (strcmp(configTracker->fileMap[i].fileName, config->fileName) != 0) continue;

        // Java: throw new RuntimeException(errorMessage)
        LIBMATTI_ML_Logger_Error(LOGGER(), &CONFIG_MARKER,
                                 "Detected config file conflict on {} from {} (already registered by {})",
                                 config->fileName, LIBMATTI_FML_ModConfig_GetModId(config),
                                 LIBMATTI_FML_ModConfig_GetModId(configTracker->fileMap[i].config));
        return;
    }

    configTracker->fileMap = realloc(configTracker->fileMap, sizeof(FileMapEntry) * (configTracker->fileMapCount + 1));
    configTracker->fileMap[configTracker->fileMapCount].fileName = strdup(config->fileName);
    configTracker->fileMap[configTracker->fileMapCount].config = config;
    configTracker->fileMapCount++;

    // Java: this.configSets.get(config.getType()).add(config)
    LIBMATTI_FML_ModConfig_Type type = config->type;
    configTracker->configSets[type][configTracker->configSetCounts[type]++] = config;

    // Java: this.configsByMod.computeIfAbsent(config.getModId(), k -> new ArrayList<>()).add(config)
    ConfigsByModEntry *entry = NULL;
    for (size_t i = 0; i < configTracker->configsByModCount; i++)
    {
        if (strcmp(configTracker->configsByMod[i].modId, LIBMATTI_FML_ModConfig_GetModId(config)) == 0)
        {
            entry = &configTracker->configsByMod[i];
            break;
        }
    }
    if (entry == NULL)
    {
        configTracker->configsByMod =
            realloc(configTracker->configsByMod, sizeof(ConfigsByModEntry) * (configTracker->configsByModCount + 1));
        entry = &configTracker->configsByMod[configTracker->configsByModCount++];
        entry->modId = strdup(LIBMATTI_FML_ModConfig_GetModId(config));
        entry->configs = NULL;
        entry->configCount = 0;
    }
    entry->configs = realloc(entry->configs, sizeof(LIBMATTI_FML_ModConfig *) * (entry->configCount + 1));
    entry->configs[entry->configCount++] = config;

    // Java: LOGGER.debug(CONFIG, "Config file {} for {} tracking", config.getFileName(), config.getModId());
    LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Config file {} for {} tracking", config->fileName,
                             LIBMATTI_FML_ModConfig_GetModId(config));
}

// Java: public ModConfig registerConfig(ModConfig.Type type, IConfigSpec spec, ModContainer container)
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_RegisterConfig(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                  LIBMATTI_FML_ModConfig_Type type,
                                                                  LIBMATTI_FML_IConfigSpec *spec,
                                                                  LIBMATTI_FML_ModContainer *container)
{
    char *name = default_config_name(type, LIBMATTI_FML_ModContainer_GetModId(container));
    LIBMATTI_FML_ModConfig *config = LIBMATTI_FML_ConfigTracker_RegisterConfigNamed(configTracker, type, spec,
                                                                                    container, name);
    free(name);
    return config;
}

// Java: public ModConfig registerConfig(ModConfig.Type type, IConfigSpec spec, ModContainer container, String fileName) {
//           var lock = locksByMod.computeIfAbsent(container.getModId(), m -> new ReentrantLock());
//           var modConfig = new ModConfig(type, spec, container, fileName, lock);
//           spec.validateSpec(modConfig); trackConfig(modConfig);
//           if (modConfig.getType() == ModConfig.Type.STARTUP) openConfig(modConfig, FMLPaths.CONFIGDIR.get(), null);
//           return modConfig; }
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_RegisterConfigNamed(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                       LIBMATTI_FML_ModConfig_Type type,
                                                                       LIBMATTI_FML_IConfigSpec *spec,
                                                                       LIBMATTI_FML_ModContainer *container,
                                                                       const char *fileName)
{
    // Java: locksByMod.computeIfAbsent(container.getModId(), m -> new ReentrantLock())
    pthread_mutex_t *lock = NULL;
    for (size_t i = 0; i < configTracker->locksByModCount; i++)
    {
        if (strcmp(configTracker->locksByMod[i].modId, LIBMATTI_FML_ModContainer_GetModId(container)) == 0)
        {
            lock = configTracker->locksByMod[i].lock;
            break;
        }
    }
    if (lock == NULL)
    {
        configTracker->locksByMod =
            realloc(configTracker->locksByMod, sizeof(*configTracker->locksByMod) * (configTracker->locksByModCount + 1));
        configTracker->locksByMod[configTracker->locksByModCount].modId =
            strdup(LIBMATTI_FML_ModContainer_GetModId(container));
        lock = malloc(sizeof(*lock));
        pthread_mutex_init(lock, NULL);
        configTracker->locksByMod[configTracker->locksByModCount].lock = lock;
        configTracker->locksByModCount++;
    }

    LIBMATTI_FML_ModConfig *modConfig = LIBMATTI_FML_ModConfig_New(type, spec, container, fileName, lock);

    // Java: spec.validateSpec(modConfig)
    if (spec->validateSpec != NULL) spec->validateSpec(spec->self, modConfig);

    track_config(configTracker, modConfig);

    // Java: if (modConfig.getType() == ModConfig.Type.STARTUP) openConfig(modConfig, FMLPaths.CONFIGDIR.get(), null);
    if (modConfig->type == LIBMATTI_FML_ModConfig_TYPE_STARTUP)
        LIBMATTI_FML_ConfigTracker_OpenConfig(modConfig, LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_CONFIGDIR),
                                              NULL);

    return modConfig;
}

// Java: public void loadConfigs(ModConfig.Type type, Path configBasePath) / (type, basePath, overridePath)
void LIBMATTI_FML_ConfigTracker_LoadConfigs(LIBMATTI_FML_ConfigTracker *configTracker,
                                            LIBMATTI_FML_ModConfig_Type type, const char *configBasePath)
{
    LIBMATTI_FML_ConfigTracker_LoadConfigsWithOverride(configTracker, type, configBasePath, NULL);
}

void LIBMATTI_FML_ConfigTracker_LoadConfigsWithOverride(LIBMATTI_FML_ConfigTracker *configTracker,
                                                        LIBMATTI_FML_ModConfig_Type type,
                                                        const char *configBasePath,
                                                        const char *configOverrideBasePath)
{
    // Java: LOGGER.debug(CONFIG, "Loading configs type {}", type);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Loading configs type {}",
                             LIBMATTI_FML_ModConfig_Type_Extension(type));
    // Java: this.configSets.get(type).forEach(config -> openConfig(config, configBasePath, configOverrideBasePath));
    // The set does not change during loading, so the size snapshot is safe (Java iterates the live set).
    size_t count = configTracker->configSetCounts[type];
    for (size_t i = 0; i < count; i++)
        LIBMATTI_FML_ConfigTracker_OpenConfig(configTracker->configSets[type][i], configBasePath,
                                              configOverrideBasePath);
}

// Java: private static Path resolveBasePath(ModConfig config, Path configBasePath, @Nullable Path configOverrideBasePath)
static char *resolve_base_path(LIBMATTI_FML_ModConfig *config, const char *configBasePath,
                               const char *configOverrideBasePath)
{
    if (configOverrideBasePath != NULL)
    {
        char *overrideFilePath = LIBMATTI_JNF_Path_Resolve(configOverrideBasePath, config->fileName);
        int exists = LIBMATTI_JNF_Files_Exists(overrideFilePath);
        free(overrideFilePath);
        if (exists)
        {
            LIBMATTI_ML_Logger_Info(LOGGER(), &CONFIG_MARKER, "Found config file override in path {}/{}",
                                    configOverrideBasePath, config->fileName);
            return strdup(configOverrideBasePath);
        }
    }
    return strdup(configBasePath);
}

// Java: static void openConfig(ModConfig config, Path configBasePath, @Nullable Path configOverrideBasePath) {
//           if (config.loadedConfig != null) LOGGER.warn(...);
//           var basePath = resolveBasePath(config, configBasePath, configOverrideBasePath);
//           var configPath = basePath.resolve(config.getFileName());
//           loadConfig(config, configPath, ModConfigEvent.Loading::new);
//           LOGGER.debug(CONFIG, "Loaded TOML config file {}", configPath);
//           if (!FMLConfig.getBoolConfigValue(FMLConfig.ConfigValue.DISABLE_CONFIG_WATCHER)) {
//               FileWatcher.defaultInstance().addWatch(configPath, new ConfigWatcher(config, configPath, ...));
//               LOGGER.debug(CONFIG, "Watching TOML config file {} for changes", configPath); } }
void LIBMATTI_FML_ConfigTracker_OpenConfig(LIBMATTI_FML_ModConfig *config, const char *configBasePath,
                                           const char *configOverrideBasePath)
{
    if (config->loadedConfig != NULL)
    {
        LIBMATTI_ML_Logger_Warn(LOGGER(), &CONFIG_MARKER, "Opening a config that was already loaded at path {}",
                                config->fileName);
    }

    char *basePath = resolve_base_path(config, configBasePath, configOverrideBasePath);
    char *configPath = LIBMATTI_JNF_Path_Resolve(basePath, config->fileName);
    free(basePath);

    LIBMATTI_FML_ConfigTracker_LoadConfig(config, configPath, LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_New,
                                          NULL);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Loaded TOML config file {}", configPath);

    // Java: FileWatcher.defaultInstance().addWatch(configPath, new ConfigWatcher(config, configPath,
    //           Thread.currentThread().getContextClassLoader()))
    if (!LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_DISABLE_CONFIG_WATCHER))
    {
        LIBMATTI_NC_FileWatcher *watcher = LIBMATTI_NC_FileWatcher_DefaultInstance();
        LIBMATTI_NC_FileWatcher_WatchingException *exception = LIBMATTI_NC_FileWatcher_AddWatch(
            watcher, configPath, config_watcher_handler, config, NULL);
        if (exception != NULL)
        {
            // Java: WatchingException propagates out of addWatch
            LIBMATTI_ML_Logger_Error(LOGGER(), &CONFIG_MARKER, "Failed to watch config file {}: {}", configPath,
                                     exception->message != NULL ? exception->message : "watch failed");
            LIBMATTI_NC_FileWatcher_WatchingException_Free(exception);
        }
        else
        {
            LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Watching TOML config file {} for changes", configPath);
        }
    }

    free(configPath);
}

// Java: private static void backUpConfig(Path commentedFileConfig) { backUpConfig(commentedFileConfig, 5); }
static void back_up_config(const char *commentedFileConfig)
{
    // Java: Path bakFileLocation = commentedFileConfig.getParent();
    char *bakFileLocation = LIBMATTI_JNF_Path_GetParent(commentedFileConfig);
    // Java: String bakFileName = FilenameUtils.removeExtension(...getFileName());
    char *fileName = LIBMATTI_JNF_Path_GetFileName(commentedFileConfig);
    const char *dot = fileName != NULL ? strrchr(fileName, '.') : NULL;
    size_t baseLength = dot != NULL ? (size_t) (dot - fileName) : strlen(fileName);
    // Java: String bakFileExtension = FilenameUtils.getExtension(...) + ".bak"
    const char *extension = dot != NULL ? dot + 1 : "";
    size_t extLength = strlen(extension) + 4;

    int maxBackups = 5;
    char *previousBak = malloc(baseLength + 1 + 2 + 1 + extLength + 1);
    char *currentBak = malloc(baseLength + 1 + 2 + 1 + extLength + 1);

    // Java: for (int i = maxBackups; i > 0; i--) { ... shift the .bak files, then copy the config }
    for (int i = maxBackups; i > 0; i--)
    {
        snprintf(previousBak, baseLength + 6 + extLength, "%.*s-%d.%s.bak", (int) baseLength, fileName, i, extension);
        snprintf(currentBak, baseLength + 6 + extLength, "%.*s-%d.%s.bak", (int) baseLength, fileName, i - 1,
                 extension);

        if (i == maxBackups)
        {
            if (!LIBMATTI_JNF_Files_Exists(previousBak)) continue;
            LIBMATTI_JNF_Files_DeleteIfExists(previousBak);
        }
        else if (LIBMATTI_JNF_Files_Exists(currentBak))
        {
            // Java: Files.move(oldBak, bakFileLocation.resolve(bakFileName + "-" + (i + 1) + "." + bakFileExtension))
            FILE *from = fopen(currentBak, "rb");
            FILE *to = fopen(previousBak, "wb");
            if (from != NULL && to != NULL)
            {
                char buffer[4096];
                size_t read;
                while ((read = fread(buffer, 1, sizeof(buffer), from)) > 0) fwrite(buffer, 1, read, to);
            }
            if (from != NULL) fclose(from);
            if (to != NULL) fclose(to);
        }
    }

    // Java: Files.copy(commentedFileConfig, bakFile)
    snprintf(currentBak, baseLength + 5 + extLength, "%.*s-1.%s.bak", (int) baseLength, fileName, extension);
    FILE *from = fopen(commentedFileConfig, "rb");
    FILE *to = fopen(currentBak, "wb");
    if (from != NULL && to != NULL)
    {
        char buffer[4096];
        size_t read;
        while ((read = fread(buffer, 1, sizeof(buffer), from)) > 0) fwrite(buffer, 1, read, to);
    }
    else
    {
        // Java: LOGGER.warn(CONFIG, "Failed to back up config file {}", commentedFileConfig, exception);
        LIBMATTI_ML_Logger_Warn(LOGGER(), &CONFIG_MARKER, "Failed to back up config file {}", commentedFileConfig);
    }
    if (from != NULL) fclose(from);
    if (to != NULL) fclose(to);

    free(previousBak);
    free(currentBak);
    free(fileName);
    free(bakFileLocation);
}

// Java: private static CommentedConfig createDefaultConfig(IConfigSpec spec) {
//           var commentedConfig = new SynchronizedConfig(InMemoryCommentedFormat.defaultInstance(), LinkedHashMap::new);
//           commentedConfig.bulkCommentedUpdate(spec::correct); return commentedConfig; }
static LIBMATTI_NC_Config *create_default_config(LIBMATTI_FML_IConfigSpec *spec)
{
    LIBMATTI_NC_Config *commentedConfig = LIBMATTI_NC_Config_New();
    spec->correct(spec->self, commentedConfig);
    return commentedConfig;
}

// Java: private static void setupConfigFile(ModConfig modConfig, Path file) throws IOException {
//           Files.createDirectories(file.getParent());
//           Path p = defaultConfigPath.resolve(modConfig.getFileName());
//           if (Files.exists(p)) { LOGGER.info(CONFIG, "Loading default config file from path {}", p); Files.copy(p, file); }
//           else writeConfig(file, createDefaultConfig(modConfig.getSpec())); }
static int setup_config_file(LIBMATTI_FML_ModConfig *modConfig, const char *file)
{
    char *parent = LIBMATTI_JNF_Path_GetParent(file);
    if (parent != NULL && !LIBMATTI_JNF_Files_CreateDirectories(parent))
    {
        free(parent);
        return 0;
    }
    free(parent);

    char *defaultPath = default_config_path();
    char *defaultFile = LIBMATTI_JNF_Path_Resolve(defaultPath, modConfig->fileName);
    free(defaultPath);

    int ok;
    if (LIBMATTI_JNF_Files_Exists(defaultFile))
    {
        LIBMATTI_ML_Logger_Info(LOGGER(), &CONFIG_MARKER, "Loading default config file from path {}", defaultFile);

        FILE *from = fopen(defaultFile, "rb");
        FILE *to = fopen(file, "wb");
        ok = from != NULL && to != NULL;
        if (ok)
        {
            char buffer[4096];
            size_t read;
            while ((read = fread(buffer, 1, sizeof(buffer), from)) > 0) fwrite(buffer, 1, read, to);
        }
        if (from != NULL) fclose(from);
        if (to != NULL) fclose(to);
    }
    else
    {
        LIBMATTI_NC_Config *defaultConfig = create_default_config(modConfig->spec);
        ok = LIBMATTI_FML_ConfigTracker_WriteConfig(file, defaultConfig);
        LIBMATTI_NC_Config_Free(defaultConfig);
    }

    free(defaultFile);
    return ok;
}

// Java: private static ConcurrentCommentedConfig readConfig(Path path) throws IOException, ParsingException
// Returns NULL when the file does not exist (NoSuchFileException) or cannot be parsed; *failed is
// set for the parse failure so the caller can log it (Java would throw).
static LIBMATTI_NC_Config *read_config(const char *path, int *failed, char **errorMessage)
{
    *failed = 0;
    *errorMessage = NULL;

    if (!LIBMATTI_JNF_Files_Exists(path)) return NULL;

    // Java: try (var reader = Files.newBufferedReader(path))
    FILE *stream = fopen(path, "rb");
    if (stream == NULL)
    {
        *failed = 1;
        return NULL;
    }
    fseek(stream, 0, SEEK_END);
    long size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    char *data = malloc((size_t) size + 1);
    size_t read = fread(data, 1, (size_t) size, stream);
    fclose(stream);
    data[read] = '\0';

    // Java: var config = new SynchronizedConfig(TomlFormat.instance(), LinkedHashMap::new);
    //       config.bulkCommentedUpdate(view -> new TomlParser().parse(reader, view, ParsingMode.REPLACE));
    LIBMATTI_NC_TomlParser *parser = LIBMATTI_NC_TomlParser_New();
    LIBMATTI_NC_Config *config = LIBMATTI_NC_TomlParser_Parse(parser, data, read);
    if (config == NULL)
    {
        // Java: throw new ParsingException(...)
        *failed = 1;
        *errorMessage = strdup(LIBMATTI_NC_TomlParser_ErrorMessage(parser));
    }
    LIBMATTI_NC_TomlParser_Free(parser);
    free(data);

    return config;
}

// Java: static void loadConfig(ModConfig modConfig, Path path, Function<ModConfig, ModConfigEvent> eventConstructor)
void LIBMATTI_FML_ConfigTracker_LoadConfig(LIBMATTI_FML_ModConfig *modConfig, const char *path,
                                           LIBMATTI_FML_Event_Config_ModConfigEvent *(*eventConstructor)(
                                               LIBMATTI_FML_ModConfig *config),
                                           void *eventUserdata)
{
    LIBMATTI_NC_Config *config;
    int failed;
    char *errorMessage;

    config = read_config(path, &failed, &errorMessage);

    if (!failed && config != NULL)
    {
        // Java: if (!modConfig.getSpec().isCorrect(config)) { LOGGER.warn(CONFIG, "Configuration file {} is not correct. Correcting", path);
        //           backUpConfig(path); modConfig.getSpec().correct(config); writeConfig(path, config); }
        if (!modConfig->spec->isCorrect(modConfig->spec->self, config))
        {
            LIBMATTI_ML_Logger_Warn(LOGGER(), &CONFIG_MARKER, "Configuration file {} is not correct. Correcting", path);
            back_up_config(path);
            modConfig->spec->correct(modConfig->spec->self, config);
            LIBMATTI_FML_ConfigTracker_WriteConfig(path, config);
        }
    }
    else if (config == NULL && !failed)
    {
        // Java: catch (NoSuchFileException ignored) { setupConfigFile(modConfig, path); config = readConfig(path); }
        //       - "Config file does not exist yet"
        if (!setup_config_file(modConfig, path))
        {
            // Java: throw new RuntimeException("Failed to create default config file " + ...)
            LIBMATTI_ML_Logger_Error(LOGGER(), &CONFIG_MARKER,
                                     "Failed to create default config file {} of type {} for modid {}",
                                     modConfig->fileName, LIBMATTI_FML_ModConfig_Type_Extension(modConfig->type),
                                     LIBMATTI_FML_ModConfig_GetModId(modConfig));
            return;
        }

        config = read_config(path, &failed, &errorMessage);
        if (config == NULL)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &CONFIG_MARKER,
                                     "Failed to create default config file {} of type {} for modid {}",
                                     modConfig->fileName, LIBMATTI_FML_ModConfig_Type_Extension(modConfig->type),
                                     LIBMATTI_FML_ModConfig_GetModId(modConfig));
            return;
        }
    }
    else
    {
        // Java: catch (IOException | ParsingException ex) { LOGGER.warn(CONFIG, "Failed to load config {}: {}. Attempting to recreate", ...);
        //           backUpConfig(path); Files.delete(path); setupConfigFile(modConfig, path); config = readConfig(path); }
        LIBMATTI_ML_Logger_Warn(LOGGER(), &CONFIG_MARKER,
                                "Failed to load config {}: {}. Attempting to recreate", modConfig->fileName,
                                errorMessage != NULL ? errorMessage : "unreadable");

        back_up_config(path);
        LIBMATTI_JNF_Files_DeleteIfExists(path);

        if (!setup_config_file(modConfig, path) || (config = read_config(path, &failed, &errorMessage)) == NULL)
        {
            // Java: throw new RuntimeException("Failed to recreate config file " + ...)
            LIBMATTI_ML_Logger_Error(LOGGER(), &CONFIG_MARKER,
                                     "Failed to recreate config file {} of type {} for modid {}",
                                     modConfig->fileName, LIBMATTI_FML_ModConfig_Type_Extension(modConfig->type),
                                     LIBMATTI_FML_ModConfig_GetModId(modConfig));
            return;
        }
    }

    // Java: modConfig.setConfig(new LoadedConfig(config, path, modConfig), eventConstructor);
    LIBMATTI_FML_LoadedConfig *loaded = LIBMATTI_FML_LoadedConfig_New(config, path, modConfig);
    (void) eventUserdata;
    LIBMATTI_FML_ModConfig_SetConfig(modConfig, loaded, eventConstructor, NULL);
    // Java leaves the LoadedConfig to the garbage collector; the port frees the wrapper, the config
    // stays alive through the spec (acceptConfig) and the file.
    LIBMATTI_FML_LoadedConfig_Free(loaded);
}

// Java: static void writeConfig(Path file, UnmodifiableCommentedConfig config) {
//           new TomlWriter().write(config, file, WritingMode.REPLACE_ATOMIC); }
int LIBMATTI_FML_ConfigTracker_WriteConfig(const char *file, const LIBMATTI_NC_Config *config)
{
    LIBMATTI_NC_TomlWriter *writer = LIBMATTI_NC_TomlWriter_New();
    int ok = LIBMATTI_NC_TomlWriter_WriteFile(writer, config, file);
    LIBMATTI_NC_TomlWriter_Free(writer);
    return ok;
}

// Java: private static void unload(Path path) { if (DISABLE_CONFIG_WATCHER) return;
//           try { FileWatcher.defaultInstance().removeWatch(path); } catch (RuntimeException e) { LOGGER.error(...); } }
static void unload(const char *path)
{
    if (LIBMATTI_FML_FMLConfig_GetBoolConfigValue(LIBMATTI_FML_FMLConfig_DISABLE_CONFIG_WATCHER))
        return;

    // Java: FileWatcher.defaultInstance().removeWatch(path) - removeWatch does not throw in the port,
    //       the catch clause is unreachable (kept for the 1:1 flow).
    LIBMATTI_NC_FileWatcher_RemoveWatch(LIBMATTI_NC_FileWatcher_DefaultInstance(), path);
}

// Java: private static void unloadConfig(ModConfig config) {
//           if (config.loadedConfig != null) { ... unload(config.loadedConfig.path); config.setConfig(null, ModConfigEvent.Unloading::new); } }
static void unload_config(LIBMATTI_FML_ModConfig *config)
{
    if (config->loadedConfig == NULL) return;

    // Java: if (config.loadedConfig.path() != null) LOGGER.trace(... unload path) else LOGGER.trace(non-file)
    if (config->loadedConfig->path != NULL)
    {
        LIBMATTI_ML_Logger_Trace(LOGGER(), &CONFIG_MARKER, "Closing and unloading config file type {} at {} for {}",
                                 LIBMATTI_FML_ModConfig_Type_Extension(config->type), config->fileName,
                                 LIBMATTI_FML_ModConfig_GetModId(config));
        unload(config->loadedConfig->path);
    }
    else
    {
        LIBMATTI_ML_Logger_Trace(LOGGER(), &CONFIG_MARKER, "Unloading non-file config {} at path {}",
                                 config->fileName, LIBMATTI_FML_ModConfig_GetModId(config));
    }

    LIBMATTI_FML_ModConfig_SetConfig(config, NULL, LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_New, NULL);
}

// Java: public void unloadConfigs(ModConfig.Type type)
void LIBMATTI_FML_ConfigTracker_UnloadConfigs(LIBMATTI_FML_ConfigTracker *configTracker,
                                              LIBMATTI_FML_ModConfig_Type type)
{
    // Java: LOGGER.debug(CONFIG, "Unloading configs type {}", type);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &CONFIG_MARKER, "Unloading configs type {}",
                             LIBMATTI_FML_ModConfig_Type_Extension(type));
    for (size_t i = 0; i < configTracker->configSetCounts[type]; i++)
        unload_config(configTracker->configSets[type][i]);
}

// Java: public void loadDefaultServerConfigs() - the configs get the corrected defaults without a file
void LIBMATTI_FML_ConfigTracker_LoadDefaultServerConfigs(LIBMATTI_FML_ConfigTracker *configTracker)
{
    for (size_t i = 0; i < configTracker->configSetCounts[LIBMATTI_FML_ModConfig_TYPE_SERVER]; i++)
    {
        LIBMATTI_FML_ModConfig *modConfig = configTracker->configSets[LIBMATTI_FML_ModConfig_TYPE_SERVER][i];
        if (modConfig->loadedConfig != NULL)
        {
            LIBMATTI_ML_Logger_Warn(LOGGER(), &CONFIG_MARKER,
                                    "Overwriting non-null config at path {} with default server config",
                                    modConfig->fileName);
        }

        LIBMATTI_NC_Config *defaultConfig = create_default_config(modConfig->spec);
        LIBMATTI_FML_LoadedConfig *loaded = LIBMATTI_FML_LoadedConfig_New(defaultConfig, NULL, modConfig);
        LIBMATTI_FML_ModConfig_SetConfig(modConfig, loaded, LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_New, NULL);
        LIBMATTI_FML_LoadedConfig_Free(loaded);
    }
}

LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_GetByFileName(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                 const char *fileName)
{
    for (size_t i = 0; i < configTracker->fileMapCount; i++)
        if (strcmp(configTracker->fileMap[i].fileName, fileName) == 0) return configTracker->fileMap[i].config;
    return NULL;
}

// Java: public static List<ModConfig> getModConfigs(String modId) (ModConfigs)
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ModConfigs_GetModConfigs(const char *modId, size_t *count)
{
    LIBMATTI_FML_ConfigTracker *configTracker = LIBMATTI_FML_ConfigTracker_Instance();
    for (size_t i = 0; i < configTracker->configsByModCount; i++)
    {
        if (strcmp(configTracker->configsByMod[i].modId, modId) == 0)
        {
            *count = configTracker->configsByMod[i].configCount;
            return configTracker->configsByMod[i].configs;
        }
    }
    *count = 0;
    return NULL;
}

// Java: public static List<String> getConfigFileNames(String modId, ModConfig.Type type) (ModConfigs)
char **LIBMATTI_FML_ModConfigs_GetConfigFileNames(const char *modId, LIBMATTI_FML_ModConfig_Type type, size_t *count)
{
    size_t modConfigCount = 0;
    LIBMATTI_FML_ModConfig **modConfigs = LIBMATTI_FML_ModConfigs_GetModConfigs(modId, &modConfigCount);

    char **names = malloc(sizeof(char *) * (modConfigCount + 1));
    size_t actual = 0;
    for (size_t i = 0; i < modConfigCount; i++)
    {
        if (modConfigs[i]->type != type) continue;
        names[actual++] = strdup(modConfigs[i]->fileName);
    }

    *count = actual;
    return names;
}

// Java: public static Set<ModConfig> getConfigSet(ModConfig.Type type) (ModConfigs)
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ConfigTracker_GetConfigSet(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                 LIBMATTI_FML_ModConfig_Type type, size_t *count)
{
    *count = configTracker->configSetCounts[type];
    return configTracker->configSets[type];
}

// Java: public static Map<String, ModConfig> getFileMap() (ModConfigs)
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ModConfigs_GetFileMap(size_t *count)
{
    LIBMATTI_FML_ConfigTracker *configTracker = LIBMATTI_FML_ConfigTracker_Instance();
    LIBMATTI_FML_ModConfig **configs = malloc(sizeof(LIBMATTI_FML_ModConfig *) * (configTracker->fileMapCount + 1));
    for (size_t i = 0; i < configTracker->fileMapCount; i++) configs[i] = configTracker->fileMap[i].config;
    *count = configTracker->fileMapCount;
    return configs;
}
