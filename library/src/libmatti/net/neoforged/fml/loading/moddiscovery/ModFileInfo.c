#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/NightConfigWrapper.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/StringUtils.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/InvalidModFileException.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: config.<String>getConfigElement(key).orElse(null)
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

// Java: !this.license.isBlank()
static int isBlank(const char *value)
{
    for (const char *p = value; *p != '\0'; p++)
        if (!isspace((unsigned char) *p))
            return 0;
    return 1;
}

static LIBMATTI_NEOFORGESPI_IModInfo **modFileInfo_getMods(void *self, size_t *count)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    *count = modFileInfo->modCount;
    return modFileInfo->mods;
}

static LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *modFileInfo_requiredLanguageLoaders(void *self, size_t *count)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    *count = modFileInfo->languageSpecCount;
    return modFileInfo->languageSpecs;
}

static int modFileInfo_showAsResourcePack(void *self)
{
    return ((LIBMATTI_FML_ModFileInfo *) self)->showAsResourcePack;
}

static int modFileInfo_showAsDataPack(void *self)
{
    return ((LIBMATTI_FML_ModFileInfo *) self)->showAsDataPack;
}

static void *modFileInfo_getFileProperties(void *self)
{
    return ((LIBMATTI_FML_ModFileInfo *) self)->properties;
}

static const char *modFileInfo_getLicense(void *self)
{
    return ((LIBMATTI_FML_ModFileInfo *) self)->license;
}

// Java: public String versionString() { return getMods().get(0).getVersion().toString(); }
static const char *modFileInfo_versionString(void *self)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    if (modFileInfo->modCount == 0) return NULL;
    return LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(modFileInfo->mods[0])->version;
}

static char **modFileInfo_usesServices(void *self, size_t *count)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    *count = modFileInfo->usesServiceCount;
    return modFileInfo->usesServices;
}

static LIBMATTI_NEOFORGESPI_IModFile *modFileInfo_getFile(void *self)
{
    return (LIBMATTI_NEOFORGESPI_IModFile *) &((LIBMATTI_FML_ModFileInfo *) self)->modFile->modFile;
}

static void *modFileInfo_getConfig(void *self)
{
    return ((LIBMATTI_FML_ModFileInfo *) self)->config;
}

// Java: the GC
static void modFileInfo_free(void *self)
{
    LIBMATTI_FML_ModFileInfo_Free(self);
}

static void mod_file_info_init(LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    modFileInfo->modFileInfo.self = modFileInfo;
    modFileInfo->modFileInfo.getMods = modFileInfo_getMods;
    modFileInfo->modFileInfo.requiredLanguageLoaders = modFileInfo_requiredLanguageLoaders;
    modFileInfo->modFileInfo.showAsResourcePack = modFileInfo_showAsResourcePack;
    modFileInfo->modFileInfo.showAsDataPack = modFileInfo_showAsDataPack;
    modFileInfo->modFileInfo.getFileProperties = modFileInfo_getFileProperties;
    modFileInfo->modFileInfo.getLicense = modFileInfo_getLicense;
    modFileInfo->modFileInfo.versionString = modFileInfo_versionString;
    modFileInfo->modFileInfo.usesServices = modFileInfo_usesServices;
    modFileInfo->modFileInfo.getFile = modFileInfo_getFile;
    modFileInfo->modFileInfo.getConfig = modFileInfo_getConfig;
    modFileInfo->modFileInfo.free = modFileInfo_free;

    modFileInfo->configurable.self = modFileInfo;
    modFileInfo->configurable.getConfigElement = LIBMATTI_FML_ModFileInfo_GetConfigElement;
    modFileInfo->configurable.getConfigList = LIBMATTI_FML_ModFileInfo_GetConfigList;
}

// Java: @ApiStatus.Internal public ModFileInfo(ModFile modFile, IConfigurable config, Consumer<IModFileInfo> configFileConsumer)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModFileInfo_New(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_IConfigurable *config,
    LIBMATTI_FML_ModFileInfo_ConfigFileConsumer configFileConsumer, void *userdata)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = calloc(1, sizeof(LIBMATTI_FML_ModFileInfo));
    mod_file_info_init(modFileInfo);
    modFileInfo->modFile = modFile;
    modFileInfo->config = config;

    // Java: configFileConsumer.accept(this); (nightconfig's wrapper::setFile)
    if (configFileConsumer != NULL)
        configFileConsumer(LIBMATTI_FML_ModFileInfo_AsModFileInfo(modFileInfo), userdata);

    // Java: var modLoader = config.<String>getConfigElement("modLoader").orElse(null);
    char *modLoader = configString(config, "modLoader");
    // Java: config.<String>getConfigElement("loaderVersion").map(MavenVersionAdapter::createFromVersionSpec).orElse(null);
    char *loaderVersion = configString(config, "loaderVersion");
    LIBMATTI_NEOFORGESPI_VersionRange *modLoaderVersion =
        loaderVersion != NULL ? LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(loaderVersion) : NULL;

    if (modLoaderVersion != NULL && modLoader == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "You cannot specify a loaderVersion without specifying a modLoader");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New(
            "You cannot specify a loaderVersion without specifying a modLoader",
            LIBMATTI_FML_ModFileInfo_AsModFileInfo(modFileInfo));
        LIBMATTI_FML_ModFileInfo_Free(modFileInfo);
        free(modLoader);
        free(loaderVersion);
        return NULL;
    }
    free(loaderVersion);

    // Java: this.languageSpecs = new ArrayList<>(List.of(new LanguageSpec(modLoader, modLoaderVersion)));
    modFileInfo->languageSpecs = calloc(1, sizeof(LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec));
    modFileInfo->languageSpecs[0].languageName = modLoader;
    modFileInfo->languageSpecs[0].acceptedVersions = modLoaderVersion;
    modFileInfo->languageSpecCount = 1;

    // Java: this.license = config.<String>getConfigElement("license").orElse("");
    modFileInfo->license = configString(config, "license");
    if (modFileInfo->license == NULL) modFileInfo->license = strdup("");
    if (isBlank(modFileInfo->license))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Missing license");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Missing license",
                                                        LIBMATTI_FML_ModFileInfo_AsModFileInfo(modFileInfo));
        LIBMATTI_FML_ModFileInfo_Free(modFileInfo);
        return NULL;
    }

    // Java: this.showAsResourcePack = config.<Boolean>getConfigElement("showAsResourcePack").orElse(false);
    if (!configBool(config, "showAsResourcePack", &modFileInfo->showAsResourcePack))
        modFileInfo->showAsResourcePack = 0;
    // Java: this.showAsDataPack = config.<Boolean>getConfigElement("showAsDataPack").orElse(false);
    if (!configBool(config, "showAsDataPack", &modFileInfo->showAsDataPack))
        modFileInfo->showAsDataPack = 0;

    // Java: this.usesServices = config.<List<String>>getConfigElement("services").orElse(List.of());
    LIBMATTI_NEOFORGESPI_IConfigurable_Value services;
    if (LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(config, "services", &services) &&
        services.type == LIBMATTI_NEOFORGESPI_IConfigurable_STRING_LIST)
    {
        modFileInfo->usesServiceCount = services.listCount;
        modFileInfo->usesServices = calloc(services.listCount, sizeof(char *));
        for (size_t i = 0; i < services.listCount; i++) modFileInfo->usesServices[i] = strdup(services.list[i]);
    }

    // Java: this.properties = config.<Map<String, Object>>getConfigElement("properties").orElse(Collections.emptyMap());
    LIBMATTI_NEOFORGESPI_IConfigurable_Value properties;
    if (LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(config, "properties", &properties) &&
        properties.type == LIBMATTI_NEOFORGESPI_IConfigurable_MAP)
        modFileInfo->properties = LIBMATTI_FML_NightConfigWrapper_ValueMap(properties.map);

    // Java: this.modFile.setFileProperties(this.properties);
    LIBMATTI_FML_ModFile_SetFileProperties(modFile, modFileInfo->properties);

    // Java: this.issueURL = config.<String>getConfigElement("issueTrackerURL").map(StringUtils::toURL).orElse(null);
    char *issueTrackerUrl = configString(config, "issueTrackerURL");
    if (issueTrackerUrl != NULL)
    {
        modFileInfo->issueURL = LIBMATTI_FML_StringUtils_ToURL(issueTrackerUrl);
        free(issueTrackerUrl);
    }

    // Java: List<? extends IConfigurable> modConfigs = config.getConfigList("mods");
    const char *key[] = {"mods"};
    size_t modConfigCount = 0;
    LIBMATTI_NEOFORGESPI_IConfigurable **modConfigs =
        LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(LIBMATTI_FML_ModFileInfo_AsConfigurable(modFileInfo), key,
                                                        1, &modConfigCount);
    if (modConfigCount == 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Missing mods list");
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New("Missing mods list",
                                                        LIBMATTI_FML_ModFileInfo_AsModFileInfo(modFileInfo));
        LIBMATTI_FML_ModFileInfo_Free(modFileInfo);
        return NULL;
    }

    // Java: this.mods = modConfigs.stream().map(mi -> (IModInfo) new ModInfo(this, mi)).toList();
    modFileInfo->mods = calloc(modConfigCount, sizeof(LIBMATTI_NEOFORGESPI_IModInfo *));
    for (size_t i = 0; i < modConfigCount; i++)
    {
        LIBMATTI_FML_ModInfo *modInfo = LIBMATTI_FML_ModInfo_New(modFileInfo, modConfigs[i]);
        if (modInfo == NULL)
        {
            LIBMATTI_FML_ModFileInfo_Free(modFileInfo);
            return NULL;
        }
        modFileInfo->mods[modFileInfo->modCount++] = LIBMATTI_FML_ModInfo_AsModInfo(modInfo);
    }

    // Java: LOGGER.debug(LogMarkers.LOADING, "Found valid mod file {} with {} mods - versions {}", ...)
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Found valid mod file {} with mods",
                             LIBMATTI_NEOFORGESPI_IModFile_GetFileName(
                                 (LIBMATTI_NEOFORGESPI_IModFile *) &modFile->modFile));

    // The wrappers themselves are owned by the mod infos, only the list is ours.
    free(modConfigs);

    return modFileInfo;
}

// Java: public ModFileInfo(ModFile file, IConfigurable config, Consumer<IModFileInfo> configFileConsumer, List<LanguageSpec> languageSpecs)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_ModFileInfo_NewWithLanguageSpecs(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_IConfigurable *config,
    LIBMATTI_FML_ModFileInfo_ConfigFileConsumer configFileConsumer, void *userdata,
    const LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *languageSpecs, size_t languageSpecCount)
{
    // Java: this(file, config, configFileConsumer); this.languageSpecs.addAll(languageSpecs);
    LIBMATTI_FML_ModFileInfo *modFileInfo =
        LIBMATTI_FML_ModFileInfo_New(modFile, config, configFileConsumer, userdata);
    if (modFileInfo == NULL) return NULL;

    if (languageSpecCount > 0)
    {
        modFileInfo->languageSpecs = realloc(modFileInfo->languageSpecs,
                                             sizeof(LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec) *
                                             (modFileInfo->languageSpecCount + languageSpecCount));
        for (size_t i = 0; i < languageSpecCount; i++)
            modFileInfo->languageSpecs[modFileInfo->languageSpecCount + i] = languageSpecs[i];
        modFileInfo->languageSpecCount += languageSpecCount;
    }
    return modFileInfo;
}

void LIBMATTI_FML_ModFileInfo_Free(LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    if (modFileInfo == NULL) return;
    for (size_t i = 0; i < modFileInfo->modCount; i++)
        LIBMATTI_FML_ModInfo_Free(LIBMATTI_FML_ModInfo_AsConcrete(modFileInfo->mods[i]));
    free(modFileInfo->mods);
    for (size_t i = 0; i < modFileInfo->languageSpecCount; i++)
        free(modFileInfo->languageSpecs[i].languageName);
    free(modFileInfo->languageSpecs);
    for (size_t i = 0; i < modFileInfo->usesServiceCount; i++) free(modFileInfo->usesServices[i]);
    free(modFileInfo->usesServices);
    LIBMATTI_JU_HashMap_Free(modFileInfo->properties);
    free(modFileInfo->license);
    free(modFileInfo);
}

LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileInfo_AsModFileInfo(LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    return modFileInfo != NULL ? &modFileInfo->modFileInfo : NULL;
}

LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_ModFileInfo_AsConfigurable(LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    return modFileInfo != NULL ? &modFileInfo->configurable : NULL;
}

// Java: public ModFile getFile()
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFileInfo_GetFile(const LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    return modFileInfo != NULL ? modFileInfo->modFile : NULL;
}

// Java: public URL getIssueURL()
void *LIBMATTI_FML_ModFileInfo_GetIssueURL(const LIBMATTI_FML_ModFileInfo *modFileInfo)
{
    return modFileInfo->issueURL;
}

// Java: public Optional<String> getCodeSigningFingerprint() { return this.modFile.getContents().getChecksum(); }
int LIBMATTI_FML_ModFileInfo_GetCodeSigningFingerprint(const LIBMATTI_FML_ModFileInfo *modFileInfo, char **out)
{
    return LIBMATTI_FML_JarContents_GetChecksum(LIBMATTI_FML_ModFile_GetContents(modFileInfo->modFile), out);
}

// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_FML_ModFileInfo_GetMods(const LIBMATTI_FML_ModFileInfo *modFileInfo,
                                                                 size_t *count)
{
    *count = modFileInfo->modCount;
    return modFileInfo->mods;
}

// Java: public <T> Optional<T> getConfigElement(String... key)
// (Java: this.config.getConfigElement(key) - self is the concrete ModFileInfo, as configured by
// configurable.self, so the wrapper's interface is reached through its own dispatch)
int LIBMATTI_FML_ModFileInfo_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                              LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    return LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(modFileInfo->config, key, keyCount, out);
}

// Java: public List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_ModFileInfo_GetConfigList(void *self, const char *const *key,
                                                                            size_t keyCount, size_t *count)
{
    LIBMATTI_FML_ModFileInfo *modFileInfo = self;
    return LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(modFileInfo->config, key, keyCount, count);
}
