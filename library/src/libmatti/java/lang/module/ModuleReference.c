//
// Created by administrator on 09.09.26.
//

#include "ModuleReference.h"

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleReference_Descriptor(const LIBMATTI_JL_ModuleReference *reference)
{
    return reference->descriptor;
}

const LIBMATTI_JN_URI *LIBMATTI_JL_ModuleReference_Location(const LIBMATTI_JL_ModuleReference *reference)
{
    // Java: Optional.ofNullable(location)
    return reference->location;
}