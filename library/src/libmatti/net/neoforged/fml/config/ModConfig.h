// Port of net.neoforged.fml.config.ModConfig.
// Java's package-private constructor runs inside ConfigTracker.registerConfig; the port keeps the
// fields and the per-mod ReentrantLock (a pthread mutex) the same way.

#ifndef MATTICRAFT_FML_CONFIG_MODCONFIG_H
#define MATTICRAFT_FML_CONFIG_MODCONFIG_H

#include "libmatti/net/neoforged/fml/config/IConfigSpec.h"

#include <pthread.h>
#include <stddef.h>

typedef struct LIBMATTI_FML_ModContainer LIBMATTI_FML_ModContainer;
typedef struct LIBMATTI_FML_LoadedConfig LIBMATTI_FML_LoadedConfig;
typedef struct LIBMATTI_FML_ModConfig LIBMATTI_FML_ModConfig;
typedef struct LIBMATTI_FML_Event_Config_ModConfigEvent LIBMATTI_FML_Event_Config_ModConfigEvent;

// Java: public enum Type { COMMON, CLIENT, SERVER, STARTUP }
typedef enum
{
    LIBMATTI_FML_ModConfig_TYPE_COMMON,
    LIBMATTI_FML_ModConfig_TYPE_CLIENT,
    LIBMATTI_FML_ModConfig_TYPE_SERVER,
    LIBMATTI_FML_ModConfig_TYPE_STARTUP
} LIBMATTI_FML_ModConfig_Type;

// Java: public String extension() { return StringUtils.toLowerCase(name()); }
const char *LIBMATTI_FML_ModConfig_Type_Extension(LIBMATTI_FML_ModConfig_Type type);

// Java: public final class ModConfig
struct LIBMATTI_FML_ModConfig
{
    // Java: private final Type type
    LIBMATTI_FML_ModConfig_Type type;
    // Java: private final IConfigSpec spec
    LIBMATTI_FML_IConfigSpec *spec;
    // Java: private final String fileName
    char *fileName;
    // Java: final ModContainer container
    LIBMATTI_FML_ModContainer *container;
    // Java: @Nullable LoadedConfig loadedConfig (package-private)
    LIBMATTI_FML_LoadedConfig *loadedConfig;
    // Java: final Lock lock (ReentrantLock) - guards reloads and event dispatches
    pthread_mutex_t lock;
};

// Java: ModConfig(Type type, IConfigSpec spec, ModContainer container, String fileName, ReentrantLock lock)
// The caller owns the mutex (ConfigTracker keeps one per mod id) and frees it with ModConfig_FreeLock.
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ModConfig_New(LIBMATTI_FML_ModConfig_Type type, LIBMATTI_FML_IConfigSpec *spec,
                                                   LIBMATTI_FML_ModContainer *container, const char *fileName,
                                                   pthread_mutex_t *lock);
void LIBMATTI_FML_ModConfig_Free(LIBMATTI_FML_ModConfig *config);
// Java keeps the lock object alive separately (locksByMod); the port frees it the same way.
void LIBMATTI_FML_ModConfig_FreeLock(pthread_mutex_t *lock);

// Java: public Type getType()
LIBMATTI_FML_ModConfig_Type LIBMATTI_FML_ModConfig_GetType(const LIBMATTI_FML_ModConfig *config);
// Java: public String getFileName()
const char *LIBMATTI_FML_ModConfig_GetFileName(const LIBMATTI_FML_ModConfig *config);
// Java: public IConfigSpec getSpec()
LIBMATTI_FML_IConfigSpec *LIBMATTI_FML_ModConfig_GetSpec(const LIBMATTI_FML_ModConfig *config);
// Java: public String getModId() { return container.getModId(); }
const char *LIBMATTI_FML_ModConfig_GetModId(const LIBMATTI_FML_ModConfig *config);
// Java: @Nullable public ILoadedConfig getLoadedConfig()
LIBMATTI_FML_ILoadedConfig *LIBMATTI_FML_ModConfig_GetLoadedConfig(const LIBMATTI_FML_ModConfig *config);
// Java: @Nullable public Path getFullPath() - NULL when not loaded or loaded from memory
const char *LIBMATTI_FML_ModConfig_GetFullPath(const LIBMATTI_FML_ModConfig *config);

// Java: void setConfig(@Nullable LoadedConfig loadedConfig, Function<ModConfig, ModConfigEvent> eventConstructor)
// The event constructor is ModConfigEvent.Loading::new / Reloading::new / Unloading::new; the port
// passes the factory returning the event (posted to the owning container's mod bus).
void LIBMATTI_FML_ModConfig_SetConfig(LIBMATTI_FML_ModConfig *config, LIBMATTI_FML_LoadedConfig *loadedConfig,
                                      LIBMATTI_FML_Event_Config_ModConfigEvent *(*eventConstructor)(
                                          LIBMATTI_FML_ModConfig *config),
                                      void *eventUserdata);

#endif //MATTICRAFT_FML_CONFIG_MODCONFIG_H
