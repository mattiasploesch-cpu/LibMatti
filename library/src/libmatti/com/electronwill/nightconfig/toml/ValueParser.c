// Port of com.electronwill.nightconfig.toml.ValueParser.

#include "libmatti/com/electronwill/nightconfig/toml/ValueParser.h"

#include "libmatti/com/electronwill/nightconfig/core/io/Utils.h"
#include "libmatti/com/electronwill/nightconfig/toml/ArrayParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/StringParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/TableParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/TemporalParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final char[] END_OF_VALUE = {'\t',' ','#','\n','\r',',',']','}'}
static const char END_OF_VALUE[] = "\t #\n\r,]}";
// Java: private static final char[] END_OF_VALUE_DATE = {'\t','#','\n','\r',',',']','}'}
static const char END_OF_VALUE_DATE[] = "\t#\n\r,]}";
// Java: private static final char[] TRUE_END = {'r','u','e'}, FALSE_END = {'a','l','s','e'}
static const char TRUE_END[] = "rue";
static const char FALSE_END[] = "alse";
// Java: private static final char[] ONLY_IN_FP_NUMBER = {'.','e','E'}
static const char ONLY_IN_FP_NUMBER[] = ".eE";
// Java: private static final char[] FP_INFINITY = {'i','n','f'}
// (renamed: math.h defines the FP_INFINITY and FP_NAN macros)
static const char FP_INFINITY_CHARS[] = "inf";
// Java: private static final char[] FP_NAN = {'n','a','n'}
static const char FP_NAN_CHARS[] = "nan";

static int isDigitChar(char c, int base)
{
    switch (base)
    {
    case 2:
        return c == '0' || c == '1';
    case 8:
        return c >= '0' && c <= '7';
    case 10:
        return c >= '0' && c <= '9';
    default:
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
}

// Java: private static CharsWrapper simplifyNumber(CharsWrapper numberChars, int base)
// Java returns a view over the builder's array; the port copies it into a new buffer.
static int simplifyNumber(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_CharsWrapper *numberChars, int base,
                          char **out, size_t *outLength)
{
    LIBMATTI_NC_CharsWrapper_Builder *builder = LIBMATTI_NC_CharsWrapper_Builder_New(16);
    int previousChar = -1;

    for (size_t i = 0; i < numberChars->length; i++)
    {
        char c = numberChars->data[i];
        switch (c)
        {
        case '_':
        {
            if (previousChar == -1)
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading underscore in number");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return 0;
            }
            else if (!isDigitChar((char) previousChar, base))
            {
                LIBMATTI_NC_TomlParser_Error(
                    parser, "Invalid underscore in number (each underscore must be surrounded by digits)");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return 0;
            }
            break;
        }
        case '.':
        {
            if (previousChar == -1)
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading decimal point in number");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return 0;
            }
            else if (!isDigitChar((char) previousChar, base))
            {
                LIBMATTI_NC_TomlParser_Error(
                    parser, "Invalid decimal point in number (each decimal point must be surrounded by digits)");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return 0;
            }
            LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, c);
            break;
        }
        case 'e':
        {
            if (base != 16 && !isDigitChar((char) previousChar, base))
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid sequence 'e' in number");
                LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
                return 0;
            }
        }
        default:
        {
            LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, c);
            break;
        }
        }
        previousChar = c;
    }

    if (previousChar == '_')
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid trailing underscore in number");
        LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
        return 0;
    }
    else if (previousChar == '.')
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid trailing decimal point in number");
        LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
        return 0;
    }
    else if (previousChar == 'e' || previousChar == 'E' || previousChar == '-' || previousChar == '+')
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid trailing character in number");
        LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
        return 0;
    }

    *out = LIBMATTI_NC_CharsWrapper_Builder_ToString(builder);
    *outLength = builder->length;
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
    return 1;
}

// Java: private static Number parseNumber(CharsWrapper valueChars)
static LIBMATTI_NC_ValueResult parseNumber(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_CharsWrapper *valueChars)
{
    // Parse +-inf and +-nan
    char first = valueChars->data[0];
    LIBMATTI_NC_CharsWrapper remaining;
    if (first == '-')
    {
        remaining = LIBMATTI_NC_CharsWrapper_SubView(valueChars, 1);
        if (LIBMATTI_NC_CharsWrapper_ContentEquals(&remaining, FP_INFINITY_CHARS, 3))
            return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfDouble(-INFINITY));
    }
    else if (first == '+')
    {
        remaining = LIBMATTI_NC_CharsWrapper_SubView(valueChars, 1);
    }
    else
    {
        remaining = *valueChars;
    }

    if (LIBMATTI_NC_CharsWrapper_ContentEquals(&remaining, FP_INFINITY_CHARS, 3))
        return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfDouble(INFINITY));
    if (LIBMATTI_NC_CharsWrapper_ContentEquals(&remaining, FP_NAN_CHARS, 3))
        return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfDouble(NAN));

    // Detect the base of integers
    LIBMATTI_NC_CharsWrapper numberChars = *valueChars;
    int base = 10;
    if (valueChars->length > 2)
    {
        char prefix[3] = {valueChars->data[0], valueChars->data[1], '\0'};
        if (strcmp(prefix, "0x") == 0)
            base = 16;
        else if (strcmp(prefix, "0b") == 0)
            base = 2;
        else if (strcmp(prefix, "0o") == 0)
            base = 8;

        if (base != 10) numberChars = LIBMATTI_NC_CharsWrapper_SubView(valueChars, 2);
    }

    char *simplified = NULL;
    size_t simplifiedLength = 0;
    if (!simplifyNumber(parser, &numberChars, base, &simplified, &simplifiedLength))
        return LIBMATTI_NC_ValueResult_Fail();

    LIBMATTI_NC_CharsWrapper simple = LIBMATTI_NC_CharsWrapper_Of(simplified, simplifiedLength);
    LIBMATTI_NC_ValueResult result = LIBMATTI_NC_ValueResult_Fail();
    int ok = 0;

    // Parse floating-point numbers with additional TOML restrictions.
    if (base == 10 && LIBMATTI_NC_CharsWrapper_IndexOfFirst(&simple, ONLY_IN_FP_NUMBER, 3) != -1)
    {
        if (simplifiedLength > 1)
        {
            if (simple.data[0] == '0' && isDigitChar(simple.data[1], 10))
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading zero in floating-point number");
                goto done;
            }
            else if (simplifiedLength > 2 &&
                     (LIBMATTI_NC_CharsWrapper_StartsWith(&simple, "-0") ||
                      LIBMATTI_NC_CharsWrapper_StartsWith(&simple, "+0")) &&
                     isDigitChar(simple.data[2], 10))
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading zero (after sign) in floating-point number");
                goto done;
            }
        }

        double doubleValue = LIBMATTI_NC_Utils_ParseDouble(&simple, &ok);
        if (!ok)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid floating-point value");
            goto done;
        }
        result = LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfDouble(doubleValue));
        goto done;
    }

    // Parse integers with additional TOML restrictions.
    if (base == 10)
    {
        if (simplifiedLength > 1 && simple.data[0] == '0')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading zero in base 10 integer");
            goto done;
        }
        else if (simplifiedLength > 2 &&
                 (LIBMATTI_NC_CharsWrapper_StartsWith(&simple, "-0") ||
                  LIBMATTI_NC_CharsWrapper_StartsWith(&simple, "+0")) &&
                 isDigitChar(simple.data[2], 10))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid leading zero (after sign) in base 10 integer");
            goto done;
        }
    }

    long long longValue = LIBMATTI_NC_Utils_ParseLong(&simple, base, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid integer value");
        goto done;
    }
    // forbid 0x-123
    if (longValue < 0 && base != 10)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Cannot use a negative integer with a base prefix");
        goto done;
    }

    int intValue = (int) longValue;
    result = LIBMATTI_NC_ValueResult_Ok(intValue == longValue
                                            ? LIBMATTI_NC_Value_OfInt(intValue)
                                            : LIBMATTI_NC_Value_OfLong(longValue));

done:
    free(simplified);
    return result;
}

// Java: private static Boolean parseFalse(CharacterInput input)
static LIBMATTI_NC_ValueResult parseFalse(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_ReaderInput *input)
{
    LIBMATTI_NC_CharsWrapper remaining = LIBMATTI_NC_ReaderInput_ReadUntil(input, END_OF_VALUE, sizeof(END_OF_VALUE) - 1);
    if (!LIBMATTI_NC_CharsWrapper_ContentEquals(&remaining, FALSE_END, sizeof(FALSE_END) - 1))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid value - Expected the boolean value false.");
        return LIBMATTI_NC_ValueResult_Fail();
    }
    return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfBoolean(0));
}

// Java: private static Boolean parseTrue(CharacterInput input)
static LIBMATTI_NC_ValueResult parseTrue(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_ReaderInput *input)
{
    LIBMATTI_NC_CharsWrapper remaining = LIBMATTI_NC_ReaderInput_ReadUntil(input, END_OF_VALUE, sizeof(END_OF_VALUE) - 1);
    if (!LIBMATTI_NC_CharsWrapper_ContentEquals(&remaining, TRUE_END, sizeof(TRUE_END) - 1))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid value - Expected the boolean value true.");
        return LIBMATTI_NC_ValueResult_Fail();
    }
    return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfBoolean(1));
}

// Java: private static boolean shouldBeTemporal(CharsWrapper valueChars)
static int shouldBeTemporal(const LIBMATTI_NC_CharsWrapper *valueChars)
{
    return valueChars->length >= 5 &&
           (valueChars->data[2] == ':' || (valueChars->data[4] == '-' && valueChars->data[7] == '-'));
}

// Java: static Object parse(CharacterInput input, char firstChar, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueParser_Parse(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                                      LIBMATTI_NC_TomlParser *parser,
                                                      LIBMATTI_NC_Config *parentConfig)
{
    switch (firstChar)
    {
    case '{':
    {
        LIBMATTI_NC_Config *subConfig = LIBMATTI_NC_Config_CreateSubConfig(parentConfig);
        if (!LIBMATTI_NC_TableParser_ParseInline(input, parser, subConfig))
        {
            LIBMATTI_NC_Config_Free(subConfig);
            return LIBMATTI_NC_ValueResult_Fail();
        }
        return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfConfig(subConfig));
    }
    case '[':
        return LIBMATTI_NC_ArrayParser_Parse(input, parser, parentConfig);
    case '\'':
    {
        char *string;
        if (LIBMATTI_NC_ReaderInput_Peek(input) == '\'' && LIBMATTI_NC_ReaderInput_PeekN(input, 1) == '\'')
        {
            LIBMATTI_NC_ReaderInput_SkipPeeks(input); // Don't include the opening quotes in the String
            string = LIBMATTI_NC_StringParser_ParseMultiLiteral(input, parser);
        }
        else
        {
            string = LIBMATTI_NC_StringParser_ParseLiteral(input, parser);
        }
        if (string == NULL) return LIBMATTI_NC_ValueResult_Fail();
        return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OwnString(string));
    }
    case '"':
    {
        char *string;
        if (LIBMATTI_NC_ReaderInput_Peek(input) == '"' && LIBMATTI_NC_ReaderInput_PeekN(input, 1) == '"')
        {
            LIBMATTI_NC_ReaderInput_SkipPeeks(input); // Don't include the opening quotes in the String
            string = LIBMATTI_NC_StringParser_ParseMultiBasic(input, parser);
        }
        else
        {
            string = LIBMATTI_NC_StringParser_ParseBasic(input, parser);
        }
        if (string == NULL) return LIBMATTI_NC_ValueResult_Fail();
        return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OwnString(string));
    }
    case 't':
        return parseTrue(parser, input);
    case 'f':
        return parseFalse(parser, input);
    case '+':
    case '-':
    {
        LIBMATTI_NC_ReaderInput_PushBack(input, firstChar);
        LIBMATTI_NC_CharsWrapper numberChars =
            LIBMATTI_NC_ReaderInput_ReadUntil(input, END_OF_VALUE, sizeof(END_OF_VALUE) - 1);
        return parseNumber(parser, &numberChars);
    }
    default:
    {
        LIBMATTI_NC_ReaderInput_PushBack(input, firstChar);
        LIBMATTI_NC_CharsWrapper valueChars =
            LIBMATTI_NC_ReaderInput_ReadUntil(input, END_OF_VALUE_DATE, sizeof(END_OF_VALUE_DATE) - 1);
        if (shouldBeTemporal(&valueChars))
        {
            // Java: return TemporalParser.parse(valueChars, parser.tomlVersion)
            LIBMATTI_JT_Temporal temporal;
            if (!LIBMATTI_NC_TemporalParser_Parse(&valueChars, LIBMATTI_NC_TomlParser_GetTomlVersion(parser), parser,
                                                  &temporal))
                return LIBMATTI_NC_ValueResult_Fail();
            return LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value_OfTemporal(temporal));
        }
        LIBMATTI_NC_CharsWrapper trimmed = LIBMATTI_NC_CharsWrapper_TrimmedView(&valueChars);
        if (LIBMATTI_NC_CharsWrapper_IsEmpty(&trimmed))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid value containing only whitespaces");
            return LIBMATTI_NC_ValueResult_Fail();
        }
        return parseNumber(parser, &trimmed);
    }
    }
}

// Java: static Object parse(CharacterInput input, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueParser_ParseValue(LIBMATTI_NC_ReaderInput *input,
                                                           LIBMATTI_NC_TomlParser *parser,
                                                           LIBMATTI_NC_Config *parentConfig)
{
    return LIBMATTI_NC_ValueParser_Parse(input, (char) LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0), parser, parentConfig);
}
