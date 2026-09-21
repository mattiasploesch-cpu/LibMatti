// Port of net.neoforged.neoforgespi.language.IModFileInfo.
// getFileProperties() carries the ported java.util.HashMap; VersionRange resolves
// through the ported MavenVersionAdapter. IConfigurable is implemented by ModFileInfo.

#ifndef MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODFILEINFO_H
#define MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODFILEINFO_H

#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include <stddef.h>

typedef struct LIBMATTI_NEOFORGESPI_IModFile LIBMATTI_NEOFORGESPI_IModFile;
typedef struct LIBMATTI_NEOFORGESPI_IModInfo LIBMATTI_NEOFORGESPI_IModInfo;

// Java: record LanguageSpec(@Nullable String languageName, @Nullable VersionRange acceptedVersions) {}
typedef struct
{
    char *languageName; // Java: @Nullable String languageName
    LIBMATTI_NEOFORGESPI_VersionRange *acceptedVersions; // Java: @Nullable VersionRange acceptedVersions
} LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec;

// Java: public interface IModFileInfo
typedef struct LIBMATTI_NEOFORGESPI_IModFileInfo LIBMATTI_NEOFORGESPI_IModFileInfo;

struct LIBMATTI_NEOFORGESPI_IModFileInfo
{
    void *self;

    // Java: List<IModInfo> getMods()
    LIBMATTI_NEOFORGESPI_IModInfo **(*getMods)(void *self, size_t *count);
    // Java: List<LanguageSpec> requiredLanguageLoaders()
    LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *(*requiredLanguageLoaders)(void *self, size_t *count);
    // Java: boolean showAsResourcePack()
    int (*showAsResourcePack)(void *self);
    // Java: boolean showAsDataPack()
    int (*showAsDataPack)(void *self);
    // Java: Map<String, Object> getFileProperties()
    void *(*getFileProperties)(void *self);
    // Java: String getLicense()
    const char *(*getLicense)(void *self);
    // Java: String versionString()
    const char *(*versionString)(void *self);
    // Java: List<String> usesServices()
    char **(*usesServices)(void *self, size_t *count);
    // Java: IModFile getFile()
    LIBMATTI_NEOFORGESPI_IModFile *(*getFile)(void *self);
    // Java: IConfigurable getConfig()
    void *(*getConfig)(void *self);
    // Java: the GC - the port releases the implementation explicitly
    void (*free)(void *self);
};

// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count);
// Java: public List<LanguageSpec> requiredLanguageLoaders()
LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *LIBMATTI_NEOFORGESPI_IModFileInfo_RequiredLanguageLoaders(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count);
// Java: public boolean showAsResourcePack()
int LIBMATTI_NEOFORGESPI_IModFileInfo_ShowAsResourcePack(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public boolean showAsDataPack()
int LIBMATTI_NEOFORGESPI_IModFileInfo_ShowAsDataPack(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public Map<String, Object> getFileProperties()
void *LIBMATTI_NEOFORGESPI_IModFileInfo_GetFileProperties(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public String getLicense()
const char *LIBMATTI_NEOFORGESPI_IModFileInfo_GetLicense(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public String versionString()
const char *LIBMATTI_NEOFORGESPI_IModFileInfo_VersionString(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public List<String> usesServices()
char **LIBMATTI_NEOFORGESPI_IModFileInfo_UsesServices(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo,
                                                     size_t *count);
// Java: public IModFile getFile()
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(
    const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: public IConfigurable getConfig()
void *LIBMATTI_NEOFORGESPI_IModFileInfo_GetConfig(const LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);
// Java: the GC - releases the implementation
void LIBMATTI_NEOFORGESPI_IModFileInfo_Free(LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo);

#endif //MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODFILEINFO_H
