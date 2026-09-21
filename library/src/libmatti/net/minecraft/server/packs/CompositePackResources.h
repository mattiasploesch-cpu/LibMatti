// Port of net.minecraft.server.packs.CompositePackResources.

#ifndef MATTICRAFT_MC_PACKS_COMPOSITEPACKRESOURCES_H
#define MATTICRAFT_MC_PACKS_COMPOSITEPACKRESOURCES_H

#include "libmatti/net/minecraft/server/packs/PackResources.h"

typedef struct LIBMATTI_MC_CompositePackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
    LIBMATTI_MC_PackLocationInfo *location;
    // Java: private final List<PackResources> stacks
    LIBMATTI_MC_PackResources **stacks;
    size_t stackCount;
} LIBMATTI_MC_CompositePackResources;

// Java: public CompositePackResources(PackLocationInfo info, List<PackResources> stacks)
LIBMATTI_MC_CompositePackResources *LIBMATTI_MC_CompositePackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                           LIBMATTI_MC_PackResources *const *stacks,
                                                                           size_t stackCount);
void LIBMATTI_MC_CompositePackResources_Free(LIBMATTI_MC_CompositePackResources *pack);

#endif
