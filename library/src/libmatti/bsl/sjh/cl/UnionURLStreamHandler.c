#include "UnionURLStreamHandler.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystemProvider.h"

#include <stdlib.h>
#include <string.h>

const char *LIBMATTI_CL_UnionURLStreamHandler_Protocol(void)
{
    return "union";
}

// Java: Paths.get(u.toURI()) instanceof UnionPath - resolves "union:key!path" to the UnionPath
static LIBMATTI_UNION_UnionPath *uriToUnionPath(LIBMATTI_CL_URL *url)
{
    const char *ssp = LIBMATTI_JN_URI_SchemeSpecificPart(url);
    if (ssp == NULL) return NULL;

    const char *bang = strchr(ssp, '!');
    if (bang == NULL) return NULL;

    size_t keyLen = (size_t)(bang - ssp);
    const char *pathStr = bang + 1;

    // percent-decode the key ('#' was encoded as %23 in toUri)
    char *decodedKey = malloc(keyLen + 1);
    size_t d = 0;
    for (size_t i = 0; i < keyLen; i++)
    {
        if (ssp[i] == '%' && i + 2 < keyLen && ssp[i + 1] == '2' && ssp[i + 2] == '3')
        {
            decodedKey[d++] = '#';
            i += 2;
        }
        else
        {
            decodedKey[d++] = ssp[i];
        }
    }
    decodedKey[d] = '\0';

    LIBMATTI_UNION_UnionFileSystemProvider *provider = LIBMATTI_UNION_UnionFileSystemProvider_Get();
    for (size_t i = 0; i < provider->count; i++)
    {
        if (strcmp(provider->keys[i], decodedKey) == 0)
        {
            if (pathStr[0] == '/') pathStr++;
            const char **parts = NULL;
            size_t partCount = 0;
            if (pathStr[0] != '\0')
            {
                // split on '/'
                const char *p = pathStr;
                while (*p != '\0')
                {
                    const char *slash = strchr(p, '/');
                    size_t len = slash == NULL ? strlen(p) : (size_t)(slash - p);
                    parts = realloc(parts, sizeof(char *) * (partCount + 1));
                    parts[partCount] = strndup(p, len);
                    partCount++;
                    p = slash == NULL ? p + len : slash + 1;
                }
            }
            LIBMATTI_UNION_UnionPath *path = LIBMATTI_UNION_UnionFileSystem_GetPath(provider->fileSystems[i], parts, partCount);
            for (size_t j = 0; j < partCount; j++) free((void *)parts[j]);
            free(parts);
            free(decodedKey);
            return path;
        }
    }
    free(decodedKey);
    return NULL;
}

LIBMATTI_JI_InputStream *LIBMATTI_CL_UnionURLStreamHandler_InputStream(LIBMATTI_CL_URL *url)
{
    // Java: if (Paths.get(u.toURI()) instanceof UnionPath upath) return upath.buildInputStream();
    //       else throw new IllegalArgumentException("Invalid Path " + u.toURI() + " at UnionURLStreamHandler")
    LIBMATTI_UNION_UnionPath *path = uriToUnionPath(url);
    if (path == NULL) return NULL; // Java: IllegalArgumentException

    unsigned char *bytes = NULL;
    size_t length = 0;
    // Java: buildInputStream() -> fileSystem.buildInputStream(this)
    int ok = LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(path->fileSystem, path, &bytes, &length);
    LIBMATTI_UNION_UnionPath_Free(path);
    if (!ok)
    {
        free(bytes);
        return NULL;
    }
    LIBMATTI_JI_InputStream *stream = LIBMATTI_JI_InputStream_Create(bytes, length);
    free(bytes);
    return stream;
}

long LIBMATTI_CL_UnionURLStreamHandler_GetLastModified(LIBMATTI_CL_URL *url)
{
    LIBMATTI_UNION_UnionPath *path = uriToUnionPath(url);
    if (path == NULL) return 0; // Java: RuntimeException/UncheckedIOException
    LIBMATTI_UNION_BasicFileAttributes attributes;
    int found = LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(path->fileSystem, path, &attributes);
    LIBMATTI_UNION_UnionPath_Free(path);
    if (!found) return 0; // Java: UncheckedIOException
    // Java: Files.getLastModifiedTime(...).toMillis() - not tracked; Java IURLProvider default is 0
    return 0;
}

long LIBMATTI_CL_UnionURLStreamHandler_GetContentLength(LIBMATTI_CL_URL *url)
{
    LIBMATTI_UNION_UnionPath *path = uriToUnionPath(url);
    if (path == NULL) return -1; // Java: UncheckedIOException
    LIBMATTI_UNION_BasicFileAttributes attributes;
    int found = LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(path->fileSystem, path, &attributes);
    LIBMATTI_UNION_UnionPath_Free(path);
    if (!found) return -1; // Java: UncheckedIOException
    return (long)attributes.size;
}

LIBMATTI_CL_IURLProvider *LIBMATTI_CL_UnionURLStreamHandler_GetProvider(void)
{
    static LIBMATTI_CL_IURLProvider provider = {
        .protocol = LIBMATTI_CL_UnionURLStreamHandler_Protocol,
        .inputStreamFunction = LIBMATTI_CL_UnionURLStreamHandler_InputStream,
        .getLastModified = LIBMATTI_CL_UnionURLStreamHandler_GetLastModified,
        .getContentLength = LIBMATTI_CL_UnionURLStreamHandler_GetContentLength,
    };
    return &provider;
}