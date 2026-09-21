//
// Port of cpw.mods.modlauncher.Launcher.
//
// Java reads JVM system properties (System.getProperty) and the package
// metadata of the modlauncher jar; following the convention already used by the
// BootstrapLauncher port, system properties are read from the environment.
//

#include "Launcher.h"

#include "DefaultLaunchHandlerService.h"
#include "LogManager.h"
#include "LogMarkers.h"
#include "TestingLaunchHandlerService.h"
#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_ML_Launcher *LIBMATTI_ML_Launcher_INSTANCE = NULL;

// ---------------------------------------------------------------------------
// Java: System.getProperty(...)
// ---------------------------------------------------------------------------

static const char *system_property(const char *name)
{
    return getenv(name);
}

// Java: the package metadata of the modlauncher artifact (its jar manifest).
// The C artifact is the running executable, whose manifest is the ELF
// ".matti_manifest" section, read through the jar contents reader.
static char *artifact_manifest_value(const char *key)
{
#ifdef __linux__
    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContents_Of("/proc/self/exe");
    if (contents == NULL) return NULL;

    LIBMATTI_JU_Manifest *manifest = LIBMATTI_JH_JarContents_GetManifest(contents);
    const char *value = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, key) : NULL;
    char *copy = value != NULL ? strdup(value) : NULL;

    LIBMATTI_JH_JarContents_Close(contents);
    return copy;
#else
    (void)key;
    return NULL;
#endif
}

// Java: IEnvironment.class.getPackage().getImplementationVersion()
static const char *implementation_version(void)
{
    // loaded once, like the package metadata of the jar
    static int loaded = 0;
    static char *version = NULL;

    if (!loaded)
    {
        loaded = 1;
        version = artifact_manifest_value("Implementation-Version");
    }

    return version;
}

// Java: IEnvironment.class.getPackage().getSpecificationVersion()
static const char *specification_version(void)
{
    static int loaded = 0;
    static char *version = NULL;

    if (!loaded)
    {
        loaded = 1;
        version = artifact_manifest_value("Specification-Version");
    }

    return version;
}

// ---------------------------------------------------------------------------
// stream helpers (Java: Collectors.groupingBy(Resource::target), mapMulti)
// ---------------------------------------------------------------------------

// Java: Map<IModuleLayerManager.Layer, List<Resource>> (an EnumMap-shaped grouping)
typedef struct
{
    LIBMATTI_MLA_ITransformationService_Resource **items;
    size_t count;
} ResourceGroup;

static void group_by_target(LIBMATTI_MLA_ITransformationService_Resource *resources, size_t resourceCount,
                            ResourceGroup groups[4])
{
    for (size_t i = 0; i < 4; i++)
    {
        groups[i].items = NULL;
        groups[i].count = 0;
    }

    for (size_t i = 0; i < resourceCount; i++)
    {
        LIBMATTI_MLA_Layer target = resources[i].target;
        if (target < LIBMATTI_MLA_LAYER_BOOT || target > LIBMATTI_MLA_LAYER_GAME) continue;

        ResourceGroup *group = &groups[target];
        group->items = realloc(group->items, sizeof(*group->items) * (group->count + 1));
        group->items[group->count++] = &resources[i];
    }
}

static void group_free(ResourceGroup groups[4])
{
    for (size_t i = 0; i < 4; i++) free(groups[i].items);
}

// Java: mapMulti((resource, action) -> resource.resources().forEach(action))
static void collect_resources(LIBMATTI_JH_SecureJar ***out, size_t *outCount, const ResourceGroup *group)
{
    for (size_t i = 0; i < group->count; i++)
    {
        LIBMATTI_MLA_ITransformationService_Resource *resource = group->items[i];

        for (size_t j = 0; j < resource->resourceCount; j++)
        {
            *out = realloc(*out, sizeof(**out) * (*outCount + 1));
            (*out)[*outCount] = resource->resources[j];
            (*outCount)++;
        }
    }
}

// Java: .forEach(np -> moduleLayerHandler.addToLayer(layer, np))
static void add_to_layer(LIBMATTI_ML_Launcher *launcher, LIBMATTI_MLA_Layer layer, const ResourceGroup *group)
{
    for (size_t i = 0; i < group->count; i++)
    {
        LIBMATTI_MLA_ITransformationService_Resource *resource = group->items[i];

        for (size_t j = 0; j < resource->resourceCount; j++)
            LIBMATTI_ML_ModuleLayerHandler_AddToLayer(launcher->moduleLayerHandler, layer, resource->resources[j]);
    }
}

// Java: List.toString() -> "[a, b, c]" (used for the parameterized log messages)
static char *format_string_list(char **items, size_t count)
{
    size_t length = 3; // "[]" + NUL
    for (size_t i = 0; i < count; i++) length += strlen(items[i]) + 2;

    char *result = malloc(length);
    if (result == NULL) return NULL;

    strcpy(result, "[");
    for (size_t i = 0; i < count; i++)
    {
        if (i > 0) strcat(result, ", ");
        strcat(result, items[i]);
    }
    strcat(result, "]");

    return result;
}

// ---------------------------------------------------------------------------
// the computePropertyIfAbsent value functions (Java lambdas)
// ---------------------------------------------------------------------------

// Java: s -> IEnvironment.class.getPackage().getSpecificationVersion()
static void *specification_version_supplier(LIBMATTI_MLA_Key *key, void *userdata)
{
    (void)key;
    (void)userdata;
    return (void *)specification_version();
}

// Java: s -> IEnvironment.class.getPackage().getImplementationVersion()
static void *implementation_version_supplier(LIBMATTI_MLA_Key *key, void *userdata)
{
    (void)key;
    (void)userdata;
    return (void *)implementation_version();
}

// Java: s -> new ArrayList<>()
static void *mod_list_supplier(LIBMATTI_MLA_Key *key, void *userdata)
{
    (void)key;
    (void)userdata;
    return LIBMATTI_MLA_ModList_New();
}

// ---------------------------------------------------------------------------
// Launcher
// ---------------------------------------------------------------------------

// Java: module-info provides cpw.mods.modlauncher.api.ILaunchHandlerService with
//       DefaultLaunchHandlerService, TestingLaunchHandlerService
// The module system exposes them whenever the launcher module is present; the C
// registry is filled once here so the Launcher constructor sees them.
static void register_provided_services(void)
{
    static int registered = 0;
    if (registered) return;
    registered = 1;

    LIBMATTI_ML_DefaultLaunchHandlerService *defaultHandler = LIBMATTI_ML_DefaultLaunchHandlerService_New();
    LIBMATTI_JU_ServiceLoader_Register("cpw.mods.modlauncher.api.ILaunchHandlerService", &defaultHandler->base,
                                       "cpw.mods.modlauncher.DefaultLaunchHandlerService", "modlauncher");

    LIBMATTI_ML_TestingLaunchHandlerService *testingHandler = LIBMATTI_ML_TestingLaunchHandlerService_New();
    LIBMATTI_JU_ServiceLoader_Register("cpw.mods.modlauncher.api.ILaunchHandlerService", &testingHandler->base,
                                       "cpw.mods.modlauncher.TestingLaunchHandlerService", "modlauncher");
}

// Java: private Launcher()
LIBMATTI_ML_Launcher *LIBMATTI_ML_Launcher_New(void)
{
    LIBMATTI_ML_Launcher *launcher = calloc(1, sizeof(LIBMATTI_ML_Launcher));
    if (launcher == NULL) return NULL;

    register_provided_services();

    LIBMATTI_ML_Launcher_INSTANCE = launcher;

    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "ModLauncher {} starting: java version {} by {}; OS {} arch {} version {}",
                            implementation_version(), system_property("java.version"), system_property("java.vendor"),
                            system_property("os.name"), system_property("os.arch"), system_property("os.version"));

    launcher->moduleLayerHandler = LIBMATTI_ML_ModuleLayerHandler_New();
    launcher->launchService = LIBMATTI_ML_LaunchServiceHandler_New(launcher->moduleLayerHandler);
    launcher->blackboard = LIBMATTI_MLA_TypesafeMap_New();
    launcher->environment = LIBMATTI_ML_Environment_New(launcher);

    // Java: environment.computePropertyIfAbsent(Keys.MLSPEC_VERSION.get(), s -> ...specificationVersion())
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(launcher->environment,
                                                    LIBMATTI_MLA_IEnvironment_Keys_MlSpecVersion(),
                                                    specification_version_supplier, NULL);
    // Java: environment.computePropertyIfAbsent(Keys.MLIMPL_VERSION.get(), s -> ...implementationVersion())
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(launcher->environment,
                                                    LIBMATTI_MLA_IEnvironment_Keys_MlImplVersion(),
                                                    implementation_version_supplier, NULL);
    // Java: environment.computePropertyIfAbsent(Keys.MODLIST.get(), s -> new ArrayList<>())
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(launcher->environment,
                                                    LIBMATTI_MLA_IEnvironment_Keys_ModList(),
                                                    mod_list_supplier, NULL);

    launcher->transformStore = LIBMATTI_ML_TransformStore_New();

    launcher->transformationServicesHandler =
        LIBMATTI_ML_TransformationServicesHandler_New(launcher->transformStore, launcher->moduleLayerHandler);
    launcher->argumentHandler = LIBMATTI_ML_ArgumentHandler_New();
    launcher->launchPlugins = LIBMATTI_ML_LaunchPluginHandler_New(launcher->moduleLayerHandler);

    return launcher;
}

// Java: public static void main(String... args)
int LIBMATTI_ML_Launcher_Main(int argc, char *argv[])
{
    // Java: var props = System.getProperties(); if (props.getProperty("java.vm.name").contains("OpenJ9"))
    const char *vmName = system_property("java.vm.name");
    if (vmName != NULL && strstr(vmName, "OpenJ9") != NULL)
    {
        fprintf(stderr,
                "\n            WARNING: OpenJ9 is detected. This is definitely unsupported and you may encounter issues and significantly worse performance.\n"
                "            For support and performance reasons, we recommend installing a temurin JVM from https://adoptium.net/\n"
                "            JVM information: %s %s %s\n            ",
                system_property("java.vm.vendor"), vmName, system_property("java.vm.version"));
    }

    // Java: () -> LaunchServiceHandler.hideAccessToken(args)
    size_t hiddenCount = 0;
    char **hidden = LIBMATTI_ML_LaunchServiceHandler_HideAccessToken(argc, argv, &hiddenCount);
    char *hiddenString = format_string_list(hidden, hiddenCount);

    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "ModLauncher running: args {}", hiddenString);
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "JVM identified as {} {} {}", system_property("java.vm.vendor"),
                            system_property("java.vm.name"), system_property("java.vm.version"));

    for (size_t i = 0; i < hiddenCount; i++) free(hidden[i]);
    free(hidden);
    free(hiddenString);

    LIBMATTI_ML_Launcher *launcher = LIBMATTI_ML_Launcher_New();
    if (launcher == NULL) return 1;

    int status = LIBMATTI_ML_Launcher_Run(launcher, argc, argv);

    LIBMATTI_ML_Launcher_Free(launcher);
    return status;
}

// Java: public final TypesafeMap blackboard()
LIBMATTI_MLA_TypesafeMap *LIBMATTI_ML_Launcher_Blackboard(const LIBMATTI_ML_Launcher *launcher)
{
    return launcher->blackboard;
}

// Java: private void run(String... args)
// 0 = ran to completion, 1 = the Java method threw (the JVM exits non-zero)
int LIBMATTI_ML_Launcher_Run(LIBMATTI_ML_Launcher *launcher, int argc, char *argv[])
{
    // Java: final ArgumentHandler.DiscoveryData discoveryData = this.argumentHandler.setArgs(args);
    LIBMATTI_ML_ArgumentHandler_DiscoveryData discoveryData =
        LIBMATTI_ML_ArgumentHandler_SetArgs(launcher->argumentHandler, argc, argv);

    // Java: this.transformationServicesHandler.discoverServices(discoveryData);
    LIBMATTI_ML_TransformationServicesHandler_DiscoverServices(launcher->transformationServicesHandler, discoveryData);

    // Java: scanResults = initializeTransformationServices(argumentHandler, environment)
    //                 .stream().collect(Collectors.groupingBy(ITransformationService.Resource::target))
    size_t scanResultCount = 0;
    LIBMATTI_MLA_ITransformationService_Resource *scanResults =
        LIBMATTI_ML_TransformationServicesHandler_InitializeTransformationServices(
            launcher->transformationServicesHandler, launcher->argumentHandler, launcher->environment,
            &scanResultCount);

    ResourceGroup scanGroups[4];
    group_by_target(scanResults, scanResultCount, scanGroups);

    // Java: scanResults.getOrDefault(Layer.PLUGIN, List.of()) -> mapMulti(resources) -> addToLayer(PLUGIN, np)
    add_to_layer(launcher, LIBMATTI_MLA_LAYER_PLUGIN, &scanGroups[LIBMATTI_MLA_LAYER_PLUGIN]);

    // Java: this.moduleLayerHandler.buildLayer(IModuleLayerManager.Layer.PLUGIN)
    LIBMATTI_ML_ModuleLayerHandler_BuildLayer(launcher->moduleLayerHandler, LIBMATTI_MLA_LAYER_PLUGIN);

    // Java: gameResults = triggerScanCompletion(moduleLayerHandler).stream().collect(groupingBy(Resource::target))
    size_t gameResultCount = 0;
    LIBMATTI_MLA_ITransformationService_Resource *gameResults =
        LIBMATTI_ML_TransformationServicesHandler_TriggerScanCompletion(
            launcher->transformationServicesHandler,
            (LIBMATTI_MLA_IModuleLayerManager *)launcher->moduleLayerHandler, &gameResultCount);

    ResourceGroup gameResultGroups[4];
    group_by_target(gameResults, gameResultCount, gameResultGroups);

    // Java: Stream.of(scanResults, gameResults)
    //         .flatMap(m -> m.getOrDefault(Layer.GAME, List.of()).stream())
    //         .mapMulti((resource, action) -> resource.resources().forEach(action))
    //         .toList()
    LIBMATTI_JH_SecureJar **gameContents = NULL;
    size_t gameContentCount = 0;
    collect_resources(&gameContents, &gameContentCount, &scanGroups[LIBMATTI_MLA_LAYER_GAME]);
    collect_resources(&gameContents, &gameContentCount, &gameResultGroups[LIBMATTI_MLA_LAYER_GAME]);

    // Java: gameContents.forEach(j -> this.moduleLayerHandler.addToLayer(Layer.GAME, j))
    for (size_t i = 0; i < gameContentCount; i++)
        LIBMATTI_ML_ModuleLayerHandler_AddToLayer(launcher->moduleLayerHandler, LIBMATTI_MLA_LAYER_GAME,
                                                  gameContents[i]);

    // Java: this.transformationServicesHandler.initialiseServiceTransformers()
    LIBMATTI_ML_TransformationServicesHandler_InitialiseServiceTransformers(launcher->transformationServicesHandler);

    // Java: this.launchPlugins.offerScanResultsToPlugins(gameContents)
    LIBMATTI_ML_LaunchPluginHandler_OfferScanResultsToPlugins(launcher->launchPlugins, gameContents, gameContentCount);

    // Java: this.launchService.validateLaunchTarget(this.argumentHandler)
    int ran = LIBMATTI_ML_LaunchServiceHandler_ValidateLaunchTarget(launcher->launchService,
                                                                    launcher->argumentHandler);
    // The game's exit status (Java: the JVM carries it out of main); 0 until the launch ran.
    int exitCode = 0;

    if (ran)
    {
        // Java: this.classLoader = this.transformationServicesHandler.buildTransformingClassLoader(...)
        launcher->classLoader = LIBMATTI_ML_TransformationServicesHandler_BuildTransformingClassLoader(
            launcher->transformationServicesHandler, launcher->launchPlugins, launcher->environment,
            launcher->moduleLayerHandler);

        // Java: Thread.currentThread().setContextClassLoader(this.classLoader) - no C equivalent

        // Java: this.launchService.launch(this.argumentHandler,
        //         this.moduleLayerHandler.getLayer(Layer.GAME).orElseThrow(), this.classLoader, this.launchPlugins)
        LIBMATTI_JL_ModuleLayer *gameLayer =
            LIBMATTI_ML_ModuleLayerHandler_GetLayer(launcher->moduleLayerHandler, LIBMATTI_MLA_LAYER_GAME);

        if (gameLayer == NULL)
        {
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                     "No GAME layer available, unable to launch");
            ran = 0;
        }
        else
        {
            exitCode = LIBMATTI_ML_LaunchServiceHandler_Launch(launcher->launchService, launcher->argumentHandler,
                                                               gameLayer, launcher->classLoader,
                                                               launcher->launchPlugins);
        }
    }

    free(gameContents);
    group_free(scanGroups);
    group_free(gameResultGroups);
    return ran ? exitCode : 1;
}

// Java: public Environment environment()
LIBMATTI_ML_Environment *LIBMATTI_ML_Launcher_Environment(const LIBMATTI_ML_Launcher *launcher)
{
    return launcher->environment;
}

// Java: Optional<ILaunchPluginService> findLaunchPlugin(final String name)
LIBMATTI_MLS_ILaunchPluginService *LIBMATTI_ML_Launcher_FindLaunchPlugin(LIBMATTI_ML_Launcher *launcher,
                                                                         const char *name)
{
    return LIBMATTI_ML_LaunchPluginHandler_Get(launcher->launchPlugins, name);
}

// Java: Optional<ILaunchHandlerService> findLaunchHandler(final String name)
LIBMATTI_MLA_ILaunchHandlerService *LIBMATTI_ML_Launcher_FindLaunchHandler(LIBMATTI_ML_Launcher *launcher,
                                                                          const char *name)
{
    return LIBMATTI_ML_LaunchServiceHandler_FindLaunchHandler(launcher->launchService, name);
}

// Java: public Optional<IModuleLayerManager> findLayerManager()
LIBMATTI_MLA_IModuleLayerManager *LIBMATTI_ML_Launcher_FindLayerManager(LIBMATTI_ML_Launcher *launcher)
{
    // Java: Optional.ofNullable(this.moduleLayerHandler)
    return (LIBMATTI_MLA_IModuleLayerManager *)launcher->moduleLayerHandler;
}

void LIBMATTI_ML_Launcher_Free(LIBMATTI_ML_Launcher *launcher)
{
    if (launcher == NULL) return;

    if (LIBMATTI_ML_Launcher_INSTANCE == launcher) LIBMATTI_ML_Launcher_INSTANCE = NULL;

    // Java has no teardown for the launcher; the owned components are released
    // in creation order.
    LIBMATTI_ML_LaunchPluginHandler_Free(launcher->launchPlugins);
    LIBMATTI_ML_ArgumentHandler_Free(launcher->argumentHandler);
    LIBMATTI_ML_TransformationServicesHandler_Free(launcher->transformationServicesHandler);
    LIBMATTI_ML_TransformStore_Free(launcher->transformStore);
    LIBMATTI_ML_LaunchServiceHandler_Free(launcher->launchService);
    LIBMATTI_ML_ModuleLayerHandler_Free(launcher->moduleLayerHandler);

    free(launcher);
}
