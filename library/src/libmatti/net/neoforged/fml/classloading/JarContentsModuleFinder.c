#include "libmatti/net/neoforged/fml/classloading/JarContentsModuleFinder.h"

#include <stdlib.h>
#include <string.h>

// Java: public JarContentsModuleFinder(Collection<JarContentsModule> jars)
LIBMATTI_FML_JarContentsModuleFinder *LIBMATTI_FML_JarContentsModuleFinder_New(
    const LIBMATTI_FML_JarContentsModule **jars, size_t jarCount)
{
    LIBMATTI_FML_JarContentsModuleFinder *finder = calloc(1, sizeof(LIBMATTI_FML_JarContentsModuleFinder));
    finder->names = calloc(jarCount > 0 ? jarCount : 1, sizeof(char *));
    finder->references = calloc(jarCount > 0 ? jarCount : 1, sizeof(LIBMATTI_FML_JarContentsModuleReference *));

    // Java: Collectors.toMap(JarContentsModule::moduleName, sj -> new JarContentsModuleReference(sj.moduleDescriptor(), sj.contents()))
    for (size_t i = 0; i < jarCount; i++)
    {
        const char *name = LIBMATTI_FML_JarContentsModule_ModuleName(jars[i]);

        // Java: Collectors.toMap without a merge function - a duplicate key throws
        int duplicate = 0;
        for (size_t existing = 0; existing < finder->count; existing++)
        {
            if (strcmp(finder->names[existing], name) == 0)
            {
                duplicate = 1;
                break;
            }
        }
        if (duplicate)
            continue;

        finder->names[finder->count] = strdup(name);
        finder->references[finder->count] = LIBMATTI_FML_JarContentsModuleReference_New(
            jars[i]->moduleDescriptor, jars[i]->contents);
        finder->count++;
    }

    return finder;
}

void LIBMATTI_FML_JarContentsModuleFinder_Free(LIBMATTI_FML_JarContentsModuleFinder *finder)
{
    if (finder == NULL)
        return;
    for (size_t i = 0; i < finder->count; i++)
    {
        free(finder->names[i]);
        free(finder->references[i]);
    }
    free(finder->names);
    free(finder->references);
    free(finder);
}

// Java: public Optional<ModuleReference> find(String name) { return Optional.ofNullable(jars.get(name)); }
LIBMATTI_FML_JarContentsModuleReference *LIBMATTI_FML_JarContentsModuleFinder_Find(
    const LIBMATTI_FML_JarContentsModuleFinder *finder, const char *name)
{
    for (size_t i = 0; i < finder->count; i++)
        if (strcmp(finder->names[i], name) == 0)
            return finder->references[i];
    return NULL;
}

// Java: public Set<ModuleReference> findAll() { return Set.copyOf(jars.values()); }
LIBMATTI_FML_JarContentsModuleReference **LIBMATTI_FML_JarContentsModuleFinder_FindAll(
    const LIBMATTI_FML_JarContentsModuleFinder *finder, size_t *count)
{
    LIBMATTI_FML_JarContentsModuleReference **result =
        calloc(finder->count > 0 ? finder->count : 1, sizeof(LIBMATTI_FML_JarContentsModuleReference *));
    for (size_t i = 0; i < finder->count; i++)
        result[i] = finder->references[i];

    *count = finder->count;
    return result;
}
