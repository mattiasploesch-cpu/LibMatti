// Port of net.neoforged.neoforge.internal.CommonModLoader.

#include "libmatti/net/neoforged/neoforge/internal/CommonModLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModWorkManager.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/FMLClientSetupEvent.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/FMLCommonSetupEvent.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/FMLDedicatedServerSetupEvent.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/FMLLoadCompleteEvent.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/InterModEnqueueEvent.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/InterModProcessEvent.h"
#include "libmatti/net/neoforged/fml/config/ConfigTracker.h"
#include "libmatti/net/neoforged/fml/config/ModConfig.h"
#include "libmatti/net/neoforged/fml/loading/FMLEnvironment.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/neoforge/common/NeoForge.h"
#include "libmatti/net/neoforged/neoforge/internal/RegistrationEvents.h"
#include "libmatti/net/neoforged/neoforge/registries/GameData.h"
#include "libmatti/net/neoforged/neoforge/registries/RegistryManager.h"
#include "libmatti/net/neoforged/neoforge/network/registration/NetworkRegistry.h"

#include <stdio.h>
#include <stdlib.h>

// Java: private static boolean registriesLoaded = false;
static int registriesLoaded = 0;

// Java: public static boolean areRegistriesLoaded()
int LIBMATTI_NEOFORGE_CommonModLoader_AreRegistriesLoaded(void)
{
    return registriesLoaded;
}

// Java: () -> RegistryManager.postNewRegistryEvent(); GameData.unfreezeData();
//       GameData.postRegisterEvents(); GameData.freezeData(); registriesLoaded = true;
static void registry_initialization(void *userdata)
{
    (void) userdata;
    // Java: Main.main runs Bootstrap.bootStrap() (via BackgroundWaiter) before the mod loading
    //       starts - the guard goes up, the registries class-initialise and bootStrap() freezes
    //       them; the per-registry game contents are the game port's part
    LIBMATTI_MC_Bootstrap_BootStrap();
    LIBMATTI_NEOFORGE_RegistryManager_PostNewRegistryEvent();
    LIBMATTI_NEOFORGE_GameData_UnfreezeData();
    LIBMATTI_NEOFORGE_GameData_PostRegisterEvents();
    LIBMATTI_NEOFORGE_GameData_FreezeData();
    registriesLoaded = 1;
}

// Java: () -> { if (FMLEnvironment.getDist() == Dist.CLIENT) ConfigTracker.INSTANCE.loadConfigs(CLIENT, ...);
//       ConfigTracker.INSTANCE.loadConfigs(COMMON, ...); }
static void config_loading(void *userdata)
{
    (void) userdata;

    if (LIBMATTI_FML_FMLEnvironment_GetDist() == LIBMATTI_DIST_CLIENT)
        LIBMATTI_FML_ConfigTracker_LoadConfigs(LIBMATTI_FML_ConfigTracker_Instance(),
                                               LIBMATTI_FML_ModConfig_TYPE_CLIENT,
                                               LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_CONFIGDIR));
    LIBMATTI_FML_ConfigTracker_LoadConfigs(LIBMATTI_FML_ConfigTracker_Instance(),
                                           LIBMATTI_FML_ModConfig_TYPE_COMMON,
                                           LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_CONFIGDIR));
}

// Java: begin's init task "Registry initialization"
typedef struct
{
    void (*periodicTask)(void *userdata);
    void *userdata;
} PeriodicUserdata;

// Java: public static void noop() {} - the Runnable periodicTask the server path uses
static void noop_periodic(void *userdata)
{
    (void) userdata;
}

// Java: protected static void begin(Runnable periodicTask, boolean datagen)
void LIBMATTI_NEOFORGE_CommonModLoader_Begin(void (*periodicTask)(void *userdata), void *userdata, int datagen)
{
    // Java: var syncExecutor = ModWorkManager.syncExecutor();
    // The port's executors run on the caller's thread; ModLoader takes the callbacks directly.

    LIBMATTI_FML_ModLoader_GatherAndInitializeMods(periodicTask, userdata);

    LIBMATTI_FML_ModLoader_RunInitTask("Registry initialization", periodicTask, userdata, registry_initialization,
                                       NULL);

    if (!datagen)
    {
        LIBMATTI_FML_ModLoader_RunInitTask("Config loading", periodicTask, userdata, config_loading, NULL);
    }

    // Java: NeoForge.EVENT_BUS.start();
    LIBMATTI_BUS_IEventBus_Start(LIBMATTI_NEOFORGE_NeoForge_EVENT_BUS());
}

// Java: ModLoader.dispatchParallelEvent("Common setup", ..., FMLCommonSetupEvent::new)
static LIBMATTI_BUS_Event *new_common_setup(LIBMATTI_FML_ModContainer *container,
                                            LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata)
{
    (void) userdata;
    LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *event =
        LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_New(container, workQueue);
    return &event->base.base.base;
}

// Java: FMLEnvironment.getDist().isClient() ? FMLClientSetupEvent::new : FMLDedicatedServerSetupEvent::new
static LIBMATTI_BUS_Event *new_sided_setup(LIBMATTI_FML_ModContainer *container,
                                           LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata)
{
    (void) userdata;
    if (LIBMATTI_FML_FMLEnvironment_GetDist() == LIBMATTI_DIST_CLIENT)
    {
        LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *event =
            LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_New(container, workQueue);
        return &event->base.base.base;
    }
    LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *event =
        LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_New(container, workQueue);
    return &event->base.base.base;
}

// Java: protected static void load(Executor syncExecutor, Executor parallelExecutor)
void LIBMATTI_NEOFORGE_CommonModLoader_Load(void)
{
    // Java: Runnable periodicTask = () -> {}; server: no progress screen; client ticks its own screen
    void (*periodicTask)(void *userdata) = noop_periodic;
    void *userdata = NULL;

    LIBMATTI_FML_ModLoader_DispatchParallelEvent("Common setup", periodicTask, userdata, new_common_setup, NULL);
    LIBMATTI_FML_ModLoader_DispatchParallelEvent("Sided setup", periodicTask, userdata, new_sided_setup, NULL);

    // Java: ModLoader.runInitTask("Registration events", syncExecutor, periodicTask, RegistrationEvents::init);
    LIBMATTI_FML_ModLoader_RunInitTask("Registration events", periodicTask, userdata,
                                       LIBMATTI_NEOFORGE_RegistrationEvents_Init, NULL);
}

// Java: ModLoader.dispatchParallelEvent("Enqueue IMC", ..., InterModEnqueueEvent::new)
static LIBMATTI_BUS_Event *new_imc_enqueue(LIBMATTI_FML_ModContainer *container,
                                           LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata)
{
    (void) userdata;
    LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *event =
        LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_New(container, workQueue);
    return &event->base.base.base;
}

// Java: ModLoader.dispatchParallelEvent("Process IMC", ..., InterModProcessEvent::new)
static LIBMATTI_BUS_Event *new_imc_process(LIBMATTI_FML_ModContainer *container,
                                           LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata)
{
    (void) userdata;
    LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *event =
        LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_New(container, workQueue);
    return &event->base.base.base;
}

// Java: ModLoader.dispatchParallelEvent("Complete loading of %d mods".formatted(ModList.get().size()), ..., FMLLoadCompleteEvent::new)
static LIBMATTI_BUS_Event *new_load_complete(LIBMATTI_FML_ModContainer *container,
                                             LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata)
{
    (void) userdata;
    LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *event =
        LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_New(container, workQueue);
    return &event->base.base.base;
}

// Java: "Complete loading of %d mods".formatted(ModList.get().size()) - the port formats the count
// into a caller-owned buffer because the dispatch happens before the event constructor runs.
static char *load_complete_name(void)
{
    size_t size = LIBMATTI_FML_ModList_Size(LIBMATTI_FML_ModList_Get());
    char *name = malloc(64);
    if (name != NULL) snprintf(name, 64, "Complete loading of %zu mods", size);
    return name;
}

// Java: NetworkRegistry::setup - the method reference adapts to the Runnable init-task signature
static void network_registry_setup_task(void *userdata)
{
    (void) userdata;
    LIBMATTI_NEOFORGE_NetworkRegistry_Setup();
}

// Java: protected static void finish(Executor syncExecutor, Executor parallelExecutor)
void LIBMATTI_NEOFORGE_CommonModLoader_Finish(void)
{
    void (*periodicTask)(void *userdata) = noop_periodic;
    void *userdata = NULL;

    LIBMATTI_FML_ModLoader_DispatchParallelEvent("Enqueue IMC", periodicTask, userdata, new_imc_enqueue, NULL);
    LIBMATTI_FML_ModLoader_DispatchParallelEvent("Process IMC", periodicTask, userdata, new_imc_process, NULL);

    char *name = load_complete_name();
    LIBMATTI_FML_ModLoader_DispatchParallelEvent(name != NULL ? name : "Complete loading of mods", periodicTask,
                                                 userdata, new_load_complete, NULL);
    free(name);

    // Java: ModLoader.runInitTask("Network registry lock", syncExecutor, periodicTask, NetworkRegistry::setup)
    LIBMATTI_FML_ModLoader_RunInitTask("Network registry lock", periodicTask, userdata,
                                       network_registry_setup_task, NULL);
}
