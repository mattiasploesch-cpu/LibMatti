// Port of org.spongepowered.asm.launch.platform.IMixinPlatformAgent
// (sponge-mixin 0.17.3+mixin.0.8.7). The interface becomes the virtual-dispatch struct the C
// port uses for its interfaces.

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_IMIXINPLATFORMAGENT_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_IMIXINPLATFORMAGENT_H

#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"

typedef struct LIBMATTI_SP_MixinPlatformManager LIBMATTI_SP_MixinPlatformManager;

// Java: public enum AcceptResult
typedef enum
{
    LIBMATTI_SP_AcceptResult_ACCEPTED,
    LIBMATTI_SP_AcceptResult_REJECTED,
    LIBMATTI_SP_AcceptResult_INVALID
} LIBMATTI_SP_AcceptResult;

// Java: public interface IMixinPlatformAgent
typedef struct LIBMATTI_SP_IMixinPlatformAgent LIBMATTI_SP_IMixinPlatformAgent;

struct LIBMATTI_SP_IMixinPlatformAgent
{
    void *self;

    // Java: AcceptResult accept(MixinPlatformManager manager, IContainerHandle handle)
    LIBMATTI_SP_AcceptResult (*accept)(void *self, LIBMATTI_SP_MixinPlatformManager *manager,
                                       LIBMATTI_SP_ContainerHandle *handle);
    // Java: String getPhaseProvider() - NULL when none
    const char *(*getPhaseProvider)(void *self);
    // Java: void prepare()
    void (*prepare)(void *self);
    // Java: void initPrimaryContainer()
    void (*initPrimaryContainer)(void *self);
    // Java: void inject()
    void (*inject)(void *self);
};

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_IMIXINPLATFORMAGENT_H
