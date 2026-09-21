// Port of java.lang.module.ResolvedModule.

#ifndef MATTICRAFT_RESOLVEDMODULE_H
#define MATTICRAFT_RESOLVEDMODULE_H

#include "libmatti/java/lang/module/ModuleReference.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_Configuration LIBMATTI_JL_Configuration;

// Java: class ResolvedModule
typedef struct LIBMATTI_JL_ResolvedModule
{
    LIBMATTI_JL_Configuration *configuration; // Java: Configuration configuration()
    char *name;                               // Java: String name()
    LIBMATTI_JL_ModuleReference *reference;   // Java: ModuleReference reference() (borrowed)
    struct LIBMATTI_JL_ResolvedModule **reads; // Java: Set<ResolvedModule> reads() (borrowed)
    size_t readCount;
} LIBMATTI_JL_ResolvedModule;

// Java: Configuration configuration()
LIBMATTI_JL_Configuration *LIBMATTI_JL_ResolvedModule_Configuration(const LIBMATTI_JL_ResolvedModule *module);
// Java: String name()
const char *LIBMATTI_JL_ResolvedModule_Name(const LIBMATTI_JL_ResolvedModule *module);
// Java: ModuleReference reference()
LIBMATTI_JL_ModuleReference *LIBMATTI_JL_ResolvedModule_Reference(const LIBMATTI_JL_ResolvedModule *module);
// Java: Set<ResolvedModule> reads() - internal array, do not free
LIBMATTI_JL_ResolvedModule **LIBMATTI_JL_ResolvedModule_Reads(const LIBMATTI_JL_ResolvedModule *module, size_t *count);

#endif //MATTICRAFT_RESOLVEDMODULE_H