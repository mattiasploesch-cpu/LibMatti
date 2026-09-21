// Port of net.neoforged.fml.loading.moddiscovery.locators.MinecraftModInfo.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MINECRAFTMODINFO_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MINECRAFTMODINFO_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

typedef struct LIBMATTI_FML_MinecraftModInfo LIBMATTI_FML_MinecraftModInfo;

struct LIBMATTI_FML_MinecraftModInfo
{
    // Java: var conf = Config.inMemory()
    LIBMATTI_NC_Config *config;
    char *minecraftVersion;
};

// Java: public MinecraftModInfo(String minecraftVersion)
LIBMATTI_FML_MinecraftModInfo *LIBMATTI_FML_MinecraftModInfo_New(const char *minecraftVersion);

// Java: public IModFileInfo buildMinecraftModInfo(IModFile iModFile) - as a ModFileInfoParser
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_MinecraftModInfo_BuildMinecraftModInfo(void *iModFile,
                                                                                      void *userdata);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MINECRAFTMODINFO_H
