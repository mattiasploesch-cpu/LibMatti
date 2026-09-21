//
// Created by administrator on 09.09.26.
//
// Port of java.lang.module.ModuleReference.

#ifndef MATTICRAFT_MODULEREFERENCE_H
#define MATTICRAFT_MODULEREFERENCE_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/java/net/URI.h"

// Java: abstract class ModuleReference
typedef struct
{
    LIBMATTI_JL_ModuleDescriptor *descriptor;
    LIBMATTI_JN_URI *location; // Java: URI location
} LIBMATTI_JL_ModuleReference;

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleReference_Descriptor(const LIBMATTI_JL_ModuleReference *reference);
// Java: Optional<URI> location() (NULL = empty)
const LIBMATTI_JN_URI *LIBMATTI_JL_ModuleReference_Location(const LIBMATTI_JL_ModuleReference *reference);
// Java: abstract ModuleReader open() - provided by subclasses

#endif //MATTICRAFT_MODULEREFERENCE_H