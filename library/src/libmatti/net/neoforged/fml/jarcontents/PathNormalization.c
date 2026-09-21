#include "libmatti/net/neoforged/fml/jarcontents/PathNormalization.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char SEPARATOR = '/';

// Java: public static void assertNormalized(CharSequence path)
int LIBMATTI_FML_PathNormalization_AssertNormalized(const char *path)
{
    // Java: throw new IllegalArgumentException("Path is not a valid relative path: " + path)
    return LIBMATTI_FML_PathNormalization_IsNormalized(path);
}

// Java: public static boolean isNormalized(CharSequence path)
int LIBMATTI_FML_PathNormalization_IsNormalized(const char *path)
{
    size_t length = strlen(path);

    // Java: if (path.isEmpty()) return true; // This will fail for other reasons
    if (length == 0)
        return 1;

    // Java: normalized paths use forward slashes
    for (size_t i = 0; i < length; i++)
        if (path[i] == '\\')
            return 0;

    char prevCh = '\0';
    size_t segmentStart = 0;
    for (size_t i = 0; i < length; i++)
    {
        int atEnd = i == length - 1;
        char ch = path[i];

        // Java: no leading or trailing separators
        if ((i == 0 || atEnd) && ch == SEPARATOR)
            return 0;
        // Java: no repeated separators
        if (ch == SEPARATOR && prevCh == SEPARATOR)
            return 0;

        // Java: validate path segments either when we encounter separators or at the last character
        if (ch == SEPARATOR || atEnd)
        {
            size_t segmentEnd = ch == SEPARATOR ? i - 1 : i;
            size_t segmentLength = (segmentEnd - segmentStart) + 1;
            if (segmentLength == 1 && path[segmentStart] == '.')
                return 0; // No '.' segments
            if (segmentLength == 2 && path[segmentStart] == '.' && path[segmentEnd] == '.')
                return 0; // No '..' segments

            segmentStart = i + 1;
        }
        prevCh = ch;
    }

    return 1;
}

// Java: private static void validateSegment(CharSequence segment)
static void validateSegment(const char *segment, size_t length)
{
    // Java: throw new IllegalArgumentException("./ or ../ segments in paths are not supported")
    if ((length == 1 && segment[0] == '.') || (length == 2 && segment[0] == '.' && segment[1] == '.'))
        fprintf(stderr, "./ or ../ segments in paths are not supported\n");
}

// Java: private static String normalize(CharSequence path, boolean folderPrefix)
static char *normalize(const char *path, int folderPrefix)
{
    size_t length = strlen(path);
    char *result = calloc(1, length + 2); // +2 for a possible trailing separator and the NUL
    size_t resultLength = 0;

    size_t startOfSegment = 0;
    for (size_t i = 0; i < length; i++)
    {
        char ch = path[i];
        if (ch == '\\')
            ch = SEPARATOR;

        if (ch == SEPARATOR)
        {
            if (i > startOfSegment)
            {
                if (resultLength > 0)
                    result[resultLength++] = SEPARATOR;

                size_t segmentLength = i - startOfSegment;
                validateSegment(path + startOfSegment, segmentLength);
                memcpy(result + resultLength, path + startOfSegment, segmentLength);
                resultLength += segmentLength;
            }
            startOfSegment = i + 1;
        }
    }

    if (startOfSegment < length)
    {
        if (resultLength > 0)
            result[resultLength++] = SEPARATOR;

        size_t segmentLength = length - startOfSegment;
        validateSegment(path + startOfSegment, segmentLength);
        memcpy(result + resultLength, path + startOfSegment, segmentLength);
        resultLength += segmentLength;
    }

    if (folderPrefix && resultLength > 0)
        result[resultLength++] = SEPARATOR;

    result[resultLength] = '\0';
    return result;
}

// Java: public static String normalize(CharSequence path)
char *LIBMATTI_FML_PathNormalization_Normalize(const char *path)
{
    return normalize(path, 0);
}

// Java: public static String normalizeFolderPrefix(CharSequence path)
char *LIBMATTI_FML_PathNormalization_NormalizeFolderPrefix(const char *path)
{
    return normalize(path, 1);
}
