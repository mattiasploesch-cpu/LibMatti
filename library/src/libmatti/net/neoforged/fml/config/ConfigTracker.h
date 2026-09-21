// Port of net.neoforged.fml.config.ConfigTracker.
// Java: public class ConfigTracker - "The configuration tracker manages various types of mod
// configurations." The INSTANCE singleton becomes LIBMATTI_FML_ConfigTracker_Instance(); the
// concurrent maps become arrays guarded by one mutex (the loader is single threaded in the port).

#ifndef MATTICRAFT_FML_CONFIG_CONFIGTRACKER_H
#define MATTICRAFT_FML_CONFIG_CONFIGTRACKER_H

#include "libmatti/net/neoforged/fml/config/ModConfig.h"

#include <stddef.h>

// Java: public enum ModConfig.Type - re-exported from ModConfig.h
typedef LIBMATTI_FML_ModConfig_Type LIBMATTI_FML_ConfigTracker_ModConfigType;

// Java: public class ConfigTracker
typedef struct LIBMATTI_FML_ConfigTracker LIBMATTI_FML_ConfigTracker;

// Java: public static final ConfigTracker INSTANCE
LIBMATTI_FML_ConfigTracker *LIBMATTI_FML_ConfigTracker_Instance(void);

// Java: public ConfigTracker()
LIBMATTI_FML_ConfigTracker *LIBMATTI_FML_ConfigTracker_New(void);
void LIBMATTI_FML_ConfigTracker_Free(LIBMATTI_FML_ConfigTracker *configTracker);

// Java: public ModConfig registerConfig(ModConfig.Type type, IConfigSpec spec, ModContainer container)
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_RegisterConfig(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                  LIBMATTI_FML_ModConfig_Type type,
                                                                  LIBMATTI_FML_IConfigSpec *spec,
                                                                  LIBMATTI_FML_ModContainer *container);
// Java: public ModConfig registerConfig(ModConfig.Type type, IConfigSpec spec, ModContainer container, String fileName)
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_RegisterConfigNamed(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                       LIBMATTI_FML_ModConfig_Type type,
                                                                       LIBMATTI_FML_IConfigSpec *spec,
                                                                       LIBMATTI_FML_ModContainer *container,
                                                                       const char *fileName);

// Java: public void loadConfigs(ModConfig.Type type, Path configBasePath)
void LIBMATTI_FML_ConfigTracker_LoadConfigs(LIBMATTI_FML_ConfigTracker *configTracker,
                                            LIBMATTI_FML_ModConfig_Type type, const char *configBasePath);
// Java: public void loadConfigs(ModConfig.Type type, Path configBasePath, @Nullable Path configOverrideBasePath)
void LIBMATTI_FML_ConfigTracker_LoadConfigsWithOverride(LIBMATTI_FML_ConfigTracker *configTracker,
                                                        LIBMATTI_FML_ModConfig_Type type,
                                                        const char *configBasePath,
                                                        const char *configOverrideBasePath);

// Java: public void unloadConfigs(ModConfig.Type type)
void LIBMATTI_FML_ConfigTracker_UnloadConfigs(LIBMATTI_FML_ConfigTracker *configTracker,
                                              LIBMATTI_FML_ModConfig_Type type);

// Java: static void openConfig(ModConfig config, Path configBasePath, @Nullable Path configOverrideBasePath)
void LIBMATTI_FML_ConfigTracker_OpenConfig(LIBMATTI_FML_ModConfig *config, const char *configBasePath,
                                           const char *configOverrideBasePath);

// Java: static void loadConfig(ModConfig modConfig, Path path, Function<ModConfig, ModConfigEvent> eventConstructor)
void LIBMATTI_FML_ConfigTracker_LoadConfig(LIBMATTI_FML_ModConfig *modConfig, const char *path,
                                           LIBMATTI_FML_Event_Config_ModConfigEvent *(*eventConstructor)(
                                               LIBMATTI_FML_ModConfig *config),
                                           void *eventUserdata);

// Java: static void writeConfig(Path file, UnmodifiableCommentedConfig config)
int LIBMATTI_FML_ConfigTracker_WriteConfig(const char *file, const LIBMATTI_NC_Config *config);

// Java: public void loadDefaultServerConfigs()
void LIBMATTI_FML_ConfigTracker_LoadDefaultServerConfigs(LIBMATTI_FML_ConfigTracker *configTracker);

// Java: public static void acceptSyncedConfig(ModConfig modConfig, byte[] bytes)
// TODO: server sync - the network layer is not ported, so the method stays unimplemented.

// Java: ModConfigs.getModConfigs / getConfigFileNames / getConfigSet / getFileMap
// The port returns the internal arrays (do not free); the sets are lists in insertion order.
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ModConfigs_GetModConfigs(const char *modId, size_t *count);
char **LIBMATTI_FML_ModConfigs_GetConfigFileNames(const char *modId, LIBMATTI_FML_ModConfig_Type type, size_t *count);
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ConfigTracker_GetConfigSet(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                 LIBMATTI_FML_ModConfig_Type type, size_t *count);
LIBMATTI_FML_ModConfig **LIBMATTI_FML_ModConfigs_GetFileMap(size_t *count);

// Java: final ConcurrentHashMap<String, ModConfig> fileMap - the filename -> config lookup
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ConfigTracker_GetByFileName(LIBMATTI_FML_ConfigTracker *configTracker,
                                                                 const char *fileName);

#endif //MATTICRAFT_FML_CONFIG_CONFIGTRACKER_H
