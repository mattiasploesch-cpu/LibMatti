// Port of net.neoforged.fml.config.LoadedConfig.

#include "libmatti/net/neoforged/fml/config/LoadedConfig.h"

#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/config/ConfigTracker.h"
#include "libmatti/net/neoforged/fml/config/ModConfig.h"
#include "libmatti/net/neoforged/fml/event/config/ModConfigEvent.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_NC_Config *loaded_config(void *self)
{
    LIBMATTI_FML_LoadedConfig *config = self;
    return config->config;
}

// Java: @Override public void save()
static void loaded_save(void *self)
{
    LIBMATTI_FML_LoadedConfig *config = self;

    // Java: if (path != null) { ConfigTracker.writeConfig(path, config); }
    if (config->path != NULL) LIBMATTI_FML_ConfigTracker_WriteConfig(config->path, config->config);

    // Java: modConfig.lock.lock(); try { modConfig.container.acceptEvent(new ModConfigEvent.Reloading(modConfig)); }
    //       finally { modConfig.lock.unlock(); }
    pthread_mutex_lock(&config->modConfig->lock);
    LIBMATTI_FML_Event_Config_ModConfigEvent *event =
        LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_New(config->modConfig);
    LIBMATTI_FML_ModContainer_AcceptEvent(config->modConfig->container, &event->base);
    pthread_mutex_unlock(&config->modConfig->lock);
}

LIBMATTI_FML_LoadedConfig *LIBMATTI_FML_LoadedConfig_New(LIBMATTI_NC_Config *config, const char *path,
                                                         LIBMATTI_FML_ModConfig *modConfig)
{
    LIBMATTI_FML_LoadedConfig *loaded = calloc(1, sizeof(*loaded));
    loaded->config = config;
    loaded->path = path != NULL ? strdup(path) : NULL;
    loaded->modConfig = modConfig;
    loaded->loaded.self = loaded;
    loaded->loaded.config = loaded_config;
    loaded->loaded.save = loaded_save;
    return loaded;
}

void LIBMATTI_FML_LoadedConfig_Free(LIBMATTI_FML_LoadedConfig *config)
{
    if (config == NULL) return;

    LIBMATTI_NC_Config_Free(config->config);
    free(config->path);
    free(config);
}
