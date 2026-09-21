#include "libmatti/net/neoforged/fml/jarcontents/JarFileContents.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/bsl/sjh/niofs/union/UnionFileSystemProvider.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"
#include "libmatti/java/security/MessageDigest.h"
#include "libmatti/net/neoforged/fml/jarcontents/EmptyManifest.h"
#include "libmatti/net/neoforged/fml/jarcontents/PathNormalization.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: JarFile.MANIFEST_NAME
static const char *MANIFEST_NAME = "META-INF/MANIFEST.MF";

// Java: final class JarEntryResource implements JarResource
typedef struct JarFileResource
{
    LIBMATTI_FML_JarResource base;
    LIBMATTI_UNION_UnionFileSystem *fileSystem;
    char *relativePath;
    // Java: private final boolean mutable
    int mutable;
    // Set by retain(); tells visitContent that it must not free the resource
    int retained;
} JarFileResource;

typedef struct
{
    LIBMATTI_FML_JarContents base;
    char *path;
    LIBMATTI_UNION_UnionFileSystem *fileSystem;
    LIBMATTI_JU_Manifest *manifest;
} JarFileContents;

// Java: relativePath = PathNormalization.normalize(relativePath)
static char *normalize(const char *relativePath)
{
    return LIBMATTI_FML_PathNormalization_Normalize(relativePath);
}

// Java: jarFile.getEntry(name) - 1 if the entry exists and is not a directory
static int hasEntry(LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *relativePath)
{
    const char *parts[1] = {relativePath};
    LIBMATTI_UNION_UnionPath *entry = LIBMATTI_UNION_UnionFileSystem_GetPath(fileSystem, parts, 1);

    int exists = LIBMATTI_UNION_UnionFileSystem_Exists(fileSystem, entry);

    LIBMATTI_UNION_BasicFileAttributes attributes;
    int found = LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(fileSystem, entry, &attributes);

    LIBMATTI_UNION_UnionPath_Free(entry);
    return exists && found && attributes.isRegularFile;
}

// Java: jarFile.getInputStream(entry)
static unsigned char *readEntry(LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *relativePath,
                                size_t *outLength)
{
    *outLength = 0;
    if (relativePath[0] == '\0')
        return NULL; // Java: throw new IOException("The path refers to the root directory")

    const char *parts[1] = {relativePath};
    LIBMATTI_UNION_UnionPath *entry = LIBMATTI_UNION_UnionFileSystem_GetPath(fileSystem, parts, 1);

    unsigned char *bytes = NULL;
    size_t length = 0;
    int read = LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(fileSystem, entry, &bytes, &length);

    LIBMATTI_UNION_UnionPath_Free(entry);

    if (!read)
        return NULL; // Java: return null when the entry was not found

    *outLength = length;
    return bytes;
}

// Java: public InputStream open() throws IOException
static unsigned char *resourceOpen(void *self, size_t *outLength)
{
    JarFileResource *resource = self;
    return readEntry(resource->fileSystem, resource->relativePath, outLength);
}

// Java: public JarResourceAttributes attributes() throws IOException
static void resourceAttributes(void *self, LIBMATTI_FML_JarResourceAttributes *out)
{
    JarFileResource *resource = self;
    out->lastModifiedMillis = 0;
    out->size = 0;

    // Java: entry.getSize(); the ELF section header carries no timestamp
    const char *parts[1] = {resource->relativePath};
    LIBMATTI_UNION_UnionPath *entry = LIBMATTI_UNION_UnionFileSystem_GetPath(resource->fileSystem, parts, 1);

    LIBMATTI_UNION_BasicFileAttributes attributes;
    if (LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(resource->fileSystem, entry, &attributes))
        out->size = attributes.size;

    LIBMATTI_UNION_UnionPath_Free(entry);
}

static void freeJarFileResource(JarFileResource *resource);

// Java has no release; the port frees the handle a get() allocated.
static void resourceFree(void *self)
{
    freeJarFileResource(self);
}

// Java: public JarResource retain() { if (mutable) return new JarEntryResource(entry, false); else return this; }
static LIBMATTI_FML_JarResource *resourceRetain(void *self)
{
    JarFileResource *resource = self;
    if (!resource->mutable)
        return &resource->base;

    JarFileResource *copy = calloc(1, sizeof(JarFileResource));
    copy->fileSystem = resource->fileSystem;
    copy->relativePath = strdup(resource->relativePath);
    copy->mutable = 0;
    copy->retained = 1; // the owner releases it together with the contents
    copy->base.self = copy;
    copy->base.open = resourceOpen;
    copy->base.attributes = resourceAttributes;
    copy->base.retain = resourceRetain;
    copy->base.freeResource = resourceFree;

    resource->retained = 1;
    return &copy->base;
}

static void freeJarFileResource(JarFileResource *resource)
{
    if (resource == NULL)
        return;
    free(resource->relativePath);
    free(resource);
}

static int getChecksum(void *self, char **out)
{
    JarFileContents *contents = self;

    // Java: MessageDigest.getInstance("SHA-256") over DigestInputStream(in).transferTo(nullOutputStream())
    FILE *file = fopen(contents->path, "rb");
    if (file == NULL)
        return 0;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *bytes = malloc((size_t)length);
    if (fread(bytes, 1, (size_t)length, file) != (size_t)length)
    {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);

    LIBMATTI_JS_MessageDigest *digest = LIBMATTI_JS_MessageDigest_GetInstance("SHA-256");
    size_t digestLength = 0;
    unsigned char *hash = LIBMATTI_JS_MessageDigest_Digest(digest, bytes, (size_t)length, &digestLength);
    LIBMATTI_JS_MessageDigest_Free(digest);
    free(bytes);

    // Java: HexFormat.of().formatHex(checksum)
    char *hex = malloc(digestLength * 2 + 1);
    for (size_t i = 0; i < digestLength; i++)
        snprintf(hex + i * 2, 3, "%02x", hash[i]);
    free(hash);

    *out = hex;
    return 1;
}

// Java: public Path getPrimaryPath()
static const char *getPrimaryPath(void *self)
{
    return ((JarFileContents *)self)->path;
}

// Java: public Collection<Path> getContentRoots() { return List.of(path); }
static const char **getContentRoots(void *self, size_t *count)
{
    JarFileContents *contents = self;
    const char **roots = malloc(sizeof(char *));
    roots[0] = contents->path;
    *count = 1;
    return roots;
}

// Java: public @Nullable JarResource get(String relativePath)
static LIBMATTI_FML_JarResource *get(void *self, const char *relativePath)
{
    JarFileContents *contents = self;
    char *normalized = normalize(relativePath);

    LIBMATTI_FML_JarResource *result = NULL;
    if (hasEntry(contents->fileSystem, normalized))
    {
        JarFileResource *resource = calloc(1, sizeof(JarFileResource));
        resource->fileSystem = contents->fileSystem;
        resource->relativePath = normalized;
        resource->mutable = 0;
        resource->base.self = resource;
        resource->base.open = resourceOpen;
        resource->base.attributes = resourceAttributes;
        resource->base.retain = resourceRetain;
        resource->base.freeResource = resourceFree;
        result = &resource->base;
    }
    else
    {
        free(normalized);
    }

    return result;
}

// Java: public boolean containsFile(String relativePath)
static int containsFile(void *self, const char *relativePath)
{
    char *normalized = normalize(relativePath);
    int result = hasEntry(((JarFileContents *)self)->fileSystem, normalized);
    free(normalized);
    return result;
}

// Java: public Optional<URI> findFile(String relativePath)
static int findFile(void *self, const char *relativePath, LIBMATTI_JN_URI **out)
{
    JarFileContents *contents = self;
    char *normalized = normalize(relativePath);

    int found = 0;
    if (hasEntry(contents->fileSystem, normalized))
    {
        // Java: URI.create("jar:" + path.toUri() + "!/" + relativePath)
        const char *parts[1] = {normalized};
        LIBMATTI_UNION_UnionPath *entry = LIBMATTI_UNION_UnionFileSystem_GetPath(contents->fileSystem, parts, 1);
        *out = LIBMATTI_UNION_UnionPath_ToUri(entry);
        LIBMATTI_UNION_UnionPath_Free(entry);
        found = 1;
    }

    free(normalized);
    return found;
}

// Java: public @Nullable InputStream openFile(String relativePath) throws IOException
static unsigned char *openFile(void *self, const char *relativePath, size_t *outLength)
{
    JarFileContents *contents = self;
    char *normalized = normalize(relativePath);

    unsigned char *bytes = NULL;
    if (hasEntry(contents->fileSystem, normalized))
        bytes = readEntry(contents->fileSystem, normalized, outLength);

    free(normalized);
    return bytes;
}

// Java: public Manifest getManifest()
static LIBMATTI_JU_Manifest *getManifest(void *self)
{
    return ((JarFileContents *)self)->manifest;
}

typedef struct
{
    LIBMATTI_FML_JarResourceVisitor visitor;
    void *userdata;
    JarFileContents *contents;
} VisitContext;

// Java: visitContent - skips directories and entries outside startingFolder
static int visitEntry(const char *relativePath, int isDirectory, void *userdata)
{
    if (isDirectory)
        return 1;

    VisitContext *context = userdata;

    JarFileResource *resource = calloc(1, sizeof(JarFileResource));
    resource->fileSystem = context->contents->fileSystem;
    resource->relativePath = strdup(relativePath);
    resource->mutable = 1;
    resource->base.self = resource;
    resource->base.open = resourceOpen;
    resource->base.attributes = resourceAttributes;
    resource->base.retain = resourceRetain;
    resource->base.freeResource = resourceFree;

    context->visitor(relativePath, &resource->base, context->userdata);

    // Java reuses one resource object; the port allocates per visit and releases
    // it again unless the visitor retained a copy.
    if (!resource->retained)
        freeJarFileResource(resource);

    return 1;
}

// Java: public void visitContent(String startingFolder, JarResourceVisitor visitor)
static void visitContent(void *self, const char *startingFolder, LIBMATTI_FML_JarResourceVisitor visitor,
                         void *userdata)
{
    JarFileContents *contents = self;
    char *folderPrefix = LIBMATTI_FML_PathNormalization_NormalizeFolderPrefix(startingFolder);

    VisitContext context = {visitor, userdata, contents};
    (void) folderPrefix; // the walk is rooted at the requested folder already

    LIBMATTI_UNION_UnionFileSystem_Walk(contents->fileSystem, folderPrefix, 1, visitEntry, &context);

    free(folderPrefix);
}

static void closeContents(void *self)
{
    JarFileContents *contents = self;
    LIBMATTI_UNION_UnionFileSystem_Close(contents->fileSystem);
    // Java: the empty manifest is the shared EmptyManifest.INSTANCE
    if (contents->manifest != LIBMATTI_FML_EmptyManifest_Instance())
        LIBMATTI_JU_Manifest_Free(contents->manifest);
    free(contents->path);
    free(contents);
}

// Java: public String toString() { return "jar(" + PathPrettyPrinting.prettyPrint(path) + ")"; }
static char *toStringImpl(void *self)
{
    char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(((JarFileContents *)self)->path);
    size_t length = strlen(pretty) + 7;
    char *result = malloc(length);
    snprintf(result, length, "jar(%s)", pretty);
    free(pretty);
    return result;
}

// Java: public JarFileContents(Path path) throws IOException
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarFileContents_New(const char *path)
{
    JarFileContents *contents = calloc(1, sizeof(JarFileContents));
    contents->path = strdup(path);

    // Java: new JarFile(path.toFile(), true, JarFile.OPEN_READ, JarFile.runtimeVersion())
    const char *paths[1] = {path};
    contents->fileSystem =
        LIBMATTI_UNION_UnionFileSystem_New(LIBMATTI_UNION_UnionFileSystemProvider_Get(), NULL, path, paths, 1);
    if (contents->fileSystem == NULL)
    {
        free(contents->path);
        free(contents);
        return NULL;
    }

    contents->base.self = contents;
    contents->base.kind = LIBMATTI_FML_JARCONTENTS_KIND_JARFILE;
    contents->base.getChecksum = getChecksum;
    contents->base.getPrimaryPath = getPrimaryPath;
    contents->base.getContentRoots = getContentRoots;
    contents->base.get = get;
    contents->base.findFile = findFile;
    contents->base.openFile = openFile;
    contents->base.containsFile = containsFile;
    contents->base.getManifest = getManifest;
    contents->base.visitContent = visitContent;
    contents->base.close = closeContents;
    contents->base.toStringImpl = toStringImpl;

    // Java: Objects.requireNonNullElse(jarFile.getManifest(), EmptyManifest.INSTANCE)
    size_t manifestLength = 0;
    unsigned char *manifestBytes = readEntry(contents->fileSystem, MANIFEST_NAME, &manifestLength);
    if (manifestBytes != NULL)
    {
        contents->manifest = LIBMATTI_JU_Manifest_Parse((const char *)manifestBytes, manifestLength);
        free(manifestBytes);
    }
    if (contents->manifest == NULL)
        contents->manifest = LIBMATTI_FML_EmptyManifest_Instance();

    return &contents->base;
}
