//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.ModuleJarMetadata.
// Java: reads the module descriptor from the module-info.class bytes through
// ModuleDescriptor.read and derives name/version/packages from it.

#include "ModuleJarMetadata.h"

#include "libmatti/java/lang/module/ModuleDescriptorRead.h"

#include <stdlib.h>
#include <string.h>

// Java: private final Supplier<ModuleDescriptor> descriptorSupplier -
// the port computes the descriptor from the captured bytes
static LIBMATTI_JL_ModuleDescriptor *compute_descriptor(LIBMATTI_JH_LazyJarMetadata *lazy)
{
    LIBMATTI_JH_ModuleJarMetadata *metadata = (LIBMATTI_JH_ModuleJarMetadata *) lazy;
    return LIBMATTI_JL_ModuleDescriptor_Read(metadata->descriptorBytes, metadata->descriptorByteCount);
}

// Java: public ModuleJarMetadata(Supplier<byte[]> moduleInfoBytes, String[] targets) -
// the port receives the already-read module-info.class bytes
LIBMATTI_JH_ModuleJarMetadata *LIBMATTI_JH_ModuleJarMetadata_Create(const unsigned char *descriptorBytes,
                                                                    size_t descriptorByteCount,
                                                                    LIBMATTI_JH_Provider *providers,
                                                                    size_t providerCount)
{
    LIBMATTI_JH_ModuleJarMetadata *metadata = calloc(1, sizeof(LIBMATTI_JH_ModuleJarMetadata));
    metadata->descriptorBytes = malloc(descriptorByteCount > 0 ? descriptorByteCount : 1);
    memcpy(metadata->descriptorBytes, descriptorBytes, descriptorByteCount);
    metadata->descriptorByteCount = descriptorByteCount;
    metadata->providers = providers;
    metadata->providerCount = providerCount;

    metadata->lazy.compute_descriptor = compute_descriptor;
    return metadata;
}

void LIBMATTI_JH_ModuleJarMetadata_Free(LIBMATTI_JH_ModuleJarMetadata *metadata)
{
    if (metadata == NULL)
        return;
    free(metadata->descriptorBytes);
    free(metadata);
}
