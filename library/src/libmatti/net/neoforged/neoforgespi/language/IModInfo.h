// Port of net.neoforged.neoforgespi.language.IModInfo.
// The update/mod/referral URLs stay opaque handles (the port keeps java.net.URL as
// its string form); ArtifactVersion / VersionRange resolve through MavenVersionAdapter
// and IConfigurable is implemented by ModInfo.

#ifndef MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODINFO_H
#define MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODINFO_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include <stddef.h>

// Java: public interface IModInfo
typedef struct LIBMATTI_NEOFORGESPI_IModInfo LIBMATTI_NEOFORGESPI_IModInfo;
typedef struct LIBMATTI_NEOFORGESPI_IModFileInfo LIBMATTI_NEOFORGESPI_IModFileInfo;

// Java: enum Ordering { BEFORE, AFTER, NONE }
typedef enum
{
    LIBMATTI_NEOFORGESPI_IModInfo_Ordering_BEFORE,
    LIBMATTI_NEOFORGESPI_IModInfo_Ordering_AFTER,
    LIBMATTI_NEOFORGESPI_IModInfo_Ordering_NONE
} LIBMATTI_NEOFORGESPI_IModInfo_Ordering;

// Java: enum DependencySide { CLIENT, SERVER, BOTH }
typedef enum
{
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_CLIENT,
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_SERVER,
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH
} LIBMATTI_NEOFORGESPI_IModInfo_DependencySide;

// Java: enum DependencyType { REQUIRED, OPTIONAL, INCOMPATIBLE, DISCOURAGED }
typedef enum
{
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED,
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_OPTIONAL,
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_INCOMPATIBLE,
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_DISCOURAGED
} LIBMATTI_NEOFORGESPI_IModInfo_DependencyType;

// Java: interface ModVersion
typedef struct LIBMATTI_NEOFORGESPI_IModInfo_ModVersion LIBMATTI_NEOFORGESPI_IModInfo_ModVersion;

struct LIBMATTI_NEOFORGESPI_IModInfo_ModVersion
{
    void *self;

    // Java: String getModId()
    const char *(*getModId)(void *self);
    // Java: VersionRange getVersionRange()
    const LIBMATTI_NEOFORGESPI_VersionRange *(*getVersionRange)(void *self);
    // Java: DependencyType getType()
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType (*getType)(void *self);
    // Java: Optional<String> getReason() - 1 if present
    int (*getReason)(void *self, const char **out);
    // Java: Ordering getOrdering()
    LIBMATTI_NEOFORGESPI_IModInfo_Ordering (*getOrdering)(void *self);
    // Java: DependencySide getSide()
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide (*getSide)(void *self);
    // Java: void setOwner(IModInfo owner)
    void (*setOwner)(void *self, LIBMATTI_NEOFORGESPI_IModInfo *owner);
    // Java: IModInfo getOwner()
    LIBMATTI_NEOFORGESPI_IModInfo *(*getOwner)(void *self);
    // Java: Optional<URL> getReferralURL() - 1 if present, opaque handle
    int (*getReferralURL)(void *self, void **out);
};

// Java: public interface IModInfo
struct LIBMATTI_NEOFORGESPI_IModInfo
{
    void *self;

    // Java: IModFileInfo getOwningFile()
    LIBMATTI_NEOFORGESPI_IModFileInfo *(*getOwningFile)(void *self);
    // Java: IModLanguageLoader getLoader()
    void *(*getLoader)(void *self);
    // Java: String getModId()
    const char *(*getModId)(void *self);
    // Java: String getDisplayName()
    const char *(*getDisplayName)(void *self);
    // Java: String getDescription()
    const char *(*getDescription)(void *self);
    // Java: ArtifactVersion getVersion()
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *(*getVersion)(void *self);
    // Java: List<? extends ModVersion> getDependencies()
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **(*getDependencies)(void *self, size_t *count);
    // Java: List<? extends ForgeFeature.Bound> getForgeFeatures()
    void **(*getForgeFeatures)(void *self, size_t *count);
    // Java: String getNamespace()
    const char *(*getNamespace)(void *self);
    // Java: Map<String, Object> getModProperties()
    void *(*getModProperties)(void *self);
    // Java: Optional<URL> getUpdateURL() - 1 if present, opaque handle
    int (*getUpdateURL)(void *self, void **out);
    // Java: Optional<URL> getModURL() - 1 if present, opaque handle
    int (*getModURL)(void *self, void **out);
    // Java: Optional<String> getLogoFile() - 1 if present
    int (*getLogoFile)(void *self, const char **out);
    // Java: boolean getLogoBlur()
    int (*getLogoBlur)(void *self);
    // Java: IConfigurable getConfig()
    void *(*getConfig)(void *self);
};

// Java: public IModFileInfo getOwningFile()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public IModLanguageLoader getLoader()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetLoader(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public String getModId()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetModId(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public String getDisplayName()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public String getDescription()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetDescription(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public ArtifactVersion getVersion()
const LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public List<? extends ModVersion> getDependencies()
LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **LIBMATTI_NEOFORGESPI_IModInfo_GetDependencies(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, size_t *count);
// Java: public List<? extends ForgeFeature.Bound> getForgeFeatures()
void **LIBMATTI_NEOFORGESPI_IModInfo_GetForgeFeatures(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, size_t *count);
// Java: public String getNamespace()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetNamespace(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public Map<String, Object> getModProperties()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetModProperties(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public Optional<URL> getUpdateURL()
int LIBMATTI_NEOFORGESPI_IModInfo_GetUpdateURL(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, void **out);
// Java: public Optional<URL> getModURL()
int LIBMATTI_NEOFORGESPI_IModInfo_GetModURL(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, void **out);
// Java: public Optional<String> getLogoFile()
int LIBMATTI_NEOFORGESPI_IModInfo_GetLogoFile(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, const char **out);
// Java: public boolean getLogoBlur()
int LIBMATTI_NEOFORGESPI_IModInfo_GetLogoBlur(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);
// Java: public IConfigurable getConfig()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetConfig(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);

// Java: public boolean isContained(Dist side)
int LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsContained(LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side,
                                                             LIBMATTI_DIST_Dist dist);
// Java: public boolean isCorrectSide()
int LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsCorrectSide(LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side);

// Java: VersionRange UNBOUNDED = MavenVersionAdapter.createFromVersionSpec(" ")
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_IModInfo_Unbounded(void);

// Java: public String getModId() on ModVersion
const char *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public VersionRange getVersionRange()
const LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public DependencyType getType()
LIBMATTI_NEOFORGESPI_IModInfo_DependencyType LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public Optional<String> getReason()
int LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetReason(const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion,
                                                      const char **out);
// Java: public Ordering getOrdering()
LIBMATTI_NEOFORGESPI_IModInfo_Ordering LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOrdering(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public DependencySide getSide()
LIBMATTI_NEOFORGESPI_IModInfo_DependencySide LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetSide(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public void setOwner(IModInfo owner)
void LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_SetOwner(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion,
                                                      LIBMATTI_NEOFORGESPI_IModInfo *owner);
// Java: public IModInfo getOwner()
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion);
// Java: public Optional<URL> getReferralURL()
int LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetReferralURL(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion, void **out);

#endif //MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODINFO_H
