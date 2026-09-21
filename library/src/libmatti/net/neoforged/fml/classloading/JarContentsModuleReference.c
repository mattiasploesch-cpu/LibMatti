#include "libmatti/net/neoforged/fml/classloading/JarContentsModuleReference.h"

#include "libmatti/java/nio/file/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: final class JarContentsModuleReader implements ModuleReader
struct LIBMATTI_FML_JarContentsModuleReader
{
    LIBMATTI_FML_JarContents *contents;
};

// Java: private static URI getModuleLocation(JarContents contents)
LIBMATTI_JN_URI *LIBMATTI_FML_JarContentsModuleReference_GetModuleLocation(LIBMATTI_FML_JarContents *contents)
{
    // Java: if (contents.getContentRoots().isEmpty()) return null;
    size_t rootCount = 0;
    const char **roots = LIBMATTI_FML_JarContents_GetContentRoots(contents, &rootCount);
    free(roots);

    if (rootCount == 0)
        return NULL;

    // Java: return contents.getPrimaryPath().toUri();
    const char *primaryPath = LIBMATTI_FML_JarContents_GetPrimaryPath(contents);
    char *absolute = LIBMATTI_JNF_Path_ToAbsolutePath(primaryPath);
    size_t length = strlen(absolute) + 8;
    char *uriText = malloc(length);
    snprintf(uriText, length, "file://%s", absolute);
    free(absolute);

    LIBMATTI_JN_URI *uri = LIBMATTI_JN_URI_Create(uriText);
    free(uriText);
    return uri;
}

// Java: JarContentsModuleReference(ModuleDescriptor descriptor, JarContents contents)
LIBMATTI_FML_JarContentsModuleReference *LIBMATTI_FML_JarContentsModuleReference_New(
    LIBMATTI_JL_ModuleDescriptor *descriptor, LIBMATTI_FML_JarContents *contents)
{
    LIBMATTI_FML_JarContentsModuleReference *reference = calloc(1, sizeof(LIBMATTI_FML_JarContentsModuleReference));
    // Java: super(descriptor, getModuleLocation(contents))
    reference->base.descriptor = descriptor;
    reference->base.location = LIBMATTI_FML_JarContentsModuleReference_GetModuleLocation(contents);
    reference->contents = contents;
    return reference;
}

// Java: public ModuleReader open() { return new JarContentsModuleReader(contents); }
LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_JarContentsModuleReference_Open(
    const LIBMATTI_FML_JarContentsModuleReference *reference)
{
    return LIBMATTI_FML_JarContentsModuleReader_New(reference->contents);
}

// Java: JarContentsModuleReader(JarContents contents)
LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_JarContentsModuleReader_New(LIBMATTI_FML_JarContents *contents)
{
    LIBMATTI_FML_JarContentsModuleReader *reader = calloc(1, sizeof(LIBMATTI_FML_JarContentsModuleReader));
    reader->contents = contents;
    return reader;
}

// Java: public Optional<InputStream> open(String name) { return Optional.ofNullable(contents.openFile(name)); }
unsigned char *LIBMATTI_FML_JarContentsModuleReader_OpenFile(const LIBMATTI_FML_JarContentsModuleReader *reader,
                                                            const char *name, size_t *outLength)
{
    return LIBMATTI_FML_JarContents_OpenFile(reader->contents, name, outLength);
}

// Java: public Optional<URI> find(String name) { return contents.findFile(name); }
LIBMATTI_JN_URI *LIBMATTI_FML_JarContentsModuleReader_Find(const LIBMATTI_FML_JarContentsModuleReader *reader,
                                                          const char *name)
{
    LIBMATTI_JN_URI *uri = NULL;
    if (!LIBMATTI_FML_JarContents_FindFile(reader->contents, name, &uri))
        return NULL;
    return uri;
}

typedef struct
{
    char **paths;
    size_t count;
} ContentSet;

// Java: contents.visitContent((relativePath, resource) -> content.add(relativePath))
static void collectPath(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    (void) resource;
    ContentSet *content = userdata;

    // Java: LinkedHashSet - keep the insertion order and skip duplicates
    for (size_t i = 0; i < content->count; i++)
        if (strcmp(content->paths[i], relativePath) == 0)
            return;

    content->paths = realloc(content->paths, sizeof(char *) * (content->count + 1));
    content->paths[content->count++] = strdup(relativePath);
}

// Java: public Stream<String> list()
char **LIBMATTI_FML_JarContentsModuleReader_List(const LIBMATTI_FML_JarContentsModuleReader *reader, size_t *count)
{
    ContentSet content = {0};
    LIBMATTI_FML_JarContents_VisitContent(reader->contents, collectPath, &content);

    *count = content.count;
    return content.paths;
}

// Java: public void close() {}
void LIBMATTI_FML_JarContentsModuleReader_Close(LIBMATTI_FML_JarContentsModuleReader *reader)
{
    free(reader);
}

// Java: public String toString() { return contents.toString(); }
char *LIBMATTI_FML_JarContentsModuleReader_ToString(const LIBMATTI_FML_JarContentsModuleReader *reader)
{
    return LIBMATTI_FML_JarContents_ToString(reader->contents);
}
