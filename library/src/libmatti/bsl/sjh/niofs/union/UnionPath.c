//
// Created by administrator on 09.09.26.
//

#include "UnionPath.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/java/net/URI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_UNION_UnionPath *union_path_new_absolute(LIBMATTI_UNION_UnionFileSystem *fileSystem, int absolute,
                                                         const char **pathParts, size_t partCount)
{
    LIBMATTI_UNION_UnionPath *path = calloc(1, sizeof(LIBMATTI_UNION_UnionPath));

    path->fileSystem = fileSystem;
    path->absolute = absolute;
    path->partCount = partCount;
    path->normalized = NULL;

    if (partCount > 0)
    {
        path->pathParts = calloc(partCount, sizeof(char *));
        for (size_t i = 0; i < partCount; i++) path->pathParts[i] = strdup(pathParts[i]);
    }

    return path;
}

static LIBMATTI_UNION_UnionPath *union_path_new_normalized(LIBMATTI_UNION_UnionFileSystem *fileSystem, int absolute,
                                                           const char **pathParts, size_t partCount)
{
    LIBMATTI_UNION_UnionPath *path = union_path_new_absolute(fileSystem, absolute, pathParts, partCount);
    path->normalized = path;
    return path;
}

// Java: getPathParts - split on '/', skip empty parts and backslashes are converted
static void get_path_parts(const char *longstring, char ***outParts, size_t *outCount)
{
    size_t len = strlen(longstring);
    char *clean = strdup(longstring);

    for (size_t i = 0; i < len; i++)
    {
        if (clean[i] == '\\') clean[i] = '/';
    }

    char **parts = NULL;
    size_t count = 0;
    size_t startIndex = 0;

    while (startIndex < len)
    {
        const char *slash = strchr(clean + startIndex, '/');
        if (slash == NULL)
        {
            parts = realloc(parts, sizeof(char *) * (count + 1));
            parts[count] = strdup(clean + startIndex);
            count++;
            break;
        }

        size_t index = (size_t)(slash - clean);
        if (index != startIndex) // skip double slash and slash at start/end
        {
            char *part = malloc(index - startIndex + 1);
            memcpy(part, clean + startIndex, index - startIndex);
            part[index - startIndex] = '\0';
            parts = realloc(parts, sizeof(char *) * (count + 1));
            parts[count] = part;
            count++;
        }
        startIndex = index + 1;
    }

    free(clean);
    *outParts = parts;
    *outCount = count;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_New(LIBMATTI_UNION_UnionFileSystem *fileSystem, const char **pathParts, size_t partCount)
{
    // Java: UnionPath(UnionFileSystem, String... pathParts)
    // Joins the non-empty parts with '/', detects absolute, then re-splits
    size_t totalLength = 1;
    size_t nonEmpty = 0;
    for (size_t i = 0; i < partCount; i++)
    {
        if (pathParts[i][0] != '\0')
        {
            totalLength += strlen(pathParts[i]);
            if (nonEmpty > 0) totalLength++;
            nonEmpty++;
        }
    }

    char *joined = malloc(totalLength);
    joined[0] = '\0';

    size_t added = 0;
    for (size_t i = 0; i < partCount; i++)
    {
        if (pathParts[i][0] == '\0') continue;
        if (added > 0) strcat(joined, "/");
        strcat(joined, pathParts[i]);
        added++;
    }

    int absolute = joined[0] == '/';

    char **parts;
    size_t count;
    get_path_parts(joined, &parts, &count);

    LIBMATTI_UNION_UnionPath *path = calloc(1, sizeof(LIBMATTI_UNION_UnionPath));
    path->fileSystem = fileSystem;
    path->absolute = absolute;
    path->partCount = count;
    path->pathParts = parts;
    path->normalized = NULL;

    free(joined);
    return path;
}

void LIBMATTI_UNION_UnionPath_Free(LIBMATTI_UNION_UnionPath *path)
{
    if (path == NULL) return;

    for (size_t i = 0; i < path->partCount; i++) free(path->pathParts[i]);
    free(path->pathParts);

    // The normalized path may be the path itself or a separate cached instance
    if (path->normalized != NULL && path->normalized != path)
        LIBMATTI_UNION_UnionPath_Free(path->normalized);

    free(path);
}

LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionPath_GetFileSystem(const LIBMATTI_UNION_UnionPath *path)
{
    return path->fileSystem;
}

int LIBMATTI_UNION_UnionPath_IsAbsolute(const LIBMATTI_UNION_UnionPath *path)
{
    return path->absolute;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetRoot(const LIBMATTI_UNION_UnionPath *path)
{
    return path->fileSystem->root;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetFileName(const LIBMATTI_UNION_UnionPath *path)
{
    if (path->partCount > 0)
    {
        const char *part = path->pathParts[path->partCount - 1];
        return union_path_new_absolute(path->fileSystem, 0, &part, 1);
    }
    return union_path_new_absolute(path->fileSystem, 0, NULL, 0);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetParent(const LIBMATTI_UNION_UnionPath *path)
{
    if (path->partCount > 0)
    {
        return union_path_new_absolute(path->fileSystem, path->absolute, (const char **)path->pathParts, path->partCount - 1);
    }
    return NULL;
}

size_t LIBMATTI_UNION_UnionPath_GetNameCount(const LIBMATTI_UNION_UnionPath *path)
{
    return path->partCount;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetName(const LIBMATTI_UNION_UnionPath *path, size_t index)
{
    const char *part = path->pathParts[index];
    return union_path_new_absolute(path->fileSystem, 0, &part, 1);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Subpath(const LIBMATTI_UNION_UnionPath *path, size_t beginIndex, size_t endIndex)
{
    // Java: UnionPath(false) for the special empty case
    if (!path->absolute && path->partCount == 0 && beginIndex == 0 && endIndex == 1)
        return union_path_new_absolute(path->fileSystem, 0, NULL, 0);

    if (!path->absolute && beginIndex == 0 && endIndex == path->partCount)
    {
        // Java: returns this
        return (LIBMATTI_UNION_UnionPath *)path;
    }

    return union_path_new_absolute(path->fileSystem, 0, (const char **)(path->pathParts + beginIndex), endIndex - beginIndex);
}

static int check_arrays_match(const char **array1, size_t length1, const char **array2, size_t length2, int reverse)
{
    size_t length = length1 < length2 ? length1 : length2;

    for (size_t i = 0; i < length; i++)
    {
        const char *a = reverse ? array1[length1 - i - 1] : array1[i];
        const char *b = reverse ? array2[length2 - i - 1] : array2[i];
        if (strcmp(a, b) != 0) return 0;
    }
    return 1;
}

int LIBMATTI_UNION_UnionPath_StartsWith(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other)
{
    if (other->fileSystem != path->fileSystem) return 0;
    if (path->absolute != other->absolute) return 0;

    return check_arrays_match((const char **)path->pathParts, path->partCount,
                              (const char **)other->pathParts, other->partCount, 0);
}

int LIBMATTI_UNION_UnionPath_EndsWith(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other)
{
    if (other->fileSystem != path->fileSystem) return 0;
    if (!path->absolute && other->absolute) return 0;

    return check_arrays_match((const char **)path->pathParts, path->partCount,
                              (const char **)other->pathParts, other->partCount, 1);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Normalize(const LIBMATTI_UNION_UnionPath *path)
{
    if (path->normalized != NULL) return path->normalized;

    // Java: Deque logic - "." is skipped, ".." pops the previous part
    char **normParts = NULL;
    size_t normCount = 0;

    for (size_t i = 0; i < path->partCount; i++)
    {
        const char *part = path->pathParts[i];

        if (strcmp(part, ".") == 0)
        {
            // skip
        }
        else if (strcmp(part, "..") == 0)
        {
            if (normCount == 0 || strcmp(normParts[normCount - 1], "..") == 0)
            {
                // .. on an empty path is allowed, keep it
                normParts = realloc(normParts, sizeof(char *) * (normCount + 1));
                normParts[normCount] = strdup(part);
                normCount++;
            }
            else
            {
                free(normParts[normCount - 1]);
                normCount--;
            }
        }
        else
        {
            normParts = realloc(normParts, sizeof(char *) * (normCount + 1));
            normParts[normCount] = strdup(part);
            normCount++;
        }
    }

    LIBMATTI_UNION_UnionPath *normalized = calloc(1, sizeof(LIBMATTI_UNION_UnionPath));
    normalized->fileSystem = path->fileSystem;
    normalized->absolute = path->absolute;
    normalized->partCount = normCount;
    normalized->pathParts = normParts;
    normalized->normalized = normalized;

    ((LIBMATTI_UNION_UnionPath *)path)->normalized = normalized;
    return normalized;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Resolve(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other)
{
    if (other->absolute)
        return (LIBMATTI_UNION_UnionPath *)other; // Java: returns the other path unchanged

    size_t mergedCount = path->partCount + other->partCount;
    char **merged = calloc(mergedCount, sizeof(char *));

    for (size_t i = 0; i < path->partCount; i++) merged[i] = path->pathParts[i];
    for (size_t i = 0; i < other->partCount; i++) merged[path->partCount + i] = other->pathParts[i];

    LIBMATTI_UNION_UnionPath *result = union_path_new_absolute(path->fileSystem, path->absolute, (const char **)merged, mergedCount);

    // Java: merged is a new List referencing the same strings; only the array is freed here
    free(merged);

    return result;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Relativize(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other)
{
    size_t length = path->partCount < other->partCount ? path->partCount : other->partCount;
    size_t i = 0;

    while (i < length)
    {
        if (strcmp(path->pathParts[i], other->pathParts[i]) != 0) break;
        i++;
    }

    size_t remaining = path->partCount - i;

    if (remaining == 0 && i == other->partCount)
    {
        return union_path_new_absolute(path->fileSystem, 0, NULL, 0);
    }
    else if (remaining == 0)
    {
        return LIBMATTI_UNION_UnionPath_Subpath(other, i, other->partCount);
    }
    else
    {
        char **upDots = calloc(remaining, sizeof(char *));
        for (size_t j = 0; j < remaining; j++) upDots[j] = strdup("..");

        if (i == other->partCount)
        {
            LIBMATTI_UNION_UnionPath *result = union_path_new_absolute(path->fileSystem, 0, (const char **)upDots, remaining);
            for (size_t j = 0; j < remaining; j++) free(upDots[j]);
            free(upDots);
            return result;
        }
        else
        {
            LIBMATTI_UNION_UnionPath *subpath = LIBMATTI_UNION_UnionPath_Subpath(other, i, other->partCount);
            size_t mergedCount = remaining + subpath->partCount;
            char **merged = calloc(mergedCount, sizeof(char *));

            for (size_t j = 0; j < remaining; j++) merged[j] = upDots[j];
            for (size_t j = 0; j < subpath->partCount; j++) merged[remaining + j] = subpath->pathParts[j];

            LIBMATTI_UNION_UnionPath *result = union_path_new_absolute(path->fileSystem, 0, (const char **)merged, mergedCount);

            // merged borrows from upDots and subpath; only the array is freed here
            free(merged);
            for (size_t j = 0; j < remaining; j++) free(upDots[j]);
            free(upDots);
            LIBMATTI_UNION_UnionPath_Free(subpath);
            return result;
        }
    }
}

char *LIBMATTI_UNION_UnionPath_ToString(const LIBMATTI_UNION_UnionPath *path)
{
    // Java: (absolute ? "/" : "") + String.join("/", pathParts)
    size_t total = (path->absolute ? 1 : 0) + 1;

    for (size_t i = 0; i < path->partCount; i++)
    {
        total += strlen(path->pathParts[i]);
        if (i > 0) total++;
    }

    char *result = malloc(total);
    result[0] = '\0';

    if (path->absolute) strcat(result, "/");
    for (size_t i = 0; i < path->partCount; i++)
    {
        if (i > 0) strcat(result, "/");
        strcat(result, path->pathParts[i]);
    }

    return result;
}

int LIBMATTI_UNION_UnionPath_Equals(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other)
{
    if (other->fileSystem != path->fileSystem) return 0;
    if (path->absolute != other->absolute) return 0;
    if (path->partCount != other->partCount) return 0;

    for (size_t i = 0; i < path->partCount; i++)
    {
        if (strcmp(path->pathParts[i], other->pathParts[i]) != 0) return 0;
    }
    return 1;
}

LIBMATTI_JN_URI *LIBMATTI_UNION_UnionPath_ToUri(const LIBMATTI_UNION_UnionPath *path)
{
    // Java: new URI(getScheme(), null, key + "!" + toAbsolutePath(), null)
    // C: build the string "union:" + key + "!" + absolute path and parse it
    char *key = LIBMATTI_UNION_UnionFileSystem_GetKey(path->fileSystem);

    int allocated = 0;
    LIBMATTI_UNION_UnionPath *absolutePath;
    if (path->absolute)
    {
        absolutePath = (LIBMATTI_UNION_UnionPath *)path;
    }
    else
    {
        absolutePath = LIBMATTI_UNION_UnionPath_Resolve(LIBMATTI_UNION_UnionPath_GetRoot(path), path);
        allocated = 1;
    }

    char *absoluteString = LIBMATTI_UNION_UnionPath_ToString(absolutePath);

    // the key may contain '#' (makeKey suffix), which must be percent-encoded in the URI
    size_t keyLength = strlen(key);
    size_t total = strlen("union:") + keyLength * 3 + 1 + strlen(absoluteString) + 1;
    char *spec = malloc(total);

    size_t o = 0;
    memcpy(spec, "union:", 6);
    o = 6;
    for (size_t i = 0; i < keyLength; i++)
    {
        if (key[i] == '#')
        {
            spec[o++] = '%';
            spec[o++] = '2';
            spec[o++] = '3';
        }
        else
        {
            spec[o++] = key[i];
        }
    }
    spec[o++] = '!';
    strcpy(spec + o, absoluteString);

    free(absoluteString);
    if (allocated) LIBMATTI_UNION_UnionPath_Free(absolutePath);
    free(key);

    LIBMATTI_JN_URI *uri = LIBMATTI_JN_URI_Create(spec);
    free(spec);
    return uri;
}