// Port of javax.lang.model.SourceVersion.

#include "libmatti/javax/lang/model/SourceVersion.h"

#include <string.h>

// Java: private static final Set<String> keywords - the JLS keywords (the literals and "_" are
// handled by isKeyword itself, exactly as in the JDK).
static const char *const KEYWORDS[] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", "const",
    "continue", "default", "do", "double", "else", "enum", "extends", "final", "finally", "float",
    "for", "goto", "if", "implements", "import", "instanceof", "int", "interface", "long", "native",
    "new", "package", "private", "protected", "public", "return", "short", "static", "strictfp",
    "super", "switch", "synchronized", "this", "throw", "throws", "transient", "try", "void",
    "volatile", "while",
};
#define KEYWORD_COUNT (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

// Java: public static boolean isKeyword(CharSequence s)
int LIBMATTI_SV_SourceVersion_IsKeyword(const char *name)
{
    if (name == NULL) return 0;

    // Java: "true", "false", "null" and "_" are keywords too
    if (strcmp(name, "true") == 0 || strcmp(name, "false") == 0 || strcmp(name, "null") == 0 ||
        strcmp(name, "_") == 0)
        return 1;

    for (size_t i = 0; i < KEYWORD_COUNT; i++)
        if (strcmp(name, KEYWORDS[i]) == 0)
            return 1;

    return 0;
}

// Java: Character.isJavaIdentifierStart(int codePoint)
static int isJavaIdentifierStart(int codePoint)
{
    // Java: Character.isLetter(codePoint) || codePoint == '$' || codePoint == '_'
    return (codePoint >= 'a' && codePoint <= 'z') || (codePoint >= 'A' && codePoint <= 'Z') ||
           codePoint == '$' || codePoint == '_';
}

// Java: Character.isJavaIdentifierPart(int codePoint)
static int isJavaIdentifierPart(int codePoint)
{
    return isJavaIdentifierStart(codePoint) || (codePoint >= '0' && codePoint <= '9');
}

// Java: public static boolean isIdentifier(CharSequence name)
int LIBMATTI_SV_SourceVersion_IsIdentifier(const char *name)
{
    if (name == NULL || name[0] == '\0') return 0;

    if (!isJavaIdentifierStart((unsigned char) name[0])) return 0;

    for (const char *c = name + 1; *c != '\0'; c++)
        if (!isJavaIdentifierPart((unsigned char) *c))
            return 0;

    return 1;
}

// Java: public static boolean isName(CharSequence name) { return isIdentifier(name) && !isKeyword(name); }
int LIBMATTI_SV_SourceVersion_IsName(const char *name)
{
    if (!LIBMATTI_SV_SourceVersion_IsIdentifier(name)) return 0;
    return !LIBMATTI_SV_SourceVersion_IsKeyword(name);
}
