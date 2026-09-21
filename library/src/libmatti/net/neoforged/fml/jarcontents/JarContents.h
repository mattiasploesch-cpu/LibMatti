// Port of net.neoforged.fml.jarcontents.JarContents.
// The artifact reader behind the interface is the .so/ELF reader in
// bsl/sjh/niofs/union (entry mapping is documented in UnionFileSystem.h).

#ifndef MATTICRAFT_FML_JARCONTENTS_JARCONTENTS_H
#define MATTICRAFT_FML_JARCONTENTS_JARCONTENTS_H

#include "libmatti/java/net/URI.h"
#include "libmatti/java/util/jar/Manifest.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarResourceVisitor.h"

#include <stddef.h>

// Java: @FunctionalInterface interface JarContents.PathFilter { boolean test(String relativePath); }
typedef int (*LIBMATTI_FML_JarContents_PathFilter)(const char *relativePath, void *userdata);

// Java: record FilteredPath(Path path, @Nullable PathFilter filter)
typedef struct
{
    const char *path;
    // Java: @Nullable PathFilter filter
    LIBMATTI_FML_JarContents_PathFilter filter;
    void *filterUserdata;
} LIBMATTI_FML_FilteredPath;

// Java: the concrete implementations are matched with pattern matching (switch (contents) { case ... });
// the C port carries the concrete type on the interface.
typedef enum
{
    LIBMATTI_FML_JARCONTENTS_KIND_COMPOSITE,
    LIBMATTI_FML_JARCONTENTS_KIND_EMPTY,
    LIBMATTI_FML_JARCONTENTS_KIND_FOLDER,
    LIBMATTI_FML_JARCONTENTS_KIND_JARFILE
} LIBMATTI_FML_JarContents_Kind;

// Java: public interface JarContents extends Closeable
typedef struct LIBMATTI_FML_JarContents LIBMATTI_FML_JarContents;

struct LIBMATTI_FML_JarContents
{
    void *self;

    // Java: the implementing class (CompositeJarContents / EmptyJarContents / FolderJarContents / JarFileContents)
    LIBMATTI_FML_JarContents_Kind kind;

    // Java: Optional<String> getChecksum() - 1 if present, *out is a new string
    int (*getChecksum)(void *self, char **out);
    // Java: Path getPrimaryPath()
    const char *(*getPrimaryPath)(void *self);
    // Java: Collection<Path> getContentRoots()
    const char **(*getContentRoots)(void *self, size_t *count);
    // Java: JarResource get(String relativePath)
    LIBMATTI_FML_JarResource *(*get)(void *self, const char *relativePath);
    // Java: Optional<URI> findFile(String relativePath) - 1 if present; the caller frees *out
    int (*findFile)(void *self, const char *relativePath, LIBMATTI_JN_URI **out);
    // Java: InputStream openFile(String relativePath) - NULL if not found
    unsigned char *(*openFile)(void *self, const char *relativePath, size_t *outLength);
    // Java: boolean containsFile(String relativePath)
    int (*containsFile)(void *self, const char *relativePath);
    // Java: Manifest getManifest()
    LIBMATTI_JU_Manifest *(*getManifest)(void *self);
    // Java: void visitContent(String startingFolder, JarResourceVisitor visitor)
    void (*visitContent)(void *self, const char *startingFolder, LIBMATTI_FML_JarResourceVisitor visitor,
                         void *userdata);
    // Java: void close()
    void (*close)(void *self);
    // Java: String toString()
    char *(*toStringImpl)(void *self);
};

// Java: static JarContents ofPath(Path path) - NULL on failure (Java: throws IOException)
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfPath(const char *path);
// Java: static JarContents ofPaths(Collection<Path> paths) - NULL on failure
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfPaths(const char **paths, size_t pathCount);
// Java: static JarContents ofFilteredPaths(Collection<FilteredPath> paths) - NULL on failure
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfFilteredPaths(const LIBMATTI_FML_FilteredPath *paths,
                                                                   size_t pathCount);
// Java: static JarContents empty(Path path)
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_Empty(const char *path);

// Java: Optional<String> getChecksum()
int LIBMATTI_FML_JarContents_GetChecksum(const LIBMATTI_FML_JarContents *contents, char **out);
// Java: Path getPrimaryPath()
const char *LIBMATTI_FML_JarContents_GetPrimaryPath(const LIBMATTI_FML_JarContents *contents);
// Java: Collection<Path> getContentRoots()
const char **LIBMATTI_FML_JarContents_GetContentRoots(const LIBMATTI_FML_JarContents *contents, size_t *count);
// Java: JarResource get(String relativePath)
LIBMATTI_FML_JarResource *LIBMATTI_FML_JarContents_Get(const LIBMATTI_FML_JarContents *contents,
                                                       const char *relativePath);
// Java: Optional<URI> findFile(String relativePath) - 1 if present; the caller frees *out
int LIBMATTI_FML_JarContents_FindFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                      LIBMATTI_JN_URI **out);
// Java: InputStream openFile(String relativePath) throws IOException
unsigned char *LIBMATTI_FML_JarContents_OpenFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                                 size_t *outLength);
// Java: default byte[] readFile(String relativePath) throws IOException
unsigned char *LIBMATTI_FML_JarContents_ReadFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                                 size_t *outLength);
// Java: boolean containsFile(String relativePath)
int LIBMATTI_FML_JarContents_ContainsFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath);
// Java: Manifest getManifest()
LIBMATTI_JU_Manifest *LIBMATTI_FML_JarContents_GetManifest(const LIBMATTI_FML_JarContents *contents);
// Java: default void visitContent(JarResourceVisitor visitor)
void LIBMATTI_FML_JarContents_VisitContent(const LIBMATTI_FML_JarContents *contents,
                                           LIBMATTI_FML_JarResourceVisitor visitor, void *userdata);
// Java: void visitContent(String startingFolder, JarResourceVisitor visitor)
void LIBMATTI_FML_JarContents_VisitContentFrom(const LIBMATTI_FML_JarContents *contents, const char *startingFolder,
                                               LIBMATTI_FML_JarResourceVisitor visitor, void *userdata);
// Java: void close()
void LIBMATTI_FML_JarContents_Close(LIBMATTI_FML_JarContents *contents);
// Java: String toString()
char *LIBMATTI_FML_JarContents_ToString(const LIBMATTI_FML_JarContents *contents);

#endif //MATTICRAFT_FML_JARCONTENTS_JARCONTENTS_H
