//
// Created by administrator on 09.09.26.
//

#include "VirtualJar.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystemProvider.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"

#include <stdlib.h>
#include <unistd.h>

// Java: newFileSystem((path, basePath) -> false, referencePath) - a filter that always excludes
static int virtual_filter_false(const char *entry, const char *basePath, void *userdata)
{
    (void)entry;
    (void)basePath;
    (void)userdata;
    return 0;
}

static const LIBMATTI_UNION_UnionPathFilter VIRTUAL_FILTER = {virtual_filter_false, NULL};

// ---------------------------------------------------------------------------
// VirtualJarModuleDataProvider
// ---------------------------------------------------------------------------

static const char *virtual_provider_name(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_VirtualJarModuleDataProvider *provider = (LIBMATTI_JH_VirtualJarModuleDataProvider *)self;
    return provider->jar->moduleDescriptor->name;
}

static LIBMATTI_JL_ModuleDescriptor *virtual_provider_descriptor(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_VirtualJarModuleDataProvider *provider = (LIBMATTI_JH_VirtualJarModuleDataProvider *)self;
    return provider->jar->moduleDescriptor;
}

static LIBMATTI_JN_URI *virtual_provider_uri(LIBMATTI_JH_ModuleDataProvider *self)
{
    (void)self;
    return NULL; // Java: @Nullable URI uri() { return null; }
}

static int virtual_provider_find_file(LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JN_URI **out)
{
    (void)self;
    (void)name;
    (void)out;
    return 0; // Java: Optional.empty()
}

static int virtual_provider_open(LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JI_InputStream **out)
{
    (void)self;
    (void)name;
    (void)out;
    return 0; // Java: Optional.empty()
}

static LIBMATTI_JU_Manifest *virtual_provider_get_manifest(LIBMATTI_JH_ModuleDataProvider *self)
{
    LIBMATTI_JH_VirtualJarModuleDataProvider *provider = (LIBMATTI_JH_VirtualJarModuleDataProvider *)self;
    return provider->jar->manifest;
}

// ---------------------------------------------------------------------------
// VirtualJar
// ---------------------------------------------------------------------------

static LIBMATTI_JH_ModuleDataProvider *virtual_module_data_provider(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;
    return &jar->moduleData.base;
}

static const char *virtual_get_primary_path(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;
    return LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(jar->dummyFileSystem);
}

static LIBMATTI_JH_Status virtual_verify_path(LIBMATTI_JH_SecureJar *self, const char *path)
{
    (void)self;
    (void)path;
    return LIBMATTI_JH_STATUS_NONE;
}

static LIBMATTI_JH_Status virtual_get_file_status(LIBMATTI_JH_SecureJar *self, const char *name)
{
    (void)self;
    (void)name;
    return LIBMATTI_JH_STATUS_NONE;
}

static int virtual_has_security_data(LIBMATTI_JH_SecureJar *self)
{
    (void)self;
    return 0;
}

static const char *virtual_name(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;
    return jar->moduleDescriptor->name;
}

static LIBMATTI_UNION_UnionPath *virtual_get_path(LIBMATTI_JH_SecureJar *self, const char *first)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;
    const char *parts[1] = {first};
    return LIBMATTI_UNION_UnionFileSystem_GetPath(jar->dummyFileSystem, parts, 1);
}

static LIBMATTI_UNION_UnionPath *virtual_get_root_path(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;
    return LIBMATTI_UNION_UnionFileSystem_GetRoot(jar->dummyFileSystem);
}

static void virtual_close(LIBMATTI_JH_SecureJar *self)
{
    LIBMATTI_JH_VirtualJar *jar = (LIBMATTI_JH_VirtualJar *)self;

    LIBMATTI_UNION_UnionFileSystem_Close(jar->dummyFileSystem);
    LIBMATTI_JL_ModuleDescriptor_Free(jar->moduleDescriptor);
    LIBMATTI_JU_Manifest_Free(jar->manifest);
    free(jar);
}

static const LIBMATTI_JH_SecureJarVTable VIRTUAL_JAR_VTABLE = {
    virtual_module_data_provider,
    virtual_get_primary_path,
    virtual_verify_path,
    virtual_get_file_status,
    virtual_has_security_data,
    virtual_name,
    virtual_get_path,
    virtual_get_root_path,
    virtual_close
};

LIBMATTI_JH_VirtualJar *LIBMATTI_JH_VirtualJar_New(const char *name, const char *referencePath,
                                                   char **packages, size_t packageCount)
{
    // Java: if (!Files.exists(referencePath)) throw IllegalArgumentException
    if (access(referencePath, F_OK) != 0) return NULL;

    LIBMATTI_JH_VirtualJar *jar = calloc(1, sizeof(LIBMATTI_JH_VirtualJar));
    jar->base.vtable = &VIRTUAL_JAR_VTABLE;
    jar->manifest = LIBMATTI_JU_Manifest_CreateEmpty();

    // Java: ModuleDescriptor.newAutomaticModule(name).packages(Set.of(packages)).build()
    jar->moduleDescriptor = LIBMATTI_JL_ModuleDescriptor_Create(name, NULL, 0);
    LIBMATTI_JL_ModuleDescriptor_AddModifier(jar->moduleDescriptor, LIBMATTI_JL_MODIFIER_AUTOMATIC);
    for (size_t i = 0; i < packageCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddPackage(jar->moduleDescriptor, packages[i]);

    // Java: UFSP.newFileSystem((path, basePath) -> false, referencePath)
    const char *refs[1] = {referencePath};
    jar->dummyFileSystem = LIBMATTI_UNION_UnionFileSystemProvider_NewFileSystem(
        LIBMATTI_UNION_UnionFileSystemProvider_Get(), (LIBMATTI_UNION_UnionPathFilter *)&VIRTUAL_FILTER, refs, 1);

    if (jar->dummyFileSystem == NULL)
    {
        LIBMATTI_JL_ModuleDescriptor_Free(jar->moduleDescriptor);
        LIBMATTI_JU_Manifest_Free(jar->manifest);
        free(jar);
        return NULL;
    }

    jar->moduleData.base.name = virtual_provider_name;
    jar->moduleData.base.descriptor = virtual_provider_descriptor;
    jar->moduleData.base.uri = virtual_provider_uri;
    jar->moduleData.base.findFile = virtual_provider_find_file;
    jar->moduleData.base.open = virtual_provider_open;
    jar->moduleData.base.getManifest = virtual_provider_get_manifest;
    jar->moduleData.jar = jar;

    return jar;
}