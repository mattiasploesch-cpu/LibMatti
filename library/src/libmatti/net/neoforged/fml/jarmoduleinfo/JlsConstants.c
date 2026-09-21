#include "libmatti/net/neoforged/fml/jarmoduleinfo/JlsConstants.h"

#include <stdlib.h>
#include <string.h>

// Java: static final Set<String> RESERVED_KEYWORDS
const char *const LIBMATTI_FML_JlsConstants_RESERVED_KEYWORDS[] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue",
    "default", "do", "double", "else", "enum", "extends", "final", "finally", "float", "for", "goto", "if",
    "implements", "import", "instanceof", "int", "interface", "long", "native", "new", "package", "private",
    "protected", "public", "return", "short", "static", "strictfp", "super", "switch", "synchronized", "this",
    "throw", "throws", "transient", "try", "void", "volatile", "while",
    // Not really keywords, but "boolean literals"
    "true", "false",
    // Not really a keyword, but the "null literal"
    "null",
    "_",
};
const size_t LIBMATTI_FML_JlsConstants_RESERVED_KEYWORD_COUNT =
    sizeof(LIBMATTI_FML_JlsConstants_RESERVED_KEYWORDS) / sizeof(char *);

// Java: Character.isJavaIdentifierStart(codePoint)
// Java walks the full Unicode tables; the port covers ASCII plus everything
// non-ASCII byte (Java identifiers allow most letters >= 0x80).
static int isJavaIdentifierStart(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || (unsigned char)c >= 0x80;
}

// Java: Character.isJavaIdentifierPart(codePoint) - same simplification as above
static int isJavaIdentifierPart(char c)
{
    return isJavaIdentifierStart(c) || (c >= '0' && c <= '9');
}

// Java: public static boolean isJavaIdentifier(String str)
int LIBMATTI_FML_JlsConstants_IsJavaIdentifier(const char *str)
{
    if (str[0] == '\0')
        return 0;

    for (size_t i = 0; i < LIBMATTI_FML_JlsConstants_RESERVED_KEYWORD_COUNT; i++)
        if (strcmp(str, LIBMATTI_FML_JlsConstants_RESERVED_KEYWORDS[i]) == 0)
            return 0;

    // Java: this iterates over the Unicode code points instead of UTF-16 characters
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (i == 0 && !isJavaIdentifierStart(str[i]))
            return 0;
        if (!isJavaIdentifierPart(str[i]))
            return 0;
    }

    return 1;
}

// Java: public static String getPackageName(String typeName)
char *LIBMATTI_FML_JlsConstants_GetPackageName(const char *typeName)
{
    const char *lastSeparator = strrchr(typeName, '.');
    if (lastSeparator == NULL)
    {
        char *empty = malloc(1);
        empty[0] = '\0';
        return empty;
    }

    size_t length = (size_t)(lastSeparator - typeName);
    char *result = malloc(length + 1);
    memcpy(result, typeName, length);
    result[length] = '\0';
    return result;
}

// Java: public static boolean isTypeName(String name)
int LIBMATTI_FML_JlsConstants_IsTypeName(const char *name)
{
    size_t lastSeparator = 0;

    // Java: iterate all segments
    for (const char *nextSeparator = strchr(name, '.'); nextSeparator != NULL;
         nextSeparator = strchr(name + lastSeparator, '.'))
    {
        size_t segmentLength = (size_t)(nextSeparator - (name + lastSeparator));
        char *segment = malloc(segmentLength + 1);
        memcpy(segment, name + lastSeparator, segmentLength);
        segment[segmentLength] = '\0';

        int valid = LIBMATTI_FML_JlsConstants_IsJavaIdentifier(segment);
        free(segment);
        if (!valid)
            return 0;

        lastSeparator = (size_t)(nextSeparator - name) + 1;
    }

    return LIBMATTI_FML_JlsConstants_IsJavaIdentifier(name + lastSeparator);
}
