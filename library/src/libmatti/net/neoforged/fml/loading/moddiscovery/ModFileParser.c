#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileParser.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/NightConfigWrapper.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: public static IModFileInfo readModList(ModFile modFile, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ReadModList(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata)
{
    // Java: return parser.build(modFile);
    return parser((LIBMATTI_NEOFORGESPI_IModFile *) &modFile->modFile, parserUserdata);
}

// Java: public static IModFileInfo modsTomlParser(IModFile imodFile)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ModsTomlParser(
    LIBMATTI_NEOFORGESPI_IModFile *imodFile)
{
    // Java: ModFile modFile = (ModFile) imodFile;
    LIBMATTI_FML_ModFile *modFile = (LIBMATTI_FML_ModFile *) imodFile;
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Considering mod file candidate {}",
                             LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(imodFile));

    // Java: var modsjson = modFile.getContents().get(JarModsDotTomlModFileReader.MODS_TOML);
    LIBMATTI_FML_JarResource *modsjson =
        LIBMATTI_FML_JarContents_Get(LIBMATTI_FML_ModFile_GetContents(modFile), LIBMATTI_FML_MODS_TOML);
    if (modsjson == NULL)
    {
        LIBMATTI_ML_Logger_Warn(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Mod file {} is missing {} file",
                                LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(imodFile), LIBMATTI_FML_MODS_TOML);
        return NULL;
    }

    // Java: config = TomlFormat.instance().createParser().parse(reader).unmodifiable();
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_FML_JarResource_ReadAllBytes(modsjson, &length);
    LIBMATTI_FML_JarResource_Free(modsjson);
    if (bytes == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Failed to read {} from {}",
                                 LIBMATTI_FML_MODS_TOML, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(imodFile));
        return NULL;
    }

    LIBMATTI_NC_TomlParser *tomlParser = LIBMATTI_NC_TomlParser_New();
    LIBMATTI_NC_Config *config = LIBMATTI_NC_TomlParser_Parse(tomlParser, (const char *) bytes, length);
    free(bytes);
    if (config == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Failed to parse {} from {}: {}",
                                 LIBMATTI_FML_MODS_TOML, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(imodFile),
                                 LIBMATTI_NC_TomlParser_ErrorMessage(tomlParser));
        LIBMATTI_NC_TomlParser_Free(tomlParser);
        return NULL;
    }
    LIBMATTI_NC_TomlParser_Free(tomlParser);

    LIBMATTI_FML_NightConfigWrapper *configWrapper = LIBMATTI_FML_NightConfigWrapper_New(config);
    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo = LIBMATTI_FML_ModFileInfo_AsModFileInfo(LIBMATTI_FML_ModFileInfo_New(
        modFile, LIBMATTI_FML_NightConfigWrapper_AsConfigurable(configWrapper),
        LIBMATTI_FML_NightConfigWrapper_SetFileConsumer, configWrapper));
    if (modFileInfo == NULL)
    {
        // the file was rejected, so nothing keeps the parsed configuration alive
        LIBMATTI_FML_NightConfigWrapper_Free(configWrapper);
        LIBMATTI_NC_Config_Free(config);
        return NULL;
    }
    return modFileInfo;
}

// Java: the method reference ModFileParser::modsTomlParser as a ModFileInfoParser
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ModsTomlParserAdapter(void *file, void *userdata)
{
    (void) userdata;
    return LIBMATTI_FML_ModFileParser_ModsTomlParser(file);
}

// Java: protected static List<MixinConfig> getMixinConfigs(IModFileInfo modFileInfo)
LIBMATTI_FML_ModFileParser_MixinConfig *LIBMATTI_FML_ModFileParser_GetMixinConfigs(
    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count)
{
    *count = 0;
    // Java: try { ... } catch (Exception exception) { LOGGER.error("Failed to load mixin configs from mod file"); return List.of(); }
    LIBMATTI_NEOFORGESPI_IConfigurable *config = LIBMATTI_NEOFORGESPI_IModFileInfo_GetConfig(modFileInfo);
    if (config == NULL) return NULL;

    const char *mixinsKey[] = {"mixins"};
    size_t entryCount = 0;
    LIBMATTI_NEOFORGESPI_IConfigurable **entries =
        LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(config, mixinsKey, 1, &entryCount);
    if (entries == NULL || entryCount == 0) return NULL;

    LIBMATTI_FML_ModFileParser_MixinConfig *mixins = calloc(entryCount, sizeof(*mixins));
    size_t actual = 0;
    for (size_t i = 0; i < entryCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IConfigurable_Value name;
        if (!LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(entries[i], "config", &name) ||
            name.type != LIBMATTI_NEOFORGESPI_IConfigurable_STRING)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                     "Missing \"config\" in [[mixins]] entry");
            continue;
        }

        LIBMATTI_FML_ModFileParser_MixinConfig mixin = {0};
        mixin.config = strdup(name.string);

        // Java: mixinsEntry.<List<String>>getConfigElement("requiredMods").orElse(List.of())
        LIBMATTI_NEOFORGESPI_IConfigurable_Value requiredMods;
        if (LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(entries[i], "requiredMods", &requiredMods) &&
            requiredMods.type == LIBMATTI_NEOFORGESPI_IConfigurable_STRING_LIST)
        {
            mixin.requiredModCount = requiredMods.listCount;
            mixin.requiredMods = calloc(requiredMods.listCount, sizeof(char *));
            for (size_t j = 0; j < requiredMods.listCount; j++) mixin.requiredMods[j] = strdup(requiredMods.list[j]);
        }

        // Java: mixinsEntry.<String>getConfigElement("behaviorVersion").map(DefaultArtifactVersion::new).orElse(null)
        LIBMATTI_NEOFORGESPI_IConfigurable_Value behaviorVersion;
        if (LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(entries[i], "behaviorVersion", &behaviorVersion) &&
            behaviorVersion.type == LIBMATTI_NEOFORGESPI_IConfigurable_STRING)
            mixin.behaviorVersion = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(behaviorVersion.string);

        mixins[actual++] = mixin;
    }

    free(entries);
    *count = actual;
    return mixins;
}

// Java: protected static Optional<List<String>> getAccessTransformers(IModFileInfo modFileInfo)
int LIBMATTI_FML_ModFileParser_GetAccessTransformers(LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, char ***out,
                                                     size_t *count)
{
    *count = 0;
    // Java: try { ... } catch (Exception exception) { LOGGER.error("Failed to load access transformers from mod file"); return Optional.of(List.of()); }
    LIBMATTI_NEOFORGESPI_IConfigurable *config = LIBMATTI_NEOFORGESPI_IModFileInfo_GetConfig(modFileInfo);
    if (config == NULL) return 0;

    const char *atKey[] = {"accessTransformers"};
    size_t entryCount = 0;
    LIBMATTI_NEOFORGESPI_IConfigurable **entries =
        LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(config, atKey, 1, &entryCount);
    if (entries == NULL || entryCount == 0) return 0;

    char **files = calloc(entryCount, sizeof(char *));
    size_t actual = 0;
    for (size_t i = 0; i < entryCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IConfigurable_Value file;
        if (!LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(entries[i], "file", &file) ||
            file.type != LIBMATTI_NEOFORGESPI_IConfigurable_STRING)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                     "Missing \"file\" in [[accessTransformers]] entry");
            continue;
        }
        files[actual++] = strdup(file.string);
    }

    *out = files;
    *count = actual;
    return 1;
}
