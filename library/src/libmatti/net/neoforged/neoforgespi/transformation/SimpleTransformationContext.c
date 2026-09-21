// Port of net.neoforged.neoforgespi.transformation.SimpleTransformationContext.

#include "libmatti/net/neoforged/neoforgespi/transformation/SimpleTransformationContext.h"

const LIBMATTI_ASM_Type *LIBMATTI_NEOFORGESPI_SimpleTransformationContext_Type(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context)
{
    return context->type(context);
}

int LIBMATTI_NEOFORGESPI_SimpleTransformationContext_Empty(LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context)
{
    return context->empty(context);
}

unsigned char *LIBMATTI_NEOFORGESPI_SimpleTransformationContext_InitialSha256(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context, size_t *length)
{
    return context->initialSha256(context, length);
}
