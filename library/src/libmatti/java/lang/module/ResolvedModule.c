#include "ResolvedModule.h"

#include <stdlib.h>

LIBMATTI_JL_Configuration *LIBMATTI_JL_ResolvedModule_Configuration(const LIBMATTI_JL_ResolvedModule *module)
{
    return module->configuration;
}

const char *LIBMATTI_JL_ResolvedModule_Name(const LIBMATTI_JL_ResolvedModule *module)
{
    return module->name;
}

LIBMATTI_JL_ModuleReference *LIBMATTI_JL_ResolvedModule_Reference(const LIBMATTI_JL_ResolvedModule *module)
{
    return module->reference;
}

LIBMATTI_JL_ResolvedModule **LIBMATTI_JL_ResolvedModule_Reads(const LIBMATTI_JL_ResolvedModule *module, size_t *count)
{
    *count = module->readCount;
    return module->reads;
}