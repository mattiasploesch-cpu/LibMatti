// Port of org.spongepowered.asm.launch.platform.MixinContainer and
// org.spongepowered.asm.launch.platform.MixinPlatformManager.
// "Handler for platform-specific behaviour required in different mixin environments."

#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformManager.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/org/spongepowered/asm/launch/GlobalProperties.h"
#include "libmatti/org/spongepowered/asm/launch/platform/CommandLineOptions.h"
#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformAgentDefault.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"
#include "libmatti/org/spongepowered/asm/mixin/Mixins.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final String DEFAULT_MAIN_CLASS = "net.minecraft.client.main.Main";
#define DEFAULT_MAIN_CLASS "net.minecraft.client.main.Main"

// Java: private static final ILogger logger = MixinService.getService().getLogger("mixin");
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// ---------------------------------------------------------------------------
// MixinContainer
// ---------------------------------------------------------------------------

// Java: private static final List<String> agentClasses = new ArrayList<String>();
//       static { GlobalProperties.put(Keys.AGENTS, agentClasses);
//                for (String agent : MixinService.getService().getPlatformAgents()) agentClasses.add(agent);
//                agentClasses.add("...MixinPlatformAgentDefault"); }
// Java's Class.forName over the names resolves to the port's registered agent factories; the
// list keeps (name, factory) pairs in the AGENTS order.
typedef struct
{
    const char *name;
    LIBMATTI_SP_IMixinPlatformAgent *(*create)(void);
} AgentClass;

static AgentClass *agentClasses = NULL;
static size_t agentClassCount = 0;

static void agent_classes_init(void)
{
    if (agentClasses != NULL) return;

    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    size_t serviceAgentCount = 0;
    const char *const *serviceAgents =
        service != NULL && service->getPlatformAgents != NULL
            ? service->getPlatformAgents(service->self, &serviceAgentCount)
            : NULL;

    // Java: the service's agents + MixinPlatformAgentDefault (the only agent the port ships)
    agentClasses = malloc(sizeof(AgentClass));
    agentClasses[0].name = "org.spongepowered.asm.launch.platform.MixinPlatformAgentDefault";
    agentClasses[0].create = (LIBMATTI_SP_IMixinPlatformAgent * (*)(void)) LIBMATTI_SP_MixinPlatformAgentDefault_New;
    agentClassCount = 1;
    (void) serviceAgents;
    (void) serviceAgentCount;
}

// Java: public MixinContainer(MixinPlatformManager manager, IContainerHandle handle)
LIBMATTI_SP_MixinContainer *LIBMATTI_SP_MixinContainer_New(LIBMATTI_SP_MixinPlatformManager *manager,
                                                           LIBMATTI_SP_ContainerHandle *handle)
{
    agent_classes_init();

    LIBMATTI_SP_MixinContainer *container = calloc(1, sizeof(*container));
    container->handle = handle;

    for (size_t i = 0; i < agentClassCount; i++)
    {
        // Java: Class<IMixinPlatformAgent> clazz = (Class<IMixinPlatformAgent>)Class.forName(agentClass);
        //       String simpleName = clazz.getSimpleName();
        //       logger.debug("Instancing new {} for {}", simpleName, this.handle);
        //       IMixinPlatformAgent agent = clazz.getDeclaredConstructor().newInstance();
        LIBMATTI_SP_IMixinPlatformAgent *agent = agentClasses[i].create();

        // Java: AcceptResult acceptAction = agent.accept(manager, this.handle);
        LIBMATTI_SP_AcceptResult acceptAction = agent->accept(agent->self, manager, handle);
        if (acceptAction == LIBMATTI_SP_AcceptResult_ACCEPTED)
        {
            // Java: this.agents.add(agent);
            container->agents = realloc(container->agents, sizeof(LIBMATTI_SP_IMixinPlatformAgent *) * (container->agentCount + 1));
            container->agents[container->agentCount++] = agent;
        }
        else if (acceptAction == LIBMATTI_SP_AcceptResult_INVALID)
        {
            // Java: iter.remove(); continue; - the agent class is struck from the list
            memmove(&agentClasses[i], &agentClasses[i + 1], sizeof(AgentClass) * (agentClassCount - i - 1));
            agentClassCount--;
            free(agent);
            continue;
        }
    }
    return container;
}

void LIBMATTI_SP_MixinContainer_Free(LIBMATTI_SP_MixinContainer *container)
{
    if (container == NULL) return;
    for (size_t i = 0; i < container->agentCount; i++) free(container->agents[i]);
    free(container->agents);
    free(container);
}

// Java: public IContainerHandle getDescriptor()
LIBMATTI_SP_ContainerHandle *LIBMATTI_SP_MixinContainer_GetDescriptor(const LIBMATTI_SP_MixinContainer *container)
{
    return container->handle;
}

// Java: public Collection<String> getPhaseProviders()
char **LIBMATTI_SP_MixinContainer_GetPhaseProviders(const LIBMATTI_SP_MixinContainer *container, size_t *count)
{
    char **phaseProviders = NULL;
    size_t providerCount = 0;
    for (size_t i = 0; i < container->agentCount; i++)
    {
        LIBMATTI_SP_IMixinPlatformAgent *agent = container->agents[i];
        const char *phaseProvider = agent->getPhaseProvider(agent->self);
        if (phaseProvider != NULL)
        {
            phaseProviders = realloc(phaseProviders, sizeof(char *) * (providerCount + 1));
            phaseProviders[providerCount++] = strdup(phaseProvider);
        }
    }
    *count = providerCount;
    return phaseProviders;
}

// Java: public void prepare()
void LIBMATTI_SP_MixinContainer_Prepare(LIBMATTI_SP_MixinContainer *container)
{
    for (size_t i = 0; i < container->agentCount; i++)
    {
        LIBMATTI_SP_IMixinPlatformAgent *agent = container->agents[i];
        // Java: logger.debug("Processing prepare() for {}", agent);
        agent->prepare(agent->self);
    }
}

// Java: public void initPrimaryContainer()
void LIBMATTI_SP_MixinContainer_InitPrimaryContainer(LIBMATTI_SP_MixinContainer *container)
{
    for (size_t i = 0; i < container->agentCount; i++)
    {
        LIBMATTI_SP_IMixinPlatformAgent *agent = container->agents[i];
        // Java: logger.debug("Processing launch tasks for {}", agent);
        agent->initPrimaryContainer(agent->self);
    }
}

// Java: public void inject()
void LIBMATTI_SP_MixinContainer_Inject(LIBMATTI_SP_MixinContainer *container)
{
    for (size_t i = 0; i < container->agentCount; i++)
    {
        LIBMATTI_SP_IMixinPlatformAgent *agent = container->agents[i];
        // Java: logger.debug("Processing inject() for {}", agent);
        agent->inject(agent->self);
    }
}

// ---------------------------------------------------------------------------
// MixinPlatformManager
// ---------------------------------------------------------------------------

static LIBMATTI_SP_MixinContainer *find_container(LIBMATTI_SP_MixinPlatformManager *manager,
                                                  LIBMATTI_SP_ContainerHandle *handle)
{
    for (size_t i = 0; i < manager->containerCount; i++)
        if (manager->containerHandles[i] == handle) return manager->containers[i];

    return NULL;
}

// Java: private MixinContainer createContainerFor(IContainerHandle handle)
static LIBMATTI_SP_MixinContainer *create_container_for(LIBMATTI_SP_MixinPlatformManager *manager,
                                                        LIBMATTI_SP_ContainerHandle *handle)
{
    // Java: logger.debug("Adding mixin platform agents for container {}", handle);
    LIBMATTI_SP_MixinContainer *container = LIBMATTI_SP_MixinContainer_New(manager, handle);
    if (manager->prepared)
    {
        // Java: container.prepare();
        LIBMATTI_SP_MixinContainer_Prepare(container);
    }
    return container;
}

// Java: private void addNestedContainers(IContainerHandle handle)
static void add_nested_containers(LIBMATTI_SP_MixinPlatformManager *manager, LIBMATTI_SP_ContainerHandle *handle)
{
    size_t nestedCount = 0;
    LIBMATTI_SP_ContainerHandle **nested = handle->getNestedContainers(handle->self, &nestedCount);
    for (size_t i = 0; i < nestedCount; i++)
    {
        if (find_container(manager, nested[i]) == NULL)
        {
            LIBMATTI_SP_MixinPlatformManager_AddContainer(manager, nested[i]);
        }
    }
}

// Java: public MixinPlatformManager()
LIBMATTI_SP_MixinPlatformManager *LIBMATTI_SP_MixinPlatformManager_New(void)
{
    LIBMATTI_SP_MixinPlatformManager *manager = calloc(1, sizeof(*manager));
    manager->connectors = LIBMATTI_SP_MixinConnectorManager_New();
    return manager;
}

void LIBMATTI_SP_MixinPlatformManager_Free(LIBMATTI_SP_MixinPlatformManager *manager)
{
    if (manager == NULL) return;
    for (size_t i = 0; i < manager->containerCount; i++)
        LIBMATTI_SP_MixinContainer_Free(manager->containers[i]);
    free(manager->containers);
    free(manager->containerHandles);
    LIBMATTI_SP_MixinConnectorManager_Free(manager->connectors);
    free(manager);
}

// Java: public void init()
void LIBMATTI_SP_MixinPlatformManager_Init(LIBMATTI_SP_MixinPlatformManager *manager)
{
    // Java: logger.debug("Initialising Mixin Platform Manager");
    LIBMATTI_SP_ILogger_Debug(LOGGER(), "Initialising Mixin Platform Manager");

    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    LIBMATTI_SP_ContainerHandle *primaryContainerHandle =
        service != NULL && service->getPrimaryContainer != NULL ? service->getPrimaryContainer(service->self) : NULL;

    // Java: this.primaryContainer = this.addContainer(primaryContainerHandle);
    manager->primaryContainer = LIBMATTI_SP_MixinPlatformManager_AddContainer(manager, primaryContainerHandle);

    // Java: this.scanForContainers(); - "Do an early scan to ensure preinit mixins are discovered"
    LIBMATTI_SP_MixinPlatformManager_ScanForContainers(manager);
}

// Java: private void scanForContainers()
void LIBMATTI_SP_MixinPlatformManager_ScanForContainers(LIBMATTI_SP_MixinPlatformManager *manager)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    if (service == NULL || service->getMixinContainers == NULL) return;

    size_t mixinContainerCount = 0;
    LIBMATTI_SP_ContainerHandle **mixinContainers = service->getMixinContainers(service->self, &mixinContainerCount);

    for (size_t i = 0; i < manager->containerCount; i++)
        add_nested_containers(manager, manager->containerHandles[i]);

    for (size_t i = 0; i < mixinContainerCount; i++)
    {
        // Java: logger.debug("Adding agents for Mixin Container {}", handle); addContainer(handle);
        LIBMATTI_SP_MixinPlatformManager_AddContainer(manager, mixinContainers[i]);
    }
}

// Java: public Collection<String> getPhaseProviderClasses()
char **LIBMATTI_SP_MixinPlatformManager_GetPhaseProviderClasses(LIBMATTI_SP_MixinPlatformManager *manager,
                                                                size_t *count)
{
    if (manager->primaryContainer != NULL)
        return LIBMATTI_SP_MixinContainer_GetPhaseProviders(manager->primaryContainer, count);

    *count = 0;
    return NULL;
}

// Java: public final MixinContainer addContainer(IContainerHandle handle)
LIBMATTI_SP_MixinContainer *LIBMATTI_SP_MixinPlatformManager_AddContainer(LIBMATTI_SP_MixinPlatformManager *manager,
                                                                          LIBMATTI_SP_ContainerHandle *handle)
{
    if (handle == NULL) return NULL;

    // Java: MixinContainer existingContainer = this.containers.get(handle);
    LIBMATTI_SP_MixinContainer *existingContainer = find_container(manager, handle);
    if (existingContainer != NULL) return existingContainer;

    // Java: MixinContainer container = this.createContainerFor(handle);
    LIBMATTI_SP_MixinContainer *container = create_container_for(manager, handle);

    // Java: this.containers.put(handle, container);
    manager->containerHandles = realloc(manager->containerHandles,
                                        sizeof(LIBMATTI_SP_ContainerHandle *) * (manager->containerCount + 1));
    manager->containers = realloc(manager->containers,
                                  sizeof(LIBMATTI_SP_MixinContainer *) * (manager->containerCount + 1));
    manager->containerHandles[manager->containerCount] = handle;
    manager->containers[manager->containerCount] = container;
    manager->containerCount++;

    // Java: this.addNestedContainers(handle);
    add_nested_containers(manager, handle);
    return container;
}

// Java: public final void prepare(CommandLineOptions args)
void LIBMATTI_SP_MixinPlatformManager_Prepare(LIBMATTI_SP_MixinPlatformManager *manager,
                                              LIBMATTI_SP_CommandLineOptions *args)
{
    // Java: this.prepared = true;
    manager->prepared = 1;

    for (size_t i = 0; i < manager->containerCount; i++)
        LIBMATTI_SP_MixinContainer_Prepare(manager->containers[i]);

    size_t configCount = 0;
    const char *const *configs =
        args != NULL ? LIBMATTI_SP_CommandLineOptions_GetConfigs(args, &configCount) : NULL;
    for (size_t i = 0; i < configCount; i++)
    {
        // Java: this.addConfig(config, null);
        LIBMATTI_SP_MixinPlatformManager_AddConfig(manager, configs[i], NULL);
    }
}

// Java: public final void inject()
void LIBMATTI_SP_MixinPlatformManager_Inject(LIBMATTI_SP_MixinPlatformManager *manager)
{
    if (manager->injected) return;
    manager->injected = 1;

    if (manager->primaryContainer != NULL)
    {
        // Java: this.primaryContainer.initPrimaryContainer();
        LIBMATTI_SP_MixinContainer_InitPrimaryContainer(manager->primaryContainer);
    }

    // Java: this.scanForContainers();
    LIBMATTI_SP_MixinPlatformManager_ScanForContainers(manager);

    // Java: logger.debug("inject() running with {} agents", this.containers.size());
    //       for (MixinContainer container : this.containers.values()) { try { container.inject(); } ... }
    for (size_t i = 0; i < manager->containerCount; i++)
        LIBMATTI_SP_MixinContainer_Inject(manager->containers[i]);

    // Java: this.connectors.inject();
    LIBMATTI_SP_MixinConnectorManager_Inject(manager->connectors);
}

// Java: public String getLaunchTarget() { return MixinPlatformManager.DEFAULT_MAIN_CLASS; }
const char *LIBMATTI_SP_MixinPlatformManager_GetLaunchTarget(LIBMATTI_SP_MixinPlatformManager *manager)
{
    (void) manager;
    return DEFAULT_MAIN_CLASS;
}

// Java: @SuppressWarnings("deprecation") final void setCompatibilityLevel(String level)
void LIBMATTI_SP_MixinPlatformManager_SetCompatibilityLevel(LIBMATTI_SP_MixinPlatformManager *manager,
                                                            const char *level)
{
    (void) manager;
    if (level == NULL) return;

    // Java: CompatibilityLevel value = CompatibilityLevel.valueOf(level.toUpperCase(Locale.ROOT));
    //       MixinEnvironment.setCompatibilityLevel(value);
    char upper[64];
    size_t len = strlen(level);
    if (len >= sizeof(upper)) return;
    for (size_t i = 0; i <= len; i++)
    {
        char c = level[i];
        upper[i] = (c >= 'a' && c <= 'z') ? (char) (c - 'a' + 'A') : c;
    }

    LIBMATTI_SP_MixinEnvironment_CompatibilityLevel value =
        LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_ForName(upper);
    if (value == LIBMATTI_SP_MixinEnvironment_COMPAT_NONE)
    {
        // Java: catch (IllegalArgumentException ex) { logger.warn("Invalid compatibility level specified: {}", level); }
        char *msg = malloc(strlen("Invalid compatibility level specified: ") + strlen(level) + 1);
        sprintf(msg, "Invalid compatibility level specified: %s", level);
        LIBMATTI_SP_ILogger_Warn(LOGGER(), msg);
        free(msg);
        return;
    }

    // Java: logger.debug("Setting mixin compatibility level: {}", value);
    LIBMATTI_SP_MixinEnvironment_SetCompatibilityLevel(value);
}

// Java: final void addConfig(String config, IMixinConfigSource source)
void LIBMATTI_SP_MixinPlatformManager_AddConfig(LIBMATTI_SP_MixinPlatformManager *manager,
                                                const char *config, LIBMATTI_SP_ContainerHandle *source)
{
    (void) manager;
    if (config == NULL) return;

    size_t len = strlen(config);
    if (len >= 5 && strcmp(config + len - 5, ".json") == 0)
    {
        // Java: logger.debug("Registering mixin config: {} source={}", config, source);
        //       Mixins.addConfiguration(config, source);
        LIBMATTI_SP_Mixins_AddConfiguration(config);
    }
    else if (strstr(config, ".json@") != NULL)
    {
        // Java: throw new MixinError("Setting config phase via manifest is no longer supported: "
        //       + config + ". Specify target in config instead");
        char *msg = malloc(strlen("Setting config phase via manifest is no longer supported: . Specify target in config instead")
                           + len + 1);
        sprintf(msg, "Setting config phase via manifest is no longer supported: %s. Specify target in config instead", config);
        LIBMATTI_SP_ILogger_Error(LOGGER(), msg);
        free(msg);
    }
}

// Java: final void addTokenProvider(String provider)
void LIBMATTI_SP_MixinPlatformManager_AddTokenProvider(LIBMATTI_SP_MixinPlatformManager *manager,
                                                       const char *provider)
{
    (void) manager;
    if (provider == NULL) return;

    // Java: token providers resolve through the environment's class provider; the port's native
    // backend has no token providers, so the registration keeps the name for diagnostics only.
    if (strstr(provider, "@") != NULL)
    {
        // Java: String[] parts = provider.split("@", 2);
        //       Phase phase = Phase.forName(parts[1]);
        //       if (phase != null) MixinEnvironment.getEnvironment(phase).registerTokenProviderClass(parts[0]);
        const char *at = strchr(provider, '@');
        char *name = strndup(provider, at - provider);
        LIBMATTI_SP_MixinEnvironment_Phase *phase = LIBMATTI_SP_MixinEnvironment_Phase_ForName(at + 1);
        free(name);
    }
    else
    {
        // Java: MixinEnvironment.getDefaultEnvironment().registerTokenProviderClass(provider);
    }
}

// Java: final void addConnector(String connectorClass) { this.connectors.addConnector(connectorClass); }
void LIBMATTI_SP_MixinPlatformManager_AddConnector(LIBMATTI_SP_MixinPlatformManager *manager,
                                                   const char *connectorClass)
{
    LIBMATTI_SP_MixinConnectorManager_AddConnector(manager->connectors, connectorClass);
}
