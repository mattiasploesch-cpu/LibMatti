// Port of net.minecraft.server.packs.repository.Pack and
// net.minecraft.server.packs.PackSelectionConfig.

#ifndef MATTICRAFT_MC_PACKS_REPOSITORY_PACK_H
#define MATTICRAFT_MC_PACKS_REPOSITORY_PACK_H

#include "libmatti/net/minecraft/server/packs/PackLocationInfo.h"
#include "libmatti/net/minecraft/server/packs/PackResources.h"

#include <stddef.h>

typedef struct LIBMATTI_MC_Pack Pack;

// Java: interface Pack.ResourcesSupplier
typedef struct LIBMATTI_MC_Pack_ResourcesSupplier
{
    // Java: PackResources open(PackLocationInfo) - the caller owns the result
    LIBMATTI_MC_PackResources *(*open)(const struct LIBMATTI_MC_Pack_ResourcesSupplier *self,
                                       LIBMATTI_MC_PackLocationInfo *location);
    // Java: PackResources openPrimary(PackLocationInfo) - the default delegates to open
    LIBMATTI_MC_PackResources *(*openPrimary)(const struct LIBMATTI_MC_Pack_ResourcesSupplier *self,
                                              LIBMATTI_MC_PackLocationInfo *location);
    void *userData;
} LIBMATTI_MC_Pack_ResourcesSupplier;

// Java: record PackSelectionConfig(boolean required, PackPosition fixedPosition, boolean hidden)
typedef struct LIBMATTI_MC_PackSelectionConfig
{
    int required;
    int fixedPosition; // Java: PackPosition, ONLY_FIXED/BELOW_NAME/AFTER_NAME -> 1 means fixed
    int hidden;
} LIBMATTI_MC_PackSelectionConfig;

// Java: enum PackPosition
typedef enum
{
    LIBMATTI_MC_PackPosition_ONLY_FIXED = 0,
    LIBMATTI_MC_PackPosition_BELOW_NAME,
    LIBMATTI_MC_PackPosition_AFTER_NAME
} LIBMATTI_MC_PackPosition;

struct LIBMATTI_MC_Pack
{
    // Java: private final PackLocationInfo location
    LIBMATTI_MC_PackLocationInfo *location;
    // Java: private final Pack.ResourcesSupplier resourcesSupplier
    const LIBMATTI_MC_Pack_ResourcesSupplier *resourcesSupplier;
    // Java: private final PackMetadataSection metadata
    LIBMATTI_MC_PackMetadataSection *metadata;
    // Java: private final PackSelectionConfig selectionConfig
    LIBMATTI_MC_PackSelectionConfig selectionConfig;
};

// Java: public static Pack readMetaAndCreate(PackLocationInfo, ResourcesSupplier, PackFormat, PackSelectionConfig)
Pack *LIBMATTI_MC_Pack_ReadMetaAndCreate(LIBMATTI_MC_PackLocationInfo *location,
                                         const LIBMATTI_MC_Pack_ResourcesSupplier *supplier,
                                         LIBMATTI_MC_InclusiveRange supportedFormats,
                                         LIBMATTI_MC_PackSelectionConfig config);
// Java: public static Pack create(PackLocationInfo, ResourcesSupplier, PackMetadataSection, PackSelectionConfig)
Pack *LIBMATTI_MC_Pack_Create(LIBMATTI_MC_PackLocationInfo *location,
                              const LIBMATTI_MC_Pack_ResourcesSupplier *supplier,
                              LIBMATTI_MC_PackMetadataSection *metadata, LIBMATTI_MC_PackSelectionConfig config);

// Java: public String getId()
const char *LIBMATTI_MC_Pack_GetId(const Pack *pack);
// Java: public PackLocationInfo location()
LIBMATTI_MC_PackLocationInfo *LIBMATTI_MC_Pack_Location(const Pack *pack);
// Java: public PackMetadataSection getMetadata()
const LIBMATTI_MC_PackMetadataSection *LIBMATTI_MC_Pack_GetMetadata(const Pack *pack);
// Java: public boolean isRequired()
int LIBMATTI_MC_Pack_IsRequired(const Pack *pack);
// Java: public boolean isFixedPosition()
int LIBMATTI_MC_Pack_IsFixedPosition(const Pack *pack);
// Java: public PackPosition getFixedPosition()
LIBMATTI_MC_PackPosition LIBMATTI_MC_Pack_GetFixedPosition(const Pack *pack);
// Java: public boolean isHidden()
int LIBMATTI_MC_Pack_IsHidden(const Pack *pack);
// Java: public PackResources open() - the caller closes
LIBMATTI_MC_PackResources *LIBMATTI_MC_Pack_Open(const Pack *pack);
// Java: public PackResources openFull(RepositorySource...) - the default delegates
LIBMATTI_MC_PackResources *LIBMATTI_MC_Pack_OpenFull(const Pack *pack, LIBMATTI_MC_PackResources *primary);

// Java: public int compareTo(Pack) - by PackSource ordering then id
int LIBMATTI_MC_Pack_CompareTo(const Pack *a, const Pack *b);

void LIBMATTI_MC_Pack_Free(Pack *pack);

#endif
