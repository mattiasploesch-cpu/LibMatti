#include "libmatti/net/neoforged/fml/jarcontents/EmptyJarContents.h"

#include "libmatti/net/neoforged/fml/jarcontents/EmptyManifest.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    LIBMATTI_FML_JarContents base;
    char *path;
} EmptyJarContents;

// Java: public Optional<String> getChecksum() { return Optional.empty(); }
static int getChecksum(void *self, char **out)
{
    (void) self;
    (void) out;
    return 0;
}

// Java: public Optional<URI> findFile(String relativePath) { return Optional.empty(); }
static int findFile(void *self, const char *relativePath, LIBMATTI_JN_URI **out)
{
    (void) self;
    (void) relativePath;
    (void) out;
    return 0;
}

// Java: public @Nullable JarResource get(String relativePath) { return null; }
static LIBMATTI_FML_JarResource *get(void *self, const char *relativePath)
{
    (void) self;
    (void) relativePath;
    return NULL;
}

// Java: public boolean containsFile(String relativePath) { return false; }
static int containsFile(void *self, const char *relativePath)
{
    (void) self;
    (void) relativePath;
    return 0;
}

// Java: public Path getPrimaryPath() { return path; }
static const char *getPrimaryPath(void *self)
{
    return ((EmptyJarContents *)self)->path;
}

// Java: public Collection<Path> getContentRoots() { return List.of(); }
static const char **getContentRoots(void *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: public Manifest getManifest() { return EmptyManifest.INSTANCE; }
static LIBMATTI_JU_Manifest *getManifest(void *self)
{
    (void) self;
    return LIBMATTI_FML_EmptyManifest_Instance();
}

// Java: public @Nullable InputStream openFile(String relativePath) { return null; }
static unsigned char *openFile(void *self, const char *relativePath, size_t *outLength)
{
    (void) self;
    (void) relativePath;
    *outLength = 0;
    return NULL;
}

// Java: public void visitContent(String startingFolder, JarResourceVisitor visitor) {}
static void visitContent(void *self, const char *startingFolder, LIBMATTI_FML_JarResourceVisitor visitor,
                         void *userdata)
{
    (void) self;
    (void) startingFolder;
    (void) visitor;
    (void) userdata;
}

// Java: public void close() {}
static void closeContents(void *self)
{
    EmptyJarContents *contents = self;
    free(contents->path);
    free(contents);
}

// Java: public String toString() { return "empty(" + PathPrettyPrinting.prettyPrint(path) + ")"; }
static char *toStringImpl(void *self)
{
    char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(((EmptyJarContents *)self)->path);
    size_t length = strlen(pretty) + 9;
    char *result = malloc(length);
    snprintf(result, length, "empty(%s)", pretty);
    free(pretty);
    return result;
}

// Java: public EmptyJarContents(Path path)
LIBMATTI_FML_JarContents *LIBMATTI_FML_EmptyJarContents_New(const char *path)
{
    EmptyJarContents *contents = calloc(1, sizeof(EmptyJarContents));
    contents->path = strdup(path);
    contents->base.self = contents;
    contents->base.kind = LIBMATTI_FML_JARCONTENTS_KIND_EMPTY;
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
