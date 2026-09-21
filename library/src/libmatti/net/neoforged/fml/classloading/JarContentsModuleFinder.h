// Port of net.neoforged.fml.classloading.JarContentsModuleFinder.

#ifndef MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEFINDER_H
#define MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEFINDER_H

#include "libmatti/net/neoforged/fml/classloading/JarContentsModule.h"
#include "libmatti/net/neoforged/fml/classloading/JarContentsModuleReference.h"

#include <stddef.h>

// Java: public final class JarContentsModuleFinder implements ModuleFinder
typedef struct
{
    // Java: Map<String, ModuleReference> jars
    char **names;
    LIBMATTI_FML_JarContentsModuleReference **references;
    size_t count;
} LIBMATTI_FML_JarContentsModuleFinder;

// Java: public JarContentsModuleFinder(Collection<JarContentsModule> jars)
LIBMATTI_FML_JarContentsModuleFinder *LIBMATTI_FML_JarContentsModuleFinder_New(
    const LIBMATTI_FML_JarContentsModule **jars, size_t jarCount);
void LIBMATTI_FML_JarContentsModuleFinder_Free(LIBMATTI_FML_JarContentsModuleFinder *finder);

// Java: public Optional<ModuleReference> find(String name) (NULL = empty optional)
LIBMATTI_FML_JarContentsModuleReference *LIBMATTI_FML_JarContentsModuleFinder_Find(
    const LIBMATTI_FML_JarContentsModuleFinder *finder, const char *name);
// Java: public Set<ModuleReference> findAll() -> Set.copyOf(values); caller frees the array
LIBMATTI_FML_JarContentsModuleReference **LIBMATTI_FML_JarContentsModuleFinder_FindAll(
    const LIBMATTI_FML_JarContentsModuleFinder *finder, size_t *count);

#endif //MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEFINDER_H
