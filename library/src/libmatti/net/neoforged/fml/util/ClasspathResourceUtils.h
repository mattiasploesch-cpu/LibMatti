// Port of net.neoforged.fml.util.ClasspathResourceUtils.
// TODO: java.net.URL - the port keeps the URL as its string form.
// TODO: ClassLoader.getResources(Enumeration) - the port uses findResources.

#ifndef MATTICRAFT_FML_UTIL_CLASSPATHRESOURCEUTILS_H
#define MATTICRAFT_FML_UTIL_CLASSPATHRESOURCEUTILS_H

#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/java/net/URL.h"

#include <stddef.h>

// Java: public static List<Path> findFileSystemRootsOfFileOnClasspath(String relativePath)
char **LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootsOfFileOnClasspath(const char *relativePath,
                                                                                size_t *count);
// Java: public static Path findJarPathFor(String resourceName, String jarName, URL resource) - a new string
char *LIBMATTI_FML_ClasspathResourceUtils_FindJarPathFor(const char *resourceName, const char *jarName,
                                                         const LIBMATTI_JN_URL *resource);
// Java: public static Path getRootFromResourceUrl(String relativePath, URL resourceUrl) - a new string
char *LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(const char *relativePath,
                                                                 const LIBMATTI_JN_URL *resourceUrl);
// Java: public static @Nullable Path findFileSystemRootOfFileOnClasspath(ClassLoader classLoader, String relativePath)
char *LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootOfFileOnClasspath(
    const LIBMATTI_JL_ClassLoader *classLoader, const char *relativePath);
// Java: public static List<Path> findFileSystemRootsOfFileOnClasspath(ClassLoader classLoader, String relativePath)
char **LIBMATTI_FML_ClasspathResourceUtils_FindRoots(const LIBMATTI_JL_ClassLoader *classLoader,
                                                     const char *relativePath, size_t *count);
// Java: public static Path findFileSystemRootOfFileOnClasspath(String relativePath) - a new string
char *LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootOfFileOnClasspathFromContext(const char *relativePath);
// Java: public static Set<Path> getAllClasspathItems(ClassLoader loader)
char **LIBMATTI_FML_ClasspathResourceUtils_GetAllClasspathItems(const LIBMATTI_JL_ClassLoader *loader,
                                                                size_t *count);

#endif //MATTICRAFT_FML_UTIL_CLASSPATHRESOURCEUTILS_H
