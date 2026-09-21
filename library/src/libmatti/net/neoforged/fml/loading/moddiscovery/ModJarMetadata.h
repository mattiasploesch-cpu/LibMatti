// Port of net.neoforged.fml.loading.moddiscovery.ModJarMetadata.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODJARMETADATA_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODJARMETADATA_H

#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

typedef struct LIBMATTI_FML_ModJarMetadata LIBMATTI_FML_ModJarMetadata;

struct LIBMATTI_FML_ModJarMetadata
{
    LIBMATTI_FML_JarModuleInfo base;

    // Java: private IModFile modFile
    LIBMATTI_NEOFORGESPI_IModFile *modFile;
};

LIBMATTI_FML_ModJarMetadata *LIBMATTI_FML_ModJarMetadata_New(void);
// Java: public void setModFile(IModFile file)
void LIBMATTI_FML_ModJarMetadata_SetModFile(LIBMATTI_FML_ModJarMetadata *metadata,
                                            LIBMATTI_NEOFORGESPI_IModFile *file);
// Java: public IModFile modFile()
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_ModJarMetadata_ModFile(const LIBMATTI_FML_ModJarMetadata *metadata);
// Java: the instance used as a JarModuleInfo
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_ModJarMetadata_AsJarModuleInfo(LIBMATTI_FML_ModJarMetadata *metadata);
// Java: @Override public String toString()
char *LIBMATTI_FML_ModJarMetadata_ToString(const LIBMATTI_FML_ModJarMetadata *metadata);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODJARMETADATA_H
