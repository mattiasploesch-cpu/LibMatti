// Port of net.neoforged.fml.loading.mixin.FMLClassBytecodeProvider.
// Java reads ClassNodes through the bytecode provider; the port's "bytecode" for a class is the
// ELF image the class was registered from, so the provider keeps the BytecodeProvider handle the
// link context handed over (the close() path releases exactly this).

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
