// Port of org.apache.maven.artifact.versioning.Restriction.
// The bounds are shared across restrictions in Java and never mutated; the C port owns them.

#ifndef MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_RESTRICTION_H
#define MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_RESTRICTION_H

#include "libmatti/org/apache/maven/artifact/versioning/ArtifactVersion.h"

typedef struct LIBMATTI_MAVEN_Restriction LIBMATTI_MAVEN_Restriction;

struct LIBMATTI_MAVEN_Restriction
{
    LIBMATTI_MAVEN_ArtifactVersion *lowerBound;
    int lowerBoundInclusive;
    LIBMATTI_MAVEN_ArtifactVersion *upperBound;
    int upperBoundInclusive;
};

// Java: public static final Restriction EVERYTHING = new Restriction(null, false, null, false)
const LIBMATTI_MAVEN_Restriction *LIBMATTI_MAVEN_Restriction_Everything(void);

// Java: public Restriction(ArtifactVersion lowerBound, boolean lowerBoundInclusive, ArtifactVersion upperBound, boolean upperBoundInclusive)
// The bounds are borrowed, as in Java; only the restriction itself is released.
LIBMATTI_MAVEN_Restriction *LIBMATTI_MAVEN_Restriction_New(LIBMATTI_MAVEN_ArtifactVersion *lowerBound,
                                                          int lowerBoundInclusive,
                                                          LIBMATTI_MAVEN_ArtifactVersion *upperBound,
                                                          int upperBoundInclusive);
void LIBMATTI_MAVEN_Restriction_Free(LIBMATTI_MAVEN_Restriction *restriction);

// Java: public boolean containsVersion(ArtifactVersion version)
int LIBMATTI_MAVEN_Restriction_ContainsVersion(const LIBMATTI_MAVEN_Restriction *restriction,
                                              const LIBMATTI_MAVEN_ArtifactVersion *version);

// Java: public boolean equals(Object other)
int LIBMATTI_MAVEN_Restriction_Equals(const LIBMATTI_MAVEN_Restriction *restriction, const void *other);
// Java: public int hashCode()
int LIBMATTI_MAVEN_Restriction_HashCode(const LIBMATTI_MAVEN_Restriction *restriction);

// Java: public String toString()
char *LIBMATTI_MAVEN_Restriction_ToString(const LIBMATTI_MAVEN_Restriction *restriction);

#endif //MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_RESTRICTION_H
