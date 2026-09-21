#include "JarModuleFinder.h"

#include "libmatti/java/lang/module/ModuleDescriptor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LIBMATTI_CL_JarModuleReader
{
    LIBMATTI_CL_ModuleDataProvider *jar;
};

LIBMATTI_CL_JarModuleFinder *LIBMATTI_CL_JarModuleFinder_Of(LIBMATTI_JH_SecureJar **jars, size_t jarCount)
{
    LIBMATTI_CL_JarModuleFinder *finder = calloc(1, sizeof(LIBMATTI_CL_JarModuleFinder));

    for (size_t i = 0; i < jarCount; i++)
    {
        // Java: jar.moduleDataProvider()
        LIBMATTI_CL_ModuleDataProvider *provider = LIBMATTI_JH_SecureJar_ModuleDataProvider(jars[i]);
        const char *name = provider->name(provider);

        // Java: Collectors.toMap(..., (r1, r2) -> r1) - first reference wins on duplicate names
        int duplicate = 0;
        for (size_t j = 0; j < finder->count; j++)
        {
            if (strcmp(finder->names[j], name) == 0)
            {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) continue;

        // Java: new JarModuleReference(jar.moduleDataProvider())
        LIBMATTI_CL_JarModuleReference *reference = calloc(1, sizeof(LIBMATTI_CL_JarModuleReference));
        reference->base.descriptor = provider->descriptor(provider);
        reference->base.location = provider->uri(provider);
        reference->jar = provider;

        finder->names = realloc(finder->names, sizeof(char *) * (finder->count + 1));
        finder->references = realloc(finder->references, sizeof(LIBMATTI_CL_JarModuleReference *) * (finder->count + 1));
        finder->names[finder->count] = strdup(name);
        finder->references[finder->count] = reference;
        finder->count++;
    }

    return finder;
}

void LIBMATTI_CL_JarModuleFinder_Free(LIBMATTI_CL_JarModuleFinder *finder)
{
    for (size_t i = 0; i < finder->count; i++)
    {
        free(finder->names[i]);
        // The descriptor is the provider's cached descriptor (LazyJarMetadata),
        // owned by the metadata and freed when the jar closes - not here.
        LIBMATTI_JN_URI_Free(finder->references[i]->base.location);
        free(finder->references[i]);
    }
    free(finder->names);
    free(finder->references);
    free(finder);
}

LIBMATTI_CL_JarModuleReference *LIBMATTI_CL_JarModuleFinder_Find(const LIBMATTI_CL_JarModuleFinder *finder, const char *name)
{
    // Java: Optional.ofNullable(moduleReferenceMap.get(name))
    for (size_t i = 0; i < finder->count; i++)
    {
        if (strcmp(finder->names[i], name) == 0) return finder->references[i];
    }
    return NULL;
}

LIBMATTI_CL_JarModuleReference **LIBMATTI_CL_JarModuleFinder_FindAll(const LIBMATTI_CL_JarModuleFinder *finder, size_t *count)
{
    // Java: new HashSet<>(moduleReferenceMap.values())
    LIBMATTI_CL_JarModuleReference **all = malloc(sizeof(LIBMATTI_CL_JarModuleReference *) * finder->count);
    for (size_t i = 0; i < finder->count; i++) all[i] = finder->references[i];
    *count = finder->count;
    return all;
}

LIBMATTI_CL_ModuleDataProvider *LIBMATTI_CL_JarModuleReference_Jar(const LIBMATTI_CL_JarModuleReference *reference)
{
    return reference->jar;
}

LIBMATTI_CL_JarModuleReader *LIBMATTI_CL_JarModuleReference_Open(const LIBMATTI_CL_JarModuleReference *reference)
{
    // Java: new JarModuleReader(this.jar)
    LIBMATTI_CL_JarModuleReader *reader = calloc(1, sizeof(LIBMATTI_CL_JarModuleReader));
    reader->jar = reference->jar;
    return reader;
}

void LIBMATTI_CL_JarModuleReader_Free(LIBMATTI_CL_JarModuleReader *reader)
{
    free(reader);
}

LIBMATTI_JN_URI *LIBMATTI_CL_JarModuleReader_Find(const LIBMATTI_CL_JarModuleReader *reader, const char *name)
{
    // Java: return jar.findFile(name);
    LIBMATTI_JN_URI *out = NULL;
    if (reader->jar->findFile(reader->jar, name, &out)) return out;
    return NULL;
}

LIBMATTI_JI_InputStream *LIBMATTI_CL_JarModuleReader_Open(const LIBMATTI_CL_JarModuleReader *reader, const char *name)
{
    // Java: return jar.open(name);
    LIBMATTI_JI_InputStream *out = NULL;
    if (reader->jar->open(reader->jar, name, &out)) return out;
    return NULL;
}

void *LIBMATTI_CL_JarModuleReader_List(const LIBMATTI_CL_JarModuleReader *reader)
{
    (void)reader;
    return NULL; // Java: return null;
}

void LIBMATTI_CL_JarModuleReader_Close(const LIBMATTI_CL_JarModuleReader *reader)
{
    (void)reader; // Java: empty body
}

char *LIBMATTI_CL_JarModuleReader_ToString(const LIBMATTI_CL_JarModuleReader *reader)
{
    // Java: getClass().getName() + "[jar=" + jar + "]"
    if (reader->jar == NULL)
        return strdup("cpw.mods.cl.JarModuleFinder$JarModuleReader[jar=null]");

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "cpw.mods.cl.JarModuleFinder$JarModuleReader[jar=%p]", (void *)reader->jar);
    return strdup(buffer);
}