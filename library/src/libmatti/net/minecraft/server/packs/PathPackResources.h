// Port of net.minecraft.server.packs.PathPackResources (NeoForge adds the
// directory/listfile modes; vanilla's folder pack is mode DIRECTORY).

#ifndef MATTICRAFT_MC_PACKS_PATHPACKRESOURCES_H
#define MATTICRAFT_MC_PACKS_PATHPACKRESOURCES_H

#include "libmatti/net/minecraft/server/packs/PackResources.h"

typedef enum
{
    LIBMATTI_MC_PathPackResources_MODE_DIRECTORY = 0,
    LIBMATTI_MC_PathPackResources_MODE_LISTFILE
} LIBMATTI_MC_PathPackResources_Mode;

typedef struct LIBMATTI_MC_PathPackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
    LIBMATTI_MC_PackLocationInfo *location;
    // Java: public final Path source
    char *source;
    // Java: public final boolean isBuiltin
    int isBuiltin;
    // NeoForge: the pack mode
    LIBMATTI_MC_PathPackResources_Mode mode;
} LIBMATTI_MC_PathPackResources;

// Java: public PathPackResources(PackLocationInfo info, Path source, boolean isBuiltin)
LIBMATTI_MC_PathPackResources *LIBMATTI_MC_PathPackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                 const char *source, int isBuiltin);
// NeoForge constructor with the mode parameter
LIBMATTI_MC_PathPackResources *LIBMATTI_MC_PathPackResources_NewWithMode(LIBMATTI_MC_PackLocationInfo *location,
                                                                         const char *source, int isBuiltin,
                                                                         LIBMATTI_MC_PathPackResources_Mode mode);

void LIBMATTI_MC_PathPackResources_Free(LIBMATTI_MC_PathPackResources *pack);

#endif
