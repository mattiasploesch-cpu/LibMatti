//
// Created by administrator on 09.09.26.
//

#include "SecureJar.h"

#include "libmatti/bsl/sjh/jarhandling/JarContentsBuilder.h"
#include "libmatti/bsl/sjh/jarhandling/JarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/impl/Jar.h"

#include <stdlib.h>

// Java: static SecureJar.from(Path... paths)
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromPaths(const char **paths, size_t pathCount)
{
    LIBMATTI_JH_JarContentsBuilder *builder = LIBMATTI_JH_JarContentsBuilder_New();
    LIBMATTI_JH_JarContentsBuilder_Paths(builder, paths, pathCount);

    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContentsBuilder_Build(builder);
    LIBMATTI_JH_JarContentsBuilder_Free(builder);

    if (contents == NULL) return NULL;

    return LIBMATTI_JH_SecureJar_FromContents(contents);
}

// Java: static SecureJar.from(JarContents contents) -> from(contents, JarMetadata.from(contents))
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromContents(LIBMATTI_JH_JarContents *contents)
{
    LIBMATTI_JH_JarMetadata *metadata = LIBMATTI_JH_JarMetadata_From(contents);
    if (metadata == NULL) return NULL;

    return LIBMATTI_JH_SecureJar_FromContentsAndMetadata(contents, metadata);
}

// Java: static SecureJar.from(JarContents contents, JarMetadata metadata) -> new Jar(...)
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromContentsAndMetadata(LIBMATTI_JH_JarContents *contents, LIBMATTI_JH_JarMetadata *metadata)
{
    return (LIBMATTI_JH_SecureJar *)LIBMATTI_JH_Jar_New((LIBMATTI_JH_JarContentsImpl *)contents, metadata);
}

LIBMATTI_JH_ModuleDataProvider *LIBMATTI_JH_SecureJar_ModuleDataProvider(LIBMATTI_JH_SecureJar *jar)
{
    return jar->vtable->moduleDataProvider(jar);
}

const char *LIBMATTI_JH_SecureJar_GetPrimaryPath(const LIBMATTI_JH_SecureJar *jar)
{
    return jar->vtable->getPrimaryPath((LIBMATTI_JH_SecureJar *)jar);
}

LIBMATTI_JH_Status LIBMATTI_JH_SecureJar_VerifyPath(const LIBMATTI_JH_SecureJar *jar, const char *path)
{
    return jar->vtable->verifyPath((LIBMATTI_JH_SecureJar *)jar, path);
}

LIBMATTI_JH_Status LIBMATTI_JH_SecureJar_GetFileStatus(const LIBMATTI_JH_SecureJar *jar, const char *name)
{
    return jar->vtable->getFileStatus((LIBMATTI_JH_SecureJar *)jar, name);
}

int LIBMATTI_JH_SecureJar_HasSecurityData(const LIBMATTI_JH_SecureJar *jar)
{
    return jar->vtable->hasSecurityData((LIBMATTI_JH_SecureJar *)jar);
}

const char *LIBMATTI_JH_SecureJar_Name(const LIBMATTI_JH_SecureJar *jar)
{
    return jar->vtable->name((LIBMATTI_JH_SecureJar *)jar);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_JH_SecureJar_GetPath(const LIBMATTI_JH_SecureJar *jar, const char *first)
{
    return jar->vtable->getPath((LIBMATTI_JH_SecureJar *)jar, first);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_JH_SecureJar_GetRootPath(const LIBMATTI_JH_SecureJar *jar)
{
    return jar->vtable->getRootPath((LIBMATTI_JH_SecureJar *)jar);
}

void LIBMATTI_JH_SecureJar_Close(LIBMATTI_JH_SecureJar *jar)
{
    jar->vtable->close(jar);
}