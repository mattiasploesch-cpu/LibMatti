// Port of org.apache.maven.artifact.versioning.VersionRange.

#include "libmatti/org/apache/maven/artifact/versioning/VersionRange.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Logger / InvalidVersionSpecificationException
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: throw new InvalidVersionSpecificationException(message)
static void invalid_version_specification(const char *message, const char *spec)
{
    // the port aborts like the uncaught exception it would be
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "InvalidVersionSpecificationException: {} ({})", message, spec);
    abort();
}

static void restrictions_add(LIBMATTI_MAVEN_Restriction ***restrictions, size_t *count,
                            LIBMATTI_MAVEN_Restriction *restriction)
{
    *restrictions = realloc(*restrictions, sizeof(**restrictions) * (*count + 1));
    (*restrictions)[(*count)++] = restriction;
}

// Java: String.substring(start, end) / String.trim()
static char *substring(const char *value, size_t start, size_t end)
{
    size_t length = end - start;
    char *result = malloc(length + 1);
    memcpy(result, value + start, length);
    result[length] = '\0';
    return result;
}

static char *trimmed(const char *value)
{
    while (*value != '\0' && (unsigned char)*value <= ' ') value++;

    size_t length = strlen(value);
    while (length > 0 && (unsigned char)value[length - 1] <= ' ') length--;

    return substring(value, 0, length);
}

// Java: private static Restriction parseRestriction(String spec)
static LIBMATTI_MAVEN_Restriction *parse_restriction(const char *spec)
{
    int lowerBoundInclusive = spec[0] == '[';
    int upperBoundInclusive = spec[strlen(spec) - 1] == ']';

    char *process = substring(spec, 1, strlen(spec) - 1);
    char *trimmedProcess = trimmed(process);
    free(process);
    process = trimmedProcess;

    const char *comma = strchr(process, ',');
    LIBMATTI_MAVEN_Restriction *restriction;

    if (comma == NULL)
    {
        if (!lowerBoundInclusive || !upperBoundInclusive)
        {
            invalid_version_specification("Single version must be surrounded by []", spec);
            free(process);
            return NULL;
        }

        LIBMATTI_MAVEN_ArtifactVersion *version = LIBMATTI_MAVEN_ArtifactVersion_New(process);
        restriction = LIBMATTI_MAVEN_Restriction_New(version, lowerBoundInclusive, version, upperBoundInclusive);
    }
    else
    {
        size_t commaIndex = (size_t)(comma - process);

        char *lowerText = substring(process, 0, commaIndex);
        char *lowerBound = trimmed(lowerText);
        free(lowerText);

        char *upperText = substring(process, commaIndex + 1, strlen(process));
        char *upperBound = trimmed(upperText);
        free(upperText);

        LIBMATTI_MAVEN_ArtifactVersion *lowerVersion = NULL;
        if (lowerBound[0] != '\0') lowerVersion = LIBMATTI_MAVEN_ArtifactVersion_New(lowerBound);

        LIBMATTI_MAVEN_ArtifactVersion *upperVersion = NULL;
        if (upperBound[0] != '\0') upperVersion = LIBMATTI_MAVEN_ArtifactVersion_New(upperBound);

        if (upperVersion != NULL && lowerVersion != NULL)
        {
            int result = LIBMATTI_MAVEN_ArtifactVersion_Compare(upperVersion, lowerVersion);
            if (result < 0 || (result == 0 && (!lowerBoundInclusive || !upperBoundInclusive)))
            {
                invalid_version_specification("Range defies version ordering", spec);
                LIBMATTI_MAVEN_ArtifactVersion_Free(lowerVersion);
                LIBMATTI_MAVEN_ArtifactVersion_Free(upperVersion);
                free(lowerBound);
                free(upperBound);
                free(process);
                return NULL;
            }
        }

        restriction = LIBMATTI_MAVEN_Restriction_New(lowerVersion, lowerBoundInclusive, upperVersion,
                                                    upperBoundInclusive);

        free(lowerBound);
        free(upperBound);
    }

    free(process);
    return restriction;
}

// Java: public static VersionRange createFromVersionSpec(String spec)
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CreateFromVersionSpec(const char *spec)
{
    if (spec == NULL) return NULL;

    LIBMATTI_MAVEN_Restriction **restrictions = NULL;
    size_t restrictionCount = 0;
    char *process = strdup(spec);
    LIBMATTI_MAVEN_ArtifactVersion *version = NULL;
    LIBMATTI_MAVEN_ArtifactVersion *upperBound = NULL;
    LIBMATTI_MAVEN_ArtifactVersion *lowerBound = NULL;

    while (process[0] == '[' || process[0] == '(')
    {
        const char *index1 = strchr(process, ')');
        const char *index2 = strchr(process, ']');

        const char *index = index2;
        if (index2 == NULL || (index1 != NULL && index1 < index2))
            if (index1 != NULL) index = index1;

        if (index == NULL)
        {
            invalid_version_specification("Unbounded range", spec);
            free(process);
            return NULL;
        }

        char *restrictionSpec = substring(process, 0, (size_t)(index - process) + 1);
        LIBMATTI_MAVEN_Restriction *restriction = parse_restriction(restrictionSpec);
        free(restrictionSpec);

        if (restriction == NULL)
        {
            free(process);
            return NULL;
        }

        if (lowerBound == NULL) lowerBound = restriction->lowerBound;

        if (upperBound != NULL)
        {
            if (restriction->lowerBound == NULL ||
                LIBMATTI_MAVEN_ArtifactVersion_Compare(restriction->lowerBound, upperBound) < 0)
            {
                invalid_version_specification("Ranges overlap", spec);
                LIBMATTI_MAVEN_Restriction_Free(restriction);
                free(process);
                return NULL;
            }
        }

        restrictions_add(&restrictions, &restrictionCount, restriction);
        upperBound = restriction->upperBound;

        char *rest = trimmed(process + (size_t)(index - process) + 1);
        free(process);
        process = rest;

        if (process[0] == ',')
        {
            char *withoutComma = trimmed(process + 1);
            free(process);
            process = withoutComma;
        }
    }

    if (process[0] != '\0')
    {
        if (restrictionCount > 0)
        {
            invalid_version_specification("Only fully-qualified sets allowed in multiple set scenario", spec);
            free(process);
            return NULL;
        }

        version = LIBMATTI_MAVEN_ArtifactVersion_New(process);
        // Java: restrictions.add(Restriction.EVERYTHING)
        restrictions_add(&restrictions, &restrictionCount,
                         (LIBMATTI_MAVEN_Restriction *)LIBMATTI_MAVEN_Restriction_Everything());
    }

    free(process);

    LIBMATTI_MAVEN_VersionRange *range = calloc(1, sizeof(LIBMATTI_MAVEN_VersionRange));
    range->recommendedVersion = version;
    range->restrictions = restrictions;
    range->restrictionCount = restrictionCount;
    range->spec = strdup(spec);
    return range;
}

// Java: public static VersionRange createFromVersion(String version)
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CreateFromVersion(const char *version)
{
    // Java: if (DefaultArtifact.empty(version)) return null
    if (version == NULL || version[0] == '\0') return NULL;

    LIBMATTI_MAVEN_VersionRange *range = calloc(1, sizeof(LIBMATTI_MAVEN_VersionRange));
    // Java: Collections.emptyList()
    range->restrictions = NULL;
    range->restrictionCount = 0;
    range->recommendedVersion = LIBMATTI_MAVEN_ArtifactVersion_New(version);
    range->spec = strdup(version);
    return range;
}

// Java: public ArtifactVersion getRecommendedVersion()
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_GetRecommendedVersion(
    const LIBMATTI_MAVEN_VersionRange *range)
{
    return range->recommendedVersion;
}

// Java: public List<Restriction> getRestrictions()
LIBMATTI_MAVEN_Restriction **LIBMATTI_MAVEN_VersionRange_GetRestrictions(
    const LIBMATTI_MAVEN_VersionRange *range, size_t *count)
{
    *count = range->restrictionCount;
    return range->restrictions;
}

// Java: public boolean containsVersion(ArtifactVersion version)
int LIBMATTI_MAVEN_VersionRange_ContainsVersion(const LIBMATTI_MAVEN_VersionRange *range,
                                               const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    for (size_t i = 0; i < range->restrictionCount; i++)
        if (LIBMATTI_MAVEN_Restriction_ContainsVersion(range->restrictions[i], version)) return 1;

    return 0;
}

// Java: public boolean hasRestrictions() { return !restrictions.isEmpty() && recommendedVersion == null; }
int LIBMATTI_MAVEN_VersionRange_HasRestrictions(const LIBMATTI_MAVEN_VersionRange *range)
{
    return range->restrictionCount > 0 && range->recommendedVersion == NULL;
}

// Java: public VersionRange cloneOf()
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_CloneOf(const LIBMATTI_MAVEN_VersionRange *range)
{
    // Java: copies the list shell, the Restriction instances stay shared (VersionRange is immutable)
    LIBMATTI_MAVEN_VersionRange *clone = calloc(1, sizeof(LIBMATTI_MAVEN_VersionRange));
    clone->recommendedVersion = range->recommendedVersion;
    clone->restrictionCount = range->restrictionCount;
    if (range->restrictionCount > 0)
    {
        clone->restrictions = malloc(sizeof(LIBMATTI_MAVEN_Restriction *) * range->restrictionCount);
        memcpy(clone->restrictions, range->restrictions, sizeof(LIBMATTI_MAVEN_Restriction *) * range->restrictionCount);
    }
    clone->spec = strdup(range->spec != NULL ? range->spec : "");
    return clone;
}

// Java: public ArtifactVersion matchVersion(List<ArtifactVersion> versions)
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_MatchVersion(
    const LIBMATTI_MAVEN_VersionRange *range, const LIBMATTI_MAVEN_ArtifactVersion **versions,
    size_t versionCount)
{
    LIBMATTI_MAVEN_ArtifactVersion *matched = NULL;
    for (size_t i = 0; i < versionCount; i++)
    {
        if (!LIBMATTI_MAVEN_VersionRange_ContainsVersion(range, versions[i])) continue;
        // valid - check if it is greater than the currently matched version
        if (matched == NULL || LIBMATTI_MAVEN_ArtifactVersion_Compare(versions[i], matched) > 0)
            matched = versions[i];
    }
    return matched;
}

// Java: public ArtifactVersion getSelectedVersion(Artifact artifact) throws OverConstrainedVersionException
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_VersionRange_GetSelectedVersion(
    const LIBMATTI_MAVEN_VersionRange *range)
{
    if (range->recommendedVersion != NULL) return range->recommendedVersion;

    // Java: if (restrictions.isEmpty()) throw new OverConstrainedVersionException(...)
    if (range->restrictionCount == 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "OverConstrainedVersionException: The artifact has no valid ranges");
        return NULL;
    }
    return NULL;
}

// Java: public boolean isSelectedVersionKnown(Artifact artifact) throws OverConstrainedVersionException
int LIBMATTI_MAVEN_VersionRange_IsSelectedVersionKnown(const LIBMATTI_MAVEN_VersionRange *range)
{
    if (range->recommendedVersion != NULL) return 1;

    if (range->restrictionCount == 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "OverConstrainedVersionException: The artifact has no valid ranges");
        return 0;
    }
    return 0;
}

// Java: private List<Restriction> intersection(List<Restriction> r1, List<Restriction> r2)
LIBMATTI_MAVEN_Restriction **LIBMATTI_MAVEN_VersionRange_Intersection(
    const LIBMATTI_MAVEN_Restriction **r1, size_t r1Count, const LIBMATTI_MAVEN_Restriction **r2,
    size_t r2Count, size_t *outCount)
{
    LIBMATTI_MAVEN_Restriction **restrictions = NULL;
    size_t count = 0;

    // Java: both iterators are advanced before the loop (i1.next() / i2.next()); empty inputs end it
    if (r1Count == 0 || r2Count == 0)
    {
        *outCount = 0;
        return NULL;
    }

    size_t i1 = 0;
    size_t i2 = 0;
    const LIBMATTI_MAVEN_Restriction *res1 = r1[i1++];
    const LIBMATTI_MAVEN_Restriction *res2 = r2[i2++];

    int done = 0;
    while (!done)
    {
        if (res1->lowerBound == NULL || res2->upperBound == NULL ||
            LIBMATTI_MAVEN_ArtifactVersion_Compare(res1->lowerBound, res2->upperBound) <= 0)
        {
            if (res1->upperBound == NULL || res2->lowerBound == NULL ||
                LIBMATTI_MAVEN_ArtifactVersion_Compare(res1->upperBound, res2->lowerBound) >= 0)
            {
                const LIBMATTI_MAVEN_ArtifactVersion *lower;
                const LIBMATTI_MAVEN_ArtifactVersion *upper;
                int lowerInclusive;
                int upperInclusive;

                // overlaps
                if (res1->lowerBound == NULL)
                {
                    lower = res2->lowerBound;
                    lowerInclusive = res2->lowerBoundInclusive;
                }
                else if (res2->lowerBound == NULL)
                {
                    lower = res1->lowerBound;
                    lowerInclusive = res1->lowerBoundInclusive;
                }
                else
                {
                    int comparison = LIBMATTI_MAVEN_ArtifactVersion_Compare(res1->lowerBound, res2->lowerBound);
                    if (comparison < 0)
                    {
                        lower = res2->lowerBound;
                        lowerInclusive = res2->lowerBoundInclusive;
                    }
                    else if (comparison == 0)
                    {
                        lower = res1->lowerBound;
                        lowerInclusive = res1->lowerBoundInclusive && res2->lowerBoundInclusive;
                    }
                    else
                    {
                        lower = res1->lowerBound;
                        lowerInclusive = res1->lowerBoundInclusive;
                    }
                }

                if (res1->upperBound == NULL)
                {
                    upper = res2->upperBound;
                    upperInclusive = res2->upperBoundInclusive;
                }
                else if (res2->upperBound == NULL)
                {
                    upper = res1->upperBound;
                    upperInclusive = res1->upperBoundInclusive;
                }
                else
                {
                    int comparison = LIBMATTI_MAVEN_ArtifactVersion_Compare(res1->upperBound, res2->upperBound);
                    if (comparison < 0)
                    {
                        upper = res1->upperBound;
                        upperInclusive = res1->upperBoundInclusive;
                    }
                    else if (comparison == 0)
                    {
                        upper = res1->upperBound;
                        upperInclusive = res1->upperBoundInclusive && res2->upperBoundInclusive;
                    }
                    else
                    {
                        upper = res2->upperBound;
                        upperInclusive = res2->upperBoundInclusive;
                    }
                }

                // don't add if they are equal and one is not inclusive
                if (lower == NULL || upper == NULL ||
                    LIBMATTI_MAVEN_ArtifactVersion_Compare(lower, upper) != 0)
                {
                    restrictions_add(&restrictions, &count, LIBMATTI_MAVEN_Restriction_New(
                        (LIBMATTI_MAVEN_ArtifactVersion *) lower, lowerInclusive,
                        (LIBMATTI_MAVEN_ArtifactVersion *) upper, upperInclusive));
                }
                else if (lowerInclusive && upperInclusive)
                {
                    restrictions_add(&restrictions, &count, LIBMATTI_MAVEN_Restriction_New(
                        (LIBMATTI_MAVEN_ArtifactVersion *) lower, lowerInclusive,
                        (LIBMATTI_MAVEN_ArtifactVersion *) upper, upperInclusive));
                }

                // Java: if (upper == res2.getUpperBound()) - advance res2, else advance res1
                if (upper == res2->upperBound)
                {
                    // advance res2
                    if (i2 < r2Count)
                        res2 = r2[i2++];
                    else
                        done = 1;
                }
                else
                {
                    // advance res1
                    if (i1 < r1Count)
                        res1 = r1[i1++];
                    else
                        done = 1;
                }
            }
            else
            {
                // move on to next in r1
                if (i1 < r1Count)
                    res1 = r1[i1++];
                else
                    done = 1;
            }
        }
        else
        {
            // move on to next in r2
            if (i2 < r2Count)
                res2 = r2[i2++];
            else
                done = 1;
        }
    }

    *outCount = count;
    return restrictions;
}

// Java: public VersionRange restrict(VersionRange restriction)
LIBMATTI_MAVEN_VersionRange *LIBMATTI_MAVEN_VersionRange_Restrict(const LIBMATTI_MAVEN_VersionRange *range,
                                                                 const LIBMATTI_MAVEN_VersionRange *restriction)
{
    const LIBMATTI_MAVEN_Restriction **r1 = (const LIBMATTI_MAVEN_Restriction **) range->restrictions;
    const LIBMATTI_MAVEN_Restriction **r2 = (const LIBMATTI_MAVEN_Restriction **) restriction->restrictions;
    LIBMATTI_MAVEN_Restriction **restrictions = NULL;
    size_t restrictionCount = 0;

    if (range->restrictionCount == 0 || restriction->restrictionCount == 0)
    {
        // Java: Collections.emptyList()
        restrictions = NULL;
        restrictionCount = 0;
    }
    else
    {
        // Java: Collections.unmodifiableList(intersection(r1, r2))
        restrictions = LIBMATTI_MAVEN_VersionRange_Intersection(r1, range->restrictionCount, r2,
                                                               restriction->restrictionCount,
                                                               &restrictionCount);
    }

    LIBMATTI_MAVEN_ArtifactVersion *version = NULL;
    if (restrictionCount > 0)
    {
        for (size_t i = 0; i < restrictionCount; i++)
        {
            const LIBMATTI_MAVEN_Restriction *r = restrictions[i];
            if (range->recommendedVersion != NULL &&
                LIBMATTI_MAVEN_Restriction_ContainsVersion(r, range->recommendedVersion))
            {
                // if we find the original, use that
                version = range->recommendedVersion;
                break;
            }
            else if (version == NULL && restriction->recommendedVersion != NULL &&
                     LIBMATTI_MAVEN_Restriction_ContainsVersion(r, restriction->recommendedVersion))
            {
                // use this if we can, but prefer the original if possible
                version = restriction->recommendedVersion;
            }
        }
    }
    // Either the original or the specified version ranges have no restrictions
    else if (range->recommendedVersion != NULL)
    {
        // Use the original recommended version since it exists
        version = range->recommendedVersion;
    }
    else if (restriction->recommendedVersion != NULL)
    {
        // Use the recommended version from the specified VersionRange since there is no
        // original recommended version
        version = restriction->recommendedVersion;
    }

    LIBMATTI_MAVEN_VersionRange *result = calloc(1, sizeof(LIBMATTI_MAVEN_VersionRange));
    result->recommendedVersion = version;
    result->restrictions = restrictions;
    result->restrictionCount = restrictionCount;
    result->spec = strdup("");
    return result;
}

// Java: public boolean equals(Object obj)
int LIBMATTI_MAVEN_VersionRange_Equals(const LIBMATTI_MAVEN_VersionRange *range, const void *other)
{
    if (range == other) return 1;
    if (range == NULL || other == NULL) return 0;

    const LIBMATTI_MAVEN_VersionRange *r = (const LIBMATTI_MAVEN_VersionRange *) other;

    if (range->recommendedVersion == NULL)
    {
        if (r->recommendedVersion != NULL) return 0;
    }
    else if (!LIBMATTI_MAVEN_ArtifactVersion_Equals(range->recommendedVersion, r->recommendedVersion))
        return 0;

    if (range->restrictionCount != r->restrictionCount) return 0;
    for (size_t i = 0; i < range->restrictionCount; i++)
        if (!LIBMATTI_MAVEN_Restriction_Equals(range->restrictions[i], r->restrictions[i])) return 0;
    return 1;
}

// Java: public int hashCode()
int LIBMATTI_MAVEN_VersionRange_HashCode(const LIBMATTI_MAVEN_VersionRange *range)
{
    int hash = 7;
    hash = 31 * hash + (range->recommendedVersion == NULL ? 0 : LIBMATTI_MAVEN_ArtifactVersion_HashCode(range->recommendedVersion));
    // Java: restrictions.hashCode() - List hash over the elements
    hash = 31 * hash;
    for (size_t i = 0; i < range->restrictionCount; i++)
        hash = 31 * hash + LIBMATTI_MAVEN_Restriction_HashCode(range->restrictions[i]);
    return hash;
}

// Java: @Override public String toString()
char *LIBMATTI_MAVEN_VersionRange_ToString(const LIBMATTI_MAVEN_VersionRange *range)
{
    if (range->recommendedVersion != NULL)
        return strdup(LIBMATTI_MAVEN_ArtifactVersion_ToString(range->recommendedVersion));

    size_t length = 1;
    char **parts = calloc(range->restrictionCount > 0 ? range->restrictionCount : 1, sizeof(char *));
    for (size_t i = 0; i < range->restrictionCount; i++)
    {
        parts[i] = LIBMATTI_MAVEN_Restriction_ToString(range->restrictions[i]);
        length += strlen(parts[i]) + 1;
    }

    char *result = malloc(length);
    result[0] = '\0';
    for (size_t i = 0; i < range->restrictionCount; i++)
    {
        if (i > 0) strcat(result, ",");
        strcat(result, parts[i]);
        free(parts[i]);
    }
    free(parts);
    return result;
}

void LIBMATTI_MAVEN_VersionRange_Free(LIBMATTI_MAVEN_VersionRange *range)
{
    if (range == NULL) return;

    LIBMATTI_MAVEN_ArtifactVersion_Free(range->recommendedVersion);

    for (size_t i = 0; i < range->restrictionCount; i++)
    {
        LIBMATTI_MAVEN_Restriction *restriction = range->restrictions[i];

        // Java: EVERYTHING is a shared constant and its bounds are null
        if (restriction != LIBMATTI_MAVEN_Restriction_Everything())
        {
            // Java: a single version uses one ArtifactVersion for both bounds
            if (restriction->lowerBound != restriction->upperBound)
                LIBMATTI_MAVEN_ArtifactVersion_Free(restriction->upperBound);
            LIBMATTI_MAVEN_ArtifactVersion_Free(restriction->lowerBound);
        }
        LIBMATTI_MAVEN_Restriction_Free(restriction);
    }

    free(range->restrictions);
    free(range->spec);
    free(range);
}
