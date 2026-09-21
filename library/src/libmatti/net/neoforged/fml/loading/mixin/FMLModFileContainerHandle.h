// Port of net.neoforged.fml.loading.mixin.FMLModFileContainerHandle.
// "We use this to expose mod files that are not mods to Mixin, purely to allow it scanning for
// manifest attributes so game libraries/plugins can also contribute Mixins."

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMODFILECONTAINERHANDLE_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMODFILECONTAINERHANDLE_H

#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"

typedef struct LIBMATTI_NEOFORGESPI_IModFile LIBMATTI_NEOFORGESPI_IModFile;

// Java: final class FMLModFileContainerHandle implements IContainerHandle
typedef struct
{
    LIBMATTI_SP_ContainerHandle base;
    // Java: private final IModFile modFile
    LIBMATTI_NEOFORGESPI_IModFile *modFile;
} LIBMATTI_FML_FMLModFileContainerHandle;

// Java: public FMLModFileContainerHandle(IModFile modFile)
LIBMATTI_FML_FMLModFileContainerHandle *LIBMATTI_FML_FMLModFileContainerHandle_New(
    LIBMATTI_NEOFORGESPI_IModFile *modFile);
void LIBMATTI_FML_FMLModFileContainerHandle_Free(LIBMATTI_FML_FMLModFileContainerHandle *handle);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMODFILECONTAINERHANDLE_H
