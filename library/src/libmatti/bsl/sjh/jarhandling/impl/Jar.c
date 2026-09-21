//
// Created by administrator on 09.09.26.
//

#include "Jar.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------
// JarModuleDataProvider (Java: private record JarModuleDataProvider(Jar jar))
// ---------------------------------------------------------------------------

static const char *jar_provider_name(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    return LIBMATTI_JH_JarMetadata_Name(provider->jar->metadata);
}

static LIBMATTI_JL_ModuleDescriptor *jar_provider_descriptor(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    return LIBMATTI_JH_Jar_ComputeDescriptor(provider->jar);
}

static LIBMATTI_JN_URI *jar_provider_uri(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    return LIBMATTI_JH_Jar_GetURI(provider->jar);
}

static int jar_provider_find_file(LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JN_URI **out)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    return LIBMATTI_JH_Jar_FindFile(provider->jar, name, out);
}

static int jar_provider_open(LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JI_InputStream **out)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    LIBMATTI_UNION_UnionFileSystem *filesystem = provider->jar->filesystem;

    const char *parts[1] = {name};
    LIBMATTI_UNION_UnionPath *path = LIBMATTI_UNION_UnionFileSystem_GetPath(filesystem, parts, 1);

    unsigned char *bytes = NULL;
    size_t length = 0;
    int found = LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(filesystem, path, &bytes, &length);
    LIBMATTI_UNION_UnionPath_Free(path);

    if (found)
    {
        *out = LIBMATTI_JI_InputStream_Create(bytes, length);
        free(bytes);
    }
    return found;
}

static LIBMATTI_JU_Manifest *jar_provider_get_manifest(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_JarModuleDataProvider *provider = (LIBMATTI_JH_JarModuleDataProvider *)self;
    return provider->jar->manifest;
}

// ---------------------------------------------------------------------------
// Jar
// ---------------------------------------------------------------------------

LIBMATTI_JN_URI *LIBMATTI_JH_Jar_GetURI(const LIBMATTI_JH_Jar *jar)
{
    // Java: filesystem.getRootDirectories().iterator().next().toUri()
    return LIBMATTI_UNION_UnionPath_ToUri(jar->filesystem->root);
}

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_Jar_ComputeDescriptor(const LIBMATTI_JH_Jar *jar)
{
    return LIBMATTI_JH_JarMetadata_Descriptor(jar->metadata);
}

int LIBMATTI_JH_Jar_FindFile(const LIBMATTI_JH_Jar *jar, const char *name, LIBMATTI_JN_URI **out)
{
    return LIBMATTI_JH_JarContents_FindFile((LIBMATTI_JH_JarContents *)jar->contents, name, out);
}

// ---------------------------------------------------------------------------
// SecureJar implementation
// ---------------------------------------------------------------------------

static LIBMATTI_JH_ModuleDataProvider *jar_module_data_provider(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;
    return &jar->moduleDataProvider.base;
}

static const char *jar_get_primary_path(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;
    return LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(jar->filesystem);
}

static LIBMATTI_JH_Status jar_verify_path(LIBMATTI_JH_SecureJar *self, const char *path)
{
    (void)self;
    (void)path;
    // TODO: signingData.verifyPath
    return LIBMATTI_JH_STATUS_NONE;
}

static LIBMATTI_JH_Status jar_get_file_status(LIBMATTI_JH_SecureJar *self, const char *name)
{
    (void)self;
    (void)name;
    // TODO: signingData.getFileStatus
    return LIBMATTI_JH_STATUS_NONE;
}

static int jar_has_security_data(LIBMATTI_JH_SecureJar *self)
{
    (void)self;
    // TODO: signingData.hasSecurityData()
    return 0;
}

static const char *jar_name(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;
    return LIBMATTI_JH_JarMetadata_Name(jar->metadata);
}

static LIBMATTI_UNION_UnionPath *jar_get_path(LIBMATTI_JH_SecureJar *self, const char *first)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;
    const char *parts[1] = {first};
    return LIBMATTI_UNION_UnionFileSystem_GetPath(jar->filesystem, parts, 1);
}

static LIBMATTI_UNION_UnionPath *jar_get_root_path(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;
    return LIBMATTI_UNION_UnionFileSystem_GetRoot(jar->filesystem);
}

static void jar_close(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_Jar *jar = (LIBMATTI_JH_Jar *)self;

    // Java: contents.close() - the metadata must outlive the contents' caches
    LIBMATTI_JH_JarMetadata_Free(jar->metadata);
    LIBMATTI_JH_JarContents_Close((LIBMATTI_JH_JarContents *)jar->contents);
    free(jar);
}

static const LIBMATTI_JH_SecureJarVTable JAR_VTABLE = {
    jar_module_data_provider,
    jar_get_primary_path,
    jar_verify_path,
    jar_get_file_status,
    jar_has_security_data,
    jar_name,
    jar_get_path,
    jar_get_root_path,
    jar_close
};

LIBMATTI_JH_Jar *LIBMATTI_JH_Jar_New(LIBMATTI_JH_JarContentsImpl *contents, LIBMATTI_JH_JarMetadata *metadata)
{
    LIBMATTI_JH_Jar *jar = calloc(1, sizeof(LIBMATTI_JH_Jar));

    jar->base.vtable = &JAR_VTABLE;
    jar->contents = contents;
    jar->manifest = contents->manifest;
    jar->filesystem = contents->filesystem;
    jar->metadata = metadata;

    jar->moduleDataProvider.base.name = jar_provider_name;
    jar->moduleDataProvider.base.descriptor = jar_provider_descriptor;
    jar->moduleDataProvider.base.uri = jar_provider_uri;
    jar->moduleDataProvider.base.findFile = jar_provider_find_file;
    jar->moduleDataProvider.base.open = jar_provider_open;
    jar->moduleDataProvider.base.getManifest = jar_provider_get_manifest;
    jar->moduleDataProvider.jar = jar;

    return jar;
}