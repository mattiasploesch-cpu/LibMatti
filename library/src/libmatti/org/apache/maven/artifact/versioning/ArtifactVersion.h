// Port of org.apache.maven.artifact.versioning.ArtifactVersion and its only implementation,
// DefaultArtifactVersion. The port keeps the concrete class for the interface.
// Java's equals is "compareTo == 0" and hashCode is 11 + comparable.orderingHashCode(); the port
// exposes both.

#ifndef MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_ARTIFACTVERSION_H
#define MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_ARTIFACTVERSION_H

#include "libmatti/org/apache/maven/artifact/versioning/ComparableVersion.h"

typedef struct LIBMATTI_MAVEN_ArtifactVersion LIBMATTI_MAVEN_ArtifactVersion;

struct LIBMATTI_MAVEN_ArtifactVersion
{
    // Java: private final ComparableVersion comparable - toString() returns comparable.toString()
    char *version;
    // Java: private Integer majorVersion / minorVersion / incrementalVersion / buildNumber (null = absent)
    int majorVersion;
    int majorVersionPresent;
    int minorVersion;
    int minorVersionPresent;
    int incrementalVersion;
    int incrementalVersionPresent;
    int buildNumber;
    int buildNumberPresent;
    // Java: private String qualifier
    char *qualifier;
    LIBMATTI_MAVEN_ComparableVersion *comparable;
};

// Java: public DefaultArtifactVersion(String version)
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_ArtifactVersion_New(const char *version);
void LIBMATTI_MAVEN_ArtifactVersion_Free(LIBMATTI_MAVEN_ArtifactVersion *version);

// Java: public int compareTo(ArtifactVersion otherVersion)
int LIBMATTI_MAVEN_ArtifactVersion_Compare(const LIBMATTI_MAVEN_ArtifactVersion *version,
                                           const LIBMATTI_MAVEN_ArtifactVersion *otherVersion);
// Java: public boolean equals(Object other) - compareTo == 0 against another ArtifactVersion
int LIBMATTI_MAVEN_ArtifactVersion_Equals(const LIBMATTI_MAVEN_ArtifactVersion *version, const void *other);
// Java: public int hashCode()
int LIBMATTI_MAVEN_ArtifactVersion_HashCode(const LIBMATTI_MAVEN_ArtifactVersion *version);
// Java: public String toString()
const char *LIBMATTI_MAVEN_ArtifactVersion_ToString(const LIBMATTI_MAVEN_ArtifactVersion *version);

// Java: public int getMajorVersion() / getMinorVersion() / getIncrementalVersion() / getBuildNumber()
int LIBMATTI_MAVEN_ArtifactVersion_GetMajorVersion(const LIBMATTI_MAVEN_ArtifactVersion *version);
int LIBMATTI_MAVEN_ArtifactVersion_GetMinorVersion(const LIBMATTI_MAVEN_ArtifactVersion *version);
int LIBMATTI_MAVEN_ArtifactVersion_GetIncrementalVersion(const LIBMATTI_MAVEN_ArtifactVersion *version);
int LIBMATTI_MAVEN_ArtifactVersion_GetBuildNumber(const LIBMATTI_MAVEN_ArtifactVersion *version);
// Java: public String getQualifier()
const char *LIBMATTI_MAVEN_ArtifactVersion_GetQualifier(const LIBMATTI_MAVEN_ArtifactVersion *version);

#endif //MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_ARTIFACTVERSION_H
