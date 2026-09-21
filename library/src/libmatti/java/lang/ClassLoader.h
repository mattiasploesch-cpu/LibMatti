// Port of java.lang.ClassLoader.
// The JVM creates the loaders and searches its internal class path (ucp); the C
// port keeps the surface the loader uses: name, parent, the class path entries
// and the resource lookup over them.
// Note: cpw.mods.cl.ModuleClassLoader (bsl/sjh) declares the same Java class as
// the opaque LIBMATTI_CL_ClassLoader handle.

#ifndef MATTICRAFT_JAVA_LANG_CLASSLOADER_H
#define MATTICRAFT_JAVA_LANG_CLASSLOADER_H

#include "libmatti/java/net/URL.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_ClassLoader LIBMATTI_JL_ClassLoader;

struct LIBMATTI_JL_ClassLoader
{
    // Java: String getName()
    char *name;
    // Java: ClassLoader getParent()
    LIBMATTI_JL_ClassLoader *parent;
    // Java: the class path this loader searches (the VM's java.class.path for the system loader)
    char **classPath;
    size_t classPathCount;
};

// Java: public static ClassLoader getSystemClassLoader()
// Java's BootstrapLauncher reads "legacyClassPath" and falls back to "java.class.path";
// the C port maps the VM's class path the same way.
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_GetSystemClassLoader(void);

// Java: ClassLoader(String name, ClassLoader parent)
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_New(const char *name, LIBMATTI_JL_ClassLoader *parent);
// Java: ClassLoader() - the unnamed parentless loader
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_NewUnnamed(LIBMATTI_JL_ClassLoader *parent);
void LIBMATTI_JL_ClassLoader_Free(LIBMATTI_JL_ClassLoader *classLoader);

// Java: public String getName()
const char *LIBMATTI_JL_ClassLoader_GetName(const LIBMATTI_JL_ClassLoader *classLoader);
// Java: public ClassLoader getParent()
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_GetParent(const LIBMATTI_JL_ClassLoader *classLoader);

// Java: public URL getResource(String name)
LIBMATTI_JN_URL *LIBMATTI_JL_ClassLoader_GetResource(const LIBMATTI_JL_ClassLoader *classLoader, const char *name);
// Java: public InputStream getResourceAsStream(String name) - the stream is the file's bytes, caller frees
char *LIBMATTI_JL_ClassLoader_GetResourceAsStream(const LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                  size_t *length);
// Java: protected URL findResource(String name)
LIBMATTI_JN_URL *LIBMATTI_JL_ClassLoader_FindResource(const LIBMATTI_JL_ClassLoader *classLoader, const char *name);
// Java: protected Enumeration<URL> findResources(String name); caller frees the array
LIBMATTI_JN_URL **LIBMATTI_JL_ClassLoader_FindResources(const LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                        size_t *count);

// Java: the class path entries backing the loader's resource search
char **LIBMATTI_JL_ClassLoader_GetClassPath(const LIBMATTI_JL_ClassLoader *classLoader, size_t *count);
void LIBMATTI_JL_ClassLoader_SetClassPath(LIBMATTI_JL_ClassLoader *classLoader, char **entries, size_t count);
// Java: URLClassLoader.addURL(URL)
void LIBMATTI_JL_ClassLoader_AddClassPathEntry(LIBMATTI_JL_ClassLoader *classLoader, const char *entry);

#endif //MATTICRAFT_JAVA_LANG_CLASSLOADER_H
