// Port of org.apache.maven.artifact.versioning.Restriction.

#include "libmatti/org/apache/maven/artifact/versioning/Restriction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public static final Restriction EVERYTHING
static LIBMATTI_MAVEN_Restriction everything = {NULL, 0, NULL, 0};

const LIBMATTI_MAVEN_Restriction *LIBMATTI_MAVEN_Restriction_Everything(void)
{
    return &everything;
}

LIBMATTI_MAVEN_Restriction *LIBMATTI_MAVEN_Restriction_New(LIBMATTI_MAVEN_ArtifactVersion *lowerBound,
                                                          int lowerBoundInclusive,
                                                          LIBMATTI_MAVEN_ArtifactVersion *upperBound,
                                                          int upperBoundInclusive)
{
    LIBMATTI_MAVEN_Restriction *restriction = malloc(sizeof(LIBMATTI_MAVEN_Restriction));
    restriction->lowerBound = lowerBound;
    restriction->lowerBoundInclusive = lowerBoundInclusive;
    restriction->upperBound = upperBound;
    restriction->upperBoundInclusive = upperBoundInclusive;
    return restriction;
}

void LIBMATTI_MAVEN_Restriction_Free(LIBMATTI_MAVEN_Restriction *restriction)
{
    // Java: EVERYTHING is a shared constant
    if (restriction == NULL || restriction == &everything) return;
    free(restriction);
}

// Java: public boolean containsVersion(ArtifactVersion version)
int LIBMATTI_MAVEN_Restriction_ContainsVersion(const LIBMATTI_MAVEN_Restriction *restriction,
                                              const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    if (restriction->lowerBound != NULL)
    {
        int comparison = LIBMATTI_MAVEN_ArtifactVersion_Compare(restriction->lowerBound, version);

        if (comparison == 0 && !restriction->lowerBoundInclusive) return 0;
        if (comparison > 0) return 0;
    }

    if (restriction->upperBound != NULL)
    {
        int comparison = LIBMATTI_MAVEN_ArtifactVersion_Compare(restriction->upperBound, version);

        if (comparison == 0 && !restriction->upperBoundInclusive) return 0;
        return comparison >= 0;
    }

    return 1;
}

// Java: public boolean equals(Object other)
int LIBMATTI_MAVEN_Restriction_Equals(const LIBMATTI_MAVEN_Restriction *restriction, const void *other)
{
    if (restriction == other) return 1;
    if (restriction == NULL || other == NULL) return 0;

    const LIBMATTI_MAVEN_Restriction *r = (const LIBMATTI_MAVEN_Restriction *) other;

    if (restriction->lowerBound != NULL)
    {
        if (!LIBMATTI_MAVEN_ArtifactVersion_Equals(restriction->lowerBound, r->lowerBound)) return 0;
    }
    else if (r->lowerBound != NULL)
        return 0;

    if (restriction->lowerBoundInclusive != r->lowerBoundInclusive) return 0;

    if (restriction->upperBound != NULL)
    {
        if (!LIBMATTI_MAVEN_ArtifactVersion_Equals(restriction->upperBound, r->upperBound)) return 0;
    }
    else if (r->upperBound != NULL)
        return 0;

    return restriction->upperBoundInclusive == r->upperBoundInclusive;
}

// Java: public int hashCode()
int LIBMATTI_MAVEN_Restriction_HashCode(const LIBMATTI_MAVEN_Restriction *restriction)
{
    int result = 13;

    if (restriction->lowerBound == NULL)
        result += 1;
    else
        result += LIBMATTI_MAVEN_ArtifactVersion_HashCode(restriction->lowerBound);

    result *= restriction->lowerBoundInclusive ? 1 : 2;

    if (restriction->upperBound == NULL)
        result -= 3;
    else
        result -= LIBMATTI_MAVEN_ArtifactVersion_HashCode(restriction->upperBound);

    result *= restriction->upperBoundInclusive ? 2 : 3;

    return result;
}

// Java: public String toString()
char *LIBMATTI_MAVEN_Restriction_ToString(const LIBMATTI_MAVEN_Restriction *restriction)
{
    const char *lower = restriction->lowerBound != NULL
                            ? LIBMATTI_MAVEN_ArtifactVersion_ToString(restriction->lowerBound)
                            : "";
    const char *upper = restriction->upperBound != NULL
                            ? LIBMATTI_MAVEN_ArtifactVersion_ToString(restriction->upperBound)
                            : "";

    size_t length = strlen(lower) + strlen(upper) + 4;
    char *result = malloc(length);
    snprintf(result, length, "%c%s,%s%c", restriction->lowerBoundInclusive ? '[' : '(', lower, upper,
             restriction->upperBoundInclusive ? ']' : ')');
    return result;
}
