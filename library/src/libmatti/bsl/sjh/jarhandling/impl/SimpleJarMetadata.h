//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.SimpleJarMetadata.

#ifndef MATTICRAFT_SIMPLEJARMETADATA_H
#define MATTICRAFT_SIMPLEJARMETADATA_H

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/bsl/sjh/jarhandling/LazyJarMetadata.h"

#include <stddef.h>

// Java: class SimpleJarMetadata extends LazyJarMetadata implements JarMetadata
// Metadata for a non-modular jar, turned into an automatic module.
typedef struct
{
    LIBMATTI_JH_LazyJarMetadata lazy; // extends LazyJarMetadata
    char *name;
    char *version; // NULL = no version
    LIBMATTI_JH_JarContents *contents; // Java: Supplier<Set<String>> packagesSupplier
    // Java: providers.stream().filter(p -> !p.providers().isEmpty()).toList()
    // Shallow copies referencing the contents' provider data
    LIBMATTI_JH_Provider *providers;
    size_t providerCount;
} LIBMATTI_JH_SimpleJarMetadata;

// Java: SimpleJarMetadata(String name, String version, Supplier<Set<String>>, List<Provider>)
LIBMATTI_JH_SimpleJarMetadata *LIBMATTI_JH_SimpleJarMetadata_Create(const char *name, const char *version,
                                                                    LIBMATTI_JH_JarContents *contents,
                                                                    LIBMATTI_JH_Provider *providers, size_t providerCount);

#endif //MATTICRAFT_SIMPLEJARMETADATA_H