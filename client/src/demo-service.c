//
// Demo services so the matticraft target exercises the whole chain:
// Stage 1 -> Launcher -> PLUGIN/GAME layer -> launch handler.
//
// In Java these providers are found through META-INF/services inside the
// modlauncher artifact and the mods. C has no service files, so the host
// registers them here (java/util/ServiceLoader.h).
//

#include "demo-service.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/cpw/modlauncher/LogMarkers.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"
#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/net/neoforged/fml/startup/NeoForgeLaunchHandler.h"

// ---------------------------------------------------------------------------
// a demo ITransformationService named "demo"
// Java: a mod's TransformationService discovered via ServiceLoader
// ---------------------------------------------------------------------------

// Java: String name() { return "demo"; }
static const char *demo_name(LIBMATTI_MLA_ITransformationService *self)
{
    (void)self;
    return "demo";
}

// Java: default void arguments(...) -> not overridden
// Java: default void argumentValues(...) -> not overridden

// Java: void initialize(IEnvironment environment)
static void demo_initialize(LIBMATTI_MLA_ITransformationService *self, LIBMATTI_MLA_IEnvironment *environment)
{
    (void)self;
    (void)environment;

    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                            "Demo transformation service '{}' initialised", "demo");
}

// Java: void onLoad(IEnvironment env, Set<String> otherServices) - 1 = loaded
static int demo_on_load(LIBMATTI_MLA_ITransformationService *self, LIBMATTI_MLA_IEnvironment *env,
                        char **otherServices, size_t otherServiceCount)
{
    (void)self;
    (void)env;
    (void)otherServices;
    (void)otherServiceCount;

    return 1;
}

// Java: default List<? extends ITransformer<?>> transformers() -> empty
static LIBMATTI_MLA_ITransformer **demo_transformers(LIBMATTI_MLA_ITransformationService *self, size_t *count)
{
    (void)self;
    *count = 0;
    return NULL;
}

// Java: default List<Resource> beginScanning(IEnvironment environment) -> empty
// (the demo mods are already resolved by Stage 1 into the BOOT layer)
static LIBMATTI_MLA_ITransformationService_Resource *demo_begin_scanning(
    LIBMATTI_MLA_ITransformationService *self, LIBMATTI_MLA_IEnvironment *environment, size_t *count)
{
    (void)self;
    (void)environment;
    *count = 0;
    return NULL;
}

// Java: default List<Resource> completeScan(IModuleLayerManager) -> NULL entry
static LIBMATTI_MLA_ITransformationService demo_service = {
    demo_name,          // name
    NULL,               // arguments (default)
    NULL,               // argumentValues (default)
    demo_initialize,    // initialize
    demo_on_load,       // onLoad
    demo_transformers,  // transformers
    demo_begin_scanning,// beginScanning
    NULL                // completeScan (default)
};

// ---------------------------------------------------------------------------

void demo_register_services(void)
{
    // Java: META-INF/services/cpw.mods.modlauncher.api.ILaunchHandlerService
    //       -> the NeoForge target that starts net.neoforged.fml.startup.Client
    LIBMATTI_FML_NeoForgeLaunchHandler *neoforgeHandler = LIBMATTI_FML_NeoForgeLaunchHandler_New();
    LIBMATTI_JU_ServiceLoader_Register("cpw.mods.modlauncher.api.ILaunchHandlerService", &neoforgeHandler->base,
                                       "net.neoforged.fml.startup.NeoForgeLaunchHandler", "neoforge");

    // Java: META-INF/services/cpw.mods.modlauncher.api.ITransformationService
    //       -> the demo mod's transformation service
    // (the launch handlers of the modlauncher module itself are registered by
    //  Launcher, mirroring the provides of module-info.java)
    LIBMATTI_JU_ServiceLoader_Register("cpw.mods.modlauncher.api.ITransformationService", &demo_service,
                                       "com.demo.DemoTransformationService", "demo-service.so");
}
