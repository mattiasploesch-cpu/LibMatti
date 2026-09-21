// Port of org.apache.maven.artifact.versioning.VersionRange.
// Java caches the ranges in a WeakHashMap keyed by the spec string and hands out the same instance;
// the port returns a fresh range so the caller can release it.

#ifndef MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_VERSIONRANGE_H
#define MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_VERSIONRANGE_H

#include "libmatti/org/apache/maven/artifact/versioning/ArtifactVersion.h"
#include "libmatti/org/apache/maven/artifact/versioning/Restriction.h"

#include <stddef.h>

typedef struct LIBMATTI_MAVEN_VersionRange LIBMATTI_MAVEN_VersionRange;

struct LIBMATTI_MAVEN_VersionRange
{
    // Java: private final ArtifactVersion recommendedVersion
    LIBMATTI_MAVEN_ArtifactVersion *recommendedVersion;
    // Java: private final List<Restriction> restrictions
    LIBMATTI_MAVEN_Restriction **restrictions;
    size_t restrictionCount;
    // Java has no such field; the C port keeps the spec the range was created from for the callers
    // that log the range
    char *spec;
};

// Java: public static VersionRange createFromVersionSpec(String spec) throws InvalidVersionSpecificationException
// NULL stands in for the thrown InvalidVersionSpecificationException (it is logged).
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CreateFromVersionSpec(const char *spec);
// Java: public static VersionRange createFromVersion(String version)
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CreateFromVersion(const char *version);

// Java: public ArtifactVersion getRecommendedVersion()
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_GetRecommendedVersion(
    const LIBMATTI_MAVEN_VersionRange *range);
// Java: public List<Restriction> getRestrictions()
LIBMATTI_MAVEN_Restriction **LIBMATTI_MAVEN_VersionRange_GetRestrictions(
    const LIBMATTI_MAVEN_VersionRange *range, size_t *count);

// Java: public boolean containsVersion(ArtifactVersion version)
int LIBMATTI_MAVEN_VersionRange_ContainsVersion(const LIBMATTI_MAVEN_VersionRange *range,
                                               const LIBMATTI_MAVEN_ArtifactVersion *version);
// Java: public boolean hasRestrictions()
int LIBMATTI_MAVEN_VersionRange_HasRestrictions(const LIBMATTI_MAVEN_VersionRange *range);

// Java: public VersionRange cloneOf()
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CloneOf(const LIBMATTI_MAVEN_VersionRange *range);

// Java: public VersionRange restrict(VersionRange restriction)
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_Restrict(const LIBMATTI_MAVEN_VersionRange *range,
                                                                 const LIBMATTI_MAVEN_VersionRange *restriction);
// Java: private List<Restriction> intersection(List<Restriction> r1, List<Restriction> r2)
// Out: array of new restrictions, caller frees each with Restriction_Free
LIBMATTI_MAVEN_Restriction **LIBMATTI_MAVEN_VersionRange_Intersection(
    const LIBMATTI_MAVEN_Restriction **r1, size_t r1Count, const LIBMATTI_MAVEN_Restriction **r2,
    size_t r2Count, size_t *outCount);

// Java: public ArtifactVersion matchVersion(List<ArtifactVersion> versions) - NULL when no version matches
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_MatchVersion(
    const LIBMATTI_MAVEN_VersionRange *range, const LIBMATTI_MAVEN_ArtifactVersion **versions,
    size_t versionCount);
// Java: public ArtifactVersion getSelectedVersion(Artifact artifact) throws OverConstrainedVersionException
// NULL stands in for the OverConstrainedVersionException of an empty restriction set
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_GetSelectedVersion(
    const LIBMATTI_MAVEN_VersionRange *range);
// Java: public boolean isSelectedVersionKnown(Artifact artifact) throws OverConstrainedVersionException
int LIBMATTI_MAVEN_VersionRange_IsSelectedVersionKnown(const LIBMATTI_MAVEN_VersionRange *range);

// Java: public boolean equals(Object obj)
int LIBMATTI_MAVEN_VersionRange_Equals(const LIBMATTI_MAVEN_VersionRange *range, const void *other);
// Java: public int hashCode()
int LIBMATTI_MAVEN_VersionRange_HashCode(const LIBMATTI_MAVEN_VersionRange *range);

// Java: @Override public String toString() - the returned string is new, the caller frees it
char *LIBMATTI_MAVEN_VersionRange_ToString(const LIBMATTI_MAVEN_VersionRange *range);

void LIBMATTI_MAVEN_VersionRange_Free(LIBMATTI_MAVEN_VersionRange *range);

#endif //MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_VERSIONRANGE_H
