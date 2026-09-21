// Port of net.neoforged.fml.common.asm.SimpleProcessorsGroup.
// A concrete ClassProcessor; it fills in the ClassProcessor vtable slots itself.

#ifndef MATTICRAFT_FML_COMMON_ASM_SIMPLEPROCESSORSGROUP_H
#define MATTICRAFT_FML_COMMON_ASM_SIMPLEPROCESSORSGROUP_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: public class SimpleProcessorsGroup implements ClassProcessor
typedef struct LIBMATTI_FML_SimpleProcessorsGroup
{
    // Java: implements ClassProcessor
    LIBMATTI_NEOFORGESPI_ClassProcessor processor;
} LIBMATTI_FML_SimpleProcessorsGroup;

// Java: public SimpleProcessorsGroup()
LIBMATTI_FML_SimpleProcessorsGroup *LIBMATTI_FML_SimpleProcessorsGroup_New(void);
void LIBMATTI_FML_SimpleProcessorsGroup_Free(LIBMATTI_FML_SimpleProcessorsGroup *group);

#endif //MATTICRAFT_FML_COMMON_ASM_SIMPLEPROCESSORSGROUP_H
