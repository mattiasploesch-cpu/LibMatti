#include "libmatti/java/lang/ClassLoader.h"

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: java.net.URLClassPath caches the opened jar per class path entry.
// The C port caches the parsed JarContents per archive path the same way.
typedef struct ArchiveCacheEntry
{
    char *path;
    LIBMATTI_JH_JarContents *contents;
    struct ArchiveCacheEntry *next;
} ArchiveCacheEntry;

static ArchiveCacheEntry *archive_cache;

static LIBMATTI_JH_JarContents *archive_contents(const char *path)
{
    for (ArchiveCacheEntry *e = archive_cache; e != NULL; e = e->next)
    {
        if (strcmp(e->path, path) == 0) return e->contents;
    }

    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContents_Of(path);
    if (contents == NULL) return NULL;

    ArchiveCacheEntry *entry = malloc(sizeof(ArchiveCacheEntry));
    entry->path = strdup(path);
    entry->contents = contents;
    entry->next = archive_cache;
    archive_cache = entry;
    return contents;
}

// Java: ClassLoader(String name, ClassLoader parent)
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_New(const char *name, LIBMATTI_JL_ClassLoader *parent)
{
    LIBMATTI_JL_ClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_JL_ClassLoader));
    classLoader->name = name != NULL ? strdup(name) : NULL;
    classLoader->parent = parent;
    return classLoader;
}

// Java: ClassLoader() - an unnamed loader
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_NewUnnamed(LIBMATTI_JL_ClassLoader *parent)
{
    return LIBMATTI_JL_ClassLoader_New(NULL, parent);
}

void LIBMATTI_JL_ClassLoader_Free(LIBMATTI_JL_ClassLoader *classLoader)
{
    if (classLoader == NULL) return;

    free(classLoader->name);
    for (size_t i = 0; i < classLoader->classPathCount; i++)
        free(classLoader->classPath[i]);
    free(classLoader->classPath);
    free(classLoader);
}

// Java: public String getName()
const char *LIBMATTI_JL_ClassLoader_GetName(const LIBMATTI_JL_ClassLoader *classLoader)
{
    return classLoader->name;
}

// Java: public ClassLoader getParent()
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_GetParent(const LIBMATTI_JL_ClassLoader *classLoader)
{
    return classLoader->parent;
}

// Java: public static ClassLoader getSystemClassLoader()
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_ClassLoader_GetSystemClassLoader(void)
{
    static LIBMATTI_JL_ClassLoader *systemClassLoader = NULL;
    if (systemClassLoader != NULL) return systemClassLoader;

    // Java: the VM builds the application class loader from the class path property
    systemClassLoader = LIBMATTI_JL_ClassLoader_New("app", NULL);

    const char *classPath = getenv("legacyClassPath");
    if (classPath == NULL) classPath = getenv("java.class.path");
    if (classPath == NULL) return systemClassLoader;

    char *copy = strdup(classPath);
    char *saveptr = NULL;

    for (char *entry = strtok_r(copy, ":", &saveptr); entry != NULL; entry = strtok_r(NULL, ":", &saveptr))
        LIBMATTI_JL_ClassLoader_AddClassPathEntry(systemClassLoader, entry);

    free(copy);
    return systemClassLoader;
}

// Java: the class path entries backing the loader's resource search
char **LIBMATTI_JL_ClassLoader_GetClassPath(const LIBMATTI_JL_ClassLoader *classLoader, size_t *count)
{
    *count = classLoader->classPathCount;
    return classLoader->classPath;
}

void LIBMATTI_JL_ClassLoader_SetClassPath(LIBMATTI_JL_ClassLoader *classLoader, char **entries, size_t count)
{
    for (size_t i = 0; i < classLoader->classPathCount; i++)
        free(classLoader->classPath[i]);
    free(classLoader->classPath);

    classLoader->classPath = entries;
    classLoader->classPathCount = count;
}

// Java: URLClassLoader.addURL(URL)
void LIBMATTI_JL_ClassLoader_AddClassPathEntry(LIBMATTI_JL_ClassLoader *classLoader, const char *entry)
{
    classLoader->classPath = realloc(classLoader->classPath, sizeof(*classLoader->classPath) * (classLoader->classPathCount + 1));
    classLoader->classPath[classLoader->classPathCount++] = strdup(entry);
}

// Java: the resource lookup over the class path (java.net.URLClassPath) - NULL when not found
static char *find_resource_path(const LIBMATTI_JL_ClassLoader *classLoader, const char *name)
{
    for (size_t i = 0; i < classLoader->classPathCount; i++)
    {
        if (LIBMATTI_JNF_Files_IsDirectory(classLoader->classPath[i]))
        {
            char *candidate = LIBMATTI_JNF_Path_Resolve(classLoader->classPath[i], name);
            if (LIBMATTI_JNF_Files_Exists(candidate)) return candidate;
            free(candidate);
            continue;
        }

        // Java reads the entry as a jar (URLClassPath$JarLoader); the port looks
        // the resource up through the parsed JarContents of the archive.
        if (LIBMATTI_JNF_Files_IsRegularFile(classLoader->classPath[i]))
        {
            LIBMATTI_JH_JarContents *contents = archive_contents(classLoader->classPath[i]);
            if (contents == NULL) continue;

            LIBMATTI_JN_URI *uri = NULL;
            if (LIBMATTI_JH_JarContents_FindFile(contents, name, &uri) && uri != NULL)
            {
                char *resource = strdup(LIBMATTI_JN_URI_ToString(uri));
                LIBMATTI_JN_URI_Free(uri);
                return resource;
            }
            LIBMATTI_JN_URI_Free(uri);
        }
    }

    return NULL;
}

// Java: public URL getResource(String name)
LIBMATTI_JN_URL *LIBMATTI_JL_ClassLoader_GetResource(const LIBMATTI_JL_ClassLoader *classLoader, const char *name)
{
    // Java: the parent-first delegation, then findResource
    if (classLoader->parent != NULL)
    {
        LIBMATTI_JN_URL *parentResource = LIBMATTI_JL_ClassLoader_GetResource(classLoader->parent, name);
        if (parentResource != NULL) return parentResource;
    }

    return LIBMATTI_JL_ClassLoader_FindResource(classLoader, name);
}

// Java: public InputStream getResourceAsStream(String name)
char *LIBMATTI_JL_ClassLoader_GetResourceAsStream(const LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                  size_t *length)
{
    *length = 0;

    // Java: getResource(name) walks the parent chain first
    char *path = NULL;
    for (const LIBMATTI_JL_ClassLoader *current = classLoader; current != NULL && path == NULL; current = current->parent)
        path = find_resource_path(current, name);

    if (path == NULL) return NULL;

    FILE *stream = fopen(path, "rb");
    if (stream == NULL)
    {
        free(path);
        return NULL;
    }

    fseek(stream, 0, SEEK_END);
    long size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    char *bytes = malloc((size_t) size + 1);
    *length = fread(bytes, 1, (size_t) size, stream);
    bytes[*length] = '\0';

    fclose(stream);
    free(path);
    return bytes;
}

// Java: protected URL findResource(String name)
LIBMATTI_JN_URL *LIBMATTI_JL_ClassLoader_FindResource(const LIBMATTI_JL_ClassLoader *classLoader, const char *name)
{
    char *path = find_resource_path(classLoader, name);
    if (path == NULL) return NULL;

    size_t length = strlen(path) + 6;
    char *spec = malloc(length);
    snprintf(spec, length, "file:%s", path);
    free(path);

    LIBMATTI_JN_URL *url = LIBMATTI_JN_URL_New(spec);
    free(spec);
    return url;
}

// Java: protected Enumeration<URL> findResources(String name)
LIBMATTI_JN_URL **LIBMATTI_JL_ClassLoader_FindResources(const LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                        size_t *count)
{
    LIBMATTI_JN_URL **urls = NULL;
    *count = 0;

    for (size_t i = 0; i < classLoader->classPathCount; i++)
    {
        if (!LIBMATTI_JNF_Files_IsDirectory(classLoader->classPath[i])) continue;

        char *candidate = LIBMATTI_JNF_Path_Resolve(classLoader->classPath[i], name);
        if (LIBMATTI_JNF_Files_Exists(candidate))
        {
            size_t length = strlen(candidate) + 6;
            char *spec = malloc(length);
            snprintf(spec, length, "file:%s", candidate);

            urls = realloc(urls, sizeof(*urls) * (*count + 1));
            urls[(*count)++] = LIBMATTI_JN_URL_New(spec);
            free(spec);
        }
        free(candidate);
    }

    return urls;
}
