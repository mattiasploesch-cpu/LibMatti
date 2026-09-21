// Port of net.minecraft.core.RegistrationInfo.
// Java: public record RegistrationInfo(Optional<KnownPack> knownPackInfo, Lifecycle lifecycle)
// KnownPack is metadata for pack-sourced entries; the port carries the optional as a flag.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_REGISTRATIONINFO_H
#define MATTICRAFT_NET_MINECRAFT_CORE_REGISTRATIONINFO_H

#include "libmatti/com/mojang/serialization/Lifecycle.h"

typedef struct LIBMATTI_MC_RegistrationInfo
{
    // Java: Optional<KnownPack> knownPackInfo - empty in the port until packs exist
    int hasKnownPackInfo;
    // Java: Lifecycle lifecycle
    LIBMATTI_MJS_Lifecycle lifecycle;
} LIBMATTI_MC_RegistrationInfo;

// Java: public static final RegistrationInfo BUILT_IN = new RegistrationInfo(Optional.empty(), Lifecycle.stable())
LIBMATTI_MC_RegistrationInfo LIBMATTI_MC_RegistrationInfo_BuiltIn(void);

// Java: the record constructor
LIBMATTI_MC_RegistrationInfo LIBMATTI_MC_RegistrationInfo_New(int hasKnownPackInfo, LIBMATTI_MJS_Lifecycle lifecycle);

#endif //MATTICRAFT_NET_MINECRAFT_CORE_REGISTRATIONINFO_H
