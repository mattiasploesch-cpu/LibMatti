// Port of java.lang.module.Configuration.

#ifndef MATTICRAFT_CONFIGURATION_H
#define MATTICRAFT_CONFIGURATION_H

#include "libmatti/java/lang/module/ModuleReference.h"
#include "libmatti/java/lang/module/ResolvedModule.h"

#include <stddef.h>

// Java: class Configuration
typedef struct LIBMATTI_JL_Configuration
{
    LIBMATTI_JL_ResolvedModule **modules; // Java: Set<ResolvedModule> modules()
    size_t moduleCount;
} LIBMATTI_JL_Configuration;

// Java: Set<ResolvedModule> modules() - internal array, do not free
LIBMATTI_JL_ResolvedModule **LIBMATTI_JL_Configuration_Modules(const LIBMATTI_JL_Configuration *configuration, size_t *count);
// Java: Optional<ResolvedModule> findModule(String name) (NULL = empty)
LIBMATTI_JL_ResolvedModule *LIBMATTI_JL_Configuration_FindModule(const LIBMATTI_JL_Configuration *configuration, const char *name);

// Java: static Configuration resolve(ModuleFinder before, ModuleFinder after, Collection<String> roots)
// 'references' plays the role of the 'before' finder's findAll(); the 'after' finder
// (ModuleFinder.ofSystem(), the JVM's boot modules) is external and omitted.
// Returns NULL when a root or required module is not found (Java: FindException).
LIBMATTI_JL_Configuration *LIBMATTI_JL_Configuration_Resolve(LIBMATTI_JL_ModuleReference **references, size_t referenceCount,
                                                             const char **roots, size_t rootCount);

// Java: static Configuration resolveAndBind(ModuleFinder before, ModuleFinder after, Collection<String> roots)
// Like resolve(), plus the service-binding pass: modules that 'use' a service gain
// read edges to every module that 'provides' it (Java: Resolver.bind).
LIBMATTI_JL_Configuration *LIBMATTI_JL_Configuration_ResolveAndBind(LIBMATTI_JL_ModuleReference **references,
                                                                     size_t referenceCount,
                                                                     const char **roots, size_t rootCount);

void LIBMATTI_JL_Configuration_Free(LIBMATTI_JL_Configuration *configuration);

#endif //MATTICRAFT_CONFIGURATION_H