// Port of net.minecraft.core.RegistrationInfo.

#include "libmatti/net/minecraft/core/RegistrationInfo.h"

// Java: public static final RegistrationInfo BUILT_IN
LIBMATTI_MC_RegistrationInfo LIBMATTI_MC_RegistrationInfo_BuiltIn(void)
{
    return LIBMATTI_MC_RegistrationInfo_New(0, LIBMATTI_MJS_Lifecycle_Stable());
}

// Java: the record constructor
LIBMATTI_MC_RegistrationInfo LIBMATTI_MC_RegistrationInfo_New(int hasKnownPackInfo, LIBMATTI_MJS_Lifecycle lifecycle)
{
    LIBMATTI_MC_RegistrationInfo info;
    info.hasKnownPackInfo = hasKnownPackInfo;
    info.lifecycle = lifecycle;
    return info;
}
