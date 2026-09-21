// Port of org.spongepowered.asm.launch.platform.IMixinPlatformAgent's connector half:
// org.spongepowered.asm.mixin.connect.IMixinConnector and
// org.spongepowered.asm.launch.platform.MixinConnectorManager.

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINCONNECTORMANAGER_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINCONNECTORMANAGER_H

#include <stddef.h>

// Java: public interface IMixinConnector
typedef struct
{
    void *self;
    // Java: void connect()
    void (*connect)(void *self);
} LIBMATTI_SP_IMixinConnector;

// Java: public class MixinConnectorManager
typedef struct LIBMATTI_SP_MixinConnectorManager LIBMATTI_SP_MixinConnectorManager;

struct LIBMATTI_SP_MixinConnectorManager
{
    // Java: private final Set<String> connectorClasses (LinkedHashSet)
    char **connectorClasses;
    size_t connectorClassCount;
    // Java: private final List<IMixinConnector> connectors
    LIBMATTI_SP_IMixinConnector **connectors;
    size_t connectorCount;
};

// Java: MixinConnectorManager() (package-private)
LIBMATTI_SP_MixinConnectorManager *LIBMATTI_SP_MixinConnectorManager_New(void);
void LIBMATTI_SP_MixinConnectorManager_Free(LIBMATTI_SP_MixinConnectorManager *manager);

// Java: void addConnector(String connectorClass)
void LIBMATTI_SP_MixinConnectorManager_AddConnector(LIBMATTI_SP_MixinConnectorManager *manager,
                                                    const char *connectorClass);
// Java: void inject() (package-private)
void LIBMATTI_SP_MixinConnectorManager_Inject(LIBMATTI_SP_MixinConnectorManager *manager);

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINCONNECTORMANAGER_H
