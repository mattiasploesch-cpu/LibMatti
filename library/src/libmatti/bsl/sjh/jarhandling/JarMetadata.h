//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.JarMetadata.

#ifndef MATTICRAFT_JARMETADATA_H
#define MATTICRAFT_JARMETADATA_H

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/bsl/sjh/jarhandling/NameAndVersion.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/java/lang/module/ModuleDescriptor.h"

#include <stddef.h>

// Java: interface JarMetadata
// Implemented by SimpleJarMetadata (ModuleJarMetadata is TODO).
typedef struct LIBMATTI_JH_JarMetadata LIBMATTI_JH_JarMetadata;

const char *LIBMATTI_JH_JarMetadata_Name(const LIBMATTI_JH_JarMetadata *metadata);
// Java: @Nullable String version()
const char *LIBMATTI_JH_JarMetadata_Version(const LIBMATTI_JH_JarMetadata *metadata);
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_JarMetadata_Descriptor(const LIBMATTI_JH_JarMetadata *metadata);
// Java: List<Provider> providers() - returns the internal array, do not free
LIBMATTI_JH_Provider *LIBMATTI_JH_JarMetadata_Providers(const LIBMATTI_JH_JarMetadata *metadata, size_t *count);
void LIBMATTI_JH_JarMetadata_Free(LIBMATTI_JH_JarMetadata *metadata);

// Java: private static NameAndVersion computeNameAndVersion(Path path)
LIBMATTI_JH_NameAndVersion *LIBMATTI_JH_JarMetadata_ComputeNameAndVersion(const char *path);

// Java: static JarMetadata from(JarContents jar)
LIBMATTI_JH_JarMetadata *LIBMATTI_JH_JarMetadata_From(LIBMATTI_JH_JarContents *contents);

#endif //MATTICRAFT_JARMETADATA_H