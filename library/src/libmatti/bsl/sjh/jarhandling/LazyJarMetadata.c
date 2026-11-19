




#include "LazyJarMetadata.h"

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_LazyJarMetadata_Descriptor(LIBMATTI_JH_LazyJarMetadata *lazy)
{
// Java: if (descriptor == null) descriptor = computeDescriptor();
    if (lazy->descriptor == NULL)
        lazy->descriptor = lazy->compute_descriptor(lazy);

    return lazy->descriptor;
}
