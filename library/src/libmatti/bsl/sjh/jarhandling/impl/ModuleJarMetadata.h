//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.ModuleJarMetadata.

#ifndef MATTICRAFT_MODULEJARMETADATA_H
#define MATTICRAFT_MODULEJARMETADATA_H

#include "libmatti/bsl/sjh/jarhandling/LazyJarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/JarMetadata.h"

#include <stddef.h>

// Java: class ModuleJarMetadata extends LazyJarMetadata
// Reads the module descriptor from the module-info.class bytes through
// java.lang.module.ModuleDescriptor.read (the port's class-file parser).
typedef struct
{
    LIBMATTI_JH_LazyJarMetadata lazy;
    unsigned char *descriptorBytes;    // Java: the Supplier<byte[]> captured bytes
    size_t descriptorByteCount;
    LIBMATTI_JH_Provider *providers;   // Java: the META-INF/services providers
    size_t providerCount;
} LIBMATTI_JH_ModuleJarMetadata;

// Java: public ModuleJarMetadata(Supplier<byte[]> moduleInfoBytes, String[] targets)
LIBMATTI_JH_ModuleJarMetadata *LIBMATTI_JH_ModuleJarMetadata_Create(const unsigned char *descriptorBytes,
                                                                    size_t descriptorByteCount,
                                                                    LIBMATTI_JH_Provider *providers,
                                                                    size_t providerCount);
void LIBMATTI_JH_ModuleJarMetadata_Free(LIBMATTI_JH_ModuleJarMetadata *metadata);

#endif //MATTICRAFT_MODULEJARMETADATA_H
