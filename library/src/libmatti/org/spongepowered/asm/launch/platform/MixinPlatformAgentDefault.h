// Port of org.spongepowered.asm.launch.platform.MixinPlatformAgentDefault.
// "Default platform agent, handles the mixin manifest keys such as MixinConfigs and
// MixinTokenProviders."

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMAGENTDEFAULT_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMAGENTDEFAULT_H

#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformAgentAbstract.h"

// Java: public class MixinPlatformAgentDefault extends MixinPlatformAgentAbstract
typedef struct
{
    LIBMATTI_SP_MixinPlatformAgentAbstract super;
} LIBMATTI_SP_MixinPlatformAgentDefault;

LIBMATTI_SP_MixinPlatformAgentDefault *LIBMATTI_SP_MixinPlatformAgentDefault_New(void);
void LIBMATTI_SP_MixinPlatformAgentDefault_Free(LIBMATTI_SP_MixinPlatformAgentDefault *agent);

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_MIXINPLATFORMAGENTDEFAULT_H
