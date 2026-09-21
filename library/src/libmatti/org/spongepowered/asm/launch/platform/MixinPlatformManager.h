// Port of org.spongepowered.asm.launch.platform.MixinContainer and
// org.spongepowered.asm.launch.platform.MixinPlatformManager
// (sponge-mixin 0.17.3+mixin.0.8.7).

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMMANAGER_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMMANAGER_H

#include "libmatti/org/spongepowered/asm/launch/platform/IMixinPlatformAgent.h"
#include "libmatti/org/spongepowered/asm/launch/platform/MixinConnectorManager.h"
#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"

#include <stddef.h>

typedef struct LIBMATTI_SP_CommandLineOptions LIBMATTI_SP_CommandLineOptions;

// Java: public class MixinContainer
typedef struct LIBMATTI_SP_MixinContainer LIBMATTI_SP_MixinContainer;

struct LIBMATTI_SP_MixinContainer
{
    // Java: private final IContainerHandle handle
    LIBMATTI_SP_ContainerHandle *handle;
    // Java: private final List<IMixinPlatformAgent> agents
    LIBMATTI_SP_IMixinPlatformAgent **agents;
    size_t agentCount;
};

// Java: public MixinContainer(MixinPlatformManager manager, IContainerHandle handle)
LIBMATTI_SP_MixinContainer *LIBMATTI_SP_MixinContainer_New(LIBMATTI_SP_MixinPlatformManager *manager,
                                                           LIBMATTI_SP_ContainerHandle *handle);
void LIBMATTI_SP_MixinContainer_Free(LIBMATTI_SP_MixinContainer *container);

// Java: public IContainerHandle getDescriptor()
LIBMATTI_SP_ContainerHandle *LIBMATTI_SP_MixinContainer_GetDescriptor(const LIBMATTI_SP_MixinContainer *container);
// Java: public Collection<String> getPhaseProviders() - caller frees the strings
char **LIBMATTI_SP_MixinContainer_GetPhaseProviders(const LIBMATTI_SP_MixinContainer *container, size_t *count);
// Java: public void prepare()
void LIBMATTI_SP_MixinContainer_Prepare(LIBMATTI_SP_MixinContainer *container);
// Java: public void initPrimaryContainer()
void LIBMATTI_SP_MixinContainer_InitPrimaryContainer(LIBMATTI_SP_MixinContainer *container);
// Java: public void inject()
void LIBMATTI_SP_MixinContainer_Inject(LIBMATTI_SP_MixinContainer *container);

// Java: public class MixinPlatformManager
typedef struct LIBMATTI_SP_MixinPlatformManager LIBMATTI_SP_MixinPlatformManager;

struct LIBMATTI_SP_MixinPlatformManager
{
    // Java: private final Map<IContainerHandle, MixinContainer> containers
    LIBMATTI_SP_ContainerHandle **containerHandles;
    LIBMATTI_SP_MixinContainer **containers;
    size_t containerCount;
    // Java: private final MixinConnectorManager connectors
    LIBMATTI_SP_MixinConnectorManager *connectors;
    // Java: private MixinContainer primaryContainer
    LIBMATTI_SP_MixinContainer *primaryContainer;
    // Java: private boolean prepared
    int prepared;
    // Java: private boolean injected
    int injected;
};

// Java: public MixinPlatformManager()
LIBMATTI_SP_MixinPlatformManager *LIBMATTI_SP_MixinPlatformManager_New(void);
void LIBMATTI_SP_MixinPlatformManager_Free(LIBMATTI_SP_MixinPlatformManager *manager);

// Java: public void init()
void LIBMATTI_SP_MixinPlatformManager_Init(LIBMATTI_SP_MixinPlatformManager *manager);
// Java: public Collection<String> getPhaseProviderClasses()
char **LIBMATTI_SP_MixinPlatformManager_GetPhaseProviderClasses(LIBMATTI_SP_MixinPlatformManager *manager,
                                                                size_t *count);
// Java: public final MixinContainer addContainer(IContainerHandle handle)
LIBMATTI_SP_MixinContainer *LIBMATTI_SP_MixinPlatformManager_AddContainer(LIBMATTI_SP_MixinPlatformManager *manager,
                                                                          LIBMATTI_SP_ContainerHandle *handle);
// Java: public final void prepare(CommandLineOptions args)
void LIBMATTI_SP_MixinPlatformManager_Prepare(LIBMATTI_SP_MixinPlatformManager *manager,
                                              LIBMATTI_SP_CommandLineOptions *args);
// Java: public final void inject()
void LIBMATTI_SP_MixinPlatformManager_Inject(LIBMATTI_SP_MixinPlatformManager *manager);
// Java: public String getLaunchTarget()
const char *LIBMATTI_SP_MixinPlatformManager_GetLaunchTarget(LIBMATTI_SP_MixinPlatformManager *manager);
// Java: private void scanForContainers() (the bootstrap drives the early scan through init)
void LIBMATTI_SP_MixinPlatformManager_ScanForContainers(LIBMATTI_SP_MixinPlatformManager *manager);

// Java: final void setCompatibilityLevel(String level) (package-private; the agents call it)
void LIBMATTI_SP_MixinPlatformManager_SetCompatibilityLevel(LIBMATTI_SP_MixinPlatformManager *manager,
                                                            const char *level);
// Java: final void addConfig(String config, IMixinConfigSource source)
void LIBMATTI_SP_MixinPlatformManager_AddConfig(LIBMATTI_SP_MixinPlatformManager *manager,
                                                const char *config, LIBMATTI_SP_ContainerHandle *source);
// Java: final void addTokenProvider(String provider)
void LIBMATTI_SP_MixinPlatformManager_AddTokenProvider(LIBMATTI_SP_MixinPlatformManager *manager,
                                                       const char *provider);
// Java: final void addConnector(String connectorClass)
void LIBMATTI_SP_MixinPlatformManager_AddConnector(LIBMATTI_SP_MixinPlatformManager *manager,
                                                   const char *connectorClass);

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMMANAGER_H
