#include "libmatti/net/neoforged/fml/loading/LibraryFinder.h"

#include "libmatti/java/lang/System.h"
#include "libmatti/java/nio/file/Files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: static Path findLibsPath()
char *LIBMATTI_FML_LibraryFinder_FindLibsPath(void)
{
    // Java: var libraryDirectoryProp = System.getProperty("libraryDirectory");
    const char *libraryDirectoryProp = LIBMATTI_JL_System_GetProperty("libraryDirectory");
    if (libraryDirectoryProp == NULL)
    {
        // Java: throw new IllegalStateException("Missing libraryDirectory system property")
        return NULL;
    }

    // Java: if (!Files.isDirectory(libsPath)) throw new IllegalStateException("libraryDirectory system property refers to a non-directory: " + libsPath);
    if (!LIBMATTI_JNF_Files_IsDirectory(libraryDirectoryProp))
    {
        return NULL;
    }

    return strdup(libraryDirectoryProp);
}

// Java: public static Path findPathForMaven(MavenCoordinate artifact)
char *LIBMATTI_FML_LibraryFinder_FindPathForMavenCoordinate(const LIBMATTI_FML_MavenCoordinate *artifact)
{
    char *libsPath = LIBMATTI_FML_LibraryFinder_FindLibsPath();
    if (libsPath == NULL) return NULL;

    char *relative = LIBMATTI_FML_MavenCoordinate_ToRelativeRepositoryPath(artifact);
    size_t length = strlen(libsPath) + strlen(relative) + 2;
    char *result = malloc(length);
    snprintf(result, length, "%s/%s", libsPath, relative);
    free(libsPath);
    free(relative);
    return result;
}

// Java: public static Path findPathForMaven(String group, String artifact, String extension, String classifier, String version)
char *LIBMATTI_FML_LibraryFinder_FindPathForMaven(const char *group, const char *artifact, const char *extension,
                                                  const char *classifier, const char *version)
{
    LIBMATTI_FML_MavenCoordinate *coordinate = LIBMATTI_FML_MavenCoordinate_New(group, artifact, extension,
                                                                               classifier, version);
    char *result = LIBMATTI_FML_LibraryFinder_FindPathForMavenCoordinate(coordinate);
    LIBMATTI_FML_MavenCoordinate_Free(coordinate);
    return result;
}
