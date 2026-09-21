//
// Created by administrator on 09.09.26.
//

#include "JarContentsImpl.h"

#include "libmatti/bsl/sjh/jarhandling/JarContentsBuilder.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/bsl/sjh/niofs/union/UnionFileSystemProvider.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ---------------------------------------------------------------------------
// Provider record helpers (Java: SecureJar.Provider.fromPath)
// ---------------------------------------------------------------------------

LIBMATTI_JH_Provider *LIBMATTI_JH_Provider_FromContent(const char *serviceName, const char *content, size_t length,
                                                       LIBMATTI_UNION_UnionPathFilter *pkgFilter, const char *root)
{
    LIBMATTI_JH_Provider *provider = calloc(1, sizeof(LIBMATTI_JH_Provider));
    provider->serviceName = strdup(serviceName);

    char *copy = malloc(length + 1);
    memcpy(copy, content, length);
    copy[length] = '\0';

    char *saveptr;
    char *line = strtok_r(copy, "\n", &saveptr);

    while (line != NULL)
    {
        // strip \r
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';

        // Java: String::trim
        char *start = line;
        while (*start != '\0' && *start <= ' ') start++;
        char *end = start + strlen(start);
        while (end > start && *(end - 1) <= ' ') end--;
        *end = '\0';

        // Java: skip empty lines and '#' comments
        if (start[0] == '\0' || start[0] == '#')
        {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        // Java: pkgFilter.test(p.replace('.','/'), path.getRoot())
        if (pkgFilter != NULL)
        {
            size_t pathLength = strlen(start);
            char *slashPath = malloc(pathLength + 1);
            for (size_t i = 0; i <= pathLength; i++)
                slashPath[i] = start[i] == '.' ? '/' : start[i];

            int allowed = pkgFilter->test(slashPath, root, pkgFilter->userdata);
            free(slashPath);

            if (!allowed)
            {
                line = strtok_r(NULL, "\n", &saveptr);
                continue;
            }
        }

        provider->providers = realloc(provider->providers, sizeof(char *) * (provider->providerCount + 1));
        provider->providers[provider->providerCount] = strdup(start);
        provider->providerCount++;

        line = strtok_r(NULL, "\n", &saveptr);
    }

    free(copy);
    return provider;
}

void LIBMATTI_JH_Provider_Free(LIBMATTI_JH_Provider *provider)
{
    if (provider == NULL) return;

    free(provider->serviceName);
    for (size_t i = 0; i < provider->providerCount; i++) free(provider->providers[i]);
    free(provider->providers);
    free(provider);
}

// ---------------------------------------------------------------------------
// JarContentsImpl
// ---------------------------------------------------------------------------

static const char *MANIFEST_NAME = "META-INF/MANIFEST.MF";
static const char *NAUGHTY_SERVICE_FILE = "org.codehaus.groovy.runtime.ExtensionModule";

// Java: readManifestAndSigningData - read META-INF/MANIFEST.MF through the union fs.
// Signing data reading (JarInputStream) is TODO.
static LIBMATTI_JU_Manifest *read_manifest(LIBMATTI_UNION_UnionFileSystem *filesystem, LIBMATTI_JU_Manifest *defaultManifest)
{
    const char *parts[1] = {MANIFEST_NAME};
    LIBMATTI_UNION_UnionPath *manifestPath = LIBMATTI_UNION_UnionFileSystem_GetPath(filesystem, parts, 1);

    unsigned char *bytes = NULL;
    size_t length = 0;

    LIBMATTI_JU_Manifest *manifest = NULL;
    if (LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(filesystem, manifestPath, &bytes, &length))
    {
        manifest = LIBMATTI_JU_Manifest_Parse((const char *)bytes, length);
        free(bytes);
    }

    LIBMATTI_UNION_UnionPath_Free(manifestPath);

    if (manifest == NULL)
    {
        // Java: defaultManifest.get()
        if (defaultManifest != NULL) return defaultManifest;
        return LIBMATTI_JU_Manifest_CreateEmpty();
    }

    LIBMATTI_JU_Manifest_Free(defaultManifest);
    return manifest;
}

// Java: getPackagesExcluding - collect packages from .class files
typedef struct
{
    char **excluded;
    size_t excludedCount;
    char **packages;
    size_t packageCount;
} PackagesScanContext;

static int string_in_list(char **list, size_t count, const char *value)
{
    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(list[i], value) == 0) return 1;
    }
    return 0;
}

static int packages_scan_callback(const char *relativePath, int isDirectory, void *userdata)
{
    (void)isDirectory;
    PackagesScanContext *ctx = userdata;

    size_t length = strlen(relativePath);
    if (length < 6 || strcmp(relativePath + length - 6, ".class") != 0) return 1;

    // package = parent path with '/' -> '.'
    const char *lastSlash = strrchr(relativePath, '/');
    if (lastSlash == NULL) return 1; // root file, no package

    size_t packageLength = (size_t)(lastSlash - relativePath);
    if (packageLength == 0) return 1;

    // Java: skip subtrees of ignored root packages (META-INF and excluded roots)
    const char *firstSlash = strchr(relativePath, '/');
    if (firstSlash != NULL)
    {
        size_t rootLength = (size_t)(firstSlash - relativePath);
        char *rootPackage = malloc(rootLength + 1);
        memcpy(rootPackage, relativePath, rootLength);
        rootPackage[rootLength] = '\0';

        int ignored = strcmp(rootPackage, "META-INF") == 0 || string_in_list(ctx->excluded, ctx->excludedCount, rootPackage);
        free(rootPackage);

        if (ignored) return 1;
    }

    char *package = malloc(packageLength + 1);
    for (size_t i = 0; i < packageLength; i++)
        package[i] = relativePath[i] == '/' ? '.' : relativePath[i];
    package[packageLength] = '\0';

    // set semantics (Java: Set.copyOf)
    if (!string_in_list(ctx->packages, ctx->packageCount, package))
    {
        ctx->packages = realloc(ctx->packages, sizeof(char *) * (ctx->packageCount + 1));
        ctx->packages[ctx->packageCount] = package;
        ctx->packageCount++;
    }
    else
    {
        free(package);
    }

    return 1;
}

// Java: getMetaInfServices - collect META-INF/services providers
typedef struct
{
    LIBMATTI_JH_JarContentsImpl *impl;
    LIBMATTI_JH_Provider *providers;
    size_t providerCount;
} ServicesScanContext;

static int services_scan_callback(const char *relativePath, int isDirectory, void *userdata)
{
    if (isDirectory) return 1;

    const char *filename = strrchr(relativePath, '/');
    filename = filename != NULL ? filename + 1 : relativePath;

    // Java: NAUGHTY_SERVICE_FILES filter
    if (strcmp(filename, NAUGHTY_SERVICE_FILE) == 0) return 1;

    ServicesScanContext *ctx = userdata;
    LIBMATTI_UNION_UnionFileSystem *filesystem = ctx->impl->filesystem;

    const char *parts[1] = {relativePath};
    LIBMATTI_UNION_UnionPath *filePath = LIBMATTI_UNION_UnionFileSystem_GetPath(filesystem, parts, 1);

    unsigned char *bytes = NULL;
    size_t length = 0;

    if (LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(filesystem, filePath, &bytes, &length))
    {
        // Java: SecureJar.Provider.fromPath(path, filesystem.getFilesystemFilter())
        LIBMATTI_JH_Provider *provider = LIBMATTI_JH_Provider_FromContent(
            filename, (const char *)bytes, length,
            LIBMATTI_UNION_UnionFileSystem_GetFilesystemFilter(filesystem),
            LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(filesystem));

        ctx->providers = realloc(ctx->providers, sizeof(LIBMATTI_JH_Provider) * (ctx->providerCount + 1));
        ctx->providers[ctx->providerCount] = *provider;
        ctx->providerCount++;
        free(provider);
        free(bytes);
    }

    LIBMATTI_UNION_UnionPath_Free(filePath);
    return 1;
}

LIBMATTI_JH_JarContentsImpl *LIBMATTI_JH_JarContentsImpl_New(const char **paths, size_t pathCount,
                                                             LIBMATTI_JU_Manifest *defaultManifest,
                                                             LIBMATTI_UNION_UnionPathFilter *pathFilter)
{
    // Java: Arrays.stream(paths).filter(Files::exists).toArray()
    char **validPaths = calloc(pathCount, sizeof(char *));
    size_t validCount = 0;

    for (size_t i = 0; i < pathCount; i++)
    {
        if (access(paths[i], F_OK) == 0)
            validPaths[validCount++] = strdup(paths[i]);
    }

    // Java: throws UncheckedIOException "Invalid paths argument"
    if (validCount == 0)
    {
        free(validPaths);
        LIBMATTI_JU_Manifest_Free(defaultManifest);
        return NULL;
    }

    // Java: this.filesystem = UFSP.newFileSystem(pathFilter, validPaths)
    LIBMATTI_UNION_UnionFileSystem *filesystem = LIBMATTI_UNION_UnionFileSystemProvider_NewFileSystem(
        LIBMATTI_UNION_UnionFileSystemProvider_Get(), pathFilter, (const char **)validPaths, validCount);

    if (filesystem == NULL)
    {
        for (size_t i = 0; i < validCount; i++) free(validPaths[i]);
        free(validPaths);
        LIBMATTI_JU_Manifest_Free(defaultManifest);
        return NULL;
    }

    LIBMATTI_JH_JarContentsImpl *impl = calloc(1, sizeof(LIBMATTI_JH_JarContentsImpl));
    impl->filesystem = filesystem;
    impl->validPaths = validPaths;
    impl->validPathCount = validCount;
    impl->manifest = read_manifest(filesystem, defaultManifest);

    return impl;
}

void LIBMATTI_JH_JarContents_Close(LIBMATTI_JH_JarContents *contents)
{
    LIBMATTI_JH_JarContentsImpl *impl = (LIBMATTI_JH_JarContentsImpl *)contents;
    if (impl == NULL) return;

    LIBMATTI_UNION_UnionFileSystem_Close(impl->filesystem);

    for (size_t i = 0; i < impl->packageCount; i++) free(impl->packages[i]);
    free(impl->packages);

    for (size_t i = 0; i < impl->providerCount; i++) LIBMATTI_JH_Provider_Free(&impl->providers[i]);
    free(impl->providers);

    LIBMATTI_JU_Manifest_Free(impl->manifest);

    for (size_t i = 0; i < impl->validPathCount; i++) free(impl->validPaths[i]);
    free(impl->validPaths);

    free(impl);
}

LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContents_Of(const char *path)
{
    const char *paths[1] = {path};
    return LIBMATTI_JH_JarContents_OfPaths(paths, 1);
}

LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContents_OfPaths(const char **paths, size_t pathCount)
{
    LIBMATTI_JH_JarContentsBuilder *builder = LIBMATTI_JH_JarContentsBuilder_New();
    LIBMATTI_JH_JarContentsBuilder_Paths(builder, paths, pathCount);
    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContentsBuilder_Build(builder);
    LIBMATTI_JH_JarContentsBuilder_Free(builder);
    return contents;
}

const char *LIBMATTI_JH_JarContents_GetPrimaryPath(const LIBMATTI_JH_JarContents *contents)
{
    const LIBMATTI_JH_JarContentsImpl *impl = (const LIBMATTI_JH_JarContentsImpl *)contents;
    return LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(impl->filesystem);
}

int LIBMATTI_JH_JarContents_FindFile(const LIBMATTI_JH_JarContents *contents, const char *name, LIBMATTI_JN_URI **out)
{
    const LIBMATTI_JH_JarContentsImpl *impl = (const LIBMATTI_JH_JarContentsImpl *)contents;

    // Java: rel = filesystem.getPath(name); root.resolve(rel) exists -> toUri
    const char *parts[1] = {name};
    LIBMATTI_UNION_UnionPath *relative = LIBMATTI_UNION_UnionFileSystem_GetPath(impl->filesystem, parts, 1);

    if (LIBMATTI_UNION_UnionFileSystem_Exists(impl->filesystem, relative))
    {
        *out = LIBMATTI_UNION_UnionPath_ToUri(relative);
        LIBMATTI_UNION_UnionPath_Free(relative);
        return 1;
    }

    LIBMATTI_UNION_UnionPath_Free(relative);
    return 0;
}

// Java: byte[] getFileBytes(String path) - reads an entry through the union fs;
// NULL when the entry is absent. Caller frees, *outLength set.
unsigned char *LIBMATTI_JH_JarContents_OpenFile(const LIBMATTI_JH_JarContents *contents, const char *name,
                                                size_t *outLength)
{
    const LIBMATTI_JH_JarContentsImpl *impl = (const LIBMATTI_JH_JarContentsImpl *)contents;
    *outLength = 0;

    const char *parts[1] = {name};
    LIBMATTI_UNION_UnionPath *relative = LIBMATTI_UNION_UnionFileSystem_GetPath(impl->filesystem, parts, 1);
    if (!LIBMATTI_UNION_UnionFileSystem_Exists(impl->filesystem, relative))
    {
        LIBMATTI_UNION_UnionPath_Free(relative);
        return NULL;
    }

    unsigned char *bytes = NULL;
    size_t length = 0;
    if (!LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(impl->filesystem, relative, &bytes, &length))
    {
        LIBMATTI_UNION_UnionPath_Free(relative);
        return NULL;
    }
    LIBMATTI_UNION_UnionPath_Free(relative);
    if (bytes == NULL)
        return NULL;
    *outLength = length;
    return bytes;
}

LIBMATTI_JU_Manifest *LIBMATTI_JH_JarContents_GetManifest(const LIBMATTI_JH_JarContents *contents)
{
    const LIBMATTI_JH_JarContentsImpl *impl = (const LIBMATTI_JH_JarContentsImpl *)contents;
    return impl->manifest;
}

char **LIBMATTI_JH_JarContents_GetPackagesExcluding(const LIBMATTI_JH_JarContents *contents,
                                                    char **excludedRootPackages, size_t excludedCount, size_t *count)
{
    LIBMATTI_JH_JarContentsImpl *impl = (LIBMATTI_JH_JarContentsImpl *)contents;

    PackagesScanContext ctx = {excludedRootPackages, excludedCount, NULL, 0};

    // Java: Files.walkFileTree(filesystem.getRoot(), ...)
    LIBMATTI_UNION_UnionFileSystem_Walk(impl->filesystem, "", 1, packages_scan_callback, &ctx);

    *count = ctx.packageCount;
    return ctx.packages;
}

char **LIBMATTI_JH_JarContents_GetPackages(const LIBMATTI_JH_JarContents *contents, size_t *count)
{
    LIBMATTI_JH_JarContentsImpl *impl = (LIBMATTI_JH_JarContentsImpl *)contents;

    // Java: lazy cache
    if (impl->packages == NULL)
    {
        impl->packages = LIBMATTI_JH_JarContents_GetPackagesExcluding(contents, NULL, 0, &impl->packageCount);
    }

    *count = impl->packageCount;
    return impl->packages;
}

LIBMATTI_JH_Provider *LIBMATTI_JH_JarContents_GetMetaInfServices(const LIBMATTI_JH_JarContents *contents, size_t *count)
{
    LIBMATTI_JH_JarContentsImpl *impl = (LIBMATTI_JH_JarContentsImpl *)contents;

    // Java: lazy cache
    if (impl->providers == NULL)
    {
        ServicesScanContext ctx = {impl, NULL, 0};

        // Java: Files.walk(services, 1) over META-INF/services/
        LIBMATTI_UNION_UnionFileSystem_Walk(impl->filesystem, "META-INF/services", 0, services_scan_callback, &ctx);

        impl->providers = ctx.providers;
        impl->providerCount = ctx.providerCount;
    }

    *count = impl->providerCount;
    return impl->providers;
}