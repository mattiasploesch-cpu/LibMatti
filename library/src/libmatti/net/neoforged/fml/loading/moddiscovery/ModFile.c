#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/LanguageProviderLoader.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/modscan/Scanner.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

static const char *modFile_getId(void *self)
{
    return ((LIBMATTI_FML_ModFile *) self)->id;
}

static LIBMATTI_FML_JarContents *modFile_getContents(void *self)
{
    return ((LIBMATTI_FML_ModFile *) self)->contents;
}

// Java: public Supplier<Map<String, Object>> getSubstitutionMap()
static void *modFile_getSubstitutionMap(void *self)
{
    // Java: () -> ImmutableMap.<String, Object>builder().put("jarVersion", jarVersion).putAll(fileProperties).build()
    return LIBMATTI_FML_ModFile_GetSubstitutionMap(self);
}

static LIBMATTI_NEOFORGESPI_IModFile_Type modFile_getType(void *self)
{
    return ((LIBMATTI_FML_ModFile *) self)->modFileType;
}

// Java: public Path getFilePath() { return getContents().getPrimaryPath(); }
static const char *modFile_getFilePath(void *self)
{
    return LIBMATTI_FML_JarContents_GetPrimaryPath(((LIBMATTI_FML_ModFile *) self)->contents);
}

// Java: public List<IModInfo> getModInfos() { return modFileInfo.getMods(); }
static LIBMATTI_NEOFORGESPI_IModInfo **modFile_getModInfos(void *self, size_t *count)
{
    LIBMATTI_FML_ModFile *modFile = self;
    return LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(modFile->modFileInfo, count);
}

static LIBMATTI_NEOFORGESPI_ModFileScanData *modFile_getScanResult(void *self)
{
    // Java: return futureScanResult.get() - the port reads the completed value
    LIBMATTI_FML_ModFile *modFile = self;
    return modFile->scanResult;
}

// Java: public String getFileName() { return getFilePath().getFileName().toString(); }
static const char *modFile_getFileName(void *self)
{
    const char *filePath = modFile_getFilePath(self);
    const char *slash = strrchr(filePath, '/');
    return slash != NULL ? slash + 1 : filePath;
}

static LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *modFile_getDiscoveryAttributes(void *self)
{
    return &((LIBMATTI_FML_ModFile *) self)->discoveryAttributes;
}

static LIBMATTI_NEOFORGESPI_IModFileInfo *modFile_getModFileInfo(void *self)
{
    return ((LIBMATTI_FML_ModFile *) self)->modFileInfo;
}

// Java: private static Type parseType(JarContents contents)
static LIBMATTI_NEOFORGESPI_IModFile_Type parse_type(LIBMATTI_FML_JarContents *contents)
{
    // Java: var value = contents.getManifest().getMainAttributes().getValue(TYPE); return value != null ? Type.valueOf(value) : Type.MOD;
    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(contents);
    const char *value = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, LIBMATTI_FML_MODFILE_TYPE)
                                         : NULL;
    if (value == NULL)
        return LIBMATTI_NEOFORGESPI_IModFile_Type_MOD;

    if (strcmp(value, "LIBRARY") == 0) return LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY;
    if (strcmp(value, "GAMELIBRARY") == 0) return LIBMATTI_NEOFORGESPI_IModFile_Type_GAMELIBRARY;
    return LIBMATTI_NEOFORGESPI_IModFile_Type_MOD;
}

// Java: public ModFile(JarContents contents, @Nullable JarModuleInfo metadata, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_NewWithMetadataAndType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata, LIBMATTI_NEOFORGESPI_IModFile_Type type,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes)
{
    LIBMATTI_FML_ModFile *modFile = calloc(1, sizeof(LIBMATTI_FML_ModFile));
    modFile->modFile.self = modFile;
    modFile->modFile.getId = modFile_getId;
    modFile->modFile.getContents = modFile_getContents;
    modFile->modFile.getSubstitutionMap = modFile_getSubstitutionMap;
    modFile->modFile.getType = modFile_getType;
    modFile->modFile.getFilePath = modFile_getFilePath;
    modFile->modFile.getModInfos = modFile_getModInfos;
    modFile->modFile.getScanResult = modFile_getScanResult;
    modFile->modFile.getFileName = modFile_getFileName;
    modFile->modFile.getDiscoveryAttributes = modFile_getDiscoveryAttributes;
    modFile->modFile.getModFileInfo = modFile_getModFileInfo;

    modFile->contents = contents;
    modFile->discoveryAttributes = *discoveryAttributes;
    modFile->modFileType = type;

    // Java: jarVersion = Optional.ofNullable(getContents().getManifest().getMainAttributes().getValue(Attributes.Name.IMPLEMENTATION_VERSION)).orElse("0.0NONE");
    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(contents);
    const char *jarVersion = manifest != NULL
                                 ? LIBMATTI_JU_Manifest_GetMainValue(manifest, "Implementation-Version")
                                 : NULL;
    modFile->jarVersion = strdup(jarVersion != NULL ? jarVersion : "0.0NONE");

    // Java: this.modFileInfo = ModFileParser.readModList(this, Objects.requireNonNull(parser, "parser"));
    //       Java throws a ModLoadingException when the mod metadata cannot be read; the port has no
    //       exception channel, so it refuses the file instead of keeping a file without metadata.
    modFile->modFileInfo = LIBMATTI_FML_ModFileParser_ReadModList(modFile, parser, parserUserdata);
    if (modFile->modFileInfo == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Failed to read the mod metadata of {}",
                                 modFile_getFilePath(modFile));
        // Java: the GC - only what this constructor has allocated so far; the caller (and the
        //       discovery pipeline) still owns the contents and the module info.
        free(modFile->id);
        free(modFile->jarVersion);
        free(modFile);
        return NULL;
    }

    // Java: jarModuleInfo = metadata != null ? metadata : JarModuleInfo.from(contents);
    modFile->jarModuleInfo = metadata != NULL ? metadata : LIBMATTI_FML_JarModuleInfo_From(contents);

    // Java: if (modFileInfo != null && !modFileInfo.getMods().isEmpty()) id = modFileInfo.getMods().getFirst().getModId();
    //       else id = jarModuleInfo.name();
    size_t modCount = 0;
    LIBMATTI_NEOFORGESPI_IModInfo **mods =
        modFile->modFileInfo != NULL ? LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(modFile->modFileInfo, &modCount)
                                     : NULL;
    if (modCount > 0)
    {
        modFile->id = strdup(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mods[0]));
    }
    else
    {
        const char *name = LIBMATTI_FML_JarModuleInfo_Name(modFile->jarModuleInfo);
        modFile->id = strdup(name != NULL ? name : "");
    }

    if (modFile->modFileInfo != NULL)
    {
        // Java: LOGGER.debug(LogMarkers.LOADING, "Loading mod file {} with languages {}", ...)
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Loading mod file {} with languages",
                                 modFile_getFilePath(modFile));

        // Java: this.mixinConfigs = ModFileParser.getMixinConfigs(this.modFileInfo);
        modFile->mixinConfigs = LIBMATTI_FML_ModFileParser_GetMixinConfigs(modFile->modFileInfo,
                                                                          &modFile->mixinConfigCount);

        // Java: this.accessTransformers = ModFileParser.getAccessTransformers(this.modFileInfo)
        //         .map(list -> list.stream().filter(path -> { if (!getContents().containsFile(path)) { LOGGER.error(...); return false; } return true; }))
        //         .orElseGet(() -> getContents().containsFile(DEFAULT_ACCESS_TRANSFORMER) ? Stream.of(DEFAULT_ACCESS_TRANSFORMER) : Stream.empty())
        //         .toList();
        char **accessTransformers = NULL;
        size_t accessTransformerCount = 0;
        if (LIBMATTI_FML_ModFileParser_GetAccessTransformers(modFile->modFileInfo, &accessTransformers,
                                                             &accessTransformerCount))
        {
            for (size_t i = 0; i < accessTransformerCount; i++)
            {
                if (!LIBMATTI_FML_JarContents_ContainsFile(contents, accessTransformers[i]))
                {
                    LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                             "Access transformer file {} provided by mod {} does not exist!",
                                             accessTransformers[i], modFile->id);
                    continue;
                }
                modFile->accessTransformers =
                    realloc(modFile->accessTransformers, sizeof(char *) * (modFile->accessTransformerCount + 1));
                modFile->accessTransformers[modFile->accessTransformerCount++] = accessTransformers[i];
            }
        }
        else if (LIBMATTI_FML_JarContents_ContainsFile(contents, LIBMATTI_FML_MODFILE_DEFAULT_ACCESS_TRANSFORMER))
        {
            modFile->accessTransformers = malloc(sizeof(char *));
            modFile->accessTransformers[0] = strdup(LIBMATTI_FML_MODFILE_DEFAULT_ACCESS_TRANSFORMER);
            modFile->accessTransformerCount = 1;
        }
    }

    return modFile;
}

// Java: public ModFile(JarContents contents, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_New(LIBMATTI_FML_JarContents *contents,
                                               LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
                                               const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    // Java: this(contents, null, parser, parseType(contents), attributes);
    return LIBMATTI_FML_ModFile_NewWithMetadataAndType(contents, NULL, parser, parserUserdata, parse_type(contents),
                                                       attributes);
}

// Java: public ModFile(JarContents contents, @Nullable JarModuleInfo metadata, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_NewWithMetadata(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    // Java: this(contents, metadata, parser, parseType(contents), attributes);
    return LIBMATTI_FML_ModFile_NewWithMetadataAndType(contents, metadata, parser, parserUserdata,
                                                       parse_type(contents), attributes);
}

void LIBMATTI_FML_ModFile_Free(LIBMATTI_FML_ModFile *modFile)
{
    if (modFile == NULL) return;
    free(modFile->id);
    free(modFile->jarVersion);
    for (size_t i = 0; i < modFile->mixinConfigCount; i++) free(modFile->mixinConfigs[i].config);
    free(modFile->mixinConfigs);
    free(modFile->accessTransformers);
    free(modFile->loaders);
    LIBMATTI_JUC_CompletableFuture_Free(modFile->futureScanResult);
    LIBMATTI_NEOFORGESPI_ModFileScanData_Free(modFile->scanResult);
    // Java: the GC - the module info, JarContents and IModFileInfo are owned by the mod file
    LIBMATTI_FML_JarModuleInfo_Free(modFile->jarModuleInfo);
    LIBMATTI_FML_JarContents_Close(modFile->contents);
    LIBMATTI_NEOFORGESPI_IModFileInfo_Free(modFile->modFileInfo);
    free(modFile);
}

LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_ModFile_AsModFile(LIBMATTI_FML_ModFile *modFile)
{
    return modFile != NULL ? &modFile->modFile : NULL;
}

// Java: the cast used by the parser and the sorter
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_AsConcrete(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile != NULL ? (LIBMATTI_FML_ModFile *) modFile->self : NULL;
}

// Java: public JarContents getContents()
LIBMATTI_FML_JarContents *LIBMATTI_FML_ModFile_GetContents(const LIBMATTI_FML_ModFile *modFile)
{
    return modFile->contents;
}

// Java: public List<IModLanguageLoader> getLoaders()
LIBMATTI_NEOFORGESPI_IModLanguageLoader **LIBMATTI_FML_ModFile_GetLoaders(const LIBMATTI_FML_ModFile *modFile,
                                                                          size_t *count)
{
    *count = modFile->loaderCount;
    return modFile->loaders;
}

// Java: the supplyAsync supplier - new Scanner(this).scan()
typedef struct
{
    LIBMATTI_FML_ModFile *modFile;
} ScanSupplier;

static void *run_scan(void *userdata)
{
    ScanSupplier *supplier = userdata;
    LIBMATTI_FML_Scanner *scanner = LIBMATTI_FML_Scanner_New(supplier->modFile);
    LIBMATTI_NEOFORGESPI_ModFileScanData *result = LIBMATTI_FML_Scanner_Scan(scanner);
    LIBMATTI_FML_Scanner_Free(scanner);
    // Java: the CompletableFuture carries the value; the port keeps the owned copy on the file
    supplier->modFile->scanResult = result;
    return result;
}

// Java: public CompletionStage<ModFileScanData> startScan(Executor executor)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_ModFile_StartScan(LIBMATTI_FML_ModFile *modFile,
                                                               LIBMATTI_JUC_Executor *executor)
{
    // Java: if (this.futureScanResult != null) throw new IllegalStateException("The mod file scan was already started.");
    if (modFile->futureScanResult != NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "The mod file scan was already started.");
        return NULL;
    }

    // Java: this.futureScanResult = CompletableFuture.supplyAsync(() -> new Scanner(this).scan(), executor);
    // The port's executors have no thread: the direct executor runs the scan inline (the same
    // result the old synchronous port had), a queued executor stores the command for DrainQueued.
    modFile->futureScanResult = LIBMATTI_JUC_CompletableFuture_New();
    if (executor == NULL || executor->queued == 0)
    {
        ScanSupplier supplier = {modFile};
        LIBMATTI_JUC_CompletableFuture_Complete(modFile->futureScanResult, run_scan(&supplier));
    }
    else
    {
        ScanSupplier *supplier = malloc(sizeof(ScanSupplier));
        supplier->modFile = modFile;
        executor->execute(executor, (LIBMATTI_JUC_Runnable) run_scan, supplier);
    }
    return modFile->futureScanResult;
}

// Java: public ModFileScanData getScanResult()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_FML_ModFile_GetScanResult(const LIBMATTI_FML_ModFile *modFile)
{
    // Java: if (this.futureScanResult == null) throw new IllegalStateException("Scanning of this mod file has not started yet.");
    //       try { return this.futureScanResult.get(); } catch (ExecutionException e) { throw new RuntimeException(e.getCause()); }
    if (modFile->futureScanResult == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Scanning of this mod file has not started yet.");
        return NULL;
    }
    return modFile->scanResult;
}

// Java: public void setFileProperties(Map<String, Object> fileProperties)
void LIBMATTI_FML_ModFile_SetFileProperties(LIBMATTI_FML_ModFile *modFile, LIBMATTI_JU_HashMap *fileProperties)
{
    modFile->fileProperties = fileProperties;
}

// Java: the putAll(fileProperties) step of the substitution map builder
typedef struct
{
    LIBMATTI_JU_HashMap *target;
} PutAllState;

static void put_all(const char *key, void *value, void *userdata)
{
    PutAllState *state = userdata;
    LIBMATTI_JU_HashMap_Put(state->target, key, value);
}

// Java: public Supplier<Map<String, Object>> getSubstitutionMap() - the supplier's value
LIBMATTI_JU_HashMap *LIBMATTI_FML_ModFile_GetSubstitutionMap(const LIBMATTI_FML_ModFile *modFile)
{
    // Java: ImmutableMap.<String, Object>builder().put("jarVersion", jarVersion).putAll(fileProperties).build()
    LIBMATTI_JU_HashMap *map = LIBMATTI_JU_HashMap_New();
    LIBMATTI_JU_HashMap_Put(map, "jarVersion", (void *) modFile->jarVersion);
    if (modFile->fileProperties != NULL)
    {
        PutAllState state = {map};
        LIBMATTI_JU_HashMap_ForEach(modFile->fileProperties, put_all, &state);
    }
    return map;
}

// Java: public List<String> getAccessTransformers()
char **LIBMATTI_FML_ModFile_GetAccessTransformers(const LIBMATTI_FML_ModFile *modFile, size_t *count)
{
    *count = modFile->accessTransformerCount;
    return modFile->accessTransformers;
}

// Java: public List<MixinConfig> getMixinConfigs()
LIBMATTI_FML_ModFileParser_MixinConfig *LIBMATTI_FML_ModFile_GetMixinConfigs(const LIBMATTI_FML_ModFile *modFile,
                                                                            size_t *count)
{
    *count = modFile->mixinConfigCount;
    return modFile->mixinConfigs;
}

// Java: public void identifyLanguage()
void LIBMATTI_FML_ModFile_IdentifyLanguage(LIBMATTI_FML_ModFile *modFile)
{
    // Java: this.loaders = this.modFileInfo.requiredLanguageLoaders().stream()
    //         .map(spec -> FMLLoader.getLanguageLoadingProvider().findLanguage(this, spec.languageName(), spec.acceptedVersions())).toList();
    size_t specCount = 0;
    LIBMATTI_NEOFORGESPI_IModFileInfo_LanguageSpec *specs =
        LIBMATTI_NEOFORGESPI_IModFileInfo_RequiredLanguageLoaders(modFile->modFileInfo, &specCount);

    LIBMATTI_FML_LanguageProviderLoader *provider =
        (LIBMATTI_FML_LanguageProviderLoader *) LIBMATTI_FML_FMLLoader_GetLanguageLoadingProvider();

    modFile->loaders = calloc(specCount > 0 ? specCount : 1, sizeof(LIBMATTI_NEOFORGESPI_IModLanguageLoader *));
    for (size_t i = 0; i < specCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader = LIBMATTI_FML_LanguageProviderLoader_FindLanguage(
            provider, modFile, specs[i].languageName, specs[i].acceptedVersions);
        if (loader != NULL) modFile->loaders[modFile->loaderCount++] = loader;
    }
}

// Java: public ModuleDescriptor getModuleDescriptor()
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_FML_ModFile_GetModuleDescriptor(LIBMATTI_FML_ModFile *modFile)
{
    // Java: this should be called once in parallel for all content, hence the double-checked lock
    if (modFile->moduleDescriptor == NULL)
        modFile->moduleDescriptor = LIBMATTI_FML_JarModuleInfo_CreateDescriptor(modFile->jarModuleInfo,
                                                                               modFile->contents);
    return modFile->moduleDescriptor;
}

// Java: public ArtifactVersion getJarVersion()
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_ModFile_GetJarVersion(const LIBMATTI_FML_ModFile *modFile)
{
    // Java: return new DefaultArtifactVersion(this.jarVersion);
    return LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(modFile->jarVersion);
}

// Java: public IModFileInfo getModFileInfo()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFile_GetModFileInfo(const LIBMATTI_FML_ModFile *modFile)
{
    return modFile->modFileInfo;
}

// Java: public ModFileDiscoveryAttributes getDiscoveryAttributes()
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *LIBMATTI_FML_ModFile_GetDiscoveryAttributes(
    const LIBMATTI_FML_ModFile *modFile)
{
    return (LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *) &modFile->discoveryAttributes;
}

// Java: public void setDiscoveryAttributes(ModFileDiscoveryAttributes discoveryAttributes)
void LIBMATTI_FML_ModFile_SetDiscoveryAttributes(LIBMATTI_FML_ModFile *modFile,
                                                 const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    modFile->discoveryAttributes = *attributes;
}

// Java: getSubstitutionMap().get().get(key)
const char *LIBMATTI_FML_ModFile_GetSubstitutionValue(const LIBMATTI_FML_ModFile *modFile, const char *key)
{
    // Java: put("jarVersion", jarVersion).putAll(fileProperties)
    if (strcmp(key, "jarVersion") == 0) return modFile->jarVersion;
    // Java: the fileProperties map (manifest + mod-file properties); the port
    // resolves the id (the file name) the loader hands to SecureJar substitutions
    if (strcmp(key, "file") == 0) return modFile->id;
    return NULL;
}

// Java: public void close()
void LIBMATTI_FML_ModFile_Close(LIBMATTI_FML_ModFile *modFile)
{
    // Java: if (futureScanResult != null) { futureScanResult.cancel(true); futureScanResult = null; }
    LIBMATTI_JUC_CompletableFuture_Free(modFile->futureScanResult);
    modFile->futureScanResult = NULL;
    LIBMATTI_NEOFORGESPI_ModFileScanData_Free(modFile->scanResult);
    modFile->scanResult = NULL;

    // Java: contents.close()
    LIBMATTI_FML_JarContents_Close(modFile->contents);
    modFile->contents = NULL;
}
