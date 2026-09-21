// Port of net.neoforged.fml.loading.mixin.FMLMixinService.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinService.h"

#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLAuditTrail.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassProvider.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassTracker.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinLogger.h"
#include "libmatti/org/spongepowered/asm/launch/platform/container/ContainerHandleVirtual.h"
#include "libmatti/org/spongepowered/asm/util/Constants.h"
#include "libmatti/org/spongepowered/asm/util/ReEntranceLock.h"

#include <stdlib.h>
#include <string.h>

// Java: private final ConcurrentHashMap<String, FMLMixinLogger> loggers
typedef struct
{
    char *name;
    LIBMATTI_SP_ILogger *logger;
} LoggerEntry;

// Java: the singleton state (Java's instance fields)
static LoggerEntry *loggers = NULL;
static size_t loggerCount = 0;
static LIBMATTI_SP_ReEntranceLock *lock = NULL;
static void *classProvider = NULL;
static void *bytecodeProvider = NULL; // Java: @Nullable IClassBytecodeProvider
static void *classTracker = NULL;
static void *auditTrail = NULL;
static void *featureValidator = NULL;
static void *adviceProvider = NULL;
static void *mixinTransformer = NULL; // Java: @Nullable IMixinTransformer

// Java: private final Map<String, byte[]> mixinConfigContents
typedef struct
{
    char *name;
    unsigned char *content;
    size_t length;
} ConfigContent;

static ConfigContent *mixinConfigContents = NULL;
static size_t mixinConfigContentCount = 0;

// Java: private final ContainerHandleVirtual primaryContainer = new ContainerHandleVirtual("fml");
static LIBMATTI_SP_ContainerHandleVirtual *primaryContainer = NULL;
// Java: private final List<IContainerHandle> mixinContainers
static LIBMATTI_SP_ContainerHandle **mixinContainers = NULL;
static size_t mixinContainerCount = 0;

static LIBMATTI_SP_IMixinService *SERVICE_SELF(void);

// Java: the singleton (Java constructs the instance through the service loader)
static int instanceReady = 0;

static void ensure_deps(void)
{
    if (lock == NULL) lock = LIBMATTI_SP_ReEntranceLock_New(1);
    if (classTracker == NULL) classTracker = LIBMATTI_FML_FMLClassTracker_Instance();
    if (auditTrail == NULL) auditTrail = LIBMATTI_FML_FMLAuditTrail_Instance();
    if (featureValidator == NULL) featureValidator = LIBMATTI_FML_FMLMixinFeatureValidator_Instance();
    if (adviceProvider == NULL) adviceProvider = LIBMATTI_FML_FMLMixinAdviceProvider_Instance();
    if (primaryContainer == NULL) primaryContainer = LIBMATTI_SP_ContainerHandleVirtual_New("fml");
}

// Java: @Override public void prepare() {}
static void svc_prepare(void *self)
{
    (void) self;
}

// Java: @Override public Phase getInitialPhase() { return Phase.PREINIT; }
// The interface returns the phase by value (Java's enum semantics)
static LIBMATTI_SP_MixinEnvironment_Phase svc_get_initial_phase(void *self)
{
    (void) self;
    return *LIBMATTI_SP_MixinEnvironment_Phase_PREINIT();
}

// Java: @Override public void offer(IMixinInternal internal)
//       { if (internal instanceof IMixinTransformerFactory) this.mixinTransformer = ...createTransformer(); }
// The port's native backend replaces the bytecode transformer, so no internal is offered.
static void svc_offer(void *self, LIBMATTI_SP_IMixinInternal *internal)
{
    (void) self;
    (void) internal;
}

// Java: @Override public void init() {}
static void svc_init(void *self)
{
    (void) self;
}

// Java: @Override public void beginPhase() {}
static void svc_begin_phase(void *self)
{
    (void) self;
}

// Java: @Override public void checkEnv(Object bootSource) {}
static void svc_check_env(void *self, void *bootSource)
{
    (void) self;
    (void) bootSource;
}

// Java: @Override public ReEntranceLock getReEntranceLock() { return lock; }
static LIBMATTI_SP_ReEntranceLock *svc_get_re_entrance_lock(void *self)
{
    (void) self;
    return lock;
}

// Java: @Override public String getSideName()
//       { return switch (FMLLoader.getCurrent().getDist()) { case CLIENT -> Constants.SIDE_CLIENT;
//         case DEDICATED_SERVER -> Constants.SIDE_SERVER; }; }
static const char *svc_get_side_name(void *self)
{
    (void) self;
    LIBMATTI_DIST_Dist dist = LIBMATTI_FML_FMLLoader_GetDist(LIBMATTI_FML_FMLLoader_GetCurrent());
    // The port's Dist enum: CLIENT first, DEDICATED_SERVER second
    return dist == 0 ? LIBMATTI_SP_Constants_SIDE_CLIENT : LIBMATTI_SP_Constants_SIDE_SERVER;
}

// Java: @Override public String getName() { return "FML"; }
static const char *svc_get_name(void *self)
{
    (void) self;
    return "FML";
}

// Java: @Override public boolean isValid() { return true; }
static int svc_is_valid(void *self)
{
    (void) self;
    return 1;
}

// Java: @Override public CompatibilityLevel getMinCompatibilityLevel() { return CompatibilityLevel.JAVA_21; }
static LIBMATTI_SP_MixinEnvironment_CompatibilityLevel svc_get_min_compatibility_level(void *self)
{
    (void) self;
    return LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_21;
}

// Java: @Override public CompatibilityLevel getMaxCompatibilityLevel() { return null; }
static LIBMATTI_SP_MixinEnvironment_CompatibilityLevel svc_get_max_compatibility_level(void *self)
{
    (void) self;
    return LIBMATTI_SP_MixinEnvironment_COMPAT_NONE;
}

// Java: @Override public ILogger getLogger(String name) { return loggers.computeIfAbsent(name, FMLMixinLogger::new); }
static LIBMATTI_SP_ILogger *svc_get_logger(void *self, const char *name)
{
    (void) self;
    for (size_t i = 0; i < loggerCount; i++)
        if (strcmp(loggers[i].name, name) == 0) return loggers[i].logger;

    loggers = realloc(loggers, sizeof(LoggerEntry) * (loggerCount + 1));
    loggers[loggerCount].name = strdup(name);
    loggers[loggerCount].logger = LIBMATTI_FML_FMLMixinLogger_Instance(name);
    return loggers[loggerCount++].logger;
}

// Java: @Override public Collection<String> getPlatformAgents()
//       { return List.of("org.spongepowered.asm.launch.platform.MixinPlatformAgentDefault"); }
static const char *const *svc_get_platform_agents(void *self, size_t *count)
{
    (void) self;
    static const char *const AGENTS[] = {"org.spongepowered.asm.launch.platform.MixinPlatformAgentDefault"};
    *count = 1;
    return AGENTS;
}

// Java: @Override public IContainerHandle getPrimaryContainer() { return primaryContainer; }
static LIBMATTI_SP_ContainerHandle *svc_get_primary_container(void *self)
{
    (void) self;
    return &primaryContainer->base;
}

// Java: @Override public Collection<IContainerHandle> getMixinContainers() { return mixinContainers; }
static LIBMATTI_SP_ContainerHandle **svc_get_mixin_containers(void *self, size_t *count)
{
    (void) self;
    *count = mixinContainerCount;
    return mixinContainers;
}

// Java: @Override public InputStream getResourceAsStream(String name)
//       { var content = mixinConfigContents.get(name); ... return content != null ? ... : null; }
static unsigned char *svc_get_resource_as_stream(void *self, const char *name, size_t *outLength)
{
    (void) self;
    for (size_t i = 0; i < mixinConfigContentCount; i++)
    {
        if (strcmp(mixinConfigContents[i].name, name) == 0)
        {
            *outLength = mixinConfigContents[i].length;
            return mixinConfigContents[i].content;
        }
    }
    return NULL;
}

// Java: @Override public IClassTracker getClassTracker() { return this.classTracker; }
static void *svc_get_class_tracker(void *self)
{
    (void) self;
    return classTracker;
}

// Java: @Override public IMixinAuditTrail getAuditTrail() { return this.auditTrail; }
static void *svc_get_audit_trail(void *self)
{
    (void) self;
    return auditTrail;
}

// Java: @Override public IFeatureValidator getFeatureValidator() { return this.featureValidator; }
static void *svc_get_feature_validator(void *self)
{
    (void) self;
    return featureValidator;
}

// Java: @Override public IAdviceProvider getAdviceProvider() { return this.adviceProvider; }
static void *svc_get_advice_provider(void *self)
{
    (void) self;
    return adviceProvider;
}

// Java: @Override public ITransformerProvider getTransformerProvider() { return null; }
static void *svc_get_transformer_provider(void *self)
{
    (void) self;
    return NULL;
}

// Java: @Override public IClassProvider getClassProvider()
//       { if (this.classProvider == null) this.classProvider = new FMLClassProvider(); return this.classProvider; }
static void *svc_get_class_provider(void *self)
{
    (void) self;
    if (classProvider == NULL) classProvider = LIBMATTI_FML_FMLClassProvider_Instance();
    return classProvider;
}

// Java: @Override public IClassBytecodeProvider getBytecodeProvider()
//       { if (this.bytecodeProvider == null) throw new IllegalStateException(...); return this.bytecodeProvider; }
static void *svc_get_bytecode_provider(void *self)
{
    (void) self;
    return bytecodeProvider;
}

// Java: public IMixinTransformer getMixinTransformer() { return Objects.requireNonNull(this.mixinTransformer); }
void *LIBMATTI_FML_FMLMixinService_GetMixinTransformer(void)
{
    return mixinTransformer;
}

// Java: public void setBytecodeProvider(@Nullable IClassBytecodeProvider bytecodeProvider)
void LIBMATTI_FML_FMLMixinService_SetBytecodeProvider(void *provider)
{
    bytecodeProvider = provider;
}

// Java: public void addMixinConfigContent(String config, byte[] resource)
void LIBMATTI_FML_FMLMixinService_AddMixinConfigContent(const char *config, const unsigned char *resource,
                                                        size_t length)
{
    for (size_t i = 0; i < mixinConfigContentCount; i++)
    {
        if (strcmp(mixinConfigContents[i].name, config) == 0)
        {
            // Java: HashMap.put replaces
            free(mixinConfigContents[i].content);
            mixinConfigContents[i].content = malloc(length);
            memcpy(mixinConfigContents[i].content, resource, length);
            mixinConfigContents[i].length = length;
            return;
        }
    }

    mixinConfigContents = realloc(mixinConfigContents, sizeof(ConfigContent) * (mixinConfigContentCount + 1));
    mixinConfigContents[mixinConfigContentCount].name = strdup(config);
    mixinConfigContents[mixinConfigContentCount].content = malloc(length);
    memcpy(mixinConfigContents[mixinConfigContentCount].content, resource, length);
    mixinConfigContents[mixinConfigContentCount].length = length;
    mixinConfigContentCount++;
}

// Java: public void addMixinContainer(IContainerHandle handle) { this.mixinContainers.add(handle); }
void LIBMATTI_FML_FMLMixinService_AddMixinContainer(LIBMATTI_SP_ContainerHandle *handle)
{
    mixinContainers = realloc(mixinContainers, sizeof(LIBMATTI_SP_ContainerHandle *) * (mixinContainerCount + 1));
    mixinContainers[mixinContainerCount++] = handle;
}

// Java: @VisibleForTesting public void clearMixinContainers() { mixinContainers.clear(); }
void LIBMATTI_FML_FMLMixinService_ClearMixinContainers(void)
{
    free(mixinContainers);
    mixinContainers = NULL;
    mixinContainerCount = 0;
}

// The interface dispatch table (Java's implements)
static LIBMATTI_SP_IMixinService theService = {
    NULL,
    svc_prepare,
    svc_get_initial_phase,
    svc_offer,
    svc_init,
    svc_begin_phase,
    svc_check_env,
    svc_get_re_entrance_lock,
    svc_get_side_name,
    svc_get_name,
    svc_is_valid,
    svc_get_min_compatibility_level,
    svc_get_max_compatibility_level,
    svc_get_logger,
    svc_get_platform_agents,
    svc_get_primary_container,
    svc_get_mixin_containers,
    svc_get_resource_as_stream,
    svc_get_class_tracker,
    svc_get_audit_trail,
    svc_get_feature_validator,
    svc_get_advice_provider,
    svc_get_transformer_provider,
    svc_get_class_provider,
    svc_get_bytecode_provider,
};

// Java: class FMLMixinService implements IMixinService (the port's singleton)
LIBMATTI_SP_IMixinService *LIBMATTI_FML_FMLMixinService_Instance(void)
{
    ensure_deps();
    return &theService;
}
