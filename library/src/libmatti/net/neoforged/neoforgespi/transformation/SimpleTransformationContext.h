// Port of net.neoforged.neoforgespi.transformation.SimpleTransformationContext.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLETRANSFORMATIONCONTEXT_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLETRANSFORMATIONCONTEXT_H

#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: public interface SimpleTransformationContext
typedef struct LIBMATTI_NEOFORGESPI_SimpleTransformationContext
{
    // Java: Type type()
    const LIBMATTI_ASM_Type *(*type)(struct LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self);
    // Java: boolean empty()
    int (*empty)(struct LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self);
    // Java: byte[] initialSha256(); caller frees
    unsigned char *(*initialSha256)(struct LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self, size_t *length);
} LIBMATTI_NEOFORGESPI_SimpleTransformationContext;

// Java: the interface methods, dispatched to the implementing vtable
const LIBMATTI_ASM_Type *LIBMATTI_NEOFORGESPI_SimpleTransformationContext_Type(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context);
int LIBMATTI_NEOFORGESPI_SimpleTransformationContext_Empty(LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context);
unsigned char *LIBMATTI_NEOFORGESPI_SimpleTransformationContext_InitialSha256(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context, size_t *length);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLETRANSFORMATIONCONTEXT_H
