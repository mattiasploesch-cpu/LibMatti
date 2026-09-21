// Port of net.neoforged.fml.config.ModConfig.

#include "libmatti/net/neoforged/fml/config/ModConfig.h"

#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/config/LoadedConfig.h"
#include "libmatti/net/neoforged/fml/event/config/ModConfigEvent.h"

#include <stdlib.h>
#include <string.h>

static const char *const TYPE_NAMES[] = {"COMMON", "CLIENT", "SERVER", "STARTUP"};

// Java: public String extension() { return StringUtils.toLowerCase(name()); }
const char *LIBMATTI_FML_ModConfig_Type_Extension(LIBMATTI_FML_ModConfig_Type type)
{
    return type == LIBMATTI_FML_ModConfig_TYPE_COMMON  ? "common"
           : type == LIBMATTI_FML_ModConfig_TYPE_CLIENT ? "client"
           : type == LIBMATTI_FML_ModConfig_TYPE_SERVER ? "server"
                                                        : "startup";
}

// Java: the enum's name(), for logging
const char *LIBMATTI_FML_ModConfig_Type_Name(LIBMATTI_FML_ModConfig_Type type)
{
    return TYPE_NAMES[type];
}

// Java: ModConfig(Type type, IConfigSpec spec, ModContainer container, String fileName, ReentrantLock lock)
LIBMATTI_FML_ModConfig *LIBMATTI_FML_ModConfig_New(LIBMATTI_FML_ModConfig_Type type, LIBMATTI_FML_IConfigSpec *spec,
                                                   LIBMATTI_FML_ModContainer *container, const char *fileName,
                                                   pthread_mutex_t *lock)
{
    LIBMATTI_FML_ModConfig *config = calloc(1, sizeof(*config));
    config->type = type;
    config->spec = spec;
    config->fileName = strdup(fileName);
    config->container = container;
    config->lock = *lock;
    return config;
}

void LIBMATTI_FML_ModConfig_Free(LIBMATTI_FML_ModConfig *config)
{
    if (config == NULL) return;

    free(config->fileName);
    free(config);
}

void LIBMATTI_FML_ModConfig_FreeLock(pthread_mutex_t *lock)
{
    if (lock == NULL) return;
    pthread_mutex_destroy(lock);
    free(lock);
}

LIBMATTI_FML_ModConfig_Type LIBMATTI_FML_ModConfig_GetType(const LIBMATTI_FML_ModConfig *config)
{
    return config->type;
}

const char *LIBMATTI_FML_ModConfig_GetFileName(const LIBMATTI_FML_ModConfig *config)
{
    return config->fileName;
}

LIBMATTI_FML_IConfigSpec *LIBMATTI_FML_ModConfig_GetSpec(const LIBMATTI_FML_ModConfig *config)
{
    return config->spec;
}

const char *LIBMATTI_FML_ModConfig_GetModId(const LIBMATTI_FML_ModConfig *config)
{
    return LIBMATTI_FML_ModContainer_GetModId(config->container);
}

LIBMATTI_FML_ILoadedConfig *LIBMATTI_FML_ModConfig_GetLoadedConfig(const LIBMATTI_FML_ModConfig *config)
{
    return config->loadedConfig != NULL ? &config->loadedConfig->loaded : NULL;
}

const char *LIBMATTI_FML_ModConfig_GetFullPath(const LIBMATTI_FML_ModConfig *config)
{
    return config->loadedConfig != NULL ? config->loadedConfig->path : NULL;
}

// Java: void setConfig(@Nullable LoadedConfig loadedConfig, Function<ModConfig, ModConfigEvent> eventConstructor) {
//           lock.lock(); try { this.loadedConfig = loadedConfig; spec.acceptConfig(loadedConfig);
//           container.acceptEvent(eventConstructor.apply(this)); } finally { lock.unlock(); } }
void LIBMATTI_FML_ModConfig_SetConfig(LIBMATTI_FML_ModConfig *config, LIBMATTI_FML_LoadedConfig *loadedConfig,
                                      LIBMATTI_FML_Event_Config_ModConfigEvent *(*eventConstructor)(
                                          LIBMATTI_FML_ModConfig *config),
                                      void *eventUserdata)
{
    pthread_mutex_lock(&config->lock);

    config->loadedConfig = loadedConfig;
    config->spec->acceptConfig(config->spec->self, loadedConfig != NULL ? &loadedConfig->loaded : NULL);

    if (eventConstructor != NULL)
    {
        LIBMATTI_FML_Event_Config_ModConfigEvent *event = eventConstructor(config);
        if (event != NULL)
        {
            LIBMATTI_FML_ModContainer_AcceptEvent(config->container, &event->base);
            LIBMATTI_FML_Event_Config_ModConfigEvent_Free(event);
        }
    }

    pthread_mutex_unlock(&config->lock);
}
