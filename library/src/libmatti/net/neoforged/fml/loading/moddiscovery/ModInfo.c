#include "libmatti/com/electronwill/nightconfig/core/Config.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/NightConfigWrapper.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/StringSubstitutor.h"
#include "libmatti/net/neoforged/fml/loading/StringUtils.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/InvalidModFileException.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final Pattern VALID_MODID = Pattern.compile("^(?=.{2,64}$)[a-z][a-z0-9_]*(\\.[a-z][a-z0-9_]*)*$")
static int validModId(const char *modId)
{
    size_t length = strlen(modId);
    if (length < 2 || length > 64) return 0;

    int atSegmentStart = 1;
    for (size_t i = 0; i < length; i++)
    {
        char c = modId[i];
        if (atSegmentStart)
        {
            if (c < 'a' || c > 'z') return 0;
            atSegmentStart = 0;
        }
        else if (c == '.')
        {
            atSegmentStart = 1;
        }
        else if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
        {
            return 0;
        }
    }
    return !atSegmentStart;
}

// Java: private static final Pattern VALID_NAMESPACE = Pattern.compile("^[a-z][a-z0-9_.-]{1,63}$")
static int validNamespace(const char *namespace)
{
    size_t length = strlen(namespace);
    if (length < 2 || length > 64) return 0;
    if (namespace[0] < 'a' || namespace[0] > 'z') return 0;

    for (size_t i = 1; i < length; i++)
    {
        char c = namespace[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-'))
            return 0;
    }
    return 1;
}

// Java: private static final Pattern VALID_VERSION = Pattern.compile("^\\d+.*")
static int validVersion(const char *version)
{
    return version[0] >= '0' && version[0] <= '9';
}

// Java: config.<String>getConfigElement(key).orElse(null) - the string, or NULL if absent
static char *configString(LIBMATTI_NEOFORGESPI_IConfigurable *config, const char *key)
{
    LIBMATTI_NEOFORGESPI_IConfigurable_Value value;
    if (!LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(config, key, &value))
        return NULL;
    if (value.type != LIBMATTI_NEOFORGESPI_IConfigurable_STRING)
        return NULL;
    return strdup(value.string);
}

// Java: config.<Boolean>getConfigElement(key) - 1 if present
static int configBool(LIBMATTI_NEOFORGESPI_IConfigurable *config, const char *key, int *out)
{
    LIBMATTI_NEOFORGESPI_IConfigurable_Value value;
    if (!LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(config, key, &value))
        return 0;
    if (value.type != LIBMATTI_NEOFORGESPI_IConfigurable_BOOL)
        return 0;
    *out = value.boolean;
    return 1;
}

// Java: LOGGER.error(LogUtils.FATAL_MARKER, "Invalid ... found in file {} - {} does not match the standard: {}", ...)
static void logInvalid(const char *what, LIBMATTI_NEOFORGESPI_IModFileInfo *owningFileInfo, const char *value,
                       const char *pattern)
{
    LIBMATTI_FML_ModFile *file = LIBMATTI_FML_ModFileInfo_GetFile((LIBMATTI_FML_ModFileInfo *) owningFileInfo);
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Invalid {} found in file {} - {} does not match the standard: {}", what,
                             LIBMATTI_NEOFORGESPI_IModFile_GetFilePath((LIBMATTI_NEOFORGESPI_IModFile *) &file->modFile),
                             value, pattern);
}

// ---------------------------------------------------------------------------
// ModVersion
// ---------------------------------------------------------------------------

static const char *modVersion_getModId(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->modId;
}

static const LIBMATTI_NEOFORGESPI_VersionRange *modVersion_getVersionRange(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->versionRange;
}

static LIBMATTI_NEOFORGESPI_IModInfo_DependencyType modVersion_getType(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->type;
}

static int modVersion_getReason(void *self, const char **out)
{
    LIBMATTI_FML_ModVersion *modVersion = self;
    if (!modVersion->reasonPresent) return 0;
    *out = modVersion->reason;
    return 1;
}

static LIBMATTI_NEOFORGESPI_IModInfo_Ordering modVersion_getOrdering(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->ordering;
}

static LIBMATTI_NEOFORGESPI_IModInfo_DependencySide modVersion_getSide(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->side;
}

static void modVersion_setOwner(void *self, LIBMATTI_NEOFORGESPI_IModInfo *owner)
{
    ((LIBMATTI_FML_ModVersion *) self)->owner = owner;
}

static LIBMATTI_NEOFORGESPI_IModInfo *modVersion_getOwner(void *self)
{
    return ((LIBMATTI_FML_ModVersion *) self)->owner;
}

static int modVersion_getReferralURL(void *self, void **out)
{
    LIBMATTI_FML_ModVersion *modVersion = self;
    if (!modVersion->referralUrlPresent) return 0;
    *out = modVersion->referralUrl;
    return 1;
}

// Java: public ModVersion(IModInfo owner, IConfigurable config)
static LIBMATTI_FML_ModVersion *mod_version_new(LIBMATTI_NEOFORGESPI_IModInfo *owner,
                                                LIBMATTI_NEOFORGESPI_IConfigurable *config,
                                                LIBMATTI_NEOFORGESPI_IModFileInfo *owningFileInfo)
{
    LIBMATTI_FML_ModVersion *modVersion = calloc(1, sizeof(LIBMATTI_FML_ModVersion));
    modVersion->owner = owner;
    modVersion->modVersion.self = modVersion;
    modVersion->modVersion.getModId = modVersion_getModId;
    modVersion->modVersion.getVersionRange = modVersion_getVersionRange;
    modVersion->modVersion.getType = modVersion_getType;
    modVersion->modVersion.getReason = modVersion_getReason;
    modVersion->modVersion.getOrdering = modVersion_getOrdering;
    modVersion->modVersion.getSide = modVersion_getSide;
    modVersion->modVersion.setOwner = modVersion_setOwner;
    modVersion->modVersion.getOwner = modVersion_getOwner;
    modVersion->modVersion.getReferralURL = modVersion_getReferralURL;

    modVersion->modId = configString(config, "modId");
    if (modVersion->modId == NULL)
    {
        // Java: orElseThrow(() -> new InvalidModFileException("Missing required field modid in dependency", getOwningFile()))
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Missing required field modid in dependency");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Missing required field modid in dependency",
                                                        owningFileInfo);
        free(modVersion);
        return NULL;
    }

    // Java: config.<String>getConfigElement("type").map(str -> str.toUpperCase(Locale.ROOT)).map(DependencyType::valueOf).orElse(REQUIRED)
    modVersion->type = LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED;
    char *type = configString(config, "type");
    if (type != NULL)
    {
        char *upper = LIBMATTI_FML_StringUtils_ToUpperCase(type);
        static const char *const TYPES[] = {"REQUIRED", "OPTIONAL", "INCOMPATIBLE", "DISCOURAGED"};
        for (int i = 0; i < 4; i++)
            if (strcmp(upper, TYPES[i]) == 0)
                modVersion->type = (LIBMATTI_NEOFORGESPI_IModInfo_DependencyType) i;
        free(upper);
        free(type);
    }

    modVersion->reason = configString(config, "reason");
    modVersion->reasonPresent = modVersion->reason != NULL;

    // Java: config.<String>getConfigElement("versionRange").map(MavenVersionAdapter::createFromVersionSpec).orElse(UNBOUNDED)
    char *versionRange = configString(config, "versionRange");
    modVersion->versionRange = versionRange != NULL
                                   ? LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(versionRange)
                                   : LIBMATTI_NEOFORGESPI_IModInfo_Unbounded();
    free(versionRange);

    // Java: config.<String>getConfigElement("ordering").map(Ordering::valueOf).orElse(Ordering.NONE)
    modVersion->ordering = LIBMATTI_NEOFORGESPI_IModInfo_Ordering_NONE;
    char *ordering = configString(config, "ordering");
    if (ordering != NULL)
    {
        if (strcmp(ordering, "BEFORE") == 0)
            modVersion->ordering = LIBMATTI_NEOFORGESPI_IModInfo_Ordering_BEFORE;
        else if (strcmp(ordering, "AFTER") == 0)
            modVersion->ordering = LIBMATTI_NEOFORGESPI_IModInfo_Ordering_AFTER;
        free(ordering);
    }

    // Java: config.<String>getConfigElement("side").map(DependencySide::valueOf).orElse(DependencySide.BOTH)
    modVersion->side = LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH;
    char *side = configString(config, "side");
    if (side != NULL)
    {
        if (strcmp(side, "CLIENT") == 0)
            modVersion->side = LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_CLIENT;
        else if (strcmp(side, "SERVER") == 0)
            modVersion->side = LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_SERVER;
        free(side);
    }

    char *referralUrl = configString(config, "referralUrl");
    if (referralUrl != NULL)
    {
        modVersion->referralUrl = LIBMATTI_FML_StringUtils_ToURL(referralUrl);
        modVersion->referralUrlPresent = modVersion->referralUrl != NULL;
        free(referralUrl);
    }

    return modVersion;
}

static void mod_version_free(LIBMATTI_FML_ModVersion *modVersion)
{
    if (modVersion == NULL) return;
    free(modVersion->modId);
    free(modVersion->reason);
    // Java: the version range is shared via UNBOUNDED, so it is not owned here
    if (modVersion->versionRange != LIBMATTI_NEOFORGESPI_IModInfo_Unbounded())
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeRange(modVersion->versionRange);
    free(modVersion);
}

// ---------------------------------------------------------------------------
// ModInfo
// ---------------------------------------------------------------------------

static LIBMATTI_NEOFORGESPI_IModFileInfo *modInfo_getOwningFile(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->owningFileInfo;
}

static void *modInfo_getLoader(void *self)
{
    // Java: return owningFile.getFile().getLoaders().getFirst();
    LIBMATTI_FML_ModInfo *modInfo = self;
    size_t count = 0;
    LIBMATTI_NEOFORGESPI_IModLanguageLoader **loaders =
        LIBMATTI_FML_ModFile_GetLoaders(LIBMATTI_FML_ModFileInfo_GetFile(modInfo->owningFile), &count);
    return count > 0 ? loaders[0] : NULL;
}

static const char *modInfo_getModId(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->modId;
}

static const char *modInfo_getDisplayName(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->displayName;
}

static const char *modInfo_getDescription(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->description;
}

static const LIBMATTI_NEOFORGESPI_ArtifactVersion *modInfo_getVersion(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->version;
}

static LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **modInfo_getDependencies(void *self, size_t *count)
{
    LIBMATTI_FML_ModInfo *modInfo = self;
    *count = modInfo->dependencyCount;
    return (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **) modInfo->dependencies;
}

static void **modInfo_getForgeFeatures(void *self, size_t *count)
{
    LIBMATTI_FML_ModInfo *modInfo = self;
    *count = modInfo->featureCount;
    return modInfo->featurePointers;
}

static const char *modInfo_getNamespace(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->namespace;
}

static void *modInfo_getModProperties(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->properties;
}

static int modInfo_getUpdateURL(void *self, void **out)
{
    LIBMATTI_FML_ModInfo *modInfo = self;
    if (!modInfo->updateURLPresent) return 0;
    *out = modInfo->updateJSONURL;
    return 1;
}

static int modInfo_getModURL(void *self, void **out)
{
    LIBMATTI_FML_ModInfo *modInfo = self;
    if (!modInfo->modUrlPresent) return 0;
    *out = modInfo->modUrl;
    return 1;
}

static int modInfo_getLogoFile(void *self, const char **out)
{
    LIBMATTI_FML_ModInfo *modInfo = self;
    if (!modInfo->logoFilePresent) return 0;
    *out = modInfo->logoFile;
    return 1;
}

static int modInfo_getLogoBlur(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->logoBlur;
}

static void *modInfo_getConfig(void *self)
{
    return ((LIBMATTI_FML_ModInfo *) self)->config;
}

// Java: public ModInfo(ModFileInfo owningFile, IConfigurable config)
LIBMATTI_FML_ModInfo *LIBMATTI_FML_ModInfo_New(LIBMATTI_FML_ModFileInfo *owningFile,
                                               LIBMATTI_NEOFORGESPI_IConfigurable *config)
{
    LIBMATTI_FML_ModInfo *modInfo = calloc(1, sizeof(LIBMATTI_FML_ModInfo));
    modInfo->modInfo.self = modInfo;
    modInfo->modInfo.getOwningFile = modInfo_getOwningFile;
    modInfo->modInfo.getLoader = modInfo_getLoader;
    modInfo->modInfo.getModId = modInfo_getModId;
    modInfo->modInfo.getDisplayName = modInfo_getDisplayName;
    modInfo->modInfo.getDescription = modInfo_getDescription;
    modInfo->modInfo.getVersion = modInfo_getVersion;
    modInfo->modInfo.getDependencies = modInfo_getDependencies;
    modInfo->modInfo.getForgeFeatures = modInfo_getForgeFeatures;
    modInfo->modInfo.getNamespace = modInfo_getNamespace;
    modInfo->modInfo.getModProperties = modInfo_getModProperties;
    modInfo->modInfo.getUpdateURL = modInfo_getUpdateURL;
    modInfo->modInfo.getModURL = modInfo_getModURL;
    modInfo->modInfo.getLogoFile = modInfo_getLogoFile;
    modInfo->modInfo.getLogoBlur = modInfo_getLogoBlur;
    modInfo->modInfo.getConfig = modInfo_getConfig;

    modInfo->configurable.self = modInfo;
    modInfo->configurable.getConfigElement = LIBMATTI_FML_ModInfo_GetConfigElement;
    modInfo->configurable.getConfigList = LIBMATTI_FML_ModInfo_GetConfigList;

    modInfo->owningFile = owningFile;
    modInfo->owningFileInfo = owningFile != NULL ? LIBMATTI_FML_ModFileInfo_AsModFileInfo(owningFile) : NULL;
    modInfo->config = config;

    // Java: this.modId = config.<String>getConfigElement("modId").orElseThrow(() -> new InvalidModFileException("Missing modId", owningFile));
    modInfo->modId = configString(config, "modId");
    if (modInfo->modId == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Missing modId");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Missing modId", modInfo->owningFileInfo);
        LIBMATTI_FML_ModInfo_Free(modInfo);
        return NULL;
    }

    // Java: if (!VALID_MODID.matcher(this.modId).matches()) { LOGGER.error(...); throw new InvalidModFileException(...); }
    if (!validModId(modInfo->modId))
    {
        logInvalid("modId", modInfo->owningFileInfo, modInfo->modId,
                   "^(?=.{2,64}$)[a-z][a-z0-9_]*(\\.[a-z][a-z0-9_]*)*$");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Invalid modId found", modInfo->owningFileInfo);
        LIBMATTI_FML_ModInfo_Free(modInfo);
        return NULL;
    }

    // Java: this.namespace = config.<String>getConfigElement("namespace").orElse(this.modId);
    char *namespace = configString(config, "namespace");
    modInfo->namespace = namespace != NULL ? namespace : strdup(modInfo->modId);

    if (!validNamespace(modInfo->namespace))
    {
        logInvalid("override namespace", modInfo->owningFileInfo, modInfo->namespace, "^[a-z][a-z0-9_.-]{1,63}$");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Invalid override namespace found",
                                                        modInfo->owningFileInfo);
        LIBMATTI_FML_ModInfo_Free(modInfo);
        return NULL;
    }

    // Java: this.version = config.<String>getConfigElement("version").map(s -> StringSubstitutor.replace(s, file)).map(DefaultArtifactVersion::new).orElse(DEFAULT_VERSION);
    char *version = configString(config, "version");
    if (version != NULL)
    {
        char *replaced = LIBMATTI_FML_StringSubstitutor_Replace(
            version, owningFile != NULL ? LIBMATTI_FML_ModFileInfo_GetFile(owningFile) : NULL);
        modInfo->version = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(replaced);
        free(replaced);
        free(version);
    }
    else
    {
        // Java: private static final DefaultArtifactVersion DEFAULT_VERSION = new DefaultArtifactVersion("1")
        modInfo->version = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion("1");
    }

    if (!validVersion(modInfo->version->version))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Illegal version number specified {}", modInfo->version->version);
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Illegal version number specified",
                                                        modInfo->owningFileInfo);
        LIBMATTI_FML_ModInfo_Free(modInfo);
        return NULL;
    }

    modInfo->displayName = configString(config, "displayName");
    if (modInfo->displayName == NULL) modInfo->displayName = strdup(modInfo->modId);

    // Java: config.<String>getConfigElement("description").map(desc -> desc.replace("\r\n", "\n")).orElse("MISSING DESCRIPTION")
    modInfo->description = configString(config, "description");
    if (modInfo->description == NULL)
    {
        modInfo->description = strdup("MISSING DESCRIPTION");
    }
    else
    {
        char *read = modInfo->description;
        char *write = read;
        while (*read != '\0')
        {
            if (read[0] == '\r' && read[1] == '\n')
            {
                *write++ = '\n';
                read += 2;
            }
            else
            {
                *write++ = *read++;
            }
        }
        *write = '\0';
    }

    // Java: this.logoFile = Optional.ofNullable(config.<String>getConfigElement("logoFile").orElseGet(() -> ownFile.flatMap(mf -> mf.<String>getConfigElement("logoFile")).orElse(null)));
    modInfo->logoFile = configString(config, "logoFile");
    if (modInfo->logoFile == NULL)
        modInfo->logoFile = configString(LIBMATTI_FML_ModFileInfo_AsConfigurable(owningFile), "logoFile");
    modInfo->logoFilePresent = modInfo->logoFile != NULL;

    // Java: this.logoBlur = config.<Boolean>getConfigElement("logoBlur").orElseGet(() -> ownFile.flatMap(f -> f.<Boolean>getConfigElement("logoBlur")).orElse(true));
    if (!configBool(config, "logoBlur", &modInfo->logoBlur) &&
        !configBool(LIBMATTI_FML_ModFileInfo_AsConfigurable(owningFile), "logoBlur", &modInfo->logoBlur))
        modInfo->logoBlur = 1;

    // Java: this.updateJSONURL = config.<String>getConfigElement("updateJSONURL").map(StringUtils::toURL);
    char *updateJson = configString(config, "updateJSONURL");
    if (updateJson != NULL)
    {
        modInfo->updateJSONURL = LIBMATTI_FML_StringUtils_ToURL(updateJson);
        modInfo->updateURLPresent = modInfo->updateJSONURL != NULL;
        free(updateJson);
    }

    // Java: this.modUrl = config.<String>getConfigElement("modUrl").map(StringUtils::toURL);
    char *modUrl = configString(config, "modUrl");
    if (modUrl != NULL)
    {
        modInfo->modUrl = LIBMATTI_FML_StringUtils_ToURL(modUrl);
        modInfo->modUrlPresent = modInfo->modUrl != NULL;
        free(modUrl);
    }

    // Java: this.dependencies = ownFile.map(mfi -> mfi.getConfigList("dependencies", this.modId)).orElse(emptyList())
    //             .stream().map(dep -> new ModVersion(this, dep)).toList();
    if (owningFile != NULL)
    {
        const char *key[] = {"dependencies", modInfo->modId};
        size_t count = 0;
        LIBMATTI_NEOFORGESPI_IConfigurable **deps =
            LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(LIBMATTI_FML_ModFileInfo_AsConfigurable(owningFile), key,
                                                             2, &count);
        for (size_t i = 0; i < count; i++)
        {
            LIBMATTI_FML_ModVersion *modVersion =
                mod_version_new(&modInfo->modInfo, deps[i], modInfo->owningFileInfo);
            if (modVersion == NULL) continue;
            modInfo->dependencies =
                realloc(modInfo->dependencies, sizeof(LIBMATTI_FML_ModVersion *) * (modInfo->dependencyCount + 1));
            modInfo->dependencies[modInfo->dependencyCount++] = modVersion;
        }
    }

    // Java: this.features = ownFile.flatMap(mfi -> mfi.<Map<String, Object>>getConfigElement("features", this.modId))
    //             .stream().flatMap(m -> m.entrySet().stream()).map(this::makeBound).toList();
    {
        LIBMATTI_NEOFORGESPI_IConfigurable *owningConfigurable = LIBMATTI_FML_ModFileInfo_AsConfigurable(
            (LIBMATTI_FML_ModFileInfo *) modInfo->owningFileInfo);
        const char *featureKey[] = {"features", modInfo->modId};
        LIBMATTI_NEOFORGESPI_IConfigurable_Value featureValue;
        if (LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(owningConfigurable, featureKey, 2, &featureValue) &&
            featureValue.type == LIBMATTI_NEOFORGESPI_IConfigurable_CONFIG)
        {
            // Java: the map is a night-config table; every entry is (featureName, boundString)
            LIBMATTI_NC_Config *featureTable = featureValue.map;
            size_t tableCount = 0;
            LIBMATTI_NC_Config_ValueMap(featureTable, &tableCount);
            for (size_t i = 0; i < tableCount; i++)
            {
                const char *featureName = LIBMATTI_NC_Config_KeyAt(featureTable, i);
                LIBMATTI_NC_Value *bound = LIBMATTI_NC_Config_GetKey(featureTable, featureName);
                if (bound == NULL || bound->type != LIBMATTI_NC_VALUE_STRING)
                {
                    // Java: throw new InvalidModFileException("Invalid feature bound {...} for key {...} only strings are accepted", this.owningFile);
                    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Invalid feature bound for key {}", featureName);
                    continue;
                }
                LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *featureBound =
                    calloc(1, sizeof(LIBMATTI_NEOFORGESPI_ForgeFeature_Bound));
                featureBound->featureName = strdup(featureName);
                featureBound->featureBound = strdup(bound->string);
                featureBound->modInfo = &modInfo->modInfo;
                modInfo->features =
                    realloc(modInfo->features, sizeof(LIBMATTI_NEOFORGESPI_ForgeFeature_Bound) * (modInfo->featureCount + 1));
                modInfo->features[modInfo->featureCount] = *featureBound;
                modInfo->featurePointers =
                    realloc(modInfo->featurePointers, sizeof(void *) * (modInfo->featureCount + 1));
                modInfo->featurePointers[modInfo->featureCount] = &modInfo->features[modInfo->featureCount];
                modInfo->featureCount++;
                free(featureBound);
            }
        }
    }

    // Java: this.properties = ownFile.flatMap(mfi -> mfi.<Map<String, Object>>getConfigElement("modproperties", this.modId)).orElse(emptyMap())
    {
        LIBMATTI_NEOFORGESPI_IConfigurable *owningConfigurable = LIBMATTI_FML_ModFileInfo_AsConfigurable(
            (LIBMATTI_FML_ModFileInfo *) modInfo->owningFileInfo);
        const char *propertyKey[] = {"modproperties", modInfo->modId};
        LIBMATTI_NEOFORGESPI_IConfigurable_Value propertyValue;
        if (LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(owningConfigurable, propertyKey, 2, &propertyValue) &&
            propertyValue.type == LIBMATTI_NEOFORGESPI_IConfigurable_CONFIG)
        {
            // Java: the Map<String, Object> view of the config table (valueMap())
            modInfo->properties = LIBMATTI_FML_NightConfigWrapper_ValueMap(propertyValue.map);
        }
    }

    return modInfo;
}

void LIBMATTI_FML_ModInfo_Free(LIBMATTI_FML_ModInfo *modInfo)
{
    if (modInfo == NULL) return;
    for (size_t i = 0; i < modInfo->dependencyCount; i++)
        mod_version_free(modInfo->dependencies[i]);
    free(modInfo->dependencies);
    free(modInfo->modId);
    free(modInfo->namespace);
    free(modInfo->displayName);
    free(modInfo->description);
    free(modInfo->logoFile);
    LIBMATTI_JU_HashMap_Free(modInfo->properties);
    LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(modInfo->version);
    free(modInfo);
}

LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModInfo_AsModInfo(LIBMATTI_FML_ModInfo *modInfo)
{
    return modInfo != NULL ? &modInfo->modInfo : NULL;
}

LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_ModInfo_AsConfigurable(LIBMATTI_FML_ModInfo *modInfo)
{
    return modInfo != NULL ? &modInfo->configurable : NULL;
}

// Java: public ModFileInfo getOwningFile()
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModInfo_GetOwningFile(const LIBMATTI_FML_ModInfo *modInfo)
{
    return modInfo->owningFile;
}

// Java: public String getModId()
const char *LIBMATTI_FML_ModInfo_GetModId(const LIBMATTI_FML_ModInfo *modInfo)
{
    return modInfo->modId;
}

// Java: public List<? extends ModVersion> getDependencies()
LIBMATTI_FML_ModVersion **LIBMATTI_FML_ModInfo_GetDependencies(const LIBMATTI_FML_ModInfo *modInfo, size_t *count)
{
    *count = modInfo->dependencyCount;
    return modInfo->dependencies;
}

// Java: public <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_FML_ModInfo_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                          LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    return LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(((LIBMATTI_FML_ModInfo *) self)->config, key, keyCount,
                                                              out);
}

// Java: @Override public List<? extends IConfigurable> getConfigList(String... key) { return null; }
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_ModInfo_GetConfigList(void *self, const char *const *key,
                                                                        size_t keyCount, size_t *count)
{
    (void) self;
    (void) key;
    (void) keyCount;
    *count = 0;
    return NULL;
}

// Java: @Override public String toString() { return modId + " (" + displayName + ") @ " + version; }
char *LIBMATTI_FML_ModInfo_ToString(const LIBMATTI_FML_ModInfo *modInfo)
{
    size_t length = strlen(modInfo->modId) + strlen(modInfo->displayName) + strlen(modInfo->version->version) + 7;
    char *result = malloc(length);
    snprintf(result, length, "%s (%s) @ %s", modInfo->modId, modInfo->displayName, modInfo->version->version);
    return result;
}

// Java: public String getModId() (ModVersion)
const char *LIBMATTI_FML_ModVersion_GetModId(const LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->modId;
}

// Java: public VersionRange getVersionRange() (ModVersion)
const LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_FML_ModVersion_GetVersionRange(
    const LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->versionRange;
}

// Java: public DependencyType getType() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_DependencyType LIBMATTI_FML_ModVersion_GetType(
    const LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->type;
}

// Java: public Ordering getOrdering() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_Ordering LIBMATTI_FML_ModVersion_GetOrdering(
    const LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->ordering;
}

// Java: public DependencySide getSide() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo_DependencySide LIBMATTI_FML_ModVersion_GetSide(
    const LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->side;
}

// Java: public Optional<String> getReason() (ModVersion)
int LIBMATTI_FML_ModVersion_GetReason(const LIBMATTI_FML_ModVersion *modVersion, const char **out)
{
    if (!modVersion->reasonPresent) return 0;
    *out = modVersion->reason;
    return 1;
}

// Java: public IModInfo getOwner() (ModVersion)
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModVersion_GetOwner(LIBMATTI_FML_ModVersion *modVersion)
{
    return modVersion->owner;
}

// Java: public void setOwner(IModInfo owner) (ModVersion)
void LIBMATTI_FML_ModVersion_SetOwner(LIBMATTI_FML_ModVersion *modVersion, LIBMATTI_NEOFORGESPI_IModInfo *owner)
{
    modVersion->owner = owner;
}

void LIBMATTI_FML_ModVersion_Free(LIBMATTI_FML_ModVersion *modVersion)
{
    mod_version_free(modVersion);
}

// Java: the sorter casts IModInfo to ModInfo, so the port needs the concrete instance behind the interface
LIBMATTI_FML_ModInfo *LIBMATTI_FML_ModInfo_AsConcrete(const LIBMATTI_NEOFORGESPI_IModInfo *modInfo)
{
    return modInfo != NULL ? (LIBMATTI_FML_ModInfo *) modInfo->self : NULL;
}
