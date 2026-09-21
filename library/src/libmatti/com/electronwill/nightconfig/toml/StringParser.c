// Port of com.electronwill.nightconfig.toml.StringParser.

#include "libmatti/com/electronwill/nightconfig/toml/StringParser.h"

#include "libmatti/com/electronwill/nightconfig/core/io/Utils.h"
#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final char[] SINGLE_QUOTE_OR_NEWLINE = {'\'', '\n', '\r'}
static const char SINGLE_QUOTE_OR_NEWLINE[] = "'\n\r";

// Java: private static String parseUnicodeCodepoint(CharsWrapper chars)
static char *parseUnicodeCodepoint(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_CharsWrapper chars)
{
    int ok = 0;
    int codePoint = LIBMATTI_NC_Utils_ParseInt(&chars, 16, &ok);
    if (!ok || !LIBMATTI_NC_Toml_IsValidCodePoint(codePoint))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid unicode codepoint");
        return NULL;
    }

    char out[4];
    size_t length = LIBMATTI_NC_Utils_EncodeUtf8(codePoint, out);
    char *result = malloc(length + 1);
    memcpy(result, out, length);
    result[length] = '\0';
    return result;
}

// Java: private static String unescape(char c, CharacterInput input, TomlVersion version)
static char *unescape(char c, LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    switch (c)
    {
    case '"':
    case '\\':
    {
        char *result = malloc(2);
        result[0] = c;
        result[1] = '\0';
        return result;
    }
    case 'b':
        return strdup("\b");
    case 'f':
        return strdup("\f");
    case 'n':
        return strdup("\n");
    case 'r':
        return strdup("\r");
    case 't':
        return strdup("\t");
    case 'x':
    {
        if (LIBMATTI_NC_TomlParser_GetTomlVersion(parser) == LIBMATTI_NC_TOMLVERSION_V1_0)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid escapement: \\x. NOTE: \\xHH is not supported in TOML v1.0");
            return NULL;
        }
        LIBMATTI_NC_CharsWrapper chars = LIBMATTI_NC_ReaderInput_ReadChars(input, 2);
        return parseUnicodeCodepoint(parser, chars);
    }
    case 'u':
    {
        LIBMATTI_NC_CharsWrapper chars = LIBMATTI_NC_ReaderInput_ReadChars(input, 4);
        return parseUnicodeCodepoint(parser, chars);
    }
    case 'U':
    {
        LIBMATTI_NC_CharsWrapper chars = LIBMATTI_NC_ReaderInput_ReadChars(input, 8);
        return parseUnicodeCodepoint(parser, chars);
    }
    case 'e':
    {
        if (LIBMATTI_NC_TomlParser_GetTomlVersion(parser) == LIBMATTI_NC_TOMLVERSION_V1_0)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid escapement: \\e. NOTE: \\e is not supported in TOML v1.0");
            return NULL;
        }
        char *result = malloc(2);
        result[0] = (char) 0x1B;
        result[1] = '\0';
        return result;
    }
    default:
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid escapement");
        return NULL;
    }
}

// Java: private static boolean isWhitespace(CharSequence csq)
static int isWhitespace(const LIBMATTI_NC_CharsWrapper *csq)
{
    for (size_t i = 0; i < csq->length; i++)
        if (csq->data[i] != '\t' && csq->data[i] != ' ') return 0;
    return 1;
}

// Java: private static String buildMultilineString(CharsWrapper.Builder builder)
static char *buildMultilineString(LIBMATTI_NC_CharsWrapper_Builder *builder)
{
    if (builder->length > 0 && builder->data[0] == '\n')
        return LIBMATTI_NC_CharsWrapper_Builder_ToStringFrom(builder, 1);
    if (builder->length > 1 && builder->data[0] == '\r' && builder->data[1] == '\n')
        return LIBMATTI_NC_CharsWrapper_Builder_ToStringFrom(builder, 2);
    return LIBMATTI_NC_CharsWrapper_Builder_ToString(builder);
}

// Java: static String parseBasic(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseBasic(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_CharsWrapper_Builder *builder = LIBMATTI_NC_TomlParser_CreateBuilder(parser);
    int escape = 0;
    int c;
    while ((c = LIBMATTI_NC_ReaderInput_ReadChar(input)) != '"' || escape)
    {
        if (c == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid basic string, the closing quote is missing");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }

        if (escape)
        {
            char *unescaped = unescape((char) c, input, parser);
            if (unescaped == NULL)
            {
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return NULL;
            }
            LIBMATTI_NC_CharsWrapper_Builder_AppendString(builder, unescaped);
            free(unescaped);
            escape = 0;
        }
        else if (c == '\\')
        {
            escape = 1;
        }
        else if (c == '\n' || c == '\r')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid newline in basic string");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }
        else if (c != '\t' && LIBMATTI_NC_Toml_IsControlChar(c))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid control character in string");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }
        else
        {
            LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, (char) c);
        }
    }

    char *result = LIBMATTI_NC_CharsWrapper_Builder_ToString(builder);
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
    return result;
}

// Java: static String parseLiteral(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseLiteral(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_CharsWrapper str =
        LIBMATTI_NC_ReaderInput_ReadCharsUntil(input, SINGLE_QUOTE_OR_NEWLINE, sizeof(SINGLE_QUOTE_OR_NEWLINE) - 1);
    int end = LIBMATTI_NC_ReaderInput_ReadChar(input);
    if (end != '\'')
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid newline in literal string");
        return NULL;
    }

    for (size_t i = 0; i < str.length; i++)
    {
        if (str.data[i] != '\t' && LIBMATTI_NC_Toml_IsControlChar((unsigned char) str.data[i]))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid control character in literal string");
            return NULL;
        }
    }

    return LIBMATTI_NC_CharsWrapper_ToString(&str);
}

// Java: static String parseMultiBasic(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseMultiBasic(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_CharsWrapper_Builder *builder = LIBMATTI_NC_TomlParser_CreateBuilder(parser);
    int c;
    while ((c = LIBMATTI_NC_ReaderInput_ReadChar(input)) != '"' || LIBMATTI_NC_ReaderInput_Peek(input) != '"' ||
           LIBMATTI_NC_ReaderInput_PeekN(input, 1) != '"')
    {
        if (c == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid multiline basic string, the closing quotes are missing");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }

        if (c == '\\')
        {
            int next = LIBMATTI_NC_ReaderInput_ReadChar(input);
            int lineIsWhitespace = 0;
            if ((next == '\t' || next == ' ') && next != '\n')
            {
                LIBMATTI_NC_CharsWrapper line = LIBMATTI_NC_Toml_ReadLine(input);
                lineIsWhitespace = isWhitespace(&line);
            }
            if (next == '\n' || (next == '\r' && LIBMATTI_NC_ReaderInput_PeekChar(input) == '\n') ||
                ((next == '\t' || next == ' ') && lineIsWhitespace))
            {
                // Goes to the next non-space char (skips newlines too)
                int nextNonSpace = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 1);
                LIBMATTI_NC_ReaderInput_PushBack(input, (char) nextNonSpace);
                continue;
            }
            else if (next == '\t' || next == ' ')
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid escapement");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return NULL;
            }

            char *unescaped = unescape((char) next, input, parser);
            if (unescaped == NULL)
            {
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return NULL;
            }
            LIBMATTI_NC_CharsWrapper_Builder_AppendString(builder, unescaped);
            free(unescaped);
        }
        else if (c != '\n' && c != '\r' && c != '\t' && LIBMATTI_NC_Toml_IsControlChar(c))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid control character in multiline string");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }
        else
        {
            LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, (char) c);
        }
    }

    LIBMATTI_NC_ReaderInput_SkipPeeks(input); // Don't include the closing quotes in the String
    if (LIBMATTI_NC_ReaderInput_Peek(input) == '"')
    {
        LIBMATTI_NC_ReaderInput_SkipPeeks(input);
        LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, '"');
    }
    if (LIBMATTI_NC_ReaderInput_Peek(input) == '"')
    {
        LIBMATTI_NC_ReaderInput_SkipPeeks(input);
        LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, '"');
    }

    char *result = buildMultilineString(builder);
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
    return result;
}

// Java: static String parseMultiLiteral(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseMultiLiteral(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_CharsWrapper_Builder *builder = LIBMATTI_NC_TomlParser_CreateBuilder(parser);
    int c;
    while ((c = LIBMATTI_NC_ReaderInput_ReadChar(input)) != '\'' || LIBMATTI_NC_ReaderInput_Peek(input) != '\'' ||
           LIBMATTI_NC_ReaderInput_PeekN(input, 1) != '\'')
    {
        if (c == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid multiline literal string, the closing quotes are missing");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }

        if (c != '\n' && c != '\r' && c != '\t' && LIBMATTI_NC_Toml_IsControlChar(c))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid control character in multiline literal string");
            LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
            return NULL;
        }
        LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, (char) c);
    }

    LIBMATTI_NC_ReaderInput_SkipPeeks(input); // Don't include the closing quotes in the String
    if (LIBMATTI_NC_ReaderInput_Peek(input) == '\'')
    {
        LIBMATTI_NC_ReaderInput_SkipPeeks(input);
        LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, '\'');
    }
    if (LIBMATTI_NC_ReaderInput_Peek(input) == '\'')
    {
        LIBMATTI_NC_ReaderInput_SkipPeeks(input);
        LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, '\'');
    }

    char *result = buildMultilineString(builder);
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
    return result;
}
