#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"

#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private record PathSubstitution(Path basePath, String prefix, String suffix) {}
typedef struct
{
    char *basePath;
    char *prefix;
    char *suffix;
} PathSubstitution;

// Java: private static volatile List<PathSubstitution> SUBSTITUTIONS = new ArrayList<>()
static PathSubstitution *SUBSTITUTIONS = NULL;
static size_t SUBSTITUTIONS_COUNT = 0;

static size_t nameCount(const char *path)
{
    size_t count = 0;
    for (const char *c = path; *c != '\0'; c++)
        if (*c == '/')
            count++;
    return count;
}

static char *duplicateString(const char *value)
{
    char *copy = malloc(strlen(value) + 1);
    strcpy(copy, value);
    return copy;
}

// Java: SUBSTITUTION_COMPARATOR - try long prefixes before short prefixes
static int compareSubstitutions(const void *left, const void *right)
{
    const PathSubstitution *a = left;
    const PathSubstitution *b = right;
    size_t countA = nameCount(a->basePath);
    size_t countB = nameCount(b->basePath);
    if (countA < countB) return 1;
    if (countA > countB) return -1;
    return 0;
}

// Java: public static void addRoot(Path root)
void LIBMATTI_FML_PathPrettyPrinting_AddRoot(const char *root)
{
    LIBMATTI_FML_PathPrettyPrinting_AddSubstitution(root, "", "");
}

// Java: public static void addSubstitution(Path root, String prefix, String suffix)
void LIBMATTI_FML_PathPrettyPrinting_AddSubstitution(const char *root, const char *prefix, const char *suffix)
{
    PathSubstitution *newSubstitutions = malloc(sizeof(PathSubstitution) * (SUBSTITUTIONS_COUNT + 1));
    for (size_t i = 0; i < SUBSTITUTIONS_COUNT; i++)
        newSubstitutions[i] = SUBSTITUTIONS[i];
    newSubstitutions[SUBSTITUTIONS_COUNT] = (PathSubstitution){
        .basePath = duplicateString(root),
        .prefix = duplicateString(prefix),
        .suffix = duplicateString(suffix),
    };
    free(SUBSTITUTIONS);
    SUBSTITUTIONS = newSubstitutions;
    SUBSTITUTIONS_COUNT++;
    qsort(SUBSTITUTIONS, SUBSTITUTIONS_COUNT, sizeof(PathSubstitution), compareSubstitutions);
}

// Java: path.startsWith(substitution.basePath)
static int pathStartsWith(const char *path, const char *basePath)
{
    size_t baseLength = strlen(basePath);
    return strncmp(path, basePath, baseLength) == 0 &&
           (path[baseLength] == '\0' || path[baseLength] == '/' || baseLength == 0 || basePath[baseLength - 1] == '/');
}

// Java: basePath.relativize(path)
static char *relativize(const char *basePath, const char *path)
{
    size_t baseLength = strlen(basePath);
    const char *rest = path + baseLength;
    while (*rest == '/')
        rest++;
    return duplicateString(rest);
}

// Java: public static String prettyPrint(Path path)
char *LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(const char *path)
{
    char *resultPath = NULL;

    for (size_t i = 0; i < SUBSTITUTIONS_COUNT; i++)
    {
        PathSubstitution *substitution = &SUBSTITUTIONS[i];
        if (pathStartsWith(path, substitution->basePath))
        {
            char *relative = relativize(substitution->basePath, path);
            size_t length = strlen(substitution->prefix) + strlen(relative) + strlen(substitution->suffix) + 1;
            resultPath = malloc(length);
            snprintf(resultPath, length, "%s%s%s", substitution->prefix, relative, substitution->suffix);
            free(relative);
            break;
        }
    }

    // Java: no known prefix, it might come from Gradle like in dev, or similar
    if (resultPath == NULL)
    {
        if (LIBMATTI_JNF_Files_IsDirectory(path))
            resultPath = LIBMATTI_JNF_Path_ToAbsolutePath(path);
        else
            resultPath = duplicateString(path);
    }

    // Java: resultPath.replace('\\', '/')
    for (char *c = resultPath; *c != '\0'; c++)
        if (*c == '\\')
            *c = '/';

    return resultPath;
}
