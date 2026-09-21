#include "libmatti/java/nio/file/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Java: static Path of(String first, String... more)
char *LIBMATTI_JNF_Path_Of(const char *first, const char **more, size_t moreCount)
{
    char *path = strdup(first);

    for (size_t i = 0; i < moreCount; i++)
    {
        char *next = LIBMATTI_JNF_Path_Resolve(path, more[i]);
        free(path);
        path = next;
    }

    return path;
}

// Java: Path resolve(String other)
char *LIBMATTI_JNF_Path_Resolve(const char *path, const char *other)
{
    if (other[0] == '/') return strdup(other);
    if (path[0] == '\0') return strdup(other);

    size_t pathLength = strlen(path);
    size_t length = pathLength + strlen(other) + 2;
    char *result = malloc(length);
    snprintf(result, length, "%s%s%s", path, path[pathLength - 1] == '/' ? "" : "/", other);
    return result;
}

// Java: Path toAbsolutePath()
char *LIBMATTI_JNF_Path_ToAbsolutePath(const char *path)
{
    if (path[0] == '/') return LIBMATTI_JNF_Path_Normalize(path);

    char *workingDirectory = getcwd(NULL, 0);
    if (workingDirectory == NULL) return strdup(path);

    char *resolved = LIBMATTI_JNF_Path_Resolve(workingDirectory, path);
    free(workingDirectory);

    char *normalized = LIBMATTI_JNF_Path_Normalize(resolved);
    free(resolved);
    return normalized;
}

// Java: Path normalize()
char *LIBMATTI_JNF_Path_Normalize(const char *path)
{
    int absolute = path[0] == '/';

    char **components = NULL;
    size_t count = 0;

    char *copy = strdup(path);
    char *saveptr = NULL;

    for (char *part = strtok_r(copy, "/", &saveptr); part != NULL; part = strtok_r(NULL, "/", &saveptr))
    {
        if (strcmp(part, ".") == 0) continue;

        if (strcmp(part, "..") == 0)
        {
            if (count > 0 && strcmp(components[count - 1], "..") != 0)
            {
                free(components[--count]);
                continue;
            }
            if (absolute) continue;
        }

        components = realloc(components, sizeof(*components) * (count + 1));
        components[count++] = strdup(part);
    }
    free(copy);

    if (count == 0)
    {
        free(components);
        return strdup(absolute ? "/" : "");
    }

    size_t length = absolute ? 1 : 0;
    for (size_t i = 0; i < count; i++)
        length += strlen(components[i]) + 1;

    char *result = malloc(length + 1);
    char *write = result;
    if (absolute) *write++ = '/';

    for (size_t i = 0; i < count; i++)
    {
        size_t componentLength = strlen(components[i]);
        if (i > 0) *write++ = '/';
        memcpy(write, components[i], componentLength);
        write += componentLength;
        free(components[i]);
    }
    *write = '\0';

    free(components);
    return result;
}

// Java: Path getFileName()
char *LIBMATTI_JNF_Path_GetFileName(const char *path)
{
    size_t length = strlen(path);
    if (length == 0 || (length == 1 && path[0] == '/')) return NULL;

    const char *end = path + length - 1;
    while (end > path && *end == '/') end--;

    const char *start = end;
    while (start > path && *(start - 1) != '/') start--;

    size_t nameLength = (size_t) (end - start) + 1;
    char *name = malloc(nameLength + 1);
    memcpy(name, start, nameLength);
    name[nameLength] = '\0';
    return name;
}

// Java: Path getParent()
char *LIBMATTI_JNF_Path_GetParent(const char *path)
{
    size_t length = strlen(path);
    if (length == 0) return NULL;

    const char *end = path + length - 1;
    while (end > path && *end == '/') end--;

    const char *slash = end;
    while (slash > path && *slash != '/') slash--;

    if (slash == path) return path[0] == '/' ? strdup("/") : NULL;

    size_t parentLength = (size_t) (slash - path);
    char *parent = malloc(parentLength + 1);
    memcpy(parent, path, parentLength);
    parent[parentLength] = '\0';
    return parent;
}
