// Port of net.minecraft.server.packs.FilePackResources (the ZIP pack).

#ifndef MATTICRAFT_MC_PACKS_FILEPACKRESOURCES_H
#define MATTICRAFT_MC_PACKS_FILEPACKRESOURCES_H

#include "libmatti/java/util/zip/ZipFile.h"
#include "libmatti/net/minecraft/server/packs/PackResources.h"

typedef struct LIBMATTI_MC_FilePackResources_FileBackup
{
    LIBMATTI_JU_ZipFile *zip;
    int opened;
} LIBMATTI_MC_FilePackResources_FileBackup;

typedef struct LIBMATTI_MC_FilePackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
    LIBMATTI_MC_PackLocationInfo *location;
    // Java: private final File source
    char *source;
    // Java: private final boolean isBuiltin
    int isBuiltin;
    // Java: private FilePackResources.FileBackup backup (the ZipFile, opened lazily)
    LIBMATTI_MC_FilePackResources_FileBackup *backup;
} LIBMATTI_MC_FilePackResources;

// Java: public FilePackResources(PackLocationInfo info, File source, boolean isBuiltin)
LIBMATTI_MC_FilePackResources *LIBMATTI_MC_FilePackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                 const char *source, int isBuiltin);
void LIBMATTI_MC_FilePackResources_Free(LIBMATTI_MC_FilePackResources *pack);

#endif
