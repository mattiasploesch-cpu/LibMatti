#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"

#include <stdlib.h>
#include <string.h>

// Java: public static Path getRootFromResourceUrl(String relativePath, URL resourceUrl)
char *LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(const char *relativePath,
                                                                 const LIBMATTI_JN_URL *resourceUrl)
{
    const char *url = LIBMATTI_JN_URL_ToString(resourceUrl);

    // Java: if ("jar".equals(resourceUrl.getProtocol())) { var fileUri = URI.create(resourceUrl.toString().split("!")[0].substring("jar:".length())); return Paths.get(fileUri); }
    if (strncmp(url, "jar:", 4) == 0)
    {
        const char *end = strstr(url, "!");
        size_t length = end != NULL ? (size_t) (end - url - 4) : strlen(url) - 4;
        const char *fileUri = url + 4;
        // Java: Paths.get(URI) strips the scheme
        if (strncmp(fileUri, "file:", 5) == 0)
        {
            fileUri += 5;
            length -= 5;
        }
        return strndup(fileUri, length);
    }

    // Java: resourcePath = Paths.get(resourceUrl.toURI()); then walk back the nesting depth of relativePath
    const char *resourcePath = url;
    if (strncmp(resourcePath, "file:", 5) == 0)
        resourcePath += 5;

    size_t depth = 0;
    for (const char *p = relativePath; *p != '\0'; p++)
        if (*p == '/')
            depth++;

    size_t length = strlen(resourcePath);
    while (depth > 0)
    {
        const char *slash = strrchr(resourcePath, '/');
        if (slash == NULL) break;
        length = (size_t) (slash - resourcePath);
        depth--;
    }

    return strndup(resourcePath, length);
}

// Java: public static Path findJarPathFor(String resourceName, String jarName, URL resource)
char *LIBMATTI_FML_ClasspathResourceUtils_FindJarPathFor(const char *resourceName, const char *jarName,
                                                         const LIBMATTI_JN_URL *resource)
{
    // Java: if (uri.getScheme().equals("jar") && uri.getRawSchemeSpecificPart().contains("!/")) { ... lastIndexOf("!/") ... }
    //       else { Paths.get(new URI("file://" + raw.substring(0, len - resourceName.length()))) }
    char *result = LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(resourceName, resource);
    if (result == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Failed to find JAR for class {} - {}", resourceName, jarName);
    }
    return result;
}

// Java: public static List<Path> findFileSystemRootsOfFileOnClasspath(ClassLoader classLoader, String relativePath)
char **LIBMATTI_FML_ClasspathResourceUtils_FindRoots(const LIBMATTI_JL_ClassLoader *classLoader,
                                                     const char *relativePath, size_t *count)
{
    // Java: classLoader.getResources(relativePath).asIterator()
    size_t resourceCount = 0;
    LIBMATTI_JN_URL **resources = LIBMATTI_JL_ClassLoader_FindResources(classLoader, relativePath, &resourceCount);

    // Java: LinkedHashSet<Path> result
    char **result = NULL;
    *count = 0;
    for (size_t i = 0; i < resourceCount; i++)
    {
        char *root = LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(relativePath, resources[i]);
        int duplicate = 0;
        for (size_t j = 0; j < *count && !duplicate; j++)
            if (strcmp(result[j], root) == 0)
                duplicate = 1;
        if (duplicate)
        {
            free(root);
            continue;
        }
        result = realloc(result, sizeof(char *) * (*count + 1));
        result[(*count)++] = root;
    }
    return result;
}

// Java: public static @Nullable Path findFileSystemRootOfFileOnClasspath(ClassLoader classLoader, String relativePath)
char *LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootOfFileOnClasspath(
    const LIBMATTI_JL_ClassLoader *classLoader, const char *relativePath)
{
    // Java: var resource = classLoader.getResource(relativePath); if (resource == null) return null;
    LIBMATTI_JN_URL *resource = LIBMATTI_JL_ClassLoader_GetResource(classLoader, relativePath);
    if (resource == NULL) return NULL;
    return LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(relativePath, resource);
}

// Java: public static List<Path> findFileSystemRootsOfFileOnClasspath(String relativePath)
char **LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootsOfFileOnClasspath(const char *relativePath,
                                                                                size_t *count)
{
    // Java: If we're loaded through a module, the original classpath is inaccessible through the context CL
    //       if (ClasspathResourceUtils.class.getModule().isNamed()) classLoader = ClassLoader.getSystemClassLoader();
    // The loader classes always live in the executable (a named module), so the port always
    // resolves through the system class loader like the named-module branch in Java.
    LIBMATTI_JL_ClassLoader *classLoader = LIBMATTI_JL_ClassLoader_GetSystemClassLoader();
    return LIBMATTI_FML_ClasspathResourceUtils_FindRoots(classLoader, relativePath, count);
}

// Java: public static Path findFileSystemRootOfFileOnClasspath(String relativePath)
char *LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootOfFileOnClasspathFromContext(const char *relativePath)
{
    size_t count = 0;
    char **paths = LIBMATTI_FML_ClasspathResourceUtils_FindFileSystemRootsOfFileOnClasspath(relativePath, &count);

    // Java: if (paths.isEmpty()) throw new ModLoadingException(ModLoadingIssue.error("fml.modloadingissue.failed_to_find_on_classpath", relativePath));
    if (count == 0)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Failed to find {} on the classpath", relativePath);
        return NULL;
    }

    // Java: else if (paths.size() > 1) throw new ModLoadingException(ModLoadingIssue.error("fml.modloadingissue.multiple_copies_on_classpath", relativePath, paths));
    if (count > 1)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_FML_MARKER_SCAN,
                                 "Found multiple copies of {} on the classpath", relativePath);
    }

    return paths[0];
}

// Java: public static Set<Path> getAllClasspathItems(ClassLoader loader)
char **LIBMATTI_FML_ClasspathResourceUtils_GetAllClasspathItems(const LIBMATTI_JL_ClassLoader *loader,
                                                                size_t *count)
{
    // Java: if (loader == ClassLoader.getSystemClassLoader()) return Arrays.stream(System.getProperty("java.class.path").split(File.pathSeparator)).map(Paths::get).collect(toSet());
    if (loader == LIBMATTI_JL_ClassLoader_GetSystemClassLoader())
    {
        const char *classPath = LIBMATTI_JL_System_GetProperty("java.class.path");
        char **items = NULL;
        *count = 0;
        if (classPath == NULL) return items;

        char *copy = strdup(classPath);
        for (char *token = strtok(copy, ":"); token != NULL; token = strtok(NULL, ":"))
        {
            items = realloc(items, sizeof(char *) * (*count + 1));
            items[(*count)++] = strdup(token);
        }
        free(copy);
        return items;
    }

    // Java: else if (loader instanceof URLClassLoader urlClassLoader) { return urlClassLoader.getURLs() ... }
    // The port's ClassLoader exposes the same URLs through GetClassPath.
    size_t itemCount = 0;
    char **items = LIBMATTI_JL_ClassLoader_GetClassPath(loader, &itemCount);
    char **result = malloc(sizeof(char *) * (itemCount > 0 ? itemCount : 1));
    for (size_t i = 0; i < itemCount; i++) result[i] = strdup(items[i]);
    *count = itemCount;
    return result;
}
