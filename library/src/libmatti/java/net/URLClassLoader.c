#include "libmatti/java/net/URLClassLoader.h"

#include <stdlib.h>
#include <string.h>

// Java: the URLs carry the content roots as file URLs
static char *to_path(const LIBMATTI_JN_URL *url)
{
    const char *value = LIBMATTI_JN_URL_ToString(url);
    if (strncmp(value, "file:", 5) == 0) return strdup(value + 5);
    return NULL;
}

// Java: public URLClassLoader(String name, URL[] urls, ClassLoader parent)
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_URLClassLoader_New(const char *name, LIBMATTI_JN_URL **urls, size_t urlCount,
                                                        LIBMATTI_JL_ClassLoader *parent)
{
    LIBMATTI_JL_ClassLoader *classLoader = LIBMATTI_JL_ClassLoader_New(name, parent);

    for (size_t i = 0; i < urlCount; i++)
    {
        char *path = to_path(urls[i]);
        if (path == NULL) continue;

        LIBMATTI_JL_ClassLoader_AddClassPathEntry(classLoader, path);
        free(path);
    }

    return classLoader;
}

// Java: public void close()
void LIBMATTI_JL_URLClassLoader_Close(LIBMATTI_JL_ClassLoader *classLoader)
{
    if (classLoader == NULL) return;

    // Java closes the jar files it opened; the C port releases the class path entries it built
    LIBMATTI_JL_ClassLoader_SetClassPath(classLoader, NULL, 0);
}

// Java: public String getName()
const char *LIBMATTI_JL_URLClassLoader_GetName(const LIBMATTI_JL_ClassLoader *classLoader)
{
    return LIBMATTI_JL_ClassLoader_GetName(classLoader);
}
