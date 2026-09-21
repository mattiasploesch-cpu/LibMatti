#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"

#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count)
{
    return modFileInfo->getMods(modFileInfo->self, count);
}

// Java: public List<LanguageSpec> requiredLanguageLoaders()
LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *LIBMATTI_NEOFORGESPI_IModFileInfo_RequiredLanguageLoaders(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count)
{
    return modFileInfo->requiredLanguageLoaders(modFileInfo->self, count);
}

// Java: public boolean showAsResourcePack()
int LIBMATTI_NEOFORGESPI_IModFileInfo_ShowAsResourcePack(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->showAsResourcePack(modFileInfo->self);
}

// Java: public boolean showAsDataPack()
int LIBMATTI_NEOFORGESPI_IModFileInfo_ShowAsDataPack(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->showAsDataPack(modFileInfo->self);
}

// Java: public Map<String, Object> getFileProperties()
void *LIBMATTI_NEOFORGESPI_IModFileInfo_GetFileProperties(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->getFileProperties(modFileInfo->self);
}

// Java: public String getLicense()
const char *LIBMATTI_NEOFORGESPI_IModFileInfo_GetLicense(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->getLicense(modFileInfo->self);
}

// Java: public String versionString()
const char *LIBMATTI_NEOFORGESPI_IModFileInfo_VersionString(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->versionString(modFileInfo->self);
}

// Java: public List<String> usesServices()
char **LIBMATTI_NEOFORGESPI_IModFileInfo_UsesServices(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo,
                                                     size_t *count)
{
    return modFileInfo->usesServices(modFileInfo->self, count);
}

// Java: public IModFile getFile()
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->getFile(modFileInfo->self);
}

// Java: public IConfigurable getConfig()
void *LIBMATTI_NEOFORGESPI_IModFileInfo_GetConfig(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    return modFileInfo->getConfig(modFileInfo->self);
}

// Java: the GC - releases the implementation
void LIBMATTI_NEOFORGESPI_IModFileInfo_Free(LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo)
{
    if (modFileInfo == NULL || modFileInfo->free == NULL)
        return;
    modFileInfo->free(modFileInfo->self);
}
