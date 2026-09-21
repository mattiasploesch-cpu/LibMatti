// Port of net.neoforged.fml.classloading.ResourceMaskingClassLoader.
// This class loader filters the resources returned from getResources(String) and getResource(String)
// by removing any results that come from a given set of classpath items (folders or jars).

#ifndef MATTICRAFT_FML_CLASSLOADING_RESOURCEMASKINGCLASSLOADER_H
#define MATTICRAFT_FML_CLASSLOADING_RESOURCEMASKINGCLASSLOADER_H

#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/java/net/URL.h"

#include <stddef.h>

// Java: public class ResourceMaskingClassLoader extends ClassLoader
typedef struct LIBMATTI_FML_ResourceMaskingClassLoader
{
    // Java: extends ClassLoader
    LIBMATTI_JL_ClassLoader base;

    // Java: private final Set<Path> maskedClasspathElements
    const char **maskedClasspathElements;
    size_t maskedClasspathElementCount;
} LIBMATTI_FML_ResourceMaskingClassLoader;

// Java: public ResourceMaskingClassLoader(ClassLoader parent, Set<Path> maskedClasspathElements)
LIBMATTI_FML_ResourceMaskingClassLoader *LIBMATTI_FML_ResourceMaskingClassLoader_New(
    LIBMATTI_JL_ClassLoader *parent, const char **maskedClasspathElements, size_t maskedClasspathElementCount);
void LIBMATTI_FML_ResourceMaskingClassLoader_Free(LIBMATTI_FML_ResourceMaskingClassLoader *classLoader);

// Java: @Nullable public URL getResource(String name)
LIBMATTI_JN_URL *LIBMATTI_FML_ResourceMaskingClassLoader_GetResource(
    LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *name);
// Java: public Enumeration<URL> getResources(String name); caller frees the array
LIBMATTI_JN_URL **LIBMATTI_FML_ResourceMaskingClassLoader_GetResources(
    LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *name, size_t *count);

#endif //MATTICRAFT_FML_CLASSLOADING_RESOURCEMASKINGCLASSLOADER_H
