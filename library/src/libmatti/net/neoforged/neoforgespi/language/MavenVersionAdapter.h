// Port of net.neoforged.neoforgespi.language.MavenVersionAdapter.
// org.apache.maven.artifact.versioning is ported in org/apache/maven/artifact/versioning; the adapter
// keeps Java's ArtifactVersion / VersionRange types, which the rest of the port refers to as
// LIBMATTI_NEOFORGESPI_ArtifactVersion / LIBMATTI_NEOFORGESPI_VersionRange.

#ifndef MATTICRAFT_NEOFORGESPI_MAVENVERSIONADAPTER_H
#define MATTICRAFT_NEOFORGESPI_MAVENVERSIONADAPTER_H

#include "libmatti/org/apache/maven/artifact/versioning/ArtifactVersion.h"
#include "libmatti/org/apache/maven/artifact/versioning/VersionRange.h"

// org.apache.maven.artifact.versioning.ArtifactVersion (the port keeps DefaultArtifactVersion)
typedef LIBMATTI_MAVEN_ArtifactVersion LIBMATTI_NEOFORGESPI_ArtifactVersion;
// org.apache.maven.artifact.versioning.VersionRange
typedef LIBMATTI_MAVEN_VersionRange LIBMATTI_NEOFORGESPI_VersionRange;

// Java: public static VersionRange createFromVersionSpec(String spec)
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(const char *spec);
// Java: new DefaultArtifactVersion(version)
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(const char *version);

// Java: VersionRange.containsVersion(ArtifactVersion)
int LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(const LIBMATTI_NEOFORGESPI_VersionRange *range,
                                                            const LIBMATTI_NEOFORGESPI_ArtifactVersion *version);
// Java: DefaultArtifactVersion.compareTo(ArtifactVersion)
int LIBMATTI_NEOFORGESPI_MavenVersionAdapter_Compare(const LIBMATTI_NEOFORGESPI_ArtifactVersion *a,
                                                     const LIBMATTI_NEOFORGESPI_ArtifactVersion *b);
// Java: VersionRange.createFromVersion(String)
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_RangeFromVersion(const char *version);
// Java: ArtifactVersion.toString()
const char *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *version);

void LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeRange(LIBMATTI_NEOFORGESPI_VersionRange *range);
void LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(LIBMATTI_NEOFORGESPI_ArtifactVersion *version);

#endif //MATTICRAFT_NEOFORGESPI_MAVENVERSIONADAPTER_H
