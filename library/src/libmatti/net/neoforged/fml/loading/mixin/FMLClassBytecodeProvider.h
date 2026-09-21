// Port of net.neoforged.fml.loading.mixin.FMLClassBytecodeProvider.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSBYTECODEPROVIDER_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSBYTECODEPROVIDER_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BytecodeProvider.h"

// Java: class FMLClassBytecodeProvider implements IClassBytecodeProvider
typedef struct
{
    // Java: private final BytecodeProvider bytecodeProvider
    LIBMATTI_NEOFORGESPI_BytecodeProvider *bytecodeProvider;
} LIBMATTI_FML_FMLClassBytecodeProvider;

LIBMATTI_FML_FMLClassBytecodeProvider *LIBMATTI_FML_FMLClassBytecodeProvider_New(
    LIBMATTI_NEOFORGESPI_BytecodeProvider *bytecodeProvider);
void LIBMATTI_FML_FMLClassBytecodeProvider_Free(LIBMATTI_FML_FMLClassBytecodeProvider *provider);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSBYTECODEPROVIDER_H
