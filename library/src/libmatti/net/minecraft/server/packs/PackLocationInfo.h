// Port of net.minecraft.server.packs.PackLocationInfo and
// net.minecraft.server.packs.repository.PackSource.

#ifndef MATTICRAFT_MC_PACKS_PACKLOCATIONINFO_H
#define MATTICRAFT_MC_PACKS_PACKLOCATIONINFO_H

#include "libmatti/net/minecraft/server/packs/repository/PackFormat.h"

typedef struct LIBMATTI_MC_PackSource LIBMATTI_MC_PackSource;

// Java: interface PackSource - the built-ins are indexed enum constants
typedef enum
{
    LIBMATTI_MC_PACK_SOURCE_NONE = 0,
    LIBMATTI_MC_PACK_SOURCE_BUILT_IN,
    LIBMATTI_MC_PACK_SOURCE_VANILLA,
    LIBMATTI_MC_PACK_SOURCE_WORLD,
    LIBMATTI_MC_PACK_SOURCE_SERVER,
    LIBMATTI_MC_PACK_SOURCE_DEFAULT,
    LIBMATTI_MC_PACK_SOURCE_BUILT_IN_OLD,
    LIBMATTI_MC_PACK_SOURCE_FEATURE,
    LIBMATTI_MC_PACK_SOURCE_TEST,
    LIBMATTI_MC_PACK_SOURCE_UNKNOWN_PACK
} LIBMATTI_MC_PackSourceType;

// Java: interface PackSource
struct LIBMATTI_MC_PackSource
{
    LIBMATTI_MC_PackSourceType type;
    // Java: Component decorated() - the port keeps the decoration description
    const char *decoration;
    // Java: boolean shouldBePinned()
    int shouldBePinned;
    // Java: boolean isForced()
    int isForced;
};

// Java: PackSource.BUILT_IN etc.
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_None(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_BuiltIn(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Vanilla(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_World(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Server(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Default(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_BuiltInOld(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Feature(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Test(void);
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_UnknownPack(void);

// Java: record PackLocationInfo(String id, Component title, PackSource source,
//       Optional<KnownPack> knownPackInfo)
typedef struct LIBMATTI_MC_PackLocationInfo
{
    char *id;
    char *title;
    const LIBMATTI_MC_PackSource *source;
    // Java: Optional<KnownPack> - the port keeps the optional fields
    int hasKnownPack;
    char *knownPackNamespace;
    char *knownPackId;
    char *knownPackVersion;
} LIBMATTI_MC_PackLocationInfo;

// Both malloc the strings; locationInfo must be freed with ..._Free
LIBMATTI_MC_PackLocationInfo *LIBMATTI_MC_PackLocationInfo_New(const char *id, const char *title,
                                                               const LIBMATTI_MC_PackSource *source);
void LIBMATTI_MC_PackLocationInfo_Free(LIBMATTI_MC_PackLocationInfo *info);

#endif
