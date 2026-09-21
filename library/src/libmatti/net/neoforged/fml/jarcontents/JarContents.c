#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/jarcontents/CompositeJarContents.h"
#include "libmatti/net/neoforged/fml/jarcontents/EmptyJarContents.h"
#include "libmatti/net/neoforged/fml/jarcontents/FolderJarContents.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarFileContents.h"

#include <stdlib.h>
#include <string.h>

// Java: static JarContents ofFilteredPaths(Collection<FilteredPath> paths) throws IOException
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfFilteredPaths(const LIBMATTI_FML_FilteredPath *paths,
                                                                   size_t pathCount)
{
    // Java: if (paths.isEmpty()) throw new IllegalArgumentException("Cannot construct jar contents without any paths.")
    if (pathCount == 0)
        return NULL;

    LIBMATTI_FML_JarContents **contents = calloc(pathCount, sizeof(LIBMATTI_FML_JarContents *));
    LIBMATTI_FML_JarContents_PathFilter *filters = calloc(pathCount, sizeof(LIBMATTI_FML_JarContents_PathFilter));
    void **filterUserdata = calloc(pathCount, sizeof(void *));
    size_t count = 0;

    for (size_t i = 0; i < pathCount; i++)
    {
        if (LIBMATTI_JNF_Files_Exists(paths[i].path))
        {
            contents[count] = LIBMATTI_FML_JarContents_OfPath(paths[i].path);
            filters[count] = paths[i].filter;
            filterUserdata[count] = paths[i].filterUserdata;
            if (contents[count] != NULL)
                count++;
        }
    }

    LIBMATTI_FML_JarContents *result = NULL;
    if (count == 0)
    {
        // Java: throw new NoSuchFileException("At least one of the paths must exist when constructing jar contents: " + pathList)
    }
    else if (count == 1 && filters[0] == NULL)
    {
        // Java: uncommon case, but we'll still optimize for it
        result = contents[0];
    }
    else
    {
        result = LIBMATTI_FML_CompositeJarContents_New(contents, filters, filterUserdata, count);
    }

    free(contents);
    free(filters);
    free(filterUserdata);
    return result;
}

// Java: static JarContents ofPaths(Collection<Path> paths) throws IOException
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfPaths(const char **paths, size_t pathCount)
{
    // Java: if (paths.isEmpty()) throw new IllegalArgumentException("Cannot construct jar contents without any paths.")
    if (pathCount == 0)
        return NULL;

    LIBMATTI_FML_JarContents **contents = calloc(pathCount, sizeof(LIBMATTI_FML_JarContents *));
    size_t count = 0;

    for (size_t i = 0; i < pathCount; i++)
    {
        if (LIBMATTI_JNF_Files_Exists(paths[i]))
        {
            LIBMATTI_FML_JarContents *content = LIBMATTI_FML_JarContents_OfPath(paths[i]);
            if (content != NULL)
                contents[count++] = content;
        }
    }

    LIBMATTI_FML_JarContents *result = NULL;
    if (count == 0)
    {
        // Java: throw new NoSuchFileException("At least one of the paths must exist when constructing jar contents: " + paths)
    }
    else if (count == 1)
    {
        result = contents[0];
    }
    else
    {
        result = LIBMATTI_FML_CompositeJarContents_New(contents, NULL, NULL, count);
    }

    free(contents);
    return result;
}

// Java: static JarContents ofPath(Path path) throws IOException
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_OfPath(const char *path)
{
    if (LIBMATTI_JNF_Files_IsRegularFile(path))
        return LIBMATTI_FML_JarFileContents_New(path);
    if (LIBMATTI_JNF_Files_IsDirectory(path))
        return LIBMATTI_FML_FolderJarContents_New(path);

    // Java: throw new NoSuchFileException("Cannot construct mod container from missing " + path)
    return NULL;
}

// Java: static JarContents empty(Path path)
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarContents_Empty(const char *path)
{
    return LIBMATTI_FML_EmptyJarContents_New(path);
}

// Java: Optional<String> getChecksum()
int LIBMATTI_FML_JarContents_GetChecksum(const LIBMATTI_FML_JarContents *contents, char **out)
{
    return contents->getChecksum(contents->self, out);
}

// Java: Path getPrimaryPath()
const char *LIBMATTI_FML_JarContents_GetPrimaryPath(const LIBMATTI_FML_JarContents *contents)
{
    return contents->getPrimaryPath(contents->self);
}

// Java: Collection<Path> getContentRoots()
const char **LIBMATTI_FML_JarContents_GetContentRoots(const LIBMATTI_FML_JarContents *contents, size_t *count)
{
    return contents->getContentRoots(contents->self, count);
}

// Java: JarResource get(String relativePath)
LIBMATTI_FML_JarResource *LIBMATTI_FML_JarContents_Get(const LIBMATTI_FML_JarContents *contents,
                                                       const char *relativePath)
{
    return contents->get(contents->self, relativePath);
}

// Java: Optional<URI> findFile(String relativePath)
int LIBMATTI_FML_JarContents_FindFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                      LIBMATTI_JN_URI **out)
{
    return contents->findFile(contents->self, relativePath, out);
}

// Java: InputStream openFile(String relativePath) throws IOException
unsigned char *LIBMATTI_FML_JarContents_OpenFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                                 size_t *outLength)
{
    return contents->openFile(contents->self, relativePath, outLength);
}

// Java: default byte[] readFile(String relativePath) throws IOException
unsigned char *LIBMATTI_FML_JarContents_ReadFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath,
                                                 size_t *outLength)
{
    return contents->openFile(contents->self, relativePath, outLength);
}

// Java: boolean containsFile(String relativePath)
int LIBMATTI_FML_JarContents_ContainsFile(const LIBMATTI_FML_JarContents *contents, const char *relativePath)
{
    return contents->containsFile(contents->self, relativePath);
}

// Java: Manifest getManifest()
LIBMATTI_JU_Manifest *LIBMATTI_FML_JarContents_GetManifest(const LIBMATTI_FML_JarContents *contents)
{
    return contents->getManifest(contents->self);
}

// Java: default void visitContent(JarResourceVisitor visitor)
void LIBMATTI_FML_JarContents_VisitContent(const LIBMATTI_FML_JarContents *contents,
                                           LIBMATTI_FML_JarResourceVisitor visitor, void *userdata)
{
    contents->visitContent(contents->self, "", visitor, userdata);
}

// Java: void visitContent(String startingFolder, JarResourceVisitor visitor)
void LIBMATTI_FML_JarContents_VisitContentFrom(const LIBMATTI_FML_JarContents *contents, const char *startingFolder,
                                               LIBMATTI_FML_JarResourceVisitor visitor, void *userdata)
{
    contents->visitContent(contents->self, startingFolder, visitor, userdata);
}

// Java: void close()
void LIBMATTI_FML_JarContents_Close(LIBMATTI_FML_JarContents *contents)
{
    if (contents == NULL)
        return;
    contents->close(contents->self);
}

// Java: String toString()
char *LIBMATTI_FML_JarContents_ToString(const LIBMATTI_FML_JarContents *contents)
{
    return contents->toStringImpl(contents->self);
}
