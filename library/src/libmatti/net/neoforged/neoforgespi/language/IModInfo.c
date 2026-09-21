#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"

// Java: public IModFileInfo getOwningFile()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getOwningFile(modInfo->self);
}

// Java: public IModLanguageLoader getLoader()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetLoader(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getLoader(modInfo->self);
}

// Java: public String getModId()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetModId(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getModId(modInfo->self);
}

// Java: public String getDisplayName()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetDisplayName(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getDisplayName(modInfo->self);
}

// Java: public String getDescription()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetDescription(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getDescription(modInfo->self);
}

// Java: public ArtifactVersion getVersion()
const LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getVersion(modInfo->self);
}

// Java: public List<? extends ModVersion> getDependencies()
LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **LIBMATTI_NEOFORGESPI_IModInfo_GetDependencies(
    const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, size_t *count)
{
    return modInfo->getDependencies(modInfo->self, count);
}

// Java: public List<? extends ForgeFeature.Bound> getForgeFeatures()
void **LIBMATTI_NEOFORGESPI_IModInfo_GetForgeFeatures(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, size_t *count)
{
    return modInfo->getForgeFeatures(modInfo->self, count);
}

// Java: public String getNamespace()
const char *LIBMATTI_NEOFORGESPI_IModInfo_GetNamespace(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getNamespace(modInfo->self);
}

// Java: public Map<String, Object> getModProperties()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetModProperties(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getModProperties(modInfo->self);
}

// Java: public Optional<URL> getUpdateURL()
int LIBMATTI_NEOFORGESPI_IModInfo_GetUpdateURL(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, void **out)
{
    return modInfo->getUpdateURL(modInfo->self, out);
}

// Java: public Optional<URL> getModURL()
int LIBMATTI_NEOFORGESPI_IModInfo_GetModURL(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, void **out)
{
    return modInfo->getModURL(modInfo->self, out);
}

// Java: public Optional<String> getLogoFile()
int LIBMATTI_NEOFORGESPI_IModInfo_GetLogoFile(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo, const char **out)
{
    return modInfo->getLogoFile(modInfo->self, out);
}

// Java: public boolean getLogoBlur()
int LIBMATTI_NEOFORGESPI_IModInfo_GetLogoBlur(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getLogoBlur(modInfo->self);
}

// Java: public IConfigurable getConfig()
void *LIBMATTI_NEOFORGESPI_IModInfo_GetConfig(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo->getConfig(modInfo->self);
}

// Java: public String getModId()
const char *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getModId(modVersion->self);
}

// Java: public VersionRange getVersionRange()
const LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getVersionRange(modVersion->self);
}

// Java: public DependencyType getType()
LIBMATTI_NEOFORGESPI_IModInfo_DependencyType LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getType(modVersion->self);
}

// Java: public Optional<String> getReason()
int LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetReason(const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion,
                                                      const char **out)
{
    return modVersion->getReason(modVersion->self, out);
}

// Java: public Ordering getOrdering()
LIBMATTI_NEOFORGESPI_IModInfo_Ordering LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOrdering(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getOrdering(modVersion->self);
}

// Java: public DependencySide getSide()
LIBMATTI_NEOFORGESPI_IModInfo_DependencySide LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetSide(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getSide(modVersion->self);
}

// Java: public void setOwner(IModInfo owner)
void LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_SetOwner(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion,
                                                      LIBMATTI_NEOFORGESPI_IModInfo *owner)
{
    modVersion->setOwner(modVersion->self, owner);
}

// Java: public IModInfo getOwner()
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion)
{
    return modVersion->getOwner(modVersion->self);
}

// Java: public Optional<URL> getReferralURL()
int LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetReferralURL(
    const LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *modVersion, void **out)
{
    return modVersion->getReferralURL(modVersion->self, out);
}

// Java: public boolean isContained(Dist side) { return this == BOTH || dist[0] == side; }
int LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsContained(LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side,
                                                             LIBMATTI_DIST_Dist dist)
{
    if (side == LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH)
        return 1;

    LIBMATTI_DIST_Dist values[] = {LIBMATTI_DIST_CLIENT, LIBMATTI_DIST_DEDICATED_SERVER};
    return values[0] == dist;
}

// Java: public boolean isCorrectSide() { return this == BOTH || FMLLoader.getCurrent().getDist().equals(dist[0]); }
int LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsCorrectSide(LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side)
{
    if (side == LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH)
        return 1;

    LIBMATTI_DIST_Dist values[] = {LIBMATTI_DIST_CLIENT, LIBMATTI_DIST_DEDICATED_SERVER};
    return LIBMATTI_FML_FMLLoader_GetDist(LIBMATTI_FML_FMLLoader_GetCurrent()) == values[0];
}

static LIBMATTI_NEOFORGESPI_VersionRange *unbounded = NULL;

// Java: VersionRange UNBOUNDED = MavenVersionAdapter.createFromVersionSpec(" ")
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_IModInfo_Unbounded(void)
{
    // Java: " " will just be the preferred version for Maven, but it will accept anything
    if (unbounded == NULL)
        unbounded = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(" ");
    return unbounded;
}
