//
// Created by administrator on 09.09.26.
//

#include "UnionFileSystemProvider.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_UNION_UnionFileSystemProvider GLOBAL_PROVIDER;

LIBMATTI_UNION_UnionFileSystemProvider *LIBMATTI_UNION_UnionFileSystemProvider_Get(void)
{
    return &GLOBAL_PROVIDER;
}

const char *LIBMATTI_UNION_UnionFileSystemProvider_GetScheme(void)
{
    return "union";
}

// Java: private synchronized String makeKey(Path path)
// key = path (with '!' -> '_') + "#" + index++
static char *make_key(const char *path)
{
    size_t len = strlen(path);
    char *key = malloc(len + 32);

    size_t o = 0;
    for (size_t i = 0; i < len; i++)
        key[o++] = path[i] == '!' ? '_' : path[i];

    key[o] = '\0';

    char suffix[32];
    snprintf(suffix, sizeof(suffix), "#%d", GLOBAL_PROVIDER.index++);
    strcat(key, suffix);

    return key;
}

LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionFileSystemProvider_NewFileSystem(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                                                     LIBMATTI_UNION_UnionPathFilter *pathFilter,
                                                                                     const char **paths, size_t pathCount)
{
    // Java: newFileSystem(pathfilter, paths...) - need at least one path
    if (pathCount == 0) return NULL;

    char *key = make_key(paths[0]);
    LIBMATTI_UNION_UnionFileSystem *fileSystem = LIBMATTI_UNION_UnionFileSystem_New(provider, pathFilter, key, paths, pathCount);
    free(key);

    if (fileSystem == NULL) return NULL;

    // Java: fileSystems.put(key, ufs)
    provider->keys = realloc(provider->keys, sizeof(char *) * (provider->count + 1));
    provider->fileSystems = realloc(provider->fileSystems, sizeof(LIBMATTI_UNION_UnionFileSystem *) * (provider->count + 1));
    provider->keys[provider->count] = strdup(fileSystem->key);
    provider->fileSystems[provider->count] = fileSystem;
    provider->count++;

    return fileSystem;
}

void LIBMATTI_UNION_UnionFileSystemProvider_RemoveFileSystem(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                             LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    for (size_t i = 0; i < provider->count; i++)
    {
        if (provider->fileSystems[i] == fileSystem)
        {
            free(provider->keys[i]);
            provider->fileSystems[i] = provider->fileSystems[provider->count - 1];
            provider->keys[i] = provider->keys[provider->count - 1];
            provider->count--;
            break;
        }
    }
}