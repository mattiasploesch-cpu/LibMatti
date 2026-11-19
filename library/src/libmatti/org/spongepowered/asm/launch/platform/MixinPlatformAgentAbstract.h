




#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMAGENTABSTRACT_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMAGENTABSTRACT_H

#include "libmatti/org/spongepowered/asm/launch/platform/IMixinPlatformAgent.h"

// Java: protected MixinPlatformManager manager / protected IContainerHandle handle - the base
// state every agent carries; concrete agent structs embed this as their first member
typedef struct
{
    LIBMATTI_SP_IMixinPlatformAgent base;
    LIBMATTI_SP_MixinPlatformManager *manager;
    LIBMATTI_SP_ContainerHandle *handle;
} LIBMATTI_SP_MixinPlatformAgentAbstract;

// Java: protected MixinPlatformAgentAbstract() - fills the base behaviour into the struct
void LIBMATTI_SP_MixinPlatformAgentAbstract_Init(LIBMATTI_SP_MixinPlatformAgentAbstract *agent);

#endif
