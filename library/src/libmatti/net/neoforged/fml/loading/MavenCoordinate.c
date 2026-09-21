#include "libmatti/net/neoforged/fml/loading/MavenCoordinate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public MavenCoordinate(String groupId, String artifactId, String extension, String classifier, String version)
LIBMATTI_FML_MavenCoordinate *LIBMATTI_FML_MavenCoordinate_New(const char *groupId, const char *artifactId,
                                                              const char *extension, const char *classifier,
                                                              const char *version)
{
    LIBMATTI_FML_MavenCoordinate *coordinate = calloc(1, sizeof(LIBMATTI_FML_MavenCoordinate));
    coordinate->groupId = strdup(groupId);
    coordinate->artifactId = strdup(artifactId);
    coordinate->extension = strdup(extension != NULL ? extension : "");
    coordinate->classifier = strdup(classifier != NULL ? classifier : "");
    coordinate->version = strdup(version);
    return coordinate;
}

void LIBMATTI_FML_MavenCoordinate_Free(LIBMATTI_FML_MavenCoordinate *coordinate)
{
    if (coordinate == NULL) return;
    free(coordinate->groupId);
    free(coordinate->artifactId);
    free(coordinate->extension);
    free(coordinate->classifier);
    free(coordinate->version);
    free(coordinate);
}

// Java: public static MavenCoordinate parse(String coordinate)
// Valid forms: groupId:artifactId:version / groupId:artifactId:version:classifier /
//              groupId:artifactId:version:classifier@extension / groupId:artifactId:version@extension
LIBMATTI_FML_MavenCoordinate *LIBMATTI_FML_MavenCoordinate_Parse(const char *coordinate)
{
    char *text = strdup(coordinate);
    char *extension = "";

    // Java: var coordinateAndExt = coordinate.split("@");
    char *at = strchr(text, '@');
    if (at != NULL)
    {
        *at = '\0';
        extension = at + 1;
        if (strchr(extension, '@') != NULL)
        {
            free(text);
            return NULL; // Java: throw new IllegalArgumentException("Malformed Maven coordinate: " + coordinate)
        }
    }

    char *parts[4] = {0};
    size_t partCount = 0;
    for (char *token = strtok(text, ":"); token != NULL && partCount < 4; token = strtok(NULL, ":"))
        parts[partCount++] = token;

    if (partCount != 3 && partCount != 4)
    {
        free(text);
        return NULL;
    }

    LIBMATTI_FML_MavenCoordinate *result =
        LIBMATTI_FML_MavenCoordinate_New(parts[0], parts[1], extension, partCount == 4 ? parts[3] : "", parts[2]);
    free(text);
    return result;
}

// Java: public Path toRelativeRepositoryPath()
char *LIBMATTI_FML_MavenCoordinate_ToRelativeRepositoryPath(const LIBMATTI_FML_MavenCoordinate *coordinate)
{
    // Java: String fileName = artifactId + "-" + version + (!classifier.isEmpty() ? "-" + classifier : "") + (!extension.isEmpty() ? "." + extension : ".jar");
    size_t fileNameLength = strlen(coordinate->artifactId) + strlen(coordinate->version) + 2 +
                            (coordinate->classifier[0] != '\0' ? strlen(coordinate->classifier) + 1 : 0) +
                            (coordinate->extension[0] != '\0' ? strlen(coordinate->extension) + 1 : 4);
    char *fileName = malloc(fileNameLength);
    snprintf(fileName, fileNameLength, "%s-%s%s%s", coordinate->artifactId, coordinate->version,
             coordinate->classifier[0] != '\0' ? "-" : "", coordinate->classifier[0] != '\0'
                                                     ? coordinate->classifier
                                                     : (coordinate->extension[0] != '\0' ? "" : ""));
    size_t written = strlen(fileName);
    if (coordinate->extension[0] != '\0')
        snprintf(fileName + written, fileNameLength - written, ".%s", coordinate->extension);
    else
        snprintf(fileName + written, fileNameLength - written, ".jar");

    // Java: String[] groups = groupId.split("\\."); Path result = Paths.get(groups[0]); for (i=1..) result = result.resolve(groups[i]);
    //       return result.resolve(artifactId).resolve(version).resolve(fileName);
    char *groupPath = strdup(coordinate->groupId);
    for (char *p = groupPath; *p != '\0'; p++)
        if (*p == '.') *p = '/';

    size_t length = strlen(groupPath) + strlen(coordinate->artifactId) + strlen(coordinate->version) +
                    strlen(fileName) + 4;
    char *path = malloc(length);
    snprintf(path, length, "%s/%s/%s/%s", groupPath, coordinate->artifactId, coordinate->version, fileName);
    free(groupPath);
    free(fileName);
    return path;
}

// Java: @Override public String toString()
char *LIBMATTI_FML_MavenCoordinate_ToString(const LIBMATTI_FML_MavenCoordinate *coordinate)
{
    size_t length = 256 + strlen(coordinate->groupId) + strlen(coordinate->artifactId) + strlen(coordinate->version) +
                    strlen(coordinate->classifier) + strlen(coordinate->extension);
    char *result = malloc(length);
    snprintf(result, length, "%s:%s:%s%s%s", coordinate->groupId, coordinate->artifactId, coordinate->version,
             coordinate->classifier[0] != '\0' ? ":" : "", coordinate->classifier);
    if (coordinate->extension[0] != '\0')
    {
        size_t written = strlen(result);
        snprintf(result + written, length - written, "@%s", coordinate->extension);
    }
    return result;
}
