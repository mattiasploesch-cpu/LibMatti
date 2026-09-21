// Port of org.apache.maven.artifact.versioning.DefaultArtifactVersion.

#include "libmatti/org/apache/maven/artifact/versioning/ArtifactVersion.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// Java: private static boolean isDigits(String cs)
static int is_digits(const char *cs)
{
    if (cs == NULL || cs[0] == '\0') return 0;
    for (size_t i = 0; cs[i] != '\0'; i++)
        if (!isdigit((unsigned char)cs[i])) return 0;
    return 1;
}

// Java: private static Integer tryParseInt(String s)
// 'out' is only written when the result is present; the return value is the presence
static int try_parse_int(const char *s, int *out)
{
    if (!is_digits(s)) return 0;

    // Java: long longValue = Long.parseLong(s)
    char *end = NULL;
    long long value = strtoll(s, &end, 10);
    if (end == s || *end != '\0') return 0;
    if (value > INT_MAX) return 0;

    *out = (int)value;
    return 1;
}

// Java: private static Integer getNextIntegerToken(String s)
static int get_next_integer_token(const char *s, int *out)
{
    if (strlen(s) > 1 && s[0] == '0') return 0;
    return try_parse_int(s, out);
}

// Java: Java 8+ String.split("\\.") - the trailing empty strings are dropped, the leading ones are kept
static char **split_on_dots(const char *value, size_t *count)
{
    size_t capacity = 4;
    char **tokens = calloc(capacity, sizeof(char *));
    size_t tokenCount = 0;

    const char *start = value;
    for (;;)
    {
        const char *end = strchr(start, '.');
        size_t length = end != NULL ? (size_t)(end - start) : strlen(start);

        if (tokenCount + 1 > capacity)
        {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(char *) * capacity);
        }

        char *token = malloc(length + 1);
        memcpy(token, start, length);
        token[length] = '\0';
        tokens[tokenCount++] = token;

        if (end == NULL) break;
        start = end + 1;
    }

    // Java: trailing empty strings are not included
    while (tokenCount > 1 && tokens[tokenCount - 1][0] == '\0')
        free(tokens[--tokenCount]);

    *count = tokenCount;
    return tokens;
}

// Java: public final void parseVersion(String version)
static void parse_version(LIBMATTI_MAVEN_ArtifactVersion *artifactVersion, const char *version)
{
    artifactVersion->comparable = LIBMATTI_MAVEN_ComparableVersion_New(version);
    // Java: toString() returns comparable.toString()
    artifactVersion->version = strdup(LIBMATTI_MAVEN_ComparableVersion_ToString(artifactVersion->comparable));

    const char *dash = strchr(version, '-');
    size_t part1Length = dash != NULL ? (size_t)(dash - version) : strlen(version);
    char *part1 = malloc(part1Length + 1);
    memcpy(part1, version, part1Length);
    part1[part1Length] = '\0';
    const char *part2 = dash != NULL ? dash + 1 : NULL;

    if (part2 != NULL)
    {
        if (strlen(part2) == 1 || part2[0] != '0')
        {
            int buildNumber = 0;
            if (try_parse_int(part2, &buildNumber))
            {
                artifactVersion->buildNumber = buildNumber;
                artifactVersion->buildNumberPresent = 1;
            }
            else
            {
                artifactVersion->qualifier = strdup(part2);
            }
        }
        else
        {
            artifactVersion->qualifier = strdup(part2);
        }
    }

    if (strchr(part1, '.') == NULL && part1[0] != '0')
    {
        int majorVersion = 0;
        if (try_parse_int(part1, &majorVersion))
        {
            artifactVersion->majorVersion = majorVersion;
            artifactVersion->majorVersionPresent = 1;
        }
        else
        {
            // Java: qualifier is the whole version, including "-"
            free(artifactVersion->qualifier);
            artifactVersion->qualifier = strdup(version);
            artifactVersion->buildNumberPresent = 0;
        }
    }
    else
    {
        int fallback = 0;

        size_t tokenCount = 0;
        char **tokens = split_on_dots(part1, &tokenCount);
        size_t index = 0;

        if (index < tokenCount)
        {
            if (get_next_integer_token(tokens[index++], &artifactVersion->majorVersion))
                artifactVersion->majorVersionPresent = 1;
            else
                fallback = 1;
        }
        else
        {
            fallback = 1;
        }

        if (index < tokenCount)
        {
            if (get_next_integer_token(tokens[index++], &artifactVersion->minorVersion))
                artifactVersion->minorVersionPresent = 1;
            else
                fallback = 1;
        }

        if (index < tokenCount)
        {
            if (get_next_integer_token(tokens[index++], &artifactVersion->incrementalVersion))
                artifactVersion->incrementalVersionPresent = 1;
            else
                fallback = 1;
        }

        if (index < tokenCount)
        {
            free(artifactVersion->qualifier);
            artifactVersion->qualifier = strdup(tokens[index]);
            fallback = is_digits(artifactVersion->qualifier);
        }

        // Java: string tokenizer won't detect these and ignores them
        part1Length = strlen(part1);
        if (strstr(part1, "..") != NULL || part1[0] == '.' ||
            (part1Length > 0 && part1[part1Length - 1] == '.'))
            fallback = 1;

        if (fallback)
        {
            // Java: qualifier is the whole version, including "-"
            free(artifactVersion->qualifier);
            artifactVersion->qualifier = strdup(version);
            artifactVersion->majorVersionPresent = 0;
            artifactVersion->minorVersionPresent = 0;
            artifactVersion->incrementalVersionPresent = 0;
            artifactVersion->buildNumberPresent = 0;
        }

        for (size_t i = 0; i < tokenCount; i++)
            free(tokens[i]);
        free(tokens);
    }

    free(part1);
}

// Java: public DefaultArtifactVersion(String version)
LIBMATTI_MAVEN_ArtifactVersion *LIBMATTI_MAVEN_ArtifactVersion_New(const char *version)
{
    LIBMATTI_MAVEN_ArtifactVersion *artifactVersion = calloc(1, sizeof(LIBMATTI_MAVEN_ArtifactVersion));
    parse_version(artifactVersion, version);
    return artifactVersion;
}

void LIBMATTI_MAVEN_ArtifactVersion_Free(LIBMATTI_MAVEN_ArtifactVersion *version)
{
    if (version == NULL) return;

    free(version->version);
    free(version->qualifier);
    LIBMATTI_MAVEN_ComparableVersion_Free(version->comparable);
    free(version);
}

// Java: public int compareTo(ArtifactVersion otherVersion) - otherVersion may be any
// ArtifactVersion implementation, which is re-parsed through its toString
int LIBMATTI_MAVEN_ArtifactVersion_Compare(const LIBMATTI_MAVEN_ArtifactVersion *version,
                                           const LIBMATTI_MAVEN_ArtifactVersion *otherVersion)
{
    return LIBMATTI_MAVEN_ComparableVersion_Compare(version->comparable, otherVersion->comparable);
}

// Java: public boolean equals(Object other)
int LIBMATTI_MAVEN_ArtifactVersion_Equals(const LIBMATTI_MAVEN_ArtifactVersion *version, const void *other)
{
    if (version == other) return 1;
    if (version == NULL || other == NULL) return 0;
    return LIBMATTI_MAVEN_ArtifactVersion_Compare(version, (const LIBMATTI_MAVEN_ArtifactVersion *) other) == 0;
}

// Java: public int hashCode() { return 11 + comparable.orderingHashCode(); }
int LIBMATTI_MAVEN_ArtifactVersion_HashCode(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return 11 + LIBMATTI_MAVEN_ComparableVersion_OrderingHashCode(version->comparable);
}

// Java: public String toString()
const char *LIBMATTI_MAVEN_ArtifactVersion_ToString(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->version;
}

// Java: public int getMajorVersion() { return majorVersion != null ? majorVersion : 0; }
int LIBMATTI_MAVEN_ArtifactVersion_GetMajorVersion(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->majorVersionPresent ? version->majorVersion : 0;
}

int LIBMATTI_MAVEN_ArtifactVersion_GetMinorVersion(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->minorVersionPresent ? version->minorVersion : 0;
}

int LIBMATTI_MAVEN_ArtifactVersion_GetIncrementalVersion(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->incrementalVersionPresent ? version->incrementalVersion : 0;
}

int LIBMATTI_MAVEN_ArtifactVersion_GetBuildNumber(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->buildNumberPresent ? version->buildNumber : 0;
}

const char *LIBMATTI_MAVEN_ArtifactVersion_GetQualifier(const LIBMATTI_MAVEN_ArtifactVersion *version)
{
    return version->qualifier;
}
