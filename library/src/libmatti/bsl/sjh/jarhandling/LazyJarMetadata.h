//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.LazyJarMetadata.

#ifndef MATTICRAFT_LAZYJARMETADATA_H
#define MATTICRAFT_LAZYJARMETADATA_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"

// Java: abstract class LazyJarMetadata implements JarMetadata
// The subclass provides the abstract computeDescriptor() via the function pointer.
typedef struct LIBMATTI_JH_LazyJarMetadata
{
    LIBMATTI_JL_ModuleDescriptor *descriptor; // cache, computed once
    LIBMATTI_JL_ModuleDescriptor *(*compute_descriptor)(struct LIBMATTI_JH_LazyJarMetadata *self);
} LIBMATTI_JH_LazyJarMetadata;

// Java: final ModuleDescriptor descriptor() - computes at most once
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_LazyJarMetadata_Descriptor(LIBMATTI_JH_LazyJarMetadata *lazy);

#endif //MATTICRAFT_LAZYJARMETADATA_H