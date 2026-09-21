#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/ModuleLayer.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/java/lang/module/ModuleReference.h"
#include "libmatti/java/net/URLClassLoader.h"
#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/net/neoforged/fml/FMLVersion.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/classloading/JarContentsModuleFinder.h"
#include "libmatti/net/neoforged/fml/classloading/ResourceMaskingClassLoader.h"
#include "libmatti/net/neoforged/fml/common/asm/AccessTransformerService.h"
#include "libmatti/net/neoforged/fml/common/asm/SimpleProcessorsGroup.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/RuntimeEnumExtender.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"
#include "libmatti/net/neoforged/fml/jarcontents/CompositeJarContents.h"
#include "libmatti/net/neoforged/fml/loading/EarlyServiceDiscovery.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"
#include "libmatti/net/neoforged/fml/loading/LanguageProviderLoader.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/fml/loading/ModSorter.h"
#include "libmatti/net/neoforged/fml/loading/VersionSupportMatrix.h"
#include "libmatti/net/neoforged/fml/loading/mixin/MixinFacade.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModDiscoverer.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/ClassLoadingGuardian.h"
#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/GameLocator.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/InDevFolderLocator.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/InDevJarLocator.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/ModsFolderLocator.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/NeoForgeDevDistCleaner.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/NestedLibraryModReader.h"
#include "libmatti/net/neoforged/fml/loading/modscan/BackgroundScanHandler.h"
#include "libmatti/net/minecraft/client/main/Main.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"
#include "libmatti/net/neoforged/fml/startup/InstrumentationHelper.h"
#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"
#include "libmatti/net/neoforged/fml/util/ServiceLoaderUtil.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorProvider.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogUtils.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final AtomicReference<@Nullable FMLLoader> current = new AtomicReference<>();
static LIBMATTI_FML_FMLLoader *s_current = NULL;

// Java: create(...) throws; the C port reads the thrown exception back here
static LIBMATTI_JL_Throwable *lastException = NULL;

LIBMATTI_JL_Throwable *LIBMATTI_FML_FMLLoader_GetLastException(void)
{
    return lastException;
}

// Java: the throw inside create(...) - the port stores what would have been thrown
void LIBMATTI_FML_FMLLoader_SetLastException(LIBMATTI_JL_Throwable *exception)
{
    lastException = exception;
}

// Java: ownedResources.add(mixinFacade) - the close() adapter of a MixinFacade
static void close_mixin_facade(void *self);
// Java: ownedResources.add(classLoadingGuardian) - the close() adapter of a ClassLoadingGuardian
static void close_class_loading_guardian(void *self);

// Java: List.add
static void add_closeable(LIBMATTI_JL_AutoCloseable **list, size_t *count, LIBMATTI_JL_AutoCloseable closeable)
{
    *list = realloc(*list, sizeof(**list) * (*count + 1));
    (*list)[*count] = closeable;
    (*count)++;
}

// Java: p.get() - the loaded service instance
static void *instantiate_service(const LIBMATTI_JU_ServiceLoader_Provider *provider)
{
    return provider->provider;
}

// Java: builtInProcessors.add(processor) - NULL is the external (Mixin) processor
static void add_processor(LIBMATTI_NEOFORGESPI_ClassProcessor ***processors, size_t *count,
                          LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    if (processor == NULL) return;

    *processors = realloc(*processors, sizeof(**processors) * (*count + 1));
    (*processors)[*count] = processor;
    (*count)++;
}

// Java: ownedResources.add(loader) - the close() adapter of a TransformingClassLoader
static void close_transforming_class_loader(void *self)
{
    LIBMATTI_FML_ModuleClassLoader_Close(&((LIBMATTI_FML_TransformingClassLoader *) self)->base);
}

// Java: ModuleLayer.defineModules(cf, parentLayers, f -> loader)
static void *module_layer_class_loader_for(const char *moduleName, void *userdata)
{
    (void) moduleName;
    return userdata;
}

// Java: private final Set<Path> locatedPaths - contains()
static int located_contains(const LIBMATTI_FML_FMLLoader *loader, const char *path)
{
    for (size_t i = 0; i < loader->locatedPathCount; i++)
    {
        if (strcmp(loader->locatedPaths[i], path) == 0) return 1;
    }

    return 0;
}

// Java: private final Set<Path> locatedPaths - add()
static int located_add(LIBMATTI_FML_FMLLoader *loader, const char *path)
{
    if (located_contains(loader, path)) return 0;

    loader->locatedPaths = realloc(loader->locatedPaths, sizeof(*loader->locatedPaths) * (loader->locatedPathCount + 1));
    loader->locatedPaths[loader->locatedPathCount] = strdup(path);
    loader->locatedPathCount++;
    return 1;
}

// Java: private FMLLoader(ClassLoader currentClassLoader, String[] programArgs, Dist dist, boolean production, Path gameDir)
static LIBMATTI_FML_FMLLoader *fmlloader_new(LIBMATTI_JL_ClassLoader *currentClassLoader, int argc, char *argv[],
                                             LIBMATTI_DIST_Dist dist, int production, const char *gameDir)
{
    LIBMATTI_FML_FMLLoader *loader = calloc(1, sizeof(LIBMATTI_FML_FMLLoader));

    // Java: private final ClassLoader originalClassLoader = Thread.currentThread().getContextClassLoader();
    loader->originalClassLoader =
        LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread());
    loader->currentClassLoader = currentClassLoader;

    // Java: this.programArgs = ProgramArgs.from(programArgs);
    loader->programArgs = LIBMATTI_FML_ProgramArgs_From(argc, argv);

    loader->dist = dist;
    loader->production = production;
    loader->gameDir = gameDir;

    // Java: private final ClassProcessorAuditLog classTransformerAuditLog = new ClassProcessorAuditLog();
    loader->classTransformerAuditLog = LIBMATTI_FML_ClassProcessorAuditLog_New();

    // Java: versionInfo = new VersionInfo(this.programArgs.remove("fml.neoForgeVersion"),
    //         this.programArgs.remove("fml.mcVersion"), this.programArgs.remove("fml.neoFormVersion"));
    char *neoForgeVersion = LIBMATTI_FML_ProgramArgs_Remove(loader->programArgs, "fml.neoForgeVersion");
    char *mcVersion = LIBMATTI_FML_ProgramArgs_Remove(loader->programArgs, "fml.mcVersion");
    char *neoFormVersion = LIBMATTI_FML_ProgramArgs_Remove(loader->programArgs, "fml.neoFormVersion");
    loader->versionInfo = LIBMATTI_FML_VersionInfo_New(neoForgeVersion, mcVersion, neoFormVersion);
    free(neoForgeVersion);
    free(mcVersion);
    free(neoFormVersion);

    // Java: LOGGER.info("Starting FancyModLoader version {} ({} in {})", FMLVersion.getVersion(), dist, production ? "PROD" : "DEV");
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Starting FancyModLoader version {} ({} in {})",
                            LIBMATTI_FML_FMLVersion_GetVersion(),
                            dist == LIBMATTI_DIST_CLIENT ? "CLIENT" : "DEDICATED_SERVER",
                            production ? "PROD" : "DEV");

    // Java: LOGGER.info("Game directory: {}", gameDir);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Game directory: {}", gameDir);

    // Java: makeCurrent();
    {
        // Java: FMLLoader witness = current.compareAndExchange(null, this);
        LIBMATTI_FML_FMLLoader *witness = s_current;
        s_current = loader;

        if (witness != NULL)
        {
            // Java: throw new IllegalStateException("Another FML loader is already active: " + witness);
            char witnessIdentity[2 * sizeof(void *) + 1];
            snprintf(witnessIdentity, sizeof(witnessIdentity), "%zx", (size_t) (uintptr_t) witness);
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Another FML loader is already active: {}", witnessIdentity);
        }
    }

    return loader;
}

// Java: @ApiStatus.Internal public ClassProcessorAuditSource getClassTransformerAuditLog()
LIBMATTI_FML_ClassProcessorAuditSource *LIBMATTI_FML_FMLLoader_GetClassTransformerAuditLog(
    const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return classTransformerAuditLog;
    return &loader->classTransformerAuditLog->source;
}

// Java: public List<ModFile> allContent()
void **LIBMATTI_FML_FMLLoader_DiscoveryResult_AllContent(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result,
                                                         size_t *count)
{
    // Java: var content = new ArrayList<ModFile>(pluginContent.size() + gameContent.size() + gameLibraryContent.size());
    *count = result->pluginContentCount + result->gameContentCount + result->gameLibraryContentCount;
    void **content = malloc(sizeof(*content) * (*count == 0 ? 1 : *count));

    // Java: content.addAll(pluginContent); content.addAll(gameContent); content.addAll(gameLibraryContent);
    size_t index = 0;
    for (size_t i = 0; i < result->pluginContentCount; i++) content[index++] = result->pluginContent[i];
    for (size_t i = 0; i < result->gameContentCount; i++) content[index++] = result->gameContent[i];
    for (size_t i = 0; i < result->gameLibraryContentCount; i++) content[index++] = result->gameLibraryContent[i];

    return content;
}

// Java: public List<ModFile> allGameContent()
void **LIBMATTI_FML_FMLLoader_DiscoveryResult_AllGameContent(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result,
                                                            size_t *count)
{
    // Java: var content = new ArrayList<ModFile>(gameContent.size() + gameLibraryContent.size());
    *count = result->gameContentCount + result->gameLibraryContentCount;
    void **content = malloc(sizeof(*content) * (*count == 0 ? 1 : *count));

    // Java: content.addAll(gameContent); content.addAll(gameLibraryContent);
    size_t index = 0;
    for (size_t i = 0; i < result->gameContentCount; i++) content[index++] = result->gameContent[i];
    for (size_t i = 0; i < result->gameLibraryContentCount; i++) content[index++] = result->gameLibraryContent[i];

    return content;
}

// Java: public boolean hasErrors()
int LIBMATTI_FML_FMLLoader_DiscoveryResult_HasErrors(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result)
{
    // Java: return discoveryIssues.stream().anyMatch(i -> i.severity() == ModLoadingIssue.Severity.ERROR);
    for (size_t i = 0; i < result->discoveryIssueCount; i++)
    {
        if (result->discoveryIssues[i]->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) return 1;
    }

    return 0;
}

// Java: @Nullable public IModFile getModFileByClass(Class<?> clazz)
void *LIBMATTI_FML_FMLLoader_GetModFileByClass(const LIBMATTI_FML_FMLLoader *loader, const LIBMATTI_JL_Class *clazz)
{
    if (loader->loadingModList == NULL || clazz == NULL) return NULL;

    // Java: var packageName = clazz.getPackageName(); - everything before the last '.'
    const char *className = LIBMATTI_JL_Class_GetName(clazz);
    const char *lastDot = className != NULL ? strrchr(className, '.') : NULL;
    if (lastDot == NULL) return NULL;

    size_t packageLength = (size_t) (lastDot - className);
    char *packageName = malloc(packageLength + 1);
    memcpy(packageName, className, packageLength);
    packageName[packageLength] = '\0';

    // Java: if (packageName.isEmpty()) return null;
    // Java: return loadingModList.getPackageIndex().get(packageName);
    const char **packageNames = NULL;
    size_t count = 0;
    LIBMATTI_NEOFORGESPI_IModFile **packageIndex =
        LIBMATTI_FML_LoadingModList_GetPackageIndex(loader->loadingModList, &packageNames, &count);
    void *result = NULL;
    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(packageNames[i], packageName) == 0)
        {
            result = packageIndex[i];
            break;
        }
    }
    free(packageName);
    return result;
}

// Java: public void addCloseCallback(AutoCloseable callback)
void LIBMATTI_FML_FMLLoader_AddCloseCallback(LIBMATTI_FML_FMLLoader *loader, LIBMATTI_JL_AutoCloseable callback)
{
    // Java: closeCallbacks.add(callback);
    add_closeable(&loader->closeCallbacks, &loader->closeCallbackCount, callback);
}

// Java: public void close()
void LIBMATTI_FML_FMLLoader_Close(LIBMATTI_FML_FMLLoader *loader)
{
    if (loader == NULL) return;

    char identity[2 * sizeof(void *) + 1];
    snprintf(identity, sizeof(identity), "%zx", (size_t) (uintptr_t) loader);

    // Java: LOGGER.info("Closing FML Loader {}", Integer.toHexString(System.identityHashCode(this)));
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Closing FML Loader {}", identity);

    // Java: for (var closeCallback : closeCallbacks) { try { closeCallback.close(); } catch (Exception e) {
    //           LOGGER.error("Failed to run mod-supplied close callback {}", closeCallback, e); } }
    for (size_t i = 0; i < loader->closeCallbackCount; i++)
    {
        LIBMATTI_JL_AutoCloseable_Close(&loader->closeCallbacks[i]);
    }
    free(loader->closeCallbacks);
    loader->closeCallbacks = NULL;
    loader->closeCallbackCount = 0;

    // Java: for (var modFile : earlyServicesJars) { modFile.close(); }
    for (size_t i = 0; i < loader->earlyServicesJarCount; i++)
        LIBMATTI_FML_ModFile_Close((LIBMATTI_FML_ModFile *) loader->earlyServicesJars[i]);

    if (loader->loadingModList != NULL)
    {
        // Java: for (var modFile : loadingModList.getModFiles()) { modFile.getFile().close(); }
        size_t modFileCount = 0;
        LIBMATTI_FML_ModFileInfo **modFiles =
            LIBMATTI_FML_LoadingModList_GetModFiles(loader->loadingModList, &modFileCount);
        for (size_t i = 0; i < modFileCount; i++)
            LIBMATTI_FML_ModFile_Close(LIBMATTI_FML_ModFileInfo_GetFile(modFiles[i]));

        // Java: for (var modFile : loadingModList.getPlugins()) { ((ModFile) modFile.getFile()).close(); }
        size_t pluginCount = 0;
        LIBMATTI_NEOFORGESPI_IModFileInfo **plugins =
            LIBMATTI_FML_LoadingModList_GetPlugins(loader->loadingModList, &pluginCount);
        for (size_t i = 0; i < pluginCount; i++)
            LIBMATTI_FML_ModFile_Close((LIBMATTI_FML_ModFile *) LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(plugins[i]));

        // Java: for (var modFile : loadingModList.getGameLibraries()) { ((ModFile) modFile).close(); }
        size_t gameLibraryCount = 0;
        LIBMATTI_NEOFORGESPI_IModFile **gameLibraries =
            LIBMATTI_FML_LoadingModList_GetGameLibraries(loader->loadingModList, &gameLibraryCount);
        for (size_t i = 0; i < gameLibraryCount; i++)
            LIBMATTI_FML_ModFile_Close((LIBMATTI_FML_ModFile *) gameLibraries[i]);
    }

    // Java: if (this == current.compareAndExchange(this, null)) { ModList.clear(); ModLoader.clear(); }
    if (loader == s_current)
    {
        s_current = NULL;
        LIBMATTI_FML_ModList_Clear();
        LIBMATTI_FML_ModLoader_Clear();
    }

    // Java: for (var ownedResource : ownedResources) { try { ownedResource.close(); } catch (Exception e) {
    //           LOGGER.error("Failed to close resource {} owned by FMLLoader", ownedResource, e); } }
    for (size_t i = 0; i < loader->ownedResourceCount; i++)
    {
        LIBMATTI_JL_AutoCloseable_Close(&loader->ownedResources[i]);
    }
    free(loader->ownedResources);
    loader->ownedResources = NULL;
    loader->ownedResourceCount = 0;

    // Java: if (Thread.currentThread().getContextClassLoader() == currentClassLoader) {
    //           Thread.currentThread().setContextClassLoader(originalClassLoader); }
    LIBMATTI_JL_Thread *thread = LIBMATTI_JL_Thread_CurrentThread();
    if (LIBMATTI_JL_Thread_GetContextClassLoader(thread) == loader->currentClassLoader)
    {
        LIBMATTI_JL_Thread_SetContextClassLoader(thread, loader->originalClassLoader);
    }
}

// Java has no destructor; the C port releases what the constructor allocated.
void LIBMATTI_FML_FMLLoader_Free(LIBMATTI_FML_FMLLoader *loader)
{
    if (loader == NULL) return;

    free(loader->closeCallbacks);
    free(loader->ownedResources);

    for (size_t i = 0; i < loader->earlyServicesJarCount; i++)
        LIBMATTI_FML_ModFile_Free((LIBMATTI_FML_ModFile *) loader->earlyServicesJars[i]);
    free(loader->earlyServicesJars);

    for (size_t i = 0; i < loader->locatedPathCount; i++)
        free(loader->locatedPaths[i]);
    free(loader->locatedPaths);

    free(loader->classTransformerAuditLog);
    LIBMATTI_JL_ModuleLayer_Free(loader->gameLayer);
    LIBMATTI_FML_ProgramArgs_Free(loader->programArgs);
    LIBMATTI_FML_VersionInfo_Free(loader->versionInfo);
    LIBMATTI_FML_VersionSupportMatrix_Free(loader->versionSupportMatrix);
    LIBMATTI_FML_BackgroundScanHandler_Free(loader->backgroundScanHandler);
    LIBMATTI_FML_LoadingModList_Free(loader->loadingModList);
    LIBMATTI_FML_LanguageProviderLoader_Free(loader->languageProviderLoader);

    if (loader->discoveryResult != NULL)
    {
        free(loader->discoveryResult->pluginContent);
        free(loader->discoveryResult->gameContent);
        free(loader->discoveryResult->gameLibraryContent);
        free(loader->discoveryResult);
    }

    free(loader);
}

// Java: public ClassLoader getCurrentClassLoader()
LIBMATTI_JL_ClassLoader *LIBMATTI_FML_FMLLoader_GetCurrentClassLoader(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return currentClassLoader;
    return loader->currentClassLoader;
}

// Java: public ProgramArgs getProgramArgs()
LIBMATTI_FML_ProgramArgs *LIBMATTI_FML_FMLLoader_GetProgramArgs(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return programArgs;
    return loader->programArgs;
}

// Java: @ApiStatus.Internal public IBindingsProvider getBindings()
LIBMATTI_FML_IBindingsProvider *LIBMATTI_FML_FMLLoader_GetBindings(LIBMATTI_FML_FMLLoader *loader)
{
    // Java: if (bindings == null) { synchronized (this) { if (bindings == null) { ... } } }
    if (loader->bindings == NULL)
    {
        if (loader->gameLayer == NULL)
        {
            // Java: throw new IllegalStateException("Cannot retrieve bindings before the game layer is initialized.");
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cannot retrieve bindings before the game layer is initialized.");
            return NULL;
        }

        // Java: var providers = ServiceLoader.load(gameLayer, IBindingsProvider.class).stream().toList();
        size_t providerCount = 0;
        LIBMATTI_JU_ServiceLoader *services =
            LIBMATTI_JU_ServiceLoader_Load(loader->gameLayer, "net.neoforged.fml.IBindingsProvider");
        LIBMATTI_JU_ServiceLoader_Provider *providers = LIBMATTI_JU_ServiceLoader_Stream(services, &providerCount);

        if (providerCount == 0)
        {
            // Java: throw new IllegalStateException("Could not find bindings provider");
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Could not find bindings provider");
            LIBMATTI_JU_ServiceLoader_Free(services);
            return NULL;
        }

        if (providerCount > 1)
        {
            // Java: String providerList = providers.stream().map(p -> p.type().getName()).collect(Collectors.joining(", "));
            //       throw new IllegalStateException("Found more than one bindings provider: " + providerList);
            size_t length = 1;
            for (size_t i = 0; i < providerCount; i++)
                length += strlen(providers[i].providerClassName) + 2;
            char *providerList = calloc(1, length);
            for (size_t i = 0; i < providerCount; i++)
            {
                if (i > 0) strcat(providerList, ", ");
                strcat(providerList, providers[i].providerClassName);
            }

            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Found more than one bindings provider: {}", providerList);
            free(providerList);
            LIBMATTI_JU_ServiceLoader_Free(services);
            return NULL;
        }

        // Java: bindings = providers.getFirst().get();
        loader->bindings = providers[0].provider;
        LIBMATTI_JU_ServiceLoader_Free(services);
    }

    return loader->bindings;
}

// Java: public static FMLLoader create(StartupArgs startupArgs)
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_Create(LIBMATTI_FML_StartupArgs *startupArgs)
{
    // Java: var instrumentation = InstrumentationHelper.obtainInstrumentation();
    return LIBMATTI_FML_FMLLoader_CreateWithInstrumentation(
        LIBMATTI_FML_InstrumentationHelper_ObtainInstrumentation(), startupArgs);
}

// Java: public static FMLLoader create(@Nullable Instrumentation instrumentation, StartupArgs startupArgs)
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_CreateWithInstrumentation(void *instrumentation,
                                                                        LIBMATTI_FML_StartupArgs *startupArgs)
{
    // Java: var initialLoader = Objects.requireNonNullElse(startupArgs.parentClassLoader(), ClassLoader.getSystemClassLoader());
    LIBMATTI_JL_ClassLoader *initialLoader = startupArgs->parentClassLoader != NULL
                                                 ? startupArgs->parentClassLoader
                                                 : LIBMATTI_JL_ClassLoader_GetSystemClassLoader();

    // Java: PathPrettyPrinting.addRoot(startupArgs.gameDirectory());
    LIBMATTI_FML_PathPrettyPrinting_AddRoot(startupArgs->gameDirectory);

    // Java: var loader = new FMLLoader(initialLoader, startupArgs.programArgs(), Objects.requireNonNullElseGet(startupArgs.dist(),
    //             () -> detectDist(initialLoader)), detectProduction(initialLoader), startupArgs.gameDirectory());
    // The C port of StartupArgs has no nullable Dist, so detectDist(initialLoader) is not reachable here.
    LIBMATTI_FML_FMLLoader *loader = fmlloader_new(initialLoader, startupArgs->argc, startupArgs->programArgs,
                                                   startupArgs->dist, LIBMATTI_FML_FMLLoader_DetectProduction(initialLoader),
                                                   startupArgs->gameDirectory);

    // Java: try {
    {
        // Java: FMLPaths.loadAbsolutePaths(startupArgs.gameDirectory());
        LIBMATTI_FML_FMLPaths_LoadAbsolutePaths(startupArgs->gameDirectory);
        // Java: FMLConfig.load();
        LIBMATTI_FML_FMLConfig_Load();

        // Java: var launchContext = loader.new LaunchContextAdapter();
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter *launchContext =
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_New(loader);

        // Java: for (var claimedFile : startupArgs.claimedFiles()) { launchContext.addLocated(claimedFile.toPath()); }
        for (size_t i = 0; i < startupArgs->claimedFileCount; i++)
        {
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AddLocated(launchContext, startupArgs->claimedFiles[i]);
        }

        // Java: loader.loadEarlyServices(startupArgs);
        LIBMATTI_FML_FMLLoader_LoadEarlyServices(loader, startupArgs);

        // Java: ImmediateWindowHandler.load(launchContext, startupArgs.headless(), loader.programArgs);
        LIBMATTI_FML_ImmediateWindowHandler_Load(
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(launchContext), startupArgs->headless,
            loader->programArgs);

        // Java: if (loader.versionInfo.neoForgeVersion() != null) ImmediateWindowHandler.setNeoForgeVersion(loader.versionInfo.neoForgeVersion());
        if (loader->versionInfo->neoForgeVersion != NULL)
            LIBMATTI_FML_ImmediateWindowHandler_SetNeoForgeVersion(loader->versionInfo->neoForgeVersion);
        // Java: if (loader.versionInfo.mcVersion() != null) ImmediateWindowHandler.setMinecraftVersion(loader.versionInfo.mcVersion());
        if (loader->versionInfo->mcVersion != NULL)
            LIBMATTI_FML_ImmediateWindowHandler_SetMinecraftVersion(loader->versionInfo->mcVersion);

        // Java: if (startupArgs.headless()) { discoveryResult = loader.runDiscovery(); }
        //       else { discoveryResult = runOffThread(loader::runDiscovery); }
        LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult =
            startupArgs->headless
                ? LIBMATTI_FML_FMLLoader_RunDiscovery(loader)
                : LIBMATTI_FML_FMLLoader_RunOffThread_DiscoveryResult(loader);

        if (discoveryResult == NULL)
        {
            // Java: catch (RuntimeException | Error e) { try { loader.close(); } catch (Throwable t) { e.addSuppressed(t); } throw e; }
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(launchContext);
            LIBMATTI_FML_FMLLoader_Close(loader);
            LIBMATTI_FML_FMLLoader_Free(loader);
            return NULL;
        }

        // Java: for (var issue : discoveryResult.discoveryIssues()) { LOGGER.atLevel(issue.severity() == ERROR ? Level.ERROR : Level.WARN).setCause(issue.cause()).log("{}", FMLTranslations.translateIssueEnglish(issue)); }
        for (size_t i = 0; i < discoveryResult->discoveryIssueCount; i++)
        {
            LIBMATTI_FML_ModLoadingIssue *issue = discoveryResult->discoveryIssues[i];

            char *translation = LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(issue);
            if (translation == NULL) translation = LIBMATTI_FML_ModLoadingIssue_ToString(issue);

            if (issue->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR)
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", translation);
            else
                LIBMATTI_ML_Logger_Warn(LOGGER(), NULL, "{}", translation);

            free(translation);
        }

        // Java: if (discoveryResult.hasErrors()) { throw new ModLoadingException(discoveryResult.discoveryIssues); }
        if (LIBMATTI_FML_FMLLoader_DiscoveryResult_HasErrors(discoveryResult))
        {
            lastException = &LIBMATTI_FML_ModLoadingException_NewWithIssues(discoveryResult->discoveryIssues,
                                                                             discoveryResult->discoveryIssueCount)->base;
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(launchContext);
            LIBMATTI_FML_FMLLoader_Close(loader);
            LIBMATTI_FML_FMLLoader_Free(loader);
            return NULL;
        }

        // Java: discoveryResult.allContent().stream().parallel().forEach(ModFile::getModuleDescriptor);
        // (the port runs it sequentially over the three content lists; the descriptor computes once per file)
        for (size_t i = 0; i < discoveryResult->pluginContentCount; i++)
            LIBMATTI_FML_ModFile_GetModuleDescriptor((LIBMATTI_FML_ModFile *) discoveryResult->pluginContent[i]);
        for (size_t i = 0; i < discoveryResult->gameContentCount; i++)
            LIBMATTI_FML_ModFile_GetModuleDescriptor((LIBMATTI_FML_ModFile *) discoveryResult->gameContent[i]);
        for (size_t i = 0; i < discoveryResult->gameLibraryContentCount; i++)
            LIBMATTI_FML_ModFile_GetModuleDescriptor((LIBMATTI_FML_ModFile *) discoveryResult->gameLibraryContent[i]);

        // Java: if (instrumentation != null) { classLoadingGuardian = new ClassLoadingGuardian(instrumentation, discoveryResult.allGameContent()); loader.ownedResources.add(classLoadingGuardian); }
        // the guardian's protected packages come from the game content module descriptors
        // Java: the guardian is an owned resource; keep it around for the hand-off below.
        LIBMATTI_FML_ClassLoadingGuardian *classLoadingGuardian = NULL;
        if (instrumentation != NULL)
        {
            size_t gameContentCount = discoveryResult->gameContentCount;
            const char **packageNames = malloc(sizeof(char *) * (gameContentCount > 0 ? gameContentCount : 1));
            size_t packageNameCount = 0;
            for (size_t i = 0; i < gameContentCount; i++)
            {
                LIBMATTI_JL_ModuleDescriptor *descriptor = LIBMATTI_FML_ModFile_GetModuleDescriptor(
                    (LIBMATTI_FML_ModFile *) discoveryResult->gameContent[i]);
                size_t packageCount = 0;
                char **packages = LIBMATTI_JL_ModuleDescriptor_Packages(descriptor, &packageCount);
                for (size_t p = 0; p < packageCount; p++)
                    packageNames[packageNameCount++] = packages[p];
            }
            classLoadingGuardian =
                LIBMATTI_FML_ClassLoadingGuardian_New(instrumentation, packageNames, packageNameCount);
            free(packageNames);
            LIBMATTI_JL_AutoCloseable guardianCloseable = {classLoadingGuardian, close_class_loading_guardian};
            add_closeable(&loader->ownedResources, &loader->ownedResourceCount, guardianCloseable);
        }

        // Java: var mixinFacade = new MixinFacade(); loader.ownedResources.add(mixinFacade);
        LIBMATTI_FML_MixinFacade *mixinFacade = LIBMATTI_FML_MixinFacade_New();
        LIBMATTI_JL_AutoCloseable mixinFacadeCloseable = {mixinFacade, close_mixin_facade};
        add_closeable(&loader->ownedResources, &loader->ownedResourceCount, mixinFacadeCloseable);

        // Java: loader.loadPlugins(loader.loadingModList.getPlugins());
        {
            size_t pluginCount = 0;
            LIBMATTI_NEOFORGESPI_IModFileInfo **plugins = LIBMATTI_FML_LoadingModList_GetPlugins(
                (LIBMATTI_FML_LoadingModList *) loader->loadingModList, &pluginCount);
            LIBMATTI_FML_FMLLoader_LoadPlugins(loader, (void **) plugins, pluginCount);
        }

        // Java: loader.languageProviderLoader = new LanguageProviderLoader(launchContext);
        loader->languageProviderLoader = LIBMATTI_FML_LanguageProviderLoader_New(
            LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(launchContext));

        // Java: for (var modFile : discoveryResult.gameContent) { modFile.identifyLanguage(); }
        for (size_t i = 0; i < discoveryResult->gameContentCount; i++)
            LIBMATTI_FML_ModFile_IdentifyLanguage(discoveryResult->gameContent[i]);

        // Java: var gameContent = new ArrayList<JarContentsModule>();
        //       for (var modFile : discoveryResult.allGameContent()) { gameContent.add(new JarContentsModule(modFile.getContents(), modFile.getModuleDescriptor())); }
        size_t allGameContentCount = 0;
        void **allGameContent =
            LIBMATTI_FML_FMLLoader_DiscoveryResult_AllGameContent(discoveryResult, &allGameContentCount);
        LIBMATTI_FML_JarContentsModule **gameContent =
            calloc(allGameContentCount + 1, sizeof(*gameContent));
        size_t gameContentCount = 0;
        for (size_t i = 0; i < allGameContentCount; i++)
        {
            LIBMATTI_FML_ModFile *modFile = allGameContent[i];
            gameContent[gameContentCount++] = LIBMATTI_FML_JarContentsModule_NewWithDescriptor(
                LIBMATTI_FML_ModFile_GetContents(modFile), LIBMATTI_FML_ModFile_GetModuleDescriptor(modFile));
        }
        free(allGameContent);

        // Java: var classProcessorSet = createClassProcessorSet(startupArgs, launchContext, discoveryResult, mixinFacade);
        LIBMATTI_FML_ClassProcessorSet *classProcessorSet = LIBMATTI_FML_FMLLoader_CreateClassProcessorSet(
            startupArgs, launchContext, discoveryResult, mixinFacade);

        // Java: if (!classProcessorSet.getGeneratedPackages().isEmpty()) {
        size_t generatedPackageCount = 0;
        const char **generatedPackages =
            LIBMATTI_FML_ClassProcessorSet_GetGeneratedPackages(classProcessorSet, &generatedPackageCount);
        if (generatedPackageCount > 0)
        {
            // Java: var descriptor = ModuleDescriptor.newAutomaticModule(ClassProcessor.GENERATED_PACKAGE_MODULE).packages(classProcessorSet.getGeneratedPackages()).build();
            LIBMATTI_JL_ModuleDescriptor *descriptor = LIBMATTI_JL_ModuleDescriptor_Create(
                LIBMATTI_NEOFORGESPI_ClassProcessor_GENERATED_PACKAGE_MODULE, NULL, 0);
            LIBMATTI_JL_ModuleDescriptor_AddModifier(descriptor, LIBMATTI_JL_MODIFIER_AUTOMATIC);
            for (size_t i = 0; i < generatedPackageCount; i++)
                LIBMATTI_JL_ModuleDescriptor_AddPackage(descriptor, generatedPackages[i]);

            // Java: gameContent.add(new JarContentsModule(JarContents.empty(Path.of("VirtualJar/" + descriptor.name())), descriptor));
            size_t virtualJarLength = strlen(descriptor->name) + 11;
            char *virtualJarPath = malloc(virtualJarLength);
            snprintf(virtualJarPath, virtualJarLength, "VirtualJar/%s", descriptor->name);
            LIBMATTI_FML_JarContents *emptyContents = LIBMATTI_FML_JarContents_Empty(virtualJarPath);
            free(virtualJarPath);

            gameContent[gameContentCount++] =
                LIBMATTI_FML_JarContentsModule_NewWithDescriptor(emptyContents, descriptor);
        }

        // Java: var transformingLoader = loader.buildTransformingLoader(classProcessorSet, loader.classTransformerAuditLog, gameContent);
        LIBMATTI_FML_TransformingClassLoader *transformingLoader = LIBMATTI_FML_FMLLoader_BuildTransformingLoader(
            loader, classProcessorSet, loader->classTransformerAuditLog, gameContent, gameContentCount);

        // Java: the local gameContent list becomes unreachable here; the ModuleDescriptors and the
        //       JarContents stay alive through the game layer's Configuration.
        for (size_t i = 0; i < gameContentCount; i++) free(gameContent[i]);
        free(gameContent);

        // Java: if (classLoadingGuardian != null) { classLoadingGuardian.setAllowedClassLoader(transformingLoader); }
        if (classLoadingGuardian != NULL)
        {
            LIBMATTI_FML_ClassLoadingGuardian_SetAllowedClassLoader(classLoadingGuardian,
                                                                    (void *) transformingLoader);
        }

        // Java: mixinFacade.finishInitialization(loader.loadingModList, transformingLoader);
        LIBMATTI_FML_MixinFacade_FinishInitialization(mixinFacade, loader->loadingModList, transformingLoader);

        // Java: ImmediateWindowHandler.updateProgress("Launching minecraft"); ImmediateWindowHandler.renderTick();
        LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress("Launching minecraft");
        LIBMATTI_FML_ImmediateWindowHandler_RenderTick();

        // Java keeps the adapter alive through LanguageProviderLoader and the class processor set, both of which
        // have taken what they need by now.
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(launchContext);

        // Java: return loader;
        return loader;
    }
}

// Java: ownedResources.add(mixinFacade) - the close() adapter of a MixinFacade
static void close_mixin_facade(void *self)
{
    LIBMATTI_FML_MixinFacade_Close(self);
    LIBMATTI_FML_MixinFacade_Free(self);
}

// Java: ownedResources.add(classLoadingGuardian) - the close() adapter of a ClassLoadingGuardian
static void close_class_loading_guardian(void *self)
{
    LIBMATTI_FML_ClassLoadingGuardian_Free(self);
}

// Java: private static ClassProcessorSet createClassProcessorSet(StartupArgs startupArgs,
//         LaunchContextAdapter launchContext, DiscoveryResult discoveryResult, MixinFacade mixinFacade)
LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_FMLLoader_CreateClassProcessorSet(
    LIBMATTI_FML_StartupArgs *startupArgs, LIBMATTI_FML_FMLLoader_LaunchContextAdapter *launchContext,
    LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult, LIBMATTI_FML_MixinFacade *mixinFacade)
{
    // Java: var builtInProcessors = new ArrayList<ClassProcessor>();
    LIBMATTI_NEOFORGESPI_ClassProcessor **builtInProcessors = NULL;
    size_t builtInCount = 0;

    // Java: builtInProcessors.add(createAccessTransformerService(discoveryResult));
    add_processor(&builtInProcessors, &builtInCount,
                  LIBMATTI_FML_FMLLoader_CreateAccessTransformerService(discoveryResult));

    // Java: builtInProcessors.add(new RuntimeEnumExtender());
    add_processor(&builtInProcessors, &builtInCount, &LIBMATTI_FML_RuntimeEnumExtender_New()->processor);

    // Java: builtInProcessors.add(new SimpleProcessorsGroup());
    add_processor(&builtInProcessors, &builtInCount, &LIBMATTI_FML_SimpleProcessorsGroup_New()->processor);

    // Java: if (startupArgs.cleanDist()) {
    if (startupArgs->cleanDist)
    {
        // Java: var minecraftModFile = discoveryResult.gameContent().stream().filter(mf -> mf.getId().equals("minecraft")).findFirst().map(ModFile::getContents).orElse(null);
        LIBMATTI_FML_JarContents *minecraftModFile = NULL;
        for (size_t i = 0; i < discoveryResult->gameContentCount; i++)
        {
            LIBMATTI_FML_ModFile *modFile = discoveryResult->gameContent[i];
            if (strcmp(modFile->id, "minecraft") == 0)
            {
                minecraftModFile = modFile->contents;
                break;
            }
        }

        // Java: if (minecraftModFile != null && NeoForgeDevDistCleaner.supportsDistCleaning(minecraftModFile)) { builtInProcessors.add(new NeoForgeDevDistCleaner(minecraftModFile, startupArgs.dist())); }
        if (minecraftModFile != NULL &&
            LIBMATTI_FML_NeoForgeDevDistCleaner_SupportsDistCleaning(minecraftModFile))
        {
            add_processor(&builtInProcessors, &builtInCount,
                          &LIBMATTI_FML_NeoForgeDevDistCleaner_New(minecraftModFile, startupArgs->dist)
                               ->processor);
        }
    }

    // Java: builtInProcessors.add(mixinFacade.getClassProcessor());
    // Java: builtInProcessors.add(mixinFacade.getGeneratingClassProcessor());
    // The two Mixin class processors come from org.spongepowered.asm (external) and are NULL in the port.
    add_processor(&builtInProcessors, &builtInCount, LIBMATTI_FML_MixinFacade_GetClassProcessor(mixinFacade));
    add_processor(&builtInProcessors, &builtInCount,
                  LIBMATTI_FML_MixinFacade_GetGeneratingClassProcessor(mixinFacade));

    LIBMATTI_FML_ServiceLoaderUtil_Hooks hooks = {
        .filter = NULL,
        .applyPriority = 0,
        .getPriority = NULL,
        .instantiate = instantiate_service,
    };

    // Java: additionalServices = the built-in processors
    LIBMATTI_FML_Service *additionalServices = calloc(builtInCount > 0 ? builtInCount : 1,
                                                      sizeof(*additionalServices));
    for (size_t i = 0; i < builtInCount; i++)
        additionalServices[i].service = builtInProcessors[i];
    free(builtInProcessors);

    // Java: ServiceLoaderUtil.loadServices(launchContext, ClassProcessor.class, builtInProcessors)
    LIBMATTI_FML_ServiceList *processorServices = LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(launchContext),
        "net.neoforged.neoforgespi.transformation.ClassProcessor", additionalServices, builtInCount, &hooks);
    free(additionalServices);

    // Java: ServiceLoaderUtil.loadServices(launchContext, ClassProcessorProvider.class)
    LIBMATTI_FML_ServiceList *providerServices = LIBMATTI_FML_ServiceLoaderUtil_LoadServices(
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(launchContext),
        "net.neoforged.neoforgespi.transformation.ClassProcessorProvider", &hooks);

    LIBMATTI_NEOFORGESPI_ClassProcessor **processors = calloc(
        processorServices->count > 0 ? processorServices->count : 1, sizeof(*processors));
    for (size_t i = 0; i < processorServices->count; i++)
        processors[i] = processorServices->services[i].service;

    LIBMATTI_NEOFORGESPI_ClassProcessorProvider **providers = calloc(
        providerServices->count > 0 ? providerServices->count : 1, sizeof(*providers));
    for (size_t i = 0; i < providerServices->count; i++)
        providers[i] = providerServices->services[i].service;

    // Java: return ClassProcessorSet.builder().markMarker(ClassProcessorIds.SIMPLE_PROCESSORS_GROUP)
    //             .markMarker(ClassProcessorIds.COMPUTING_FRAMES).addProcessors(...).addProcessorProviders(...).build();
    LIBMATTI_FML_ClassProcessorSet_Builder *builder = LIBMATTI_FML_ClassProcessorSet_Builder_New();
    LIBMATTI_FML_ClassProcessorSet_Builder_MarkMarker(
        builder, LIBMATTI_NEOFORGESPI_ClassProcessorIds_SIMPLE_PROCESSORS_GROUP());
    LIBMATTI_FML_ClassProcessorSet_Builder_MarkMarker(
        builder, LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES());
    LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessors(builder, processors, processorServices->count);
    LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessorProviders(builder, providers, providerServices->count);
    LIBMATTI_FML_ClassProcessorSet *classProcessorSet = LIBMATTI_FML_ClassProcessorSet_Builder_Build(builder);

    LIBMATTI_FML_ClassProcessorSet_Builder_Free(builder);
    free(processors);
    free(providers);
    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(processorServices);
    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(providerServices);
    return classProcessorSet;
}

// Java: private static ClassProcessor createAccessTransformerService(DiscoveryResult discoveryResult)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLLoader_CreateAccessTransformerService(
    LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult)
{
    // Java: var engine = AccessTransformerEngine.newEngine();
    LIBMATTI_AT_AccessTransformerEngine *engine = LIBMATTI_AT_AccessTransformerEngine_New();

    // Java: for (var modFile : discoveryResult.gameContent()) { for (var atPath : modFile.getAccessTransformers()) { ... } }
    for (size_t i = 0; i < discoveryResult->gameContentCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = discoveryResult->gameContent[i];

        size_t atCount = 0;
        char **atPaths = LIBMATTI_FML_ModFile_GetAccessTransformers(modFile, &atCount);
        for (size_t j = 0; j < atCount; j++)
        {
            const char *atPath = atPaths[j];

            // Java: LOGGER.debug("Adding Access Transformer {} in {}", atPath, modFile);
            LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Adding Access Transformer {} in {}", atPath, modFile->id);

            // Java: try (var in = modFile.getContents().openFile(atPath)) {
            size_t length = 0;
            unsigned char *in = LIBMATTI_FML_JarContents_OpenFile(modFile->contents, atPath, &length);
            if (in == NULL)
            {
                // Java: LOGGER.error(LogMarkers.LOADING, "Access transformer file {} provided by {} does not exist!", atPath, modFile);
                LIBMATTI_ML_Logger_Error(LOGGER(), (LIBMATTI_ML_LogMarker *) &LIBMATTI_FML_MARKER_LOADING,
                                         "Access transformer file {} provided by {} does not exist!", atPath,
                                         modFile->id);
            }
            else
            {
                // Java: engine.loadAT(new InputStreamReader(new BufferedInputStream(in)), atPath);
                in[length] = '\0';
                int errorLine = 0;
                if (!LIBMATTI_AT_AccessTransformerEngine_LoadAT(engine, (const char *) in, atPath, &errorLine))
                {
                    // Java: the reader throws with the line number
                    LIBMATTI_ML_Logger_Error(LOGGER(), (LIBMATTI_ML_LogMarker *) &LIBMATTI_FML_MARKER_LOADING,
                                             "Failed to parse access transformer file {} at line {}", atPath, errorLine);
                }
                free(in);
            }
        }
    }

    // Java: return new AccessTransformerService(engine);
    return &LIBMATTI_FML_AccessTransformerService_New(engine)->processor;
}

// Java: private TransformingClassLoader buildTransformingLoader(ClassProcessorSet classProcessorSet,
//         ClassProcessorAuditLog auditTrail, List<JarContentsModule> content)
LIBMATTI_FML_TransformingClassLoader *LIBMATTI_FML_FMLLoader_BuildTransformingLoader(
    LIBMATTI_FML_FMLLoader *loader, LIBMATTI_FML_ClassProcessorSet *classProcessorSet,
    LIBMATTI_FML_ClassProcessorAuditLog *auditTrail, LIBMATTI_FML_JarContentsModule **content,
    size_t contentCount)
{
    // Java: maskContentAlreadyOnClasspath(content);
    LIBMATTI_FML_FMLLoader_MaskContentAlreadyOnClasspath(loader, content, contentCount);

    // Java: long start = System.currentTimeMillis();
    long long start = LIBMATTI_JL_System_CurrentTimeMillis();

    // Java: var parentLayers = List.of(ModuleLayer.boot());
    LIBMATTI_JL_ModuleLayer *parentLayers[1] = {LIBMATTI_JL_ModuleLayer_Boot()};

    // Java: var cf = Configuration.resolveAndBind(new JarContentsModuleFinder(content),
    //             parentLayers.stream().map(ModuleLayer::configuration).toList(), ModuleFinder.of(),
    //             content.stream().map(JarContentsModule::moduleName).toList());
    LIBMATTI_FML_JarContentsModuleFinder *finder = LIBMATTI_FML_JarContentsModuleFinder_New(
        (const LIBMATTI_FML_JarContentsModule **) content, contentCount);
    size_t referenceCount = 0;
    LIBMATTI_FML_JarContentsModuleReference **references =
        LIBMATTI_FML_JarContentsModuleFinder_FindAll(finder, &referenceCount);

    const char **roots = malloc(sizeof(*roots) * (contentCount > 0 ? contentCount : 1));
    for (size_t i = 0; i < contentCount; i++)
        roots[i] = LIBMATTI_FML_JarContentsModule_ModuleName(content[i]);

    // Java: resolveAndBind() additionally binds service providers into the resolution graph
    LIBMATTI_JL_Configuration *cf = LIBMATTI_JL_Configuration_ResolveAndBind(
        (LIBMATTI_JL_ModuleReference **) references, referenceCount, roots, contentCount);
    free(roots);
    free(references);

    // Java: the resolved Configuration keeps its ModuleReferences alive for as long as the game layer exists;
    //       only the finder's own bookkeeping is released here.
    for (size_t i = 0; i < finder->count; i++) free(finder->names[i]);
    free(finder->names);
    free(finder->references);
    free(finder);

    // Java: var moduleNames = getModuleNameList(cf, content); LOGGER.info("Building game content classloader:\n{}", moduleNames);
    char *moduleNames = LIBMATTI_FML_FMLLoader_GetModuleNameList(cf, content, contentCount);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Building game content classloader:\n{}", moduleNames);
    free(moduleNames);

    // Java: var loader = new TransformingClassLoader(classProcessorSet, auditTrail, cf, parentLayers, currentClassLoader);
    LIBMATTI_FML_TransformingClassLoader *transformingLoader = LIBMATTI_FML_TransformingClassLoader_New(
        classProcessorSet, auditTrail, cf, parentLayers, 1, loader->currentClassLoader);

    // Java: var layer = ModuleLayer.defineModules(cf, parentLayers, f -> loader).layer();
    LIBMATTI_JL_ModuleLayerController *controller = LIBMATTI_JL_ModuleLayer_DefineModules(
        cf, parentLayers, 1, module_layer_class_loader_for, transformingLoader);
    LIBMATTI_JL_ModuleLayer *layer = LIBMATTI_JL_ModuleLayerController_Layer(controller);

    // Java: var elapsed = System.currentTimeMillis() - start; LOGGER.info("Built game content classloader in {}ms", elapsed);
    long long elapsed = LIBMATTI_JL_System_CurrentTimeMillis() - start;
    char elapsedText[32];
    snprintf(elapsedText, sizeof(elapsedText), "%lld", elapsed);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Built game content classloader in {}ms", elapsedText);

    // Java: loader.setFallbackClassLoader(currentClassLoader);
    LIBMATTI_FML_ModuleClassLoader_SetFallbackClassLoader(&transformingLoader->base, loader->currentClassLoader);

    // Java: gameLayer = layer; ownedResources.add(loader); currentClassLoader = loader;
    //       Thread.currentThread().setContextClassLoader(loader);
    loader->gameLayer = layer;
    LIBMATTI_JL_AutoCloseable closeable = {transformingLoader, close_transforming_class_loader};
    add_closeable(&loader->ownedResources, &loader->ownedResourceCount, closeable);
    loader->currentClassLoader = (LIBMATTI_JL_ClassLoader *) transformingLoader;
    LIBMATTI_JL_Thread_SetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread(), loader->currentClassLoader);

    // Java: the JVM defines net.minecraft.client.main.Main through this loader when
    // Client.main resolves it (createMainMethodCallable -> Class.forName). The port
    // registers the game stand-in class on the game content classloader here, so
    // Class.forName finds it with the defining loader the check below requires.
    LIBMATTI_MC_Client_Main_Register(loader->currentClassLoader);

    LIBMATTI_JL_ModuleLayerController_Free(controller);
    return transformingLoader;
}

// Java: private void maskContentAlreadyOnClasspath(List<JarContentsModule> content)
void LIBMATTI_FML_FMLLoader_MaskContentAlreadyOnClasspath(LIBMATTI_FML_FMLLoader *loader,
                                                          LIBMATTI_FML_JarContentsModule **content,
                                                          size_t contentCount)
{
    // Java: var classpathItems = ClasspathResourceUtils.getAllClasspathItems(currentClassLoader);
    size_t classpathItemCount = 0;
    char **classpathItems =
        LIBMATTI_FML_ClasspathResourceUtils_GetAllClasspathItems(loader->currentClassLoader, &classpathItemCount);

    // Java: Set<Path> needsMasking = new HashSet<>();
    //       for (var secureJar : content) { for (var basePath : getBasePaths(secureJar.contents(), true)) { if (classpathItems.contains(basePath)) needsMasking.add(basePath); } }
    char **needsMasking = NULL;
    size_t needsMaskingCount = 0;
    for (size_t i = 0; i < contentCount; i++)
    {
        size_t basePathCount = 0;
        char **basePaths = LIBMATTI_FML_FMLLoader_GetBasePaths(content[i]->contents, 1, &basePathCount);
        for (size_t j = 0; j < basePathCount; j++)
        {
            int onClasspath = 0;
            for (size_t k = 0; k < classpathItemCount; k++)
            {
                if (strcmp(classpathItems[k], basePaths[j]) == 0)
                {
                    onClasspath = 1;
                    break;
                }
            }

            if (onClasspath)
            {
                needsMasking = realloc(needsMasking, sizeof(*needsMasking) * (needsMaskingCount + 1));
                needsMasking[needsMaskingCount++] = strdup(basePaths[j]);
            }
            free(basePaths[j]);
        }
        free(basePaths);
    }
    free(classpathItems);

    // Java: if (!needsMasking.isEmpty()) {
    if (needsMaskingCount > 0)
    {
        // Java: if (LOGGER.isDebugEnabled()) { LOGGER.debug("Masking classpath elements: {}", needsMasking.stream().map(PathPrettyPrinting::prettyPrint).toList()); }
        for (size_t i = 0; i < needsMaskingCount; i++)
        {
            char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(needsMasking[i]);
            LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Masking classpath elements: {}", pretty);
            free(pretty);
        }

        // Java: var maskedLoader = new ResourceMaskingClassLoader(currentClassLoader, needsMasking);
        LIBMATTI_FML_ResourceMaskingClassLoader *maskedLoader = LIBMATTI_FML_ResourceMaskingClassLoader_New(
            loader->currentClassLoader, (const char **) needsMasking, needsMaskingCount);

        // Java: if (Thread.currentThread().getContextClassLoader() == currentClassLoader) { Thread.currentThread().setContextClassLoader(maskedLoader); }
        LIBMATTI_JL_Thread *thread = LIBMATTI_JL_Thread_CurrentThread();
        if (LIBMATTI_JL_Thread_GetContextClassLoader(thread) == loader->currentClassLoader)
            LIBMATTI_JL_Thread_SetContextClassLoader(thread, (LIBMATTI_JL_ClassLoader *) maskedLoader);

        // Java: currentClassLoader = maskedLoader;
        loader->currentClassLoader = (LIBMATTI_JL_ClassLoader *) maskedLoader;
    }

    for (size_t i = 0; i < needsMaskingCount; i++) free(needsMasking[i]);
    free(needsMasking);
}

// Java: private static List<Path> getBasePaths(JarContents contents, boolean ignoreFilter)
char **LIBMATTI_FML_FMLLoader_GetBasePaths(LIBMATTI_FML_JarContents *contents, int ignoreFilter, size_t *count)
{
    char **paths = NULL;
    *count = 0;

    switch (contents->kind)
    {
        case LIBMATTI_FML_JARCONTENTS_KIND_COMPOSITE:
        {
            // Java: if (!ignoreFilter && compositeModContainer.isFiltered()) throw new IllegalStateException("Cannot load filtered Jar content into a URL classloader");
            if (!ignoreFilter && LIBMATTI_FML_CompositeJarContents_IsFiltered(contents))
            {
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                         "Cannot load filtered Jar content into a URL classloader");
                return NULL;
            }

            // Java: for (var delegate : compositeModContainer.getDelegates()) { result.addAll(getBasePaths(delegate, ignoreFilter)); }
            size_t delegateCount = 0;
            LIBMATTI_FML_JarContents **delegates =
                LIBMATTI_FML_CompositeJarContents_GetDelegates(contents, &delegateCount);
            for (size_t i = 0; i < delegateCount; i++)
            {
                size_t delegatePathCount = 0;
                char **delegatePaths =
                    LIBMATTI_FML_FMLLoader_GetBasePaths(delegates[i], ignoreFilter, &delegatePathCount);
                for (size_t j = 0; j < delegatePathCount; j++)
                {
                    paths = realloc(paths, sizeof(*paths) * (*count + 1));
                    paths[*count] = delegatePaths[j];
                    (*count)++;
                }
                free(delegatePaths);
            }
            free(delegates);
            break;
        }
        case LIBMATTI_FML_JARCONTENTS_KIND_EMPTY:
            // Java: case EmptyJarContents ignored -> {}
            break;
        case LIBMATTI_FML_JARCONTENTS_KIND_FOLDER:
        case LIBMATTI_FML_JARCONTENTS_KIND_JARFILE:
            // Java: result.add(folderModContainer.getPrimaryPath()); / result.add(jarModContainer.getPrimaryPath());
            paths = malloc(sizeof(*paths));
            paths[0] = strdup(LIBMATTI_FML_JarContents_GetPrimaryPath(contents));
            *count = 1;
            break;
    }

    return paths;
}

// Java: Stream.sorted() on the module name list
static int compare_lines(const void *left, const void *right)
{
    return strcmp(*(const char *const *) left, *(const char *const *) right);
}

// Java: private static String getModuleNameList(Configuration cf, List<JarContentsModule> content)
char *LIBMATTI_FML_FMLLoader_GetModuleNameList(LIBMATTI_JL_Configuration *configuration,
                                               LIBMATTI_FML_JarContentsModule **content, size_t contentCount)
{
    // Java: var jarsById = content.stream().collect(Collectors.toMap(JarContentsModule::moduleName, Function.identity()));
    size_t moduleCount = 0;
    LIBMATTI_JL_ResolvedModule **modules = LIBMATTI_JL_Configuration_Modules(configuration, &moduleCount);

    // Java: .map(module -> " - " + module.name() + " (" + contentList + ")").sorted().collect(Collectors.joining("\n"))
    char **lines = malloc(sizeof(*lines) * (moduleCount == 0 ? 1 : moduleCount));
    for (size_t i = 0; i < moduleCount; i++)
    {
        const char *name = LIBMATTI_JL_ResolvedModule_Name(modules[i]);

        // Java: var jar = jarsById.get(module.name());
        char *contentList = NULL;
        for (size_t j = 0; j < contentCount; j++)
        {
            if (strcmp(LIBMATTI_FML_JarContentsModule_ModuleName(content[j]), name) == 0)
            {
                // Java: jar.contents() - string concatenation calls toString()
                contentList = LIBMATTI_FML_JarContents_ToString(content[j]->contents);
                break;
            }
        }

        // Java: var contentList = jar != null ? jar.contents() : module.reference().location().map(URI::toString).orElse("unknown");
        if (contentList == NULL)
        {
            LIBMATTI_JL_ModuleReference *reference = LIBMATTI_JL_ResolvedModule_Reference(modules[i]);
            const LIBMATTI_JN_URI *location = LIBMATTI_JL_ModuleReference_Location(reference);
            contentList = strdup(location != NULL ? LIBMATTI_JN_URI_ToString(location) : "unknown");
        }

        size_t length = strlen(name) + strlen(contentList) + 5;
        lines[i] = malloc(length);
        snprintf(lines[i], length, " - %s (%s)", name, contentList);
        free(contentList);
    }

    if (moduleCount > 1) qsort(lines, moduleCount, sizeof(*lines), compare_lines);

    size_t joinedLength = 1;
    for (size_t i = 0; i < moduleCount; i++)
        joinedLength += strlen(lines[i]) + 1;
    char *joined = calloc(1, joinedLength);
    for (size_t i = 0; i < moduleCount; i++)
    {
        if (i > 0) strcat(joined, "\n");
        strcat(joined, lines[i]);
        free(lines[i]);
    }
    free(lines);

    return joined;
}

// Java: private static Dist detectDist(ClassLoader classLoader)
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_DetectDist(LIBMATTI_JL_ClassLoader *classLoader)
{
    // Java: var clientAvailable = classLoader.getResource("net/minecraft/client/main/Main.class") != null;
    //       return clientAvailable ? Dist.CLIENT : Dist.DEDICATED_SERVER;
    LIBMATTI_JN_URL *resource = LIBMATTI_JL_ClassLoader_GetResource(classLoader, "net/minecraft/client/main/Main.class");
    if (resource == NULL) return LIBMATTI_DIST_DEDICATED_SERVER;

    LIBMATTI_JN_URL_Free(resource);
    return LIBMATTI_DIST_CLIENT;
}

// Java: private static boolean detectProduction(ClassLoader classLoader)
int LIBMATTI_FML_FMLLoader_DetectProduction(LIBMATTI_JL_ClassLoader *classLoader)
{
    // Java: try (var resource = classLoader.getResourceAsStream("net/minecraft/SharedConstants.class")) {
    static const char *SIGNATURE = "net/neoforged/fml";
    size_t signatureLength = strlen(SIGNATURE);

    size_t length = 0;
    char *resourceContent = LIBMATTI_JL_ClassLoader_GetResourceAsStream(classLoader, "net/minecraft/SharedConstants.class",
                                                                        &length);

    // Java: if (resource == null) return true; // Likely an error but missing classes will be reported by the game locator more competently
    if (resourceContent == NULL) return 1;

    // Java: var signature = "net/neoforged/fml".getBytes(StandardCharsets.UTF_8);
    //       for (int i = 0; i < resourceContent.length - signature.length; i++) { ... if (match) return false; }
    for (size_t i = 0; i + signatureLength < length; i++)
    {
        if (memcmp(resourceContent + i, SIGNATURE, signatureLength) != 0) continue;

        // Java: Found signature -> we're definitely in dev
        free(resourceContent);
        return 0;
    }

    free(resourceContent);
    // Java: return true; // didn't find patched-in NeoForge reference in bytecode -> not in dev
    return 1;
}

// Java: private void loadEarlyServices(StartupArgs startupArgs)
void LIBMATTI_FML_FMLLoader_LoadEarlyServices(LIBMATTI_FML_FMLLoader *loader, LIBMATTI_FML_StartupArgs *startupArgs)
{
    // Java: this.earlyServicesJars.addAll(EarlyServiceDiscovery.findEarlyServiceJars(startupArgs, FMLPaths.MODSDIR.get()));
    size_t earlyServiceJarCount = 0;
    void **earlyServiceJars = LIBMATTI_FML_EarlyServiceDiscovery_FindEarlyServiceJars(
        startupArgs, LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_MODSDIR), &earlyServiceJarCount);

    for (size_t i = 0; i < earlyServiceJarCount; i++)
    {
        loader->earlyServicesJars =
            realloc(loader->earlyServicesJars, sizeof(void *) * (loader->earlyServicesJarCount + 1));
        loader->earlyServicesJars[loader->earlyServicesJarCount++] = earlyServiceJars[i];
    }
    free(earlyServiceJars);

    // Java: if (!earlyServicesJars.isEmpty()) { appendLoader("FML Early Services", earlyServicesJars.stream().map(IModFile::getContents).toList()); }
    if (loader->earlyServicesJarCount > 0)
    {
        void **contents = malloc(sizeof(void *) * loader->earlyServicesJarCount);
        for (size_t i = 0; i < loader->earlyServicesJarCount; i++)
            contents[i] = LIBMATTI_FML_ModFile_GetContents(
                (LIBMATTI_FML_ModFile *) loader->earlyServicesJars[i]);
        LIBMATTI_FML_FMLLoader_AppendLoader(loader, "FML Early Services", contents,
                                            loader->earlyServicesJarCount);
        free(contents);
    }
}

// Java: private void loadPlugins(List<IModFileInfo> plugins)
void LIBMATTI_FML_FMLLoader_LoadPlugins(LIBMATTI_FML_FMLLoader *loader, void **plugins, size_t pluginCount)
{
    // Java: appendLoader("FML Plugins", plugins.stream().map(mfi -> mfi.getFile().getContents()).toList());
    void **contents = malloc(sizeof(void *) * (pluginCount > 0 ? pluginCount : 1));
    for (size_t i = 0; i < pluginCount; i++)
        contents[i] = LIBMATTI_NEOFORGESPI_IModFile_GetContents(LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(
            (LIBMATTI_NEOFORGESPI_IModFileInfo *) plugins[i]));

    LIBMATTI_FML_FMLLoader_AppendLoader(loader, "FML Plugins", contents, pluginCount);
    free(contents);
}// Java: ownedResources.add(loader) - the close() adapter of a URLClassLoader
static void close_url_class_loader(void *self)
{
    LIBMATTI_JL_URLClassLoader_Close(self);
    LIBMATTI_JL_ClassLoader_Free(self);
}

// Java: private void appendLoader(String loaderName, List<JarContents> jars)
void LIBMATTI_FML_FMLLoader_AppendLoader(LIBMATTI_FML_FMLLoader *loader, const char *loaderName,
                                         void **jars, size_t jarCount)
{
    if (jarCount == 0)
    {
        // Java: LOGGER.info("No additional classpath items for {} were found.", loaderName);
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "No additional classpath items for {} were found.", loaderName);
        return;
    }

    // Java: LOGGER.info("Loading {}:", loaderName);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Loading {}:", loaderName);

    LIBMATTI_JN_URL **rootUrls = NULL;
    size_t rootUrlCount = 0;

    for (size_t i = 0; i < jarCount; i++)
    {
        // Java: if (jar instanceof CompositeJarContents compositeJarContents && compositeJarContents.isFiltered()) throw new IllegalArgumentException("Cannot use simple URLClassLoader for filtered content " + jar);
        LIBMATTI_FML_JarContents *jar = jars[i];
        if (jar->kind == LIBMATTI_FML_JARCONTENTS_KIND_COMPOSITE &&
            LIBMATTI_FML_CompositeJarContents_IsFiltered(jar))
        {
            // Java: throw new IllegalArgumentException(...)
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "Cannot use simple URLClassLoader for filtered content {}", loaderName);
            free(rootUrls);
            return;
        }

        size_t rootCount = 0;
        const char **contentRoots = LIBMATTI_FML_JarContents_GetContentRoots(jar, &rootCount);
        for (size_t j = 0; j < rootCount; j++)
        {
            // Java: LOGGER.info(" - {}", PathPrettyPrinting.prettyPrint(contentRoot));
            char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(contentRoots[j]);
            LIBMATTI_ML_Logger_Info(LOGGER(), NULL, " - {}", pretty);
            free(pretty);

            // Java: rootUrls.add(contentRoot.toUri().toURL())
            size_t length = strlen(contentRoots[j]) + 6;
            char *spec = malloc(length);
            snprintf(spec, length, "file:%s", contentRoots[j]);
            rootUrls = realloc(rootUrls, sizeof(*rootUrls) * (rootUrlCount + 1));
            rootUrls[rootUrlCount++] = LIBMATTI_JN_URL_New(spec);
            free(spec);

            // Java: locatedPaths.add(contentRoot)
            located_add(loader, contentRoots[j]);
        }
        free((void *) contentRoots);
    }

    // Java: var loader = new URLClassLoader(loaderName, rootUrls.toArray(URL[]::new), currentClassLoader);
    LIBMATTI_JL_ClassLoader *classLoader =
        LIBMATTI_JL_URLClassLoader_New(loaderName, rootUrls, rootUrlCount, loader->currentClassLoader);

    for (size_t i = 0; i < rootUrlCount; i++)
        LIBMATTI_JN_URL_Free(rootUrls[i]);
    free(rootUrls);

    // Java: ownedResources.add(loader);
    LIBMATTI_JL_AutoCloseable closeable = {classLoader, close_url_class_loader};
    add_closeable(&loader->ownedResources, &loader->ownedResourceCount, closeable);

    // Java: currentClassLoader = loader; Thread.currentThread().setContextClassLoader(loader);
    loader->currentClassLoader = classLoader;
    LIBMATTI_JL_Thread_SetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread(), classLoader);
}

// Java: private DiscoveryResult runDiscovery()
LIBMATTI_FML_FMLLoader_DiscoveryResult *LIBMATTI_FML_FMLLoader_RunDiscovery(LIBMATTI_FML_FMLLoader *loader)
{
    // Java: var progress = StartupNotificationManager.prependProgressBar("Discovering mods...", 0);
    LIBMATTI_FML_ProgressMeter *progress =
        LIBMATTI_FML_StartupNotificationManager_PrependProgressBar("Discovering mods...", 0);

    // Java: var additionalLocators = new ArrayList<IModFileCandidateLocator>();
    //       additionalLocators.add(new GameLocator()); ... InDevFolderLocator, InDevJarLocator, ModsFolderLocator
    LIBMATTI_FML_GameLocator *gameLocator = LIBMATTI_FML_GameLocator_New();
    LIBMATTI_FML_InDevFolderLocator *inDevFolderLocator = LIBMATTI_FML_InDevFolderLocator_New();
    LIBMATTI_FML_InDevJarLocator *inDevJarLocator = LIBMATTI_FML_InDevJarLocator_New();
    LIBMATTI_FML_ModsFolderLocator *modsFolderLocator = LIBMATTI_FML_ModsFolderLocator_New();
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *additionalLocators[] = {
        (LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *) gameLocator,
        (LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *) inDevFolderLocator,
        (LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *) inDevJarLocator,
        (LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *) modsFolderLocator,
    };

    // Java: the readers come from "META-INF/services/net.neoforged.neoforgespi.locating.IModFileReader"
    //       in the FML artifact. C has no service files, so the port registers its built-in readers here
    //       (java/util/ServiceLoader.h), like the host registers the launch handlers.
    LIBMATTI_JU_ServiceLoader_Register(
        "net.neoforged.neoforgespi.locating.IModFileReader", LIBMATTI_FML_JarModsDotTomlModFileReader_AsReader(),
        "net.neoforged.fml.loading.moddiscovery.readers.JarModsDotTomlModFileReader", NULL);
    LIBMATTI_JU_ServiceLoader_Register(
        "net.neoforged.neoforgespi.locating.IModFileReader", LIBMATTI_FML_NestedLibraryModReader_AsReader(),
        "net.neoforged.fml.loading.moddiscovery.readers.NestedLibraryModReader", NULL);

    // Java: var modDiscoverer = new ModDiscoverer(new LaunchContextAdapter(), additionalLocators);
    //       var discoveryResult = modDiscoverer.discoverMods(earlyServicesJars);
    LIBMATTI_FML_FMLLoader_LaunchContextAdapter *discoveryContext =
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter_New(loader);
    LIBMATTI_FML_ModDiscoverer *modDiscoverer = LIBMATTI_FML_ModDiscoverer_NewWithLocators(
        LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(discoveryContext), additionalLocators,
        sizeof(additionalLocators) / sizeof(additionalLocators[0]));
    LIBMATTI_FML_ModDiscoverer_Result *discoveryResult = LIBMATTI_FML_ModDiscoverer_DiscoverMods(
        modDiscoverer, (LIBMATTI_FML_ModFile **) loader->earlyServicesJars, loader->earlyServicesJarCount);
    LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(discoveryContext);
    LIBMATTI_FML_ModDiscoverer_Free(modDiscoverer);
    free(gameLocator);
    free(inDevFolderLocator);
    free(inDevJarLocator);
    free(modsFolderLocator);

    // Java: var neoForgeVersion = versionInfo.neoForgeVersion(); var minecraftVersion = versionInfo.mcVersion();
    char *neoForgeVersion = loader->versionInfo->neoForgeVersion != NULL
                                ? strdup(loader->versionInfo->neoForgeVersion)
                                : NULL;
    char *minecraftVersion =
        loader->versionInfo->mcVersion != NULL ? strdup(loader->versionInfo->mcVersion) : NULL;

    // Java: for (var modFile : discoveryResult.modFiles()) { ... switch (modFile.getId()) { case "minecraft" / "neoforge" } }
    for (size_t i = 0; i < discoveryResult->modFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = discoveryResult->modFiles[i];
        size_t modCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **mods =
            LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(LIBMATTI_FML_ModFile_GetModFileInfo(modFile), &modCount);
        if (modCount == 0) continue;

        const char *version = LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(mods[0])->version;
        if (strcmp(modFile->id, "minecraft") == 0)
        {
            free(minecraftVersion);
            minecraftVersion = strdup(version);
        }
        else if (strcmp(modFile->id, "neoforge") == 0)
        {
            free(neoForgeVersion);
            neoForgeVersion = strdup(version);
        }
    }

    // Java: versionInfo = new VersionInfo(neoForgeVersion, minecraftVersion, getVersionInfo().neoFormVersion());
    char *neoFormVersion = loader->versionInfo->neoFormVersion != NULL
                               ? strdup(loader->versionInfo->neoFormVersion)
                               : NULL;
    LIBMATTI_FML_VersionInfo_Free(loader->versionInfo);
    loader->versionInfo = LIBMATTI_FML_VersionInfo_New(neoForgeVersion, minecraftVersion, neoFormVersion);
    free(neoForgeVersion);
    free(minecraftVersion);
    free(neoFormVersion);

    // Java: versionSupportMatrix = new VersionSupportMatrix(versionInfo);
    loader->versionSupportMatrix = LIBMATTI_FML_VersionSupportMatrix_New(loader->versionInfo);

    // Java: progress.complete();
    LIBMATTI_FML_ProgressMeter_Complete(progress);
    LIBMATTI_FML_ProgressMeter_Free(progress);

    // Java: ImmediateWindowHandler.setMinecraftVersion(versionInfo.mcVersion()); ...setNeoForgeVersion(...);
    LIBMATTI_FML_ImmediateWindowHandler_SetMinecraftVersion(loader->versionInfo->mcVersion);
    LIBMATTI_FML_ImmediateWindowHandler_SetNeoForgeVersion(loader->versionInfo->neoForgeVersion);

    // Java: loadingModList = ModSorter.sort(discoveryResult.modFiles(), discoveryResult.discoveryIssues());
    LIBMATTI_FML_LoadingModList *loadingModList = LIBMATTI_FML_ModSorter_Sort(
        discoveryResult->modFiles, discoveryResult->modFileCount, discoveryResult->discoveryIssues,
        discoveryResult->discoveryIssueCount);
    loader->loadingModList = loadingModList;

    // Java: Map<IModInfo, JarResource> enumExtensionsByMod = new HashMap<>();
    LIBMATTI_NEOFORGESPI_IModInfo **enumExtensionMods = NULL;
    LIBMATTI_FML_JarResource **enumExtensionResources = NULL;
    size_t enumExtensionCount = 0;

    // Java: for (var modFile : loadingModList.getAllModFiles())
    size_t extensionModFileCount = 0;
    LIBMATTI_NEOFORGESPI_IModFile **extensionModFiles = LIBMATTI_FML_LoadingModList_GetAllModFiles(
        loadingModList, &extensionModFileCount);
    for (size_t i = 0; i < extensionModFileCount; i++)
    {
        // Java: var mods = modFile.getModInfos();
        size_t extensionModCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **extensionMods = LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(
            extensionModFiles[i], &extensionModCount);

        for (size_t j = 0; j < extensionModCount; j++)
        {
            LIBMATTI_NEOFORGESPI_IModInfo *mod = extensionMods[j];

            // Java: mod.getConfig().<String>getConfigElement("enumExtensions").ifPresent(file -> { ... })
            LIBMATTI_NEOFORGESPI_IConfigurable *config = LIBMATTI_NEOFORGESPI_IModInfo_GetConfig(mod);
            LIBMATTI_NEOFORGESPI_IConfigurable_Value value;
            if (!LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(config, "enumExtensions", &value)) continue;
            if (value.type != LIBMATTI_NEOFORGESPI_IConfigurable_STRING) continue;
            const char *file = value.string;

            // Java: var resource = mod.getOwningFile().getFile().getContents().get(file);
            LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile = LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(mod);
            LIBMATTI_NEOFORGESPI_IModFile *modFile = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile);
            LIBMATTI_FML_JarResource *resource = LIBMATTI_FML_JarContents_Get(
                LIBMATTI_NEOFORGESPI_IModFile_GetContents(modFile), file);

            // Java: if (resource == null) { ModLoader.addLoadingIssue(ModLoadingIssue.error(
            //         "fml.modloadingissue.enumextender.file_not_found", file).withAffectedMod(mod)); return; }
            if (resource == NULL)
            {
                const char *args[] = {file};
                LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                    "fml.modloadingissue.enumextender.file_not_found", args, 1);
                LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(
                    issue, mod, modFile, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(modFile)));
                // Java's record withers are garbage collected; the port frees the issue it replaced.
                LIBMATTI_FML_ModLoadingIssue_Free(issue);
                continue;
            }

            // Java: enumExtensionsByMod.put(mod, resource);
            enumExtensionMods = realloc(enumExtensionMods, sizeof(*enumExtensionMods) * (enumExtensionCount + 1));
            enumExtensionResources = realloc(enumExtensionResources,
                                             sizeof(*enumExtensionResources) * (enumExtensionCount + 1));
            enumExtensionMods[enumExtensionCount] = mod;
            enumExtensionResources[enumExtensionCount] = resource;
            enumExtensionCount++;
        }
    }

    // Java: RuntimeEnumExtender.loadEnumPrototypes(enumExtensionsByMod)
    LIBMATTI_FML_RuntimeEnumExtender_LoadEnumPrototypes((const LIBMATTI_NEOFORGESPI_IModInfo **) enumExtensionMods,
                                                        (const LIBMATTI_FML_JarResource **) enumExtensionResources,
                                                        enumExtensionCount);
    free(enumExtensionMods);
    free(enumExtensionResources);

    // Java: backgroundScanHandler = new BackgroundScanHandler(loadingModList.getAllModFiles());
    size_t allModFileCount = 0;
    LIBMATTI_NEOFORGESPI_IModFile **allModFiles =
        LIBMATTI_FML_LoadingModList_GetAllModFiles(loadingModList, &allModFileCount);
    loader->backgroundScanHandler = LIBMATTI_FML_BackgroundScanHandler_New(allModFiles, allModFileCount);

    // Java: return this.discoveryResult = new DiscoveryResult(
    //         loadingModList.getPlugins().stream().map(mfi -> (ModFile) mfi.getFile()).toList(),
    //         loadingModList.getModFiles().stream().map(ModFileInfo::getFile).toList(),
    //         loadingModList.getGameLibraries().stream().map(mf -> (ModFile) mf).toList(),
    //         loadingModList.getModLoadingIssues());
    LIBMATTI_FML_FMLLoader_DiscoveryResult *result = calloc(1, sizeof(LIBMATTI_FML_FMLLoader_DiscoveryResult));

    size_t pluginCount = 0;
    LIBMATTI_NEOFORGESPI_IModFileInfo **plugins =
        LIBMATTI_FML_LoadingModList_GetPlugins(loadingModList, &pluginCount);
    result->pluginContent = calloc(pluginCount > 0 ? pluginCount : 1, sizeof(void *));
    for (size_t i = 0; i < pluginCount; i++)
        result->pluginContent[result->pluginContentCount++] =
            LIBMATTI_FML_ModFile_AsConcrete(LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(plugins[i]));

    size_t gameCount = 0;
    LIBMATTI_FML_ModFileInfo **modFiles =
        LIBMATTI_FML_LoadingModList_GetModFiles(loadingModList, &gameCount);
    result->gameContent = calloc(gameCount > 0 ? gameCount : 1, sizeof(void *));
    for (size_t i = 0; i < gameCount; i++)
        result->gameContent[result->gameContentCount++] = LIBMATTI_FML_ModFileInfo_GetFile(modFiles[i]);

    size_t gameLibraryCount = 0;
    LIBMATTI_NEOFORGESPI_IModFile **gameLibraries =
        LIBMATTI_FML_LoadingModList_GetGameLibraries(loadingModList, &gameLibraryCount);
    result->gameLibraryContent = calloc(gameLibraryCount > 0 ? gameLibraryCount : 1, sizeof(void *));
    for (size_t i = 0; i < gameLibraryCount; i++)
        result->gameLibraryContent[result->gameLibraryContentCount++] =
            LIBMATTI_FML_ModFile_AsConcrete(gameLibraries[i]);

    result->discoveryIssues = LIBMATTI_FML_LoadingModList_GetModLoadingIssues(loadingModList,
                                                                             &result->discoveryIssueCount);

    LIBMATTI_FML_ModDiscoverer_Result_Free(discoveryResult);
    return loader->discoveryResult = result;
}

// Java: private static <T> T runOffThread(Supplier<T> supplier)
// Java wraps the supplier in a CompletableFuture and ticks ImmediateWindowHandler while
// the worker thread runs. The port has no thread pool yet, so the discovery runs on the
// current thread and the progress window is ticked around it.
LIBMATTI_FML_FMLLoader_DiscoveryResult *LIBMATTI_FML_FMLLoader_RunOffThread_DiscoveryResult(
    LIBMATTI_FML_FMLLoader *loader)
{
    // Java: ImmediateWindowHandler.updateProgress("Discovering mods");
    LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress("Discovering mods");
    LIBMATTI_FML_FMLLoader_DiscoveryResult *result = LIBMATTI_FML_FMLLoader_RunDiscovery(loader);
    LIBMATTI_FML_ImmediateWindowHandler_RenderTick();
    return result;
}

// Java: public static LanguageProviderLoader getLanguageLoadingProvider()
void *LIBMATTI_FML_FMLLoader_GetLanguageLoadingProvider(void)
{
    // Java: return getCurrent().languageProviderLoader;
    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrent();
    return loader != NULL ? loader->languageProviderLoader : NULL;
}

// Java: public static FMLLoader getCurrent()
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_GetCurrent(void)
{
    // Java: var current = getCurrentOrNull(); if (current == null) throw new IllegalStateException("There is no current FML Loader");
    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrentOrNull();
    if (loader == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "There is no current FML Loader");
    }

    return loader;
}

// Java: @Nullable public static FMLLoader getCurrentOrNull()
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_GetCurrentOrNull(void)
{
    // Java: return current.get();
    return s_current;
}

// Java: public Dist getDist()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_GetDist(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return dist;
    return loader->dist;
}

// Java: public LoadingModList getLoadingModList()
void *LIBMATTI_FML_FMLLoader_GetLoadingModList(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: if (loadingModList == null) throw new IllegalStateException("The loading mod list isn't built yet.");
    if (loader->loadingModList == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "The loading mod list isn't built yet.");
    }

    return loader->loadingModList;
}

// Java: public Path getGameDir()
const char *LIBMATTI_FML_FMLLoader_GetGameDir(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return gameDir;
    return loader->gameDir;
}

// Java: public boolean isProduction()
int LIBMATTI_FML_FMLLoader_IsProduction(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return production;
    return loader->production;
}

// Java: public ModuleLayer getGameLayer()
LIBMATTI_JL_ModuleLayer *LIBMATTI_FML_FMLLoader_GetGameLayer(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: if (gameLayer == null) throw new IllegalStateException("This can only be called after mod discovery is completed");
    if (loader->gameLayer == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "This can only be called after mod discovery is completed");
    }

    return loader->gameLayer;
}

// Java: public VersionInfo getVersionInfo()
LIBMATTI_FML_VersionInfo *LIBMATTI_FML_FMLLoader_GetVersionInfo(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: return versionInfo;
    return loader->versionInfo;
}

// Java: VersionSupportMatrix getVersionSupportMatrix()
void *LIBMATTI_FML_FMLLoader_GetVersionSupportMatrix(const LIBMATTI_FML_FMLLoader *loader)
{
    // Java: if (versionSupportMatrix == null) throw new IllegalStateException("Mod discovery has not completed yet, versions may not be known.");
    if (loader->versionSupportMatrix == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Mod discovery has not completed yet, versions may not be known.");
    }

    return loader->versionSupportMatrix;
}

// Java: private class LaunchContextAdapter implements ILaunchContext - the ILaunchContext vtable
static LIBMATTI_DIST_Dist adapter_get_required_distribution(void *self)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetRequiredDistribution(self);
}

static const char *adapter_game_directory(void *self)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GameDirectory(self);
}

static LIBMATTI_JU_ServiceLoader *adapter_load_services(void *self, const char *serviceClass)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_LoadServices(self, serviceClass);
}

static int adapter_is_located(void *self, const char *path)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_IsLocated(self, path);
}

static int adapter_add_located(void *self, const char *path)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AddLocated(self, path);
}

static LIBMATTI_FML_VersionInfo *adapter_get_versions(void *self)
{
    return LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetVersions(self);
}

// Java: private class LaunchContextAdapter implements ILaunchContext
LIBMATTI_FML_FMLLoader_LaunchContextAdapter *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_New(
    LIBMATTI_FML_FMLLoader *loader)
{
    LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter = calloc(1, sizeof(*adapter));
    adapter->loader = loader;

    adapter->context.self = adapter;
    adapter->context.getRequiredDistribution = adapter_get_required_distribution;
    adapter->context.gameDirectory = adapter_game_directory;
    adapter->context.loadServices = adapter_load_services;
    adapter->context.isLocated = adapter_is_located;
    adapter->context.addLocated = adapter_add_located;
    adapter->context.getVersions = adapter_get_versions;
    return adapter;
}

// Java: the adapter is handed around as ILaunchContext
LIBMATTI_NEOFORGESPI_ILaunchContext *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(
    LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter)
{
    return &adapter->context;
}

void LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter)
{
    free(adapter);
}

// Java: public Dist getRequiredDistribution()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetRequiredDistribution(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter)
{
    // Java: return dist;
    return adapter->loader->dist;
}

// Java: public Path gameDirectory()
const char *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GameDirectory(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter)
{
    // Java: return gameDir;
    return adapter->loader->gameDir;
}

// Java: public <T> Stream<ServiceLoader.Provider<T>> loadServices(Class<T> serviceClass)
// Java relies on the thread context classloader being correct: ServiceLoader.load(serviceClass).stream()
LIBMATTI_JU_ServiceLoader *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_LoadServices(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter, const char *serviceClass)
{
    (void) adapter;
    return LIBMATTI_JU_ServiceLoader_LoadFrom(serviceClass);
}

// Java: public boolean isLocated(Path path)
int LIBMATTI_FML_FMLLoader_LaunchContextAdapter_IsLocated(const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter,
                                                          const char *path)
{
    // Java: return FMLLoader.this.locatedPaths.contains(path);
    return located_contains(adapter->loader, path);
}

// Java: public boolean addLocated(Path path)
int LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AddLocated(const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter,
                                                           const char *path)
{
    // Java: return FMLLoader.this.locatedPaths.add(path);
    return located_add((LIBMATTI_FML_FMLLoader *) adapter->loader, path);
}

// Java: public VersionInfo getVersions()
LIBMATTI_FML_VersionInfo *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetVersions(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter)
{
    // Java: return versionInfo;
    return adapter->loader->versionInfo;
}
