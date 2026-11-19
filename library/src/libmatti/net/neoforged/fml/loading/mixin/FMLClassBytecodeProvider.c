





#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassBytecodeProvider.h"

#include <stdlib.h>

// Java: FMLClassBytecodeProvider(BytecodeProvider bytecodeProvider, FMLMixinService service)
LIBMATTI_FML_FMLClassBytecodeProvider *LIBMATTI_FML_FMLClassBytecodeProvider_New(
    LIBMATTI_NEOFORGESPI_BytecodeProvider *bytecodeProvider)
{
    LIBMATTI_FML_FMLClassBytecodeProvider *provider = calloc(1, sizeof(*provider));
    provider->bytecodeProvider = bytecodeProvider;
    return provider;
}

void LIBMATTI_FML_FMLClassBytecodeProvider_Free(LIBMATTI_FML_FMLClassBytecodeProvider *provider)
{
    free(provider);
}
