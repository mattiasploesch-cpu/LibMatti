// Port of net.neoforged.fml.loading.FMLLoader.

#ifndef MATTICRAFT_FMLLOADER_H
#define MATTICRAFT_FMLLOADER_H

#include "libmatti/java/lang/AutoCloseable.h"
#include "libmatti/java/lang/Class.h"
#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/java/lang/ModuleLayer.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/fml/IBindingsProvider.h"
#include "libmatti/net/neoforged/fml/classloading/JarContentsModule.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditLog.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorSet.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/TransformingClassLoader.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"
#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"
#include "libmatti/net/neoforged/fml/loading/mixin/MixinFacade.h"
#include "libmatti/net/neoforged/fml/loading/VersionInfo.h"
#include "libmatti/net/neoforged/fml/startup/StartupArgs.h"
#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"

#include <stddef.h>

// Java: public final class FMLLoader implements AutoCloseable
typedef struct LIBMATTI_FML_FMLLoader LIBMATTI_FML_FMLLoader;

// Java: @VisibleForTesting record DiscoveryResult(List<ModFile> pluginContent, List<ModFile> gameContent,
//         List<ModFile> gameLibraryContent, List<ModLoadingIssue> discoveryIssues)
typedef struct
{
    // Java: List<ModFile> pluginContent (the ported moddiscovery.ModFile, kept as concrete pointers)
    void **pluginContent;
    size_t pluginContentCount;
    // Java: List<ModFile> gameContent
    void **gameContent;
    size_t gameContentCount;
    // Java: List<ModFile> gameLibraryContent
    void **gameLibraryContent;
    size_t gameLibraryContentCount;
    LIBMATTI_FML_ModLoadingIssue **discoveryIssues;
    size_t discoveryIssueCount;
} LIBMATTI_FML_FMLLoader_DiscoveryResult;

// Java: private class LaunchContextAdapter implements ILaunchContext
typedef struct LIBMATTI_FML_FMLLoader_LaunchContextAdapter
{
    // Java: the implicit outer FMLLoader instance
    LIBMATTI_FML_FMLLoader *loader;
    // Java: the adapter is an ILaunchContext; the port keeps the vtable on it
    LIBMATTI_NEOFORGESPI_ILaunchContext context;
} LIBMATTI_FML_FMLLoader_LaunchContextAdapter;

// Java: public final class FMLLoader
struct LIBMATTI_FML_FMLLoader
{
    LIBMATTI_JL_ClassLoader *originalClassLoader;
    LIBMATTI_JL_ClassLoader *currentClassLoader;
    // Java: private final List<AutoCloseable> ownedResources
    LIBMATTI_JL_AutoCloseable *ownedResources;
    size_t ownedResourceCount;
    // Java: private final List<AutoCloseable> closeCallbacks
    LIBMATTI_JL_AutoCloseable *closeCallbacks;
    size_t closeCallbackCount;
    LIBMATTI_FML_ProgramArgs *programArgs;
    // Java: private final LanguageProviderLoader languageProviderLoader (ported)
    void *languageProviderLoader;
    LIBMATTI_DIST_Dist dist;
    // Java: private final LoadingModList loadingModList (ported)
    void *loadingModList;
    const char *gameDir;
    // Java: private final Set<Path> locatedPaths
    char **locatedPaths;
    size_t locatedPathCount;
    LIBMATTI_FML_VersionInfo *versionInfo;
    // Java: private final VersionSupportMatrix versionSupportMatrix (ported)
    void *versionSupportMatrix;
    // Java: private final BackgroundScanHandler backgroundScanHandler (ported)
    void *backgroundScanHandler;
    int production;
    LIBMATTI_JL_ModuleLayer *gameLayer;
    // Java: private final List<ModFile> earlyServicesJars
    void **earlyServicesJars;
    size_t earlyServicesJarCount;
    LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult;
    LIBMATTI_FML_ClassProcessorAuditLog *classTransformerAuditLog;
    // Java: private IBindingsProvider bindings
    LIBMATTI_FML_IBindingsProvider *bindings;
};

// Java: static FMLLoader create(StartupArgs startupArgs)
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_Create(LIBMATTI_FML_StartupArgs *startupArgs);

// Java: static FMLLoader create(@Nullable Instrumentation instrumentation, StartupArgs startupArgs)
// 'instrumentation' carries the ported java.lang.instrument.Instrumentation
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_CreateWithInstrumentation(void *instrumentation,
                                                                        LIBMATTI_FML_StartupArgs *startupArgs);

// Java: create(...) throws; the C port reads the thrown exception back here (NULL = no exception)
LIBMATTI_JL_Throwable *LIBMATTI_FML_FMLLoader_GetLastException(void);
// Java: the throw inside create(...) - the port stores what would have been thrown
void LIBMATTI_FML_FMLLoader_SetLastException(LIBMATTI_JL_Throwable *exception);

// Java: public List<ModFile> allContent()
void **LIBMATTI_FML_FMLLoader_DiscoveryResult_AllContent(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result,
                                                         size_t *count);
// Java: public List<ModFile> allGameContent()
void **LIBMATTI_FML_FMLLoader_DiscoveryResult_AllGameContent(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result,
                                                             size_t *count);
// Java: public boolean hasErrors()
int LIBMATTI_FML_FMLLoader_DiscoveryResult_HasErrors(const LIBMATTI_FML_FMLLoader_DiscoveryResult *result);

// Java: @ApiStatus.Internal public ClassProcessorAuditSource getClassTransformerAuditLog()
LIBMATTI_FML_ClassProcessorAuditSource *LIBMATTI_FML_FMLLoader_GetClassTransformerAuditLog(
    const LIBMATTI_FML_FMLLoader *loader);

// Java: public IModFile getModFileByClass(Class<?> clazz)
void *LIBMATTI_FML_FMLLoader_GetModFileByClass(const LIBMATTI_FML_FMLLoader *loader, const LIBMATTI_JL_Class *clazz);

// Java: public void addCloseCallback(AutoCloseable callback)
void LIBMATTI_FML_FMLLoader_AddCloseCallback(LIBMATTI_FML_FMLLoader *loader, LIBMATTI_JL_AutoCloseable callback);

// Java: public void close()
void LIBMATTI_FML_FMLLoader_Close(LIBMATTI_FML_FMLLoader *loader);

// Java has no destructor; the C port releases what the constructor allocated.
void LIBMATTI_FML_FMLLoader_Free(LIBMATTI_FML_FMLLoader *loader);

// Java: public ClassLoader getCurrentClassLoader()
LIBMATTI_JL_ClassLoader *LIBMATTI_FML_FMLLoader_GetCurrentClassLoader(const LIBMATTI_FML_FMLLoader *loader);

// Java: public ProgramArgs getProgramArgs()
LIBMATTI_FML_ProgramArgs *LIBMATTI_FML_FMLLoader_GetProgramArgs(const LIBMATTI_FML_FMLLoader *loader);

// Java: @ApiStatus.Internal public IBindingsProvider getBindings()
LIBMATTI_FML_IBindingsProvider *LIBMATTI_FML_FMLLoader_GetBindings(LIBMATTI_FML_FMLLoader *loader);

// Java: public static LanguageProviderLoader getLanguageLoadingProvider()
void *LIBMATTI_FML_FMLLoader_GetLanguageLoadingProvider(void);

// Java: public static FMLLoader getCurrent()
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_GetCurrent(void);

// Java: public static @Nullable FMLLoader getCurrentOrNull()
LIBMATTI_FML_FMLLoader *LIBMATTI_FML_FMLLoader_GetCurrentOrNull(void);

// Java: public Dist getDist()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_GetDist(const LIBMATTI_FML_FMLLoader *loader);

// Java: public LoadingModList getLoadingModList()
void *LIBMATTI_FML_FMLLoader_GetLoadingModList(const LIBMATTI_FML_FMLLoader *loader);

// Java: public Path getGameDir()
const char *LIBMATTI_FML_FMLLoader_GetGameDir(const LIBMATTI_FML_FMLLoader *loader);

// Java: public boolean isProduction()
int LIBMATTI_FML_FMLLoader_IsProduction(const LIBMATTI_FML_FMLLoader *loader);

// Java: public ModuleLayer getGameLayer()
LIBMATTI_JL_ModuleLayer *LIBMATTI_FML_FMLLoader_GetGameLayer(const LIBMATTI_FML_FMLLoader *loader);

// Java: public VersionInfo getVersionInfo()
LIBMATTI_FML_VersionInfo *LIBMATTI_FML_FMLLoader_GetVersionInfo(const LIBMATTI_FML_FMLLoader *loader);

// Java: VersionSupportMatrix getVersionSupportMatrix()
void *LIBMATTI_FML_FMLLoader_GetVersionSupportMatrix(const LIBMATTI_FML_FMLLoader *loader);

// Java: private static Dist detectDist(ClassLoader classLoader)
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_DetectDist(LIBMATTI_JL_ClassLoader *classLoader);

// Java: private static boolean detectProduction(ClassLoader classLoader)
int LIBMATTI_FML_FMLLoader_DetectProduction(LIBMATTI_JL_ClassLoader *classLoader);

// Java: private DiscoveryResult runDiscovery()
LIBMATTI_FML_FMLLoader_DiscoveryResult *LIBMATTI_FML_FMLLoader_RunDiscovery(LIBMATTI_FML_FMLLoader *loader);

// Java: private void loadEarlyServices(StartupArgs startupArgs)
void LIBMATTI_FML_FMLLoader_LoadEarlyServices(LIBMATTI_FML_FMLLoader *loader, LIBMATTI_FML_StartupArgs *startupArgs);

// Java: private void loadPlugins(List<IModFileInfo> plugins)
void LIBMATTI_FML_FMLLoader_LoadPlugins(LIBMATTI_FML_FMLLoader *loader, void **plugins, size_t pluginCount);

// Java: private void appendLoader(String loaderName, List<JarContents> jars)
void LIBMATTI_FML_FMLLoader_AppendLoader(LIBMATTI_FML_FMLLoader *loader, const char *loaderName,
                                         void **jars, size_t jarCount);

// Java: private static <T> T runOffThread(Supplier<T> supplier)
LIBMATTI_FML_FMLLoader_DiscoveryResult *LIBMATTI_FML_FMLLoader_RunOffThread_DiscoveryResult(LIBMATTI_FML_FMLLoader *loader);

// Java: private TransformingClassLoader buildTransformingLoader(...)
LIBMATTI_FML_TransformingClassLoader *LIBMATTI_FML_FMLLoader_BuildTransformingLoader(
    LIBMATTI_FML_FMLLoader *loader, LIBMATTI_FML_ClassProcessorSet *classProcessorSet,
    LIBMATTI_FML_ClassProcessorAuditLog *auditTrail, LIBMATTI_FML_JarContentsModule **content,
    size_t contentCount);

// Java: private void maskContentAlreadyOnClasspath(List<JarContentsModule> content)
void LIBMATTI_FML_FMLLoader_MaskContentAlreadyOnClasspath(LIBMATTI_FML_FMLLoader *loader,
                                                          LIBMATTI_FML_JarContentsModule **content,
                                                          size_t contentCount);

// Java: private static List<Path> getBasePaths(JarContents contents, boolean ignoreFilter)
char **LIBMATTI_FML_FMLLoader_GetBasePaths(LIBMATTI_FML_JarContents *contents, int ignoreFilter, size_t *count);

// Java: private static String getModuleNameList(Configuration cf, List<JarContentsModule> content)
char *LIBMATTI_FML_FMLLoader_GetModuleNameList(LIBMATTI_JL_Configuration *configuration,
                                               LIBMATTI_FML_JarContentsModule **content, size_t contentCount);

// Java: private static ClassProcessorSet createClassProcessorSet(...)
LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_FMLLoader_CreateClassProcessorSet(
    LIBMATTI_FML_StartupArgs *startupArgs, LIBMATTI_FML_FMLLoader_LaunchContextAdapter *launchContext,
    LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult, LIBMATTI_FML_MixinFacade *mixinFacade);

// Java: private static ClassProcessor createAccessTransformerService(DiscoveryResult discoveryResult)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLLoader_CreateAccessTransformerService(
    LIBMATTI_FML_FMLLoader_DiscoveryResult *discoveryResult);

// Java: private class LaunchContextAdapter implements ILaunchContext
LIBMATTI_FML_FMLLoader_LaunchContextAdapter *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_New(LIBMATTI_FML_FMLLoader *loader);

// Java: the adapter is handed around as ILaunchContext
LIBMATTI_NEOFORGESPI_ILaunchContext *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AsLaunchContext(
    LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter);

// Java: public Dist getRequiredDistribution()
LIBMATTI_DIST_Dist LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetRequiredDistribution(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter);

// Java: public Path gameDirectory()
const char *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GameDirectory(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter);

// Java: public <T> Stream<ServiceLoader.Provider<T>> loadServices(Class<T> serviceClass)
LIBMATTI_JU_ServiceLoader *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_LoadServices(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter, const char *serviceClass);

// Java: public boolean isLocated(Path path)
int LIBMATTI_FML_FMLLoader_LaunchContextAdapter_IsLocated(const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter,
                                                          const char *path);

// Java: public boolean addLocated(Path path)
int LIBMATTI_FML_FMLLoader_LaunchContextAdapter_AddLocated(const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter,
                                                           const char *path);

// Java: public VersionInfo getVersions()
LIBMATTI_FML_VersionInfo *LIBMATTI_FML_FMLLoader_LaunchContextAdapter_GetVersions(
    const LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter);

void LIBMATTI_FML_FMLLoader_LaunchContextAdapter_Free(LIBMATTI_FML_FMLLoader_LaunchContextAdapter *adapter);

#endif //MATTICRAFT_FMLLOADER_H
