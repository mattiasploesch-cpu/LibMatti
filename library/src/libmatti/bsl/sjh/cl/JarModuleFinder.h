// Port of cpw.mods.cl.JarModuleFinder.

#ifndef MATTICRAFT_JARMODULEFINDER_H
#define MATTICRAFT_JARMODULEFINDER_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/java/lang/module/ModuleReference.h"

#include <stddef.h>

// Java: SecureJar.ModuleDataProvider
typedef LIBMATTI_JH_ModuleDataProvider LIBMATTI_CL_ModuleDataProvider;

// Java: static class JarModuleReader implements ModuleReader (defined in JarModuleFinder.c)
typedef struct LIBMATTI_CL_JarModuleReader LIBMATTI_CL_JarModuleReader;

// Java: static class JarModuleReference extends ModuleReference
typedef struct
{
    LIBMATTI_JL_ModuleReference base; // Java: super(jar.descriptor(), jar.uri())
    LIBMATTI_CL_ModuleDataProvider *jar;
} LIBMATTI_CL_JarModuleReference;

// Java: class JarModuleFinder implements ModuleFinder
typedef struct
{
    char **names;                              // moduleReferenceMap keys
    LIBMATTI_CL_JarModuleReference **references; // moduleReferenceMap values
    size_t count;
} LIBMATTI_CL_JarModuleFinder;

// Java: static JarModuleFinder of(SecureJar... jars)
LIBMATTI_CL_JarModuleFinder *LIBMATTI_CL_JarModuleFinder_Of(LIBMATTI_JH_SecureJar **jars, size_t jarCount);
void LIBMATTI_CL_JarModuleFinder_Free(LIBMATTI_CL_JarModuleFinder *finder);

// Java: Optional<ModuleReference> find(String name) (NULL = empty optional)
LIBMATTI_CL_JarModuleReference *LIBMATTI_CL_JarModuleFinder_Find(const LIBMATTI_CL_JarModuleFinder *finder, const char *name);

// Java: Set<ModuleReference> findAll() -> new HashSet<>(values); caller frees the array
LIBMATTI_CL_JarModuleReference **LIBMATTI_CL_JarModuleFinder_FindAll(const LIBMATTI_CL_JarModuleFinder *finder, size_t *count);

// Java: ModuleDataProvider jar()
LIBMATTI_CL_ModuleDataProvider *LIBMATTI_CL_JarModuleReference_Jar(const LIBMATTI_CL_JarModuleReference *reference);

// Java: ModuleReader open() -> new JarModuleReader(jar)
LIBMATTI_CL_JarModuleReader *LIBMATTI_CL_JarModuleReference_Open(const LIBMATTI_CL_JarModuleReference *reference);
void LIBMATTI_CL_JarModuleReader_Free(LIBMATTI_CL_JarModuleReader *reader);

// Java: static class JarModuleReader implements ModuleReader
// Java: Optional<URI> find(String name) { return jar.findFile(name); }
LIBMATTI_JN_URI *LIBMATTI_CL_JarModuleReader_Find(const LIBMATTI_CL_JarModuleReader *reader, const char *name);
// Java: Optional<InputStream> open(String name) { return jar.open(name); }
LIBMATTI_JI_InputStream *LIBMATTI_CL_JarModuleReader_Open(const LIBMATTI_CL_JarModuleReader *reader, const char *name);
// Java: Stream<String> list() { return null; }
void *LIBMATTI_CL_JarModuleReader_List(const LIBMATTI_CL_JarModuleReader *reader);
// Java: void close() {}
void LIBMATTI_CL_JarModuleReader_Close(const LIBMATTI_CL_JarModuleReader *reader);
// Java: String toString() -> getClass().getName() + "[jar=" + jar + "]"; caller frees
char *LIBMATTI_CL_JarModuleReader_ToString(const LIBMATTI_CL_JarModuleReader *reader);

#endif //MATTICRAFT_JARMODULEFINDER_H