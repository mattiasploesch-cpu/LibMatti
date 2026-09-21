// Port of java.lang.module.ModuleDescriptor.read (the module-info.class
// binary reader, Java: ModuleDescriptor.read(ByteBuffer) ->
// ModuleInfo.read). This is the class-file attribute parser the JVM uses;
// the port parses the attribute set directly from the bytes.

#ifndef MATTICRAFT_MODULEDESCRIPTORREAD_H
#define MATTICRAFT_MODULEDESCRIPTORREAD_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"

#include <stddef.h>
#include <stdint.h>

// Java: public static ModuleDescriptor read(ByteBuffer bb) - the module descriptor
// of a module-info.class; NULL on malformed input (Java: InvalidModuleDescriptorException)
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_Read(const unsigned char *bytes, size_t length);

// Java: the descriptor version this reader accepts (Java: module-info files of
// version 51.0..current); 0 = accepts any (the port skips the strict check)
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_ReadWithFlags(const unsigned char *bytes, size_t length,
                                                                         int strict);

#endif //MATTICRAFT_MODULEDESCRIPTORREAD_H
