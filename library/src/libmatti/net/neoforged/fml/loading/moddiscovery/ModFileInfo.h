// Port of net.neoforged.fml.loading.moddiscovery.ModFileInfo.
// TODO: java.net.URL - the issue URL stays an opaque handle.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEINFO_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEINFO_H

#include "libmatti/java/util/HashMap.h"
#include "libmatti/net/neoforged/neoforgespi/language/IConfigurable.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_ModFile LIBMATTI_FML_ModFile;
typedef struct LIBMATTI_FML_ModInfo LIBMATTI_FML_ModInfo;

// Java: java.util.function.Consumer<IModFileInfo>
typedef void (*LIBMATTI_FML_ModFileInfo_ConfigFileConsumer)(LIBMATTI_NEOFORGESPI_IModFileInfo *file, void *userdata);

// Java: public class ModFileInfo implements IModFileInfo, IConfigurable
typedef struct LIBMATTI_FML_ModFileInfo LIBMATTI_FML_ModFileInfo;

struct LIBMATTI_FML_ModFileInfo
{
    LIBMATTI_NEOFORGESPI_IModFileInfo modFileInfo;
    LIBMATTI_NEOFORGESPI_IConfigurable configurable;

    LIBMATTI_NEOFORGESPI_IConfigurable *config;
    LIBMATTI_FML_ModFile *modFile;
    void *issueURL; // Java: private final URL issueURL
    LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *languageSpecs;
    size_t languageSpecCount;
    int showAsResourcePack;
    int showAsDataPack;
    LIBMATTI_NEOFORGESPI_IModInfo **mods;
    size_t modCount;
    LIBMATTI_JU_HashMap *properties; // Java: Map<String, Object> properties
    char *license;
    char **usesServices;
    size_t usesServiceCount;
};

// Java: @ApiStatus.Internal public ModFileInfo(ModFile modFile, IConfigurable config, Consumer<IModFileInfo> configFileConsumer)
// NULL when the file is invalid (Java: throws InvalidModFileException)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModFileInfo_New(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_IConfigurable *config,
    LIBMATTI_FML_ModFileInfo_ConfigFileConsumer configFileConsumer, void *userdata);

// Java: public ModFileInfo(ModFile file, IConfigurable config, Consumer<IModFileInfo> configFileConsumer, List<LanguageSpec> languageSpecs)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModFileInfo_NewWithLanguageSpecs(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_IConfigurable *config,
    LIBMATTI_FML_ModFileInfo_ConfigFileConsumer configFileConsumer, void *userdata,
    const LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *languageSpecs, size_t languageSpecCount);

void LIBMATTI_FML_ModFileInfo_Free(LIBMATTI_FML_ModFileInfo *modFileInfo);

// Java: the instance used as an IModFileInfo / IConfigurable
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileInfo_AsModFileInfo(LIBMATTI_FML_ModFileInfo *modFileInfo);
LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_ModFileInfo_AsConfigurable(LIBMATTI_FML_ModFileInfo *modFileInfo);

// Java: public ModFile getFile()
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFileInfo_GetFile(const LIBMATTI_FML_ModFileInfo *modFileInfo);
// Java: public URL getIssueURL()
void *LIBMATTI_FML_ModFileInfo_GetIssueURL(const LIBMATTI_FML_ModFileInfo *modFileInfo);
// Java: public Optional<String> getCodeSigningFingerprint() - 1 if present, *out is a new string
int LIBMATTI_FML_ModFileInfo_GetCodeSigningFingerprint(const LIBMATTI_FML_ModFileInfo *modFileInfo, char **out);
// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_FML_ModFileInfo_GetMods(const LIBMATTI_FML_ModFileInfo *modFileInfo,
                                                                 size_t *count);
// Java: public <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_FML_ModFileInfo_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                              LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);
// Java: public List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_ModFileInfo_GetConfigList(void *self, const char *const *key,
                                                                            size_t keyCount, size_t *count);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEINFO_H
