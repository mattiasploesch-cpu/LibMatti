//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.SecureJar.

#ifndef MATTICRAFT_SECUREJAR_H
#define MATTICRAFT_SECUREJAR_H

#include "libmatti/bsl/sjh/niofs/union/UnionPathFilter.h"
#include "libmatti/java/io/InputStream.h"
#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/java/net/URI.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

typedef struct LIBMATTI_JH_JarContents LIBMATTI_JH_JarContents;
typedef struct LIBMATTI_JH_JarMetadata LIBMATTI_JH_JarMetadata;
typedef struct LIBMATTI_UNION_UnionPath LIBMATTI_UNION_UnionPath;

// Java: enum SecureJar.Status { NONE, INVALID, UNVERIFIED, VERIFIED }
typedef enum
{
    LIBMATTI_JH_STATUS_NONE = 0,
    LIBMATTI_JH_STATUS_INVALID,
    LIBMATTI_JH_STATUS_UNVERIFIED,
    LIBMATTI_JH_STATUS_VERIFIED
} LIBMATTI_JH_Status;

// Java: record Provider(String serviceName, List<String> providers)
typedef struct
{
    char *serviceName;
    char **providers;
    size_t providerCount;
} LIBMATTI_JH_Provider;

// Java: Provider.fromPath - parses service provider lines from the given content.
// Lines are trimmed, empty lines and '#' comments are skipped, and each provider
// is tested against the package filter (pkgFilter.test(p.replace('.','/'), root)).
LIBMATTI_JH_Provider *LIBMATTI_JH_Provider_FromContent(const char *serviceName, const char *content, size_t length,
                                                       LIBMATTI_UNION_UnionPathFilter *pkgFilter, const char *root);
void LIBMATTI_JH_Provider_Free(LIBMATTI_JH_Provider *provider);

// Java: interface ModuleDataProvider - function pointers, implementations
// (JarModuleDataProvider, VirtualJarModuleDataProvider) embed this as first member
typedef struct LIBMATTI_JH_ModuleDataProvider
{
    const char *(*name)(struct LIBMATTI_JH_ModuleDataProvider *self);
    LIBMATTI_JL_ModuleDescriptor *(*descriptor)(struct LIBMATTI_JH_ModuleDataProvider *self);
    // Java: @Nullable URI uri() - returns a new URI, caller frees
    LIBMATTI_JN_URI *(*uri)(struct LIBMATTI_JH_ModuleDataProvider *self);
    // Java: Optional<URI> findFile(String name) - 1 if found, *out is a new URI
    int (*findFile)(struct LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JN_URI **out);
    // Java: Optional<InputStream> open(String name) - 1 if found, *out is a new stream
    int (*open)(struct LIBMATTI_JH_ModuleDataProvider *self, const char *name, LIBMATTI_JI_InputStream **out);
    LIBMATTI_JU_Manifest *(*getManifest)(struct LIBMATTI_JH_ModuleDataProvider *self);
    // TODO: verifyAndGetSigners (signing)
} LIBMATTI_JH_ModuleDataProvider;

// Java: interface SecureJar - implemented by Jar and VirtualJar via a vtable
typedef struct LIBMATTI_JH_SecureJar LIBMATTI_JH_SecureJar;

typedef struct
{
    LIBMATTI_JH_ModuleDataProvider *(*moduleDataProvider)(LIBMATTI_JH_SecureJar *self);
    const char *(*getPrimaryPath)(LIBMATTI_JH_SecureJar *self);
    LIBMATTI_JH_Status (*verifyPath)(LIBMATTI_JH_SecureJar *self, const char *path);
    LIBMATTI_JH_Status (*getFileStatus)(LIBMATTI_JH_SecureJar *self, const char *name);
    int (*hasSecurityData)(LIBMATTI_JH_SecureJar *self);
    const char *(*name)(LIBMATTI_JH_SecureJar *self);
    LIBMATTI_UNION_UnionPath *(*getPath)(LIBMATTI_JH_SecureJar *self, const char *first);
    LIBMATTI_UNION_UnionPath *(*getRootPath)(LIBMATTI_JH_SecureJar *self);
    void (*close)(LIBMATTI_JH_SecureJar *self);
} LIBMATTI_JH_SecureJarVTable;

struct LIBMATTI_JH_SecureJar
{
    const LIBMATTI_JH_SecureJarVTable *vtable;
};

// Java: static SecureJar.from(Path...) / from(JarContents) / from(JarContents, JarMetadata)
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromPaths(const char **paths, size_t pathCount);
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromContents(LIBMATTI_JH_JarContents *contents);
LIBMATTI_JH_SecureJar *LIBMATTI_JH_SecureJar_FromContentsAndMetadata(LIBMATTI_JH_JarContents *contents, LIBMATTI_JH_JarMetadata *metadata);

LIBMATTI_JH_ModuleDataProvider *LIBMATTI_JH_SecureJar_ModuleDataProvider(LIBMATTI_JH_SecureJar *jar);
const char *LIBMATTI_JH_SecureJar_GetPrimaryPath(const LIBMATTI_JH_SecureJar *jar);
LIBMATTI_JH_Status LIBMATTI_JH_SecureJar_VerifyPath(const LIBMATTI_JH_SecureJar *jar, const char *path);
LIBMATTI_JH_Status LIBMATTI_JH_SecureJar_GetFileStatus(const LIBMATTI_JH_SecureJar *jar, const char *name);
int LIBMATTI_JH_SecureJar_HasSecurityData(const LIBMATTI_JH_SecureJar *jar);
const char *LIBMATTI_JH_SecureJar_Name(const LIBMATTI_JH_SecureJar *jar);
LIBMATTI_UNION_UnionPath *LIBMATTI_JH_SecureJar_GetPath(const LIBMATTI_JH_SecureJar *jar, const char *first);
LIBMATTI_UNION_UnionPath *LIBMATTI_JH_SecureJar_GetRootPath(const LIBMATTI_JH_SecureJar *jar);
// Java: close() - closes the underlying resources, frees the jar
void LIBMATTI_JH_SecureJar_Close(LIBMATTI_JH_SecureJar *jar);

#endif //MATTICRAFT_SECUREJAR_H