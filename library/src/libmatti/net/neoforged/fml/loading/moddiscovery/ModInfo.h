// Port of net.neoforged.fml.loading.moddiscovery.ModInfo.
// Java's Map<String, Object> mod properties map to the LIBMATTI_JU_HashMap port (String keys,
// the values are the night-config values converted on read).
// TODO: org.apache.maven.artifact.versioning - ArtifactVersion is kept by the port.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODINFO_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODINFO_H

#include "libmatti/java/util/HashMap.h"
#include "libmatti/net/neoforged/neoforgespi/language/IConfigurable.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ForgeFeature.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_ModFileInfo LIBMATTI_FML_ModFileInfo;
typedef struct LIBMATTI_FML_ModInfo LIBMATTI_FML_ModInfo;

// Java: class ModVersion implements net.neoforged.neoforgespi.language.IModInfo.ModVersion
typedef struct LIBMATTI_FML_ModVersion LIBMATTI_FML_ModVersion;

struct LIBMATTI_FML_ModVersion
{
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion modVersion;

    LIBMATTI_NEOFORGESPI_IModInfo *owner;
    char *modId;
    LIBMATTI_NEOFORGESPI_VersionRange *versionRange;
    LIBMATTI_NEOFORGESPI_IModInfo_DependencyType type;
    int reasonPresent;
    char *reason;
    LIBMATTI_NEOFORGESPI_IModInfo_Ordering ordering;
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side;
    int referralUrlPresent;
    void *referralUrl; // Java: Optional<URL> referralUrl
};

// Java: public class ModInfo implements IModInfo, IConfigurable
struct LIBMATTI_FML_ModInfo
{
    LIBMATTI_NEOFORGESPI_IModInfo modInfo;
    LIBMATTI_NEOFORGESPI_IConfigurable configurable;

    LIBMATTI_FML_ModFileInfo *owningFile;
    LIBMATTI_NEOFORGESPI_IModFileInfo *owningFileInfo;
    char *modId;
    char *namespace;
    LIBMATTI_NEOFORGESPI_ArtifactVersion *version;
    char *displayName;
    char *description;
    int logoFilePresent;
    char *logoFile;
    int logoBlur;
    int updateURLPresent;
    void *updateJSONURL; // Java: Optional<URL> updateJSONURL
    LIBMATTI_FML_ModVersion **dependencies;
    size_t dependencyCount;
    LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *features;
    void **featurePointers; // Java: List<Bound>
    size_t featureCount;
    LIBMATTI_JU_HashMap *properties; // Java: Map<String, Object> modProperties
    LIBMATTI_NEOFORGESPI_IConfigurable *config;
    int modUrlPresent;
    void *modUrl; // Java: Optional<URL> modUrl
};

// Java: public ModInfo(ModFileInfo owningFile, IConfigurable config) - NULL on InvalidModFileException
LIBMATTI_FML_ModInfo *LIBMATTI_FML_ModInfo_New(LIBMATTI_FML_ModFileInfo *owningFile,
                                               LIBMATTI_NEOFORGESPI_IConfigurable *config);
void LIBMATTI_FML_ModInfo_Free(LIBMATTI_FML_ModInfo *modInfo);

// Java: the instance used as an IModInfo / IConfigurable
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModInfo_AsModInfo(LIBMATTI_FML_ModInfo *modInfo);
LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_ModInfo_AsConfigurable(LIBMATTI_FML_ModInfo *modInfo);

// Java: public ModFileInfo getOwningFile()
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModInfo_GetOwningFile(const LIBMATTI_FML_ModInfo *modInfo);
// Java: public String getModId()
const char *LIBMATTI_FML_ModInfo_GetModId(const LIBMATTI_FML_ModInfo *modInfo);
// Java: public List<? extends ModVersion> getDependencies()
LIBMATTI_FML_ModVersion **LIBMATTI_FML_ModInfo_GetDependencies(const LIBMATTI_FML_ModInfo *modInfo, size_t *count);
// Java: public <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_FML_ModInfo_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                          LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);
// Java: public List<? extends IConfigurable> getConfigList(String... key) - returns null, as in Java
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_ModInfo_GetConfigList(void *self, const char *const *key,
                                                                        size_t keyCount, size_t *count);
// Java: @Override public String toString()
char *LIBMATTI_FML_ModInfo_ToString(const LIBMATTI_FML_ModInfo *modInfo);

// Java: public String getModId() (ModVersion)
const char *LIBMATTI_FML_ModVersion_GetModId(const LIBMATTI_FML_ModVersion *modVersion);
// Java: public VersionRange getVersionRange() (ModVersion)
const LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_FML_ModVersion_GetVersionRange(
    const LIBMATTI_FML_ModVersion *modVersion);
// Java: public DependencyType getType() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_DependencyType LIBMATTI_FML_ModVersion_GetType(
    const LIBMATTI_FML_ModVersion *modVersion);
// Java: public Ordering getOrdering() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_Ordering LIBMATTI_FML_ModVersion_GetOrdering(
    const LIBMATTI_FML_ModVersion *modVersion);
// Java: public DependencySide getSide() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_DependencySide LIBMATTI_FML_ModVersion_GetSide(
    const LIBMATTI_FML_ModVersion *modVersion);
// Java: public Optional<String> getReason() (ModVersion) - 1 if present
int LIBMATTI_FML_ModVersion_GetReason(const LIBMATTI_FML_ModVersion *modVersion, const char **out);
// Java: public IModInfo getOwner() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModVersion_GetOwner(LIBMATTI_FML_ModVersion *modVersion);
// Java: public void setOwner(IModInfo owner) (ModVersion)
void LIBMATTI_FML_ModVersion_SetOwner(LIBMATTI_FML_ModVersion *modVersion, LIBMATTI_NEOFORGESPI_IModInfo *owner);
void LIBMATTI_FML_ModVersion_Free(LIBMATTI_FML_ModVersion *modVersion);

// Java: public final class ModInfo implements IModInfo ... the port needs the concrete class for the sorter
LIBMATTI_FML_ModInfo *LIBMATTI_FML_ModInfo_AsConcrete(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODINFO_H
