#include "libmatti/net/neoforged/fml/jarcontents/FolderJarContents.h"

#include "libmatti/java/net/URI.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"
#include "libmatti/net/neoforged/fml/jarcontents/EmptyManifest.h"
#include "libmatti/net/neoforged/fml/jarcontents/PathNormalization.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Java: JarFile.MANIFEST_NAME
static const char *MANIFEST_NAME = "META-INF/MANIFEST.MF";

// Java: private static class FileResource implements JarResource
typedef struct FileResource
{
    LIBMATTI_FML_JarResource base;
    char *path;
    // Java: private final boolean mutable
    int mutable;
    int retained;
} FileResource;

typedef struct
{
    LIBMATTI_FML_JarContents base;
    char *path;
    LIBMATTI_JU_Manifest *cachedManifest; // Java: private Manifest cachedManifest
} FolderJarContents;

// Java: pathToFile.toUri()
static LIBMATTI_JN_URI *fileUri(const char *path)
{
    char *absolute = LIBMATTI_JNF_Path_ToAbsolutePath(path);
    size_t length = strlen(absolute) + 8;
    char *uri = malloc(length);
    snprintf(uri, length, "file://%s", absolute);
    free(absolute);

    LIBMATTI_JN_URI *result = LIBMATTI_JN_URI_Create(uri);
    free(uri);
    return result;
}

// Java: private Path fromRelativePath(String relativePath)
static char *fromRelativePath(FolderJarContents *contents, const char *relativePath)
{
    // Java: path.resolve(relativePath) - checking for normalization here prevents path escapes
    char *normalized = LIBMATTI_FML_PathNormalization_Normalize(relativePath);
    char *result = LIBMATTI_JNF_Path_Resolve(contents->path, normalized);
    free(normalized);
    return result;
}

static void freeFileResource(FileResource *resource);

// Java: public InputStream open() throws IOException
static unsigned char *fileOpen(void *self, size_t *outLength)
{
    FileResource *resource = self;
    return LIBMATTI_JNF_Files_ReadAllBytes(resource->path, outLength);
}

// Java: public JarResourceAttributes attributes() throws IOException
static void fileAttributes(void *self, LIBMATTI_FML_JarResourceAttributes *out)
{
    FileResource *resource = self;
    out->lastModifiedMillis = 0;
    out->size = 0;

    struct stat status;
    if (stat(resource->path, &status) != 0)
        return;

    // Java: attributes.lastModifiedTime()
    out->lastModifiedMillis = (long)status.st_mtime * 1000;
    out->size = (long)status.st_size;
}

// Java has no release; the port frees the handle a get() allocated.
static void fileResourceFree(void *self)
{
    freeFileResource(self);
}

// Java: public JarResource retain()
static LIBMATTI_FML_JarResource *fileRetain(void *self)
{
    FileResource *resource = self;
    if (!resource->mutable)
        return &resource->base;

    FileResource *copy = calloc(1, sizeof(FileResource));
    copy->path = strdup(resource->path);
    copy->mutable = 0;
    copy->retained = 1;
    copy->base.self = copy;
    copy->base.open = fileOpen;
    copy->base.attributes = fileAttributes;
    copy->base.retain = fileRetain;
    copy->base.freeResource = fileResourceFree;

    resource->retained = 1;
    return &copy->base;
}

static void freeFileResource(FileResource *resource)
{
    if (resource == NULL)
        return;
    free(resource->path);
    free(resource);
}

// Java: public Optional<String> getChecksum() { return Optional.empty(); }
static int getChecksum(void *self, char **out)
{
    (void) self;
    (void) out;
    return 0;
}

// Java: public Path getPrimaryPath()
static const char *getPrimaryPath(void *self)
{
    return ((FolderJarContents *)self)->path;
}

// Java: public Collection<Path> getContentRoots() { return List.of(path); }
static const char **getContentRoots(void *self, size_t *count)
{
    FolderJarContents *contents = self;
    const char **roots = malloc(sizeof(char *));
    roots[0] = contents->path;
    *count = 1;
    return roots;
}

// Java: public @Nullable JarResource get(String relativePath)
static LIBMATTI_FML_JarResource *get(void *self, const char *relativePath)
{
    FolderJarContents *contents = self;
    char *path = fromRelativePath(contents, relativePath);

    LIBMATTI_FML_JarResource *result = NULL;
    if (LIBMATTI_JNF_Files_IsRegularFile(path))
    {
        FileResource *resource = calloc(1, sizeof(FileResource));
        resource->path = path;
        resource->mutable = 0;
        resource->base.self = resource;
        resource->base.open = fileOpen;
        resource->base.attributes = fileAttributes;
        resource->base.retain = fileRetain;
        resource->base.freeResource = fileResourceFree;
        result = &resource->base;
    }
    else
    {
        free(path);
    }

    return result;
}

// Java: public boolean containsFile(String relativePath)
static int containsFile(void *self, const char *relativePath)
{
    char *path = fromRelativePath(self, relativePath);
    int result = LIBMATTI_JNF_Files_IsRegularFile(path);
    free(path);
    return result;
}

// Java: public InputStream openFile(String relativePath) throws IOException
static unsigned char *openFile(void *self, const char *relativePath, size_t *outLength)
{
    char *path = fromRelativePath(self, relativePath);
    unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(path, outLength);
    free(path);
    return bytes;
}

// Java: public Optional<URI> findFile(String relativePath)
static int findFile(void *self, const char *relativePath, LIBMATTI_JN_URI **out)
{
    char *path = fromRelativePath(self, relativePath);
    int found = 0;
    if (LIBMATTI_JNF_Files_IsRegularFile(path))
    {
        *out = fileUri(path);
        found = 1;
    }
    free(path);
    return found;
}

// Java: public Manifest getManifest()
static LIBMATTI_JU_Manifest *getManifest(void *self)
{
    FolderJarContents *contents = self;

    if (contents->cachedManifest == NULL)
    {
        char *manifestFile = LIBMATTI_JNF_Path_Resolve(contents->path, MANIFEST_NAME);

        size_t length = 0;
        unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(manifestFile, &length);
        if (bytes != NULL)
        {
            contents->cachedManifest = LIBMATTI_JU_Manifest_Parse((const char *)bytes, length);
            free(bytes);
        }

        // Java: catch (NoSuchFileException ignored) { manifest = EmptyManifest.INSTANCE; }
        if (contents->cachedManifest == NULL)
            contents->cachedManifest = LIBMATTI_FML_EmptyManifest_Instance();

        free(manifestFile);
    }

    return contents->cachedManifest;
}

typedef struct
{
    LIBMATTI_FML_JarResourceVisitor visitor;
    void *userdata;
    const char *root;
} VisitContext;

// Java: path -> if (Files.isRegularFile(path)) { relativePath = normalize(root.relativize(path)); visitor.visit(...) }
static LIBMATTI_JNF_FileVisitResult visitFile(void *self, const char *file, int isRegularFile, long size)
{
    VisitContext *context = self;

    if (isRegularFile)
    {
        size_t rootLength = strlen(context->root);
        const char *relative = file + rootLength;
        if (*relative == '/')
            relative++;

        char *normalized = LIBMATTI_FML_PathNormalization_Normalize(relative);

        FileResource *resource = calloc(1, sizeof(FileResource));
        resource->path = strdup(file);
        resource->mutable = 1;
        resource->base.self = resource;
        resource->base.open = fileOpen;
        resource->base.attributes = fileAttributes;
        resource->base.retain = fileRetain;
        resource->base.freeResource = fileResourceFree;

        context->visitor(normalized, &resource->base, context->userdata);

        if (!resource->retained)
            freeFileResource(resource);

        free(normalized);
    }

    (void) size;
    return LIBMATTI_JNF_FileVisitResult_CONTINUE;
}

// Java: public void visitContent(String startingFolder, JarResourceVisitor visitor)
static void visitContent(void *self, const char *startingFolder, LIBMATTI_FML_JarResourceVisitor visitor,
                         void *userdata)
{
    FolderJarContents *contents = self;

    char *normalizedFolder = LIBMATTI_FML_PathNormalization_Normalize(startingFolder);
    char *startingPoint = normalizedFolder[0] == '\0'
                              ? strdup(contents->path)
                              : LIBMATTI_JNF_Path_Normalize(LIBMATTI_JNF_Path_Resolve(contents->path, normalizedFolder));
    free(normalizedFolder);

    // Java: if (!Files.isDirectory(startingPoint)) return;
    if (!LIBMATTI_JNF_Files_IsDirectory(startingPoint))
    {
        free(startingPoint);
        return;
    }

    VisitContext context = {visitor, userdata, contents->path};
    LIBMATTI_JNF_SimpleFileVisitor fileVisitor = {.self = &context, .visitFile = visitFile};

    // Java: try (var stream = Files.walk(startingPoint))
    LIBMATTI_JNF_Files_WalkFileTree(startingPoint, 0, INT_MAX, &fileVisitor);

    free(startingPoint);
}

// Java: public void close() {}
static void closeContents(void *self)
{
    FolderJarContents *contents = self;
    // Java: the empty manifest is the shared EmptyManifest.INSTANCE
    if (contents->cachedManifest != NULL && contents->cachedManifest != LIBMATTI_FML_EmptyManifest_Instance())
        LIBMATTI_JU_Manifest_Free(contents->cachedManifest);
    free(contents->path);
    free(contents);
}

// Java: public String toString() { return "folder(" + PathPrettyPrinting.prettyPrint(path) + ")"; }
static char *toStringImpl(void *self)
{
    char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(((FolderJarContents *)self)->path);
    size_t length = strlen(pretty) + 10;
    char *result = malloc(length);
    snprintf(result, length, "folder(%s)", pretty);
    free(pretty);
    return result;
}

// Java: public FolderJarContents(Path path)
LIBMATTI_FML_JarContents *LIBMATTI_FML_FolderJarContents_New(const char *path)
{
    FolderJarContents *contents = calloc(1, sizeof(FolderJarContents));
    contents->path = strdup(path);
    contents->base.self = contents;
    contents->base.kind = LIBMATTI_FML_JARCONTENTS_KIND_FOLDER;
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
    return &contents->base;
}
