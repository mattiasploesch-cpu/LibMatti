// Port of net.neoforged.neoforgespi.transformation.BytecodeProvider.

#include "libmatti/net/neoforged/neoforgespi/transformation/BytecodeProvider.h"

unsigned char *LIBMATTI_NEOFORGESPI_BytecodeProvider_GetByteCode(LIBMATTI_NEOFORGESPI_BytecodeProvider *provider,
                                                                 const char *className, size_t *length)
{
    return provider->getByteCode(provider, className, length);
}
