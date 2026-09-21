// Port of net.neoforged.fml.config.LoadedConfig.
// Java: record LoadedConfig(CommentedConfig config, @Nullable Path path, ModConfig modConfig)
//       implements IConfigSpec.ILoadedConfig

#ifndef MATTICRAFT_FML_CONFIG_LOADEDCONFIG_H
#define MATTICRAFT_FML_CONFIG_LOADEDCONFIG_H

#include "libmatti/net/neoforged/fml/config/IConfigSpec.h"

typedef struct LIBMATTI_FML_ModConfig LIBMATTI_FML_ModConfig;
typedef struct LIBMATTI_FML_LoadedConfig LIBMATTI_FML_LoadedConfig;

struct LIBMATTI_FML_LoadedConfig
{
    // Java: implements ILoadedConfig - the interface the spec sees
    LIBMATTI_FML_ILoadedConfig loaded;

    // Java: the record's components
    LIBMATTI_NC_Config *config;
    char *path; // NULL = loaded from memory
    LIBMATTI_FML_ModConfig *modConfig;
};

// Java: record LoadedConfig(CommentedConfig config, @Nullable Path path, ModConfig modConfig)
// 'config' and 'path' are taken over (the tracker builds them fresh for every load).
LIBMATTI_FML_LoadedConfig *LIBMATTI_FML_LoadedConfig_New(LIBMATTI_NC_Config *config, const char *path,
                                                         LIBMATTI_FML_ModConfig *modConfig);
void LIBMATTI_FML_LoadedConfig_Free(LIBMATTI_FML_LoadedConfig *config);

#endif //MATTICRAFT_FML_CONFIG_LOADEDCONFIG_H
