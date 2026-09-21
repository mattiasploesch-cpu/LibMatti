// Port of net.neoforged.fml.loading.MavenCoordinate.

#ifndef MATTICRAFT_FML_LOADING_MAVENCOORDINATE_H
#define MATTICRAFT_FML_LOADING_MAVENCOORDINATE_H

// Java: public record MavenCoordinate(String groupId, String artifactId, String extension, String classifier, String version)
typedef struct
{
    char *groupId;
    char *artifactId;
    char *extension;
    char *classifier;
    char *version;
} LIBMATTI_FML_MavenCoordinate;

// Java: public MavenCoordinate(String groupId, String artifactId, String extension, String classifier, String version)
LIBMATTI_FML_MavenCoordinate *LIBMATTI_FML_MavenCoordinate_New(const char *groupId, const char *artifactId,
                                                              const char *extension, const char *classifier,
                                                              const char *version);
// Java: public static MavenCoordinate parse(String coordinate) - NULL on a malformed coordinate
LIBMATTI_FML_MavenCoordinate *LIBMATTI_FML_MavenCoordinate_Parse(const char *coordinate);
void LIBMATTI_FML_MavenCoordinate_Free(LIBMATTI_FML_MavenCoordinate *coordinate);

// Java: public Path toRelativeRepositoryPath() - a new string
char *LIBMATTI_FML_MavenCoordinate_ToRelativeRepositoryPath(const LIBMATTI_FML_MavenCoordinate *coordinate);
// Java: @Override public String toString() - a new string
char *LIBMATTI_FML_MavenCoordinate_ToString(const LIBMATTI_FML_MavenCoordinate *coordinate);

#endif //MATTICRAFT_FML_LOADING_MAVENCOORDINATE_H
