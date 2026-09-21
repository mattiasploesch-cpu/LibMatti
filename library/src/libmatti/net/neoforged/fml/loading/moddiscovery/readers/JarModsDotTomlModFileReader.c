#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileParser.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModJarMetadata.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: the method reference JarModsDotTomlModFileReader::manifestParser as a ModFileInfoParser
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParserAdapter(void *file,
                                                                                                    void *userdata)
{
    (void) userdata;
    return LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParser(file);
}

// Java: public static IModFile createModFile(JarContents contents, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_JarModsDotTomlModFileReader_CreateModFile(
    LIBMATTI_FML_JarContents *contents, const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes)
{
    LIBMATTI_NEOFORGESPI_IModFile_Type type = LIBMATTI_NEOFORGESPI_IModFile_Type_MOD;
    int hasType = LIBMATTI_FML_JarModsDotTomlModFileReader_GetModType(contents, &type);

    if (LIBMATTI_FML_JarContents_ContainsFile(contents, LIBMATTI_FML_MODS_TOML))
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Found {} mod of type {}: {}",
                                 LIBMATTI_FML_MODS_TOML, hasType ? "typed" : "untyped",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(contents));

        // Java: var mjm = new ModJarMetadata(); mod = new ModFile(contents, mjm, ModFileParser::modsTomlParser, discoveryAttributes); mjm.setModFile(mod);
        LIBMATTI_FML_ModJarMetadata *mjm = LIBMATTI_FML_ModJarMetadata_New();
        LIBMATTI_FML_ModFile *mod = LIBMATTI_FML_ModFile_NewWithMetadata(
            contents, LIBMATTI_FML_ModJarMetadata_AsJarModuleInfo(mjm),
            LIBMATTI_FML_ModFileParser_ModsTomlParserAdapter, NULL, discoveryAttributes);
        if (mod == NULL) return NULL;
        LIBMATTI_FML_ModJarMetadata_SetModFile(mjm, LIBMATTI_FML_ModFile_AsModFile(mod));
        return LIBMATTI_FML_ModFile_AsModFile(mod);
    }

    if (hasType)
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Found {} mod of type {}: {}",
                                 LIBMATTI_FML_MODS_TOML_MANIFEST, "typed",
                                 LIBMATTI_FML_JarContents_GetPrimaryPath(contents));

        // Java: mod = new ModFile(contents, null, JarModsDotTomlModFileReader::manifestParser, type, discoveryAttributes);
        LIBMATTI_FML_ModFile *mod = LIBMATTI_FML_ModFile_NewWithMetadataAndType(
            contents, NULL, LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParserAdapter, NULL, type,
            discoveryAttributes);
        return mod != NULL ? LIBMATTI_FML_ModFile_AsModFile(mod) : NULL;
    }

    return NULL;
}

// Java: @Nullable private static IModFile.Type getModType(JarContents jar)
int LIBMATTI_FML_JarModsDotTomlModFileReader_GetModType(LIBMATTI_FML_JarContents *jar,
                                                       LIBMATTI_NEOFORGESPI_IModFile_Type *out)
{
    // Java: Manifest jarManifest = jar.getManifest(); if (jarManifest == null) return null;
    LIBMATTI_JU_Manifest *jarManifest = LIBMATTI_FML_JarContents_GetManifest(jar);
    if (jarManifest == NULL)
        return 0;

    // Java: var typeString = jarManifest.getMainAttributes().getValue(ModFile.TYPE);
    const char *typeString = LIBMATTI_JU_Manifest_GetMainValue(jarManifest, LIBMATTI_FML_MODFILE_TYPE);
    if (typeString == NULL)
        return 0;

    // Java: catch (IllegalArgumentException e) { throw new ModLoadingException(ModLoadingIssue.error("fml.modloadingissue.brokenfile.unknownfmlmodtype", typeString)...); }
    if (strcmp(typeString, "MOD") == 0)
    {
        *out = LIBMATTI_NEOFORGESPI_IModFile_Type_MOD;
        return 1;
    }
    if (strcmp(typeString, "LIBRARY") == 0)
    {
        *out = LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY;
        return 1;
    }
    if (strcmp(typeString, "GAMELIBRARY") == 0)
    {
        *out = LIBMATTI_NEOFORGESPI_IModFile_Type_GAMELIBRARY;
        return 1;
    }

    LIBMATTI_FML_ModLoadingIssue *issue =
        LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.brokenfile.unknownfmlmodtype", &typeString, 1);
    LIBMATTI_FML_ModLoadingIssue *withPath =
        LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(issue, LIBMATTI_FML_JarContents_GetPrimaryPath(jar));
    // Java: throw new ModLoadingException(withPath) - the port records the issue with
    // ModLoader, which throwIfErrors finds.
    LIBMATTI_FML_ModLoader_AddLoadingIssue(withPath);
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", "fml.modloadingissue.brokenfile.unknownfmlmodtype");
    return 0;
}

// ---------------------------------------------------------------------------
// Java: private record DefaultModFileInfo(IModFile mod, String license, IConfigurable configurable)
//         implements IModFileInfo, IConfigurable
// ---------------------------------------------------------------------------

typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileInfo modFileInfo;
    LIBMATTI_NEOFORGESPI_IConfigurable configurable;

    LIBMATTI_NEOFORGESPI_IModFile *mod;
    char *license;
    LIBMATTI_NEOFORGESPI_IConfigurable *dummy;
} DefaultModFileInfo;

// Java: public <T> Optional<T> getConfigElement(String... strings) { return Optional.empty(); }
static int default_getConfigElement(void *self, const char *const *key, size_t keyCount,
                                    LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    (void) self;
    (void) key;
    (void) keyCount;
    (void) out;
    return 0;
}

// Java: public List<? extends IConfigurable> getConfigList(String... strings) { return null; }
static LIBMATTI_NEOFORGESPI_IConfigurable **default_getConfigList(void *self, const char *const *key,
                                                                 size_t keyCount, size_t *count)
{
    (void) self;
    (void) key;
    (void) keyCount;
    *count = 0;
    return NULL;
}

// Java: public List<IModInfo> getMods() { return Collections.emptyList(); }
static LIBMATTI_NEOFORGESPI_IModInfo **default_getMods(void *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: public List<LanguageSpec> requiredLanguageLoaders() { return Collections.emptyList(); }
static LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *default_requiredLanguageLoaders(void *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

static int default_showAsResourcePack(void *self)
{
    (void) self;
    return 0;
}

static int default_showAsDataPack(void *self)
{
    (void) self;
    return 0;
}

// Java: public Map<String, Object> getFileProperties() { return Collections.emptyMap(); }
static void *default_getFileProperties(void *self)
{
    (void) self;
    return NULL;
}

static const char *default_getLicense(void *self)
{
    return ((DefaultModFileInfo *) self)->license;
}

// Java: public String versionString() { return null; }
static const char *default_versionString(void *self)
{
    (void) self;
    return NULL;
}

// Java: public List<String> usesServices() { return null; }
static char **default_usesServices(void *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

static LIBMATTI_NEOFORGESPI_IModFile *default_getFile(void *self)
{
    return ((DefaultModFileInfo *) self)->mod;
}

// Java: the GC
static void default_free(void *self)
{
    DefaultModFileInfo *modFileInfo = self;
    free(modFileInfo->license);
    free(modFileInfo->dummy);
    free(modFileInfo);
}

static void *default_getConfig(void *self)
{
    return ((DefaultModFileInfo *) self)->dummy;
}

// Java: public static IModFileInfo manifestParser(IModFile mod)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParser(
    LIBMATTI_NEOFORGESPI_IModFile *mod)
{
    // Java: Function<String, Optional<String>> cfg = name -> Optional.ofNullable(mod.getContents().getManifest().getMainAttributes().getValue(name));
    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(
        LIBMATTI_NEOFORGESPI_IModFile_GetContents(mod));
    const char *licenseValue = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, "LICENSE") : NULL;

    DefaultModFileInfo *modFileInfo = calloc(1, sizeof(DefaultModFileInfo));
    modFileInfo->license = strdup(licenseValue != NULL ? licenseValue : "");
    modFileInfo->mod = mod;

    // Java: var dummy = new IConfigurable() { ... };
    modFileInfo->dummy = calloc(1, sizeof(LIBMATTI_NEOFORGESPI_IConfigurable));
    modFileInfo->dummy->self = modFileInfo;
    modFileInfo->dummy->getConfigElement = default_getConfigElement;
    modFileInfo->dummy->getConfigList = default_getConfigList;

    modFileInfo->configurable.self = modFileInfo;
    modFileInfo->configurable.getConfigElement = default_getConfigElement;
    modFileInfo->configurable.getConfigList = default_getConfigList;

    modFileInfo->modFileInfo.self = modFileInfo;
    modFileInfo->modFileInfo.getMods = default_getMods;
    modFileInfo->modFileInfo.requiredLanguageLoaders = default_requiredLanguageLoaders;
    modFileInfo->modFileInfo.showAsResourcePack = default_showAsResourcePack;
    modFileInfo->modFileInfo.showAsDataPack = default_showAsDataPack;
    modFileInfo->modFileInfo.getFileProperties = default_getFileProperties;
    modFileInfo->modFileInfo.getLicense = default_getLicense;
    modFileInfo->modFileInfo.versionString = default_versionString;
    modFileInfo->modFileInfo.usesServices = default_usesServices;
    modFileInfo->modFileInfo.getFile = default_getFile;
    modFileInfo->modFileInfo.getConfig = default_getConfig;
    modFileInfo->modFileInfo.free = default_free;

    return &modFileInfo->modFileInfo;
}

// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_JarModsDotTomlModFileReader_Read(
    void *self, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes)
{
    (void) self;
    // Java: return createModFile(jar, discoveryAttributes.withReader(this));
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithReader(discoveryAttributes,
                                                                   LIBMATTI_FML_JarModsDotTomlModFileReader_AsReader());
    return LIBMATTI_FML_JarModsDotTomlModFileReader_CreateModFile(jar, &attributes);
}

// Java: public int getPriority() - the interface default
static int default_priority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_DEFAULT_PRIORITY;
}

// Java: the reader singleton
LIBMATTI_NEOFORGESPI_IModFileReader *LIBMATTI_FML_JarModsDotTomlModFileReader_AsReader(void)
{
    static LIBMATTI_NEOFORGESPI_IModFileReader reader;
    static int initialised = 0;
    if (!initialised)
    {
        initialised = 1;
        reader.ordered.self = &reader;
        reader.ordered.getPriority = default_priority;
        reader.read = LIBMATTI_FML_JarModsDotTomlModFileReader_Read;
    }
    return &reader;
}

// Java: @Override public String toString() { return "mod manifest"; }
const char *LIBMATTI_FML_JarModsDotTomlModFileReader_ToString(void)
{
    return "mod manifest";
}
