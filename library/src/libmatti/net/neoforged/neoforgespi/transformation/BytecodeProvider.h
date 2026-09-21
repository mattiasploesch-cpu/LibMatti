// Port of net.neoforged.neoforgespi.transformation.BytecodeProvider.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BYTECODEPROVIDER_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BYTECODEPROVIDER_H

#include <stddef.h>

// Java: public interface BytecodeProvider
typedef struct LIBMATTI_NEOFORGESPI_BytecodeProvider
{
    // Java: byte[] getByteCode(String className) throws ClassNotFoundException; caller frees.
    // NULL = ClassNotFoundException.
    unsigned char *(*getByteCode)(struct LIBMATTI_NEOFORGESPI_BytecodeProvider *self, const char *className,
                                  size_t *length);
} LIBMATTI_NEOFORGESPI_BytecodeProvider;

// Java: the interface method, dispatched to the implementing vtable
unsigned char *LIBMATTI_NEOFORGESPI_BytecodeProvider_GetByteCode(LIBMATTI_NEOFORGESPI_BytecodeProvider *provider,
                                                                 const char *className, size_t *length);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BYTECODEPROVIDER_H
