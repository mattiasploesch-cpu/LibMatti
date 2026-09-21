// Port of java.net.URLClassLoader.
// The C port folds the URL list into the base java.lang.ClassLoader type, since
// every loader the port creates is a URL loader.

#ifndef MATTICRAFT_JAVA_NET_URLCLASSLOADER_H
#define MATTICRAFT_JAVA_NET_URLCLASSLOADER_H

#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/java/net/URL.h"

#include <stddef.h>

// Java: public URLClassLoader(String name, URL[] urls, ClassLoader parent)
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_URLClassLoader_New(const char *name, LIBMATTI_JN_URL **urls, size_t urlCount,
                                                        LIBMATTI_JL_ClassLoader *parent);

// Java: public void close() - closes the archives the loader opened
void LIBMATTI_JL_URLClassLoader_Close(LIBMATTI_JL_ClassLoader *classLoader);

// Java: public String getName()
const char *LIBMATTI_JL_URLClassLoader_GetName(const LIBMATTI_JL_ClassLoader *classLoader);

#endif //MATTICRAFT_JAVA_NET_URLCLASSLOADER_H
