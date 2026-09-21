#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/MinecraftModInfo.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/NightConfigWrapper.h"

#include <stdlib.h>
#include <string.h>

// Java: conf.set(key, value) for a String value
static void setString(LIBMATTI_NC_Config *config, const char *key, const char *value)
{
    const char *path[1] = {key};
    LIBMATTI_NC_Value stringValue = LIBMATTI_NC_Value_OfString(value);
    LIBMATTI_NC_Config_Set(config, path, 1, &stringValue);
}

// Java: public MinecraftModInfo(String minecraftVersion)
LIBMATTI_FML_MinecraftModInfo *LIBMATTI_FML_MinecraftModInfo_New(const char *minecraftVersion)
{
    LIBMATTI_FML_MinecraftModInfo *modInfo = calloc(1, sizeof(LIBMATTI_FML_MinecraftModInfo));
    modInfo->minecraftVersion = strdup(minecraftVersion);

    // Java: var conf = Config.inMemory(); conf.set("modLoader", "minecraft"); ...
    modInfo->config = LIBMATTI_NC_Config_New();
    setString(modInfo->config, "modLoader", "minecraft");
    setString(modInfo->config, "loaderVersion", "1");
    setString(modInfo->config, "license", "Mojang Studios, All Rights Reserved");

    // Java: var mods = Config.inMemory(); mods.set("modId", "minecraft"); ...
    LIBMATTI_NC_Config *mods = LIBMATTI_NC_Config_New();
    setString(mods, "modId", "minecraft");
    setString(mods, "version", minecraftVersion);
    setString(mods, "displayName", "Minecraft");
    setString(mods, "description", "Minecraft");

    // Java: conf.set("mods", List.of(mods))
    LIBMATTI_NC_Value *list = malloc(sizeof(LIBMATTI_NC_Value));
    list[0] = LIBMATTI_NC_Value_OfConfig(mods);
    LIBMATTI_NC_Value listValue = LIBMATTI_NC_Value_OfList(list, 1);
    const char *modsKey[1] = {"mods"};
    LIBMATTI_NC_Config_Set(modInfo->config, modsKey, 1, &listValue);

    return modInfo;
}

// Java: public IModFileInfo buildMinecraftModInfo(IModFile iModFile)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_MinecraftModInfo_BuildMinecraftModInfo(void *iModFile,
                                                                                      void *userdata)
{
    LIBMATTI_FML_MinecraftModInfo *minecraftModInfo = userdata;
    LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFile_AsConcrete(iModFile);

    // Java: NightConfigWrapper configWrapper = new NightConfigWrapper(conf);
    //       return new ModFileInfo(modFile, configWrapper, configWrapper::setFile, List.of());
    LIBMATTI_FML_NightConfigWrapper *configWrapper =
        LIBMATTI_FML_NightConfigWrapper_New(minecraftModInfo->config);
    LIBMATTI_FML_ModFileInfo *modFileInfo = LIBMATTI_FML_ModFileInfo_NewWithLanguageSpecs(
        modFile, LIBMATTI_FML_NightConfigWrapper_AsConfigurable(configWrapper),
        LIBMATTI_FML_NightConfigWrapper_SetFileConsumer, configWrapper, NULL, 0);
    return LIBMATTI_FML_ModFileInfo_AsModFileInfo(modFileInfo);
}
