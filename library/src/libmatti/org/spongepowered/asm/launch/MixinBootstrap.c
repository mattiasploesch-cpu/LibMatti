// Port of org.spongepowered.asm.launch.MixinBootstrap.

#include "libmatti/org/spongepowered/asm/launch/MixinBootstrap.h"

#include "libmatti/org/spongepowered/asm/launch/GlobalProperties.h"
#include "libmatti/org/spongepowered/asm/launch/MixinInitialisationError.h"
#include "libmatti/org/spongepowered/asm/launch/platform/CommandLineOptions.h"
#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformManager.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final String MIXIN_TRANSFORMER_FACTORY_CLASS = "org.spongepowered.asm.mixin.transformer.MixinTransformer$Factory";
#define MIXIN_TRANSFORMER_FACTORY_CLASS "org.spongepowered.asm.mixin.transformer.MixinTransformer$Factory"

// Java: private static boolean initialised = false;
static int initialised = 0;
// Java: private static boolean initState = true;
static int initState = 1;
// Java: private static ILogger logger;
static LIBMATTI_SP_ILogger *logger = NULL;
// Java: private static MixinPlatformManager platform;
static LIBMATTI_SP_MixinPlatformManager *platform = NULL;
// Java: static { MixinService.boot(); MixinService.getService().prepare();
//                MixinBootstrap.logger = MixinService.getService().getLogger("mixin"); }
// The C port has no static initialisers; boot runs lazily on the first entry point.
static void static_init(void)
{
    if (logger != NULL) return;

    LIBMATTI_SP_MixinService_Boot();
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    logger = service != NULL ? service->getLogger(service->self, "mixin") : NULL;
    if (service != NULL && service->prepare != NULL) service->prepare(service->self);
}

// Java: @Deprecated public static void addProxy() { MixinService.getService().beginPhase(); }
void LIBMATTI_SP_MixinBootstrap_AddProxy(void)
{
    static_init();
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    if (service != NULL && service->beginPhase != NULL) service->beginPhase(service->self);
}

// Java: public static MixinPlatformManager getPlatform()
LIBMATTI_SP_MixinPlatformManager *LIBMATTI_SP_MixinBootstrap_GetPlatform(void)
{
    static_init();

    if (platform == NULL)
    {
        // Java: Object globalPlatformManager = GlobalProperties.get(Keys.PLATFORM_MANAGER);
        LIBMATTI_SP_MixinPlatformManager *globalPlatformManager =
            LIBMATTI_SP_GlobalProperties_Get(LIBMATTI_SP_GlobalProperties_Keys_PLATFORM_MANAGER());
        if (globalPlatformManager != NULL)
        {
            platform = globalPlatformManager;
        }
        else
        {
            // Java: platform = new MixinPlatformManager(); GlobalProperties.put(...);
            //       platform.init();
            platform = LIBMATTI_SP_MixinPlatformManager_New();
            LIBMATTI_SP_GlobalProperties_Put(LIBMATTI_SP_GlobalProperties_Keys_PLATFORM_MANAGER(), platform);
            LIBMATTI_SP_MixinPlatformManager_Init(platform);
        }
    }
    return platform;
}

// Java: private static boolean isSubsystemRegistered()
static int is_subsystem_registered(void)
{
    return LIBMATTI_SP_GlobalProperties_Get(LIBMATTI_SP_GlobalProperties_Keys_INIT()) != NULL;
}

// Java: private static boolean checkSubsystemVersion()
static int check_subsystem_version(void)
{
    // Java: MixinBootstrap.VERSION.equals(MixinBootstrap.getActiveSubsystemVersion());
    const char *active = LIBMATTI_SP_GlobalProperties_GetString(LIBMATTI_SP_GlobalProperties_Keys_INIT(), "");
    return strcmp(LIBMATTI_SP_MixinBootstrap_VERSION, active) == 0;
}

// Java: private static void registerSubsystem(String version)
static void register_subsystem(const char *version)
{
    LIBMATTI_SP_GlobalProperties_Put(LIBMATTI_SP_GlobalProperties_Keys_INIT(), (void *) version);
}

// Java: private static void offerInternals() - the port's native backend offers no transformer
// factory (the IMixinInternal offer is a no-op), the Class.forName reflective instantiation has
// no native equivalent.
static void offer_internals(void)
{
}

// Java: public static void init()
void LIBMATTI_SP_MixinBootstrap_Init(void)
{
    if (!LIBMATTI_SP_MixinBootstrap_Start()) return;

    LIBMATTI_SP_MixinBootstrap_DoInit(LIBMATTI_SP_CommandLineOptions_DefaultArgs());
}

// Java: static boolean start()
int LIBMATTI_SP_MixinBootstrap_Start(void)
{
    static_init();

    if (is_subsystem_registered())
    {
        if (!check_subsystem_version())
        {
            // Java: throw new MixinInitialisationError("Mixin subsystem version " + active
            //       + " was already initialised. Cannot bootstrap version " + VERSION);
            const char *active = LIBMATTI_SP_GlobalProperties_GetString(LIBMATTI_SP_GlobalProperties_Keys_INIT(), "");
            char *msg = malloc(strlen("Mixin subsystem version  was already initialised. Cannot bootstrap version 0.8.7")
                               + strlen(active) + 1);
            sprintf(msg, "Mixin subsystem version %s was already initialised. Cannot bootstrap version %s",
                    active, LIBMATTI_SP_MixinBootstrap_VERSION);
            LIBMATTI_SP_MixinInitialisationError *error = LIBMATTI_SP_MixinInitialisationError_New(msg);
            free(msg);
            // The port surfaces the error through the launcher's error report instead of unwinding.
            LIBMATTI_SP_MixinInitialisationError_Free(error);
            return 0;
        }
        return 0;
    }

    register_subsystem(LIBMATTI_SP_MixinBootstrap_VERSION);
    offer_internals();

    if (!initialised)
    {
        initialised = 1;

        LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
        // Java: Phase initialPhase = MixinService.getService().getInitialPhase();
        LIBMATTI_SP_MixinEnvironment_Phase initialPhaseValue =
            service != NULL && service->getInitialPhase != NULL
                ? service->getInitialPhase(service->self)
                : *LIBMATTI_SP_MixinEnvironment_Phase_NOT_INITIALISED();
        LIBMATTI_SP_MixinEnvironment_Phase *initialPhase = &initialPhaseValue;

        if (initialPhase->ordinal == LIBMATTI_SP_MixinEnvironment_Phase_DEFAULT()->ordinal)
        {
            LIBMATTI_SP_ILogger_Error(logger, "Initialising mixin subsystem after game pre-init phase! Some mixins may be skipped.");
            LIBMATTI_SP_MixinEnvironment_Init(initialPhase);
            LIBMATTI_SP_MixinPlatformManager_Prepare(
                LIBMATTI_SP_MixinBootstrap_GetPlatform(), LIBMATTI_SP_CommandLineOptions_DefaultArgs());
            initState = 0;
        }
        else
        {
            LIBMATTI_SP_MixinEnvironment_Init(initialPhase);
        }

        if (service != NULL && service->beginPhase != NULL) service->beginPhase(service->self);
    }

    LIBMATTI_SP_MixinBootstrap_GetPlatform();

    return 1;
}

// Java: @Deprecated static void doInit(List<String> args) { doInit(CommandLineOptions.ofArgs(args)); }
void LIBMATTI_SP_MixinBootstrap_DoInitArgs(char **args, size_t argCount)
{
    LIBMATTI_SP_CommandLineOptions *options = LIBMATTI_SP_CommandLineOptions_OfArgs((const char *const *) args, argCount);
    LIBMATTI_SP_MixinBootstrap_DoInit(options);
    LIBMATTI_SP_CommandLineOptions_Free(options);
}

// Java: static void doInit(CommandLineOptions args)
void LIBMATTI_SP_MixinBootstrap_DoInit(LIBMATTI_SP_CommandLineOptions *args)
{
    if (!initialised)
    {
        if (is_subsystem_registered())
        {
            // Java: logger.warn("Multiple Mixin containers present, init suppressed for {}", VERSION);
            LIBMATTI_SP_ILogger_Warn(logger, "Multiple Mixin containers present, init suppressed for " LIBMATTI_SP_MixinBootstrap_VERSION);
            return;
        }

        // Java: throw new IllegalStateException("MixinBootstrap.doInit() called before MixinBootstrap.start()");
        LIBMATTI_SP_ILogger_Error(logger, "MixinBootstrap.doInit() called before MixinBootstrap.start()");
        return;
    }

    // Java: MixinBootstrap.getPlatform().getPhaseProviderClasses(); (registration is commented
    // out in the original)
    size_t phaseProviderCount = 0;
    char **phaseProviders =
        LIBMATTI_SP_MixinPlatformManager_GetPhaseProviderClasses(LIBMATTI_SP_MixinBootstrap_GetPlatform(),
                                                                 &phaseProviderCount);
    for (size_t i = 0; i < phaseProviderCount; i++) free(phaseProviders[i]);
    free(phaseProviders);

    if (initState)
    {
        // Java: MixinBootstrap.getPlatform().prepare(args); MixinService.getService().init();
        LIBMATTI_SP_MixinPlatformManager_Prepare(LIBMATTI_SP_MixinBootstrap_GetPlatform(), args);
        LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
        if (service != NULL && service->init != NULL) service->init(service->self);
    }
}

// Java: static void inject() { MixinBootstrap.getPlatform().inject(); }
void LIBMATTI_SP_MixinBootstrap_Inject(void)
{
    LIBMATTI_SP_MixinPlatformManager_Inject(LIBMATTI_SP_MixinBootstrap_GetPlatform());
}
