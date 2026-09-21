// Port of net.minecraft.nbt.TagParser + SnbtGrammar as a recursive-descent parser
// over the same grammar, plus the SnbtOperations builtins ("true"/"false" collapse
// to ByteTag like NbtOps.createBoolean).

#include "libmatti/net/minecraft/nbt/TagParser.h"

#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/nbt/NbtUtils.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEPTH 512

static char lastErrorBuffer[256];
static int hasLastError = 0;

static void store_error(const char *message)
{
    snprintf(lastErrorBuffer, sizeof(lastErrorBuffer), "%s", message);
    hasLastError = 1;
}

const char *LIBMATTI_MC_Nbt_TagParser_LastError(void)
{
    return hasLastError ? lastErrorBuffer : NULL;
}

typedef struct
{
    const char *input;
    size_t position;
    int depth;
    char error[256];
    int hasError;
    // Java: Term.cut() - a cut error is final (the float fallback may not retry)
    int cut;
} Parser;

static void set_error(Parser *parser, const char *message)
{
    if (!parser->hasError)
    {
        parser->hasError = 1;
        snprintf(parser->error, sizeof(parser->error), "%s", message);
    }
}

// Java: Term.fail after Term.cut - the error ends the whole alternative
static void set_error_cut(Parser *parser, const char *message)
{
    set_error(parser, message);
    parser->cut = 1;
}

static void set_error_at(Parser *parser, const char *message)
{
    if (parser->hasError)
        return;
    char detail[200];
    snprintf(detail, sizeof(detail), "%s (at %zu)", message, parser->position);
    set_error(parser, detail);
}

static int at_end(Parser *parser)
{
    return parser->input[parser->position] == '\0';
}

static char peek(Parser *parser)
{
    return parser->input[parser->position];
}

static char peek_at(Parser *parser, size_t offset)
{
    return parser->input[parser->position + offset];
}

// Java: StringReader.skipWhitespace()
static void skip_whitespace(Parser *parser)
{
    while (!at_end(parser))
    {
        char c = peek(parser);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            parser->position++;
        else
            break;
    }
}

static int read_char(Parser *parser, char expected)
{
    if (!at_end(parser) && peek(parser) == expected)
    {
        parser->position++;
        return 1;
    }
    return 0;
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int string_equals_ignore_case(const char *a, const char *b)
{
    for (; *a != '\0' && *b != '\0'; a++, b++)
        if (tolower((unsigned char) *a) != tolower((unsigned char) *b))
            return 0;
    return *a == '\0' && *b == '\0';
}

// ---------------------------------------------------------------------------
// numbers - Java: NumberRunParseRule with the underscore rule (digits may carry
// underscores, but not at start/end or doubled)
// ---------------------------------------------------------------------------

static char *read_digits(Parser *parser, const char *accepted)
{
    size_t start = parser->position;
    while (!at_end(parser) && strchr(accepted, peek(parser)) != NULL)
        parser->position++;
    size_t length = parser->position - start;
    char *digits = malloc(length + 1);
    memcpy(digits, parser->input + start, length);
    digits[length] = '\0';
    if (strchr(digits, '_') != NULL)
    {
        size_t digitLength = strlen(digits);
        if (digits[0] == '_' || digits[digitLength - 1] == '_')
        {
            set_error_cut(parser, "underscore not allowed here");
            free(digits);
            return NULL;
        }
        for (size_t i = 1; i < digitLength; i++)
        {
            if (digits[i] == '_' && digits[i - 1] == '_')
            {
                set_error_cut(parser, "underscore not allowed here");
                free(digits);
                return NULL;
            }
        }
        // Java: cleanAndAppend strips the underscores
        size_t out = 0;
        for (size_t i = 0; i < digitLength; i++)
            if (digits[i] != '_')
                digits[out++] = digits[i];
        digits[out] = '\0';
    }
    return digits;
}

// Java: the IntegerLiteral record - the scan keeps sign/base/digits/suffix; the tag is
// created later against a target type (the plain literal defaults to INT, the array
// elements default to the array prefix's type)
typedef struct
{
    int negative;
    int unsignedPrefix;
    int base;
    char *digits;
    // 0 = no suffix, else the TAG_* constant
    int suffixType;
} IntegerLiteral;

static void literal_free(IntegerLiteral *literal)
{
    free(literal->digits);
    literal->digits = NULL;
}

// Java: the integer_literal scan - sign, 0x/0b/decimal (0-leading forbidden), the
// suffixes (b/B, s/S, i/I, l/L and the signed/unsigned pairs u*/s*)
static int scan_integer(Parser *parser, IntegerLiteral *literal)
{
    literal->negative = 0;
    literal->unsignedPrefix = 0;
    literal->base = 10;
    literal->digits = NULL;
    literal->suffixType = 0;

    if (read_char(parser, '-'))
        literal->negative = 1;
    else
        read_char(parser, '+');

    char *digits = NULL;
    if (read_char(parser, '0'))
    {
        if (read_char(parser, 'x') || read_char(parser, 'X'))
        {
            literal->base = 16;
            digits = read_digits(parser, "0123456789abcdefABCDEF_");
        }
        else if (read_char(parser, 'b') || read_char(parser, 'B'))
        {
            literal->base = 2;
            digits = read_digits(parser, "01_");
        }
        else
        {
            digits = read_digits(parser, "0123456789_");
            if (digits == NULL)
                return 0;
            // Java: a decimal run after '0' hits the cut and fails with the
            // leading-zero error; a bare "0" is the "0" marker
            if (digits[0] != '\0')
            {
                set_error_cut(parser, "leading zero not allowed");
                free(digits);
                return 0;
            }
            free(digits);
            digits = strdup("0");
        }
    }
    else
    {
        digits = read_digits(parser, "0123456789_");
    }

    if (digits == NULL)
        return 0;
    if (digits[0] == '\0')
    {
        set_error_at(parser, "expected integer value");
        free(digits);
        return 0;
    }

    // Java: the integer_suffix rule - a bare suffix or the signed/unsigned pairs
    int type = 0;
    if (!at_end(parser))
    {
        char c = peek(parser);
        if (c == 'u' || c == 'U')
        {
            parser->position++;
            literal->unsignedPrefix = 1;
            char t = peek(parser);
            if (t == 'b' || t == 'B')
            {
                type = LIBMATTI_MC_Nbt_TAG_BYTE;
                parser->position++;
            }
            else if (t == 's' || t == 'S')
            {
                type = LIBMATTI_MC_Nbt_TAG_SHORT;
                parser->position++;
            }
            else if (t == 'i' || t == 'I')
            {
                type = LIBMATTI_MC_Nbt_TAG_INT;
                parser->position++;
            }
            else if (t == 'l' || t == 'L')
            {
                type = LIBMATTI_MC_Nbt_TAG_LONG;
                parser->position++;
            }
        }
        else if (c == 's' || c == 'S')
        {
            // 's' starts the signed pair (sb/ss/si/sl) or is the short suffix alone
            char next = peek_at(parser, 1);
            if (next == 'b' || next == 'B' || next == 's' || next == 'S' || next == 'i' || next == 'I' ||
                next == 'l' || next == 'L')
            {
                parser->position++;
                c = peek(parser);
            }
            else
            {
                type = LIBMATTI_MC_Nbt_TAG_SHORT;
                parser->position++;
            }
        }

        if (type == 0 && (c == 'b' || c == 'B'))
        {
            type = LIBMATTI_MC_Nbt_TAG_BYTE;
            parser->position++;
        }
        else if (type == 0 && (c == 's' || c == 'S'))
        {
            type = LIBMATTI_MC_Nbt_TAG_SHORT;
            parser->position++;
        }
        else if (type == 0 && (c == 'i' || c == 'I'))
        {
            type = LIBMATTI_MC_Nbt_TAG_INT;
            parser->position++;
        }
        else if (type == 0 && (c == 'l' || c == 'L'))
        {
            type = LIBMATTI_MC_Nbt_TAG_LONG;
            parser->position++;
        }
    }
    literal->suffixType = type;
    literal->digits = digits;
    return 1;
}

// Java: IntegerLiteral.create(ops, TypeSuffix, parseState) - the range-checked parse
// per target type
static LIBMATTI_MC_Nbt_Tag *literal_create(Parser *parser, const IntegerLiteral *literal, int type)
{
    if (literal->unsignedPrefix && literal->negative)
    {
        set_error_at(parser, "expected non-negative number");
        return NULL;
    }

    long long value = 0;
    {
        char *end = NULL;
        errno = 0;
        if (literal->unsignedPrefix)
        {
            unsigned long long parsed = strtoull(literal->digits, &end, literal->base);
            if (errno != 0 || (end != NULL && *end != '\0'))
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            value = (long long) parsed;
        }
        else
        {
            value = strtoll(literal->digits, &end, literal->base);
            if (errno != 0 || (end != NULL && *end != '\0'))
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            if (literal->negative)
                value = -value;
        }
    }

    // Java: the range checks of parseByte/parseShort/parseInt/parseUnsigned*
    switch (type)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        if (literal->unsignedPrefix)
        {
            if (value < 0 || value > 255)
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            return LIBMATTI_MC_Nbt_ByteTag_Of((int8_t) (uint8_t) value);
        }
        if (value < CHAR_MIN || value > CHAR_MAX)
        {
            set_error_at(parser, "number parse failure");
            return NULL;
        }
        return LIBMATTI_MC_Nbt_ByteTag_Of((int8_t) value);
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        if (literal->unsignedPrefix)
        {
            if (value < 0 || value > 65535)
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            return LIBMATTI_MC_Nbt_ShortTag_Of((int16_t) (uint16_t) value);
        }
        if (value < SHRT_MIN || value > SHRT_MAX)
        {
            set_error_at(parser, "number parse failure");
            return NULL;
        }
        return LIBMATTI_MC_Nbt_ShortTag_Of((int16_t) value);
    case LIBMATTI_MC_Nbt_TAG_LONG:
        if (literal->unsignedPrefix)
        {
            if (value < 0)
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            return LIBMATTI_MC_Nbt_LongTag_Of(value);
        }
        return LIBMATTI_MC_Nbt_LongTag_Of(value);
    default:
        if (literal->unsignedPrefix)
        {
            if (value < 0 || value > 4294967295LL)
            {
                set_error_at(parser, "number parse failure");
                return NULL;
            }
            return LIBMATTI_MC_Nbt_IntTag_Of((int32_t) (uint32_t) value);
        }
        if (value < INT_MIN || value > INT_MAX)
        {
            set_error_at(parser, "number parse failure");
            return NULL;
        }
        return LIBMATTI_MC_Nbt_IntTag_Of((int32_t) value);
    }
}

// Java: the plain integer literal - the suffix type or INT (Objects.requireNonNullElse)
static LIBMATTI_MC_Nbt_Tag *parse_integer_tag(Parser *parser)
{
    IntegerLiteral literal;
    if (!scan_integer(parser, &literal))
        return NULL;
    int type = literal.suffixType != 0 ? literal.suffixType : LIBMATTI_MC_Nbt_TAG_INT;
    LIBMATTI_MC_Nbt_Tag *tag = literal_create(parser, &literal, type);
    literal_free(&literal);
    return tag;
}

// Java: the float_literal rule - sign, whole.fraction / .fraction / whole exp /
// whole suffix, f/F -> FloatTag, d/D or nothing -> DoubleTag
static LIBMATTI_MC_Nbt_Tag *parse_float_tag(Parser *parser)
{
    int negative = 0;
    if (read_char(parser, '-'))
        negative = 1;
    else
        read_char(parser, '+');

    char *whole = NULL;
    char *fraction = NULL;
    char *exponent = NULL;
    int hasDot = 0;
    int hasExp = 0;

    if (is_digit(peek(parser)) || peek(parser) == '_')
    {
        whole = read_digits(parser, "0123456789_");
        if (whole == NULL)
            return NULL;
        if (read_char(parser, '.'))
        {
            hasDot = 1;
            fraction = read_digits(parser, "0123456789_");
            if (fraction == NULL)
            {
                free(whole);
                return NULL;
            }
        }
    }
    else if (read_char(parser, '.'))
    {
        hasDot = 1;
        fraction = read_digits(parser, "0123456789_");
        if (fraction == NULL)
            return NULL;
    }

    if (!hasDot && (peek(parser) == 'e' || peek(parser) == 'E'))
    {
        parser->position++;
        hasExp = 1;
        if (read_char(parser, '-'))
            ;
        else
            read_char(parser, '+');
        exponent = read_digits(parser, "0123456789_");
        if (exponent == NULL)
        {
            free(whole);
            return NULL;
        }
    }

    int isFloat = 0;
    if (!at_end(parser) && (peek(parser) == 'f' || peek(parser) == 'F'))
    {
        isFloat = 1;
        parser->position++;
    }
    else if (!at_end(parser) && (peek(parser) == 'd' || peek(parser) == 'D'))
    {
        parser->position++;
    }

    // Java: createFloat assembles sign/whole/'.'/fraction/exp into one string
    char number[160];
    size_t offset = 0;
    if (negative)
        number[offset++] = '-';
    if (whole != NULL && whole[0] != '\0')
        offset += (size_t) snprintf(number + offset, sizeof(number) - offset, "%s", whole);
    if (hasDot)
        number[offset++] = '.';
    if (fraction != NULL && fraction[0] != '\0')
        offset += (size_t) snprintf(number + offset, sizeof(number) - offset, "%s", fraction);
    if (hasExp)
    {
        number[offset++] = 'e';
        offset += (size_t) snprintf(number + offset, sizeof(number) - offset, "%s",
                                    exponent != NULL ? exponent : "0");
    }
    number[offset] = '\0';

    free(whole);
    free(fraction);
    free(exponent);

    char *end = NULL;
    errno = 0;
    double value = strtod(number, &end);
    if (end == number || (end != NULL && *end != '\0'))
    {
        set_error_at(parser, "number parse failure");
        return NULL;
    }

    // Java: convertFloat/convertDouble - the finite checks
    if (isFloat)
    {
        float f = (float) value;
        if (f >= FLT_MAX || f <= -FLT_MAX)
        {
            set_error_at(parser, "infinity not allowed");
            return NULL;
        }
        return LIBMATTI_MC_Nbt_FloatTag_Of(f);
    }
    if (value >= DBL_MAX || value <= -DBL_MAX)
    {
        set_error_at(parser, "infinity not allowed");
        return NULL;
    }
    return LIBMATTI_MC_Nbt_DoubleTag_Of(value);
}

// ---------------------------------------------------------------------------
// strings
// ---------------------------------------------------------------------------

// Java: the quoted_string_literal rule with the string_escape_sequence table
// (b/s/t/n/f/r/\\/'/"/xHH/uuuu/UUUUUUUU; \N{..} needs the Unicode name table and
// stays unsupported in the port)
static LIBMATTI_MC_Nbt_Tag *parse_quoted_string(Parser *parser)
{
    char quote = peek(parser);
    parser->position++;

    size_t capacity = 16;
    size_t length = 0;
    char *buffer = malloc(capacity);
    buffer[0] = '\0';

#define PUSH(text, textLength)                                              \
    do                                                                      \
    {                                                                       \
        if (length + (textLength) + 1 > capacity)                           \
        {                                                                   \
            while (length + (textLength) + 1 > capacity)                    \
                capacity *= 2;                                              \
            buffer = realloc(buffer, capacity);                             \
        }                                                                   \
        memcpy(buffer + length, (text), (textLength));                      \
        length += (textLength);                                             \
        buffer[length] = '\0';                                              \
    } while (0)

    while (!at_end(parser))
    {
        char c = peek(parser);
        if (c == quote)
        {
            parser->position++;
            return LIBMATTI_MC_Nbt_StringTag_Of(buffer);
        }
        if (c == '\\')
        {
            parser->position++;
            if (at_end(parser))
            {
                set_error_at(parser, "expected escape sequence");
                free(buffer);
                return NULL;
            }
            char esc = peek(parser);
            parser->position++;
            switch (esc)
            {
            case 'b':
                PUSH("\b", 1);
                break;
            case 's':
                PUSH(" ", 1);
                break;
            case 't':
                PUSH("\t", 1);
                break;
            case 'n':
                PUSH("\n", 1);
                break;
            case 'f':
                PUSH("\f", 1);
                break;
            case 'r':
                PUSH("\r", 1);
                break;
            case '\\':
                PUSH("\\", 1);
                break;
            case '\'':
                PUSH("'", 1);
                break;
            case '"':
                PUSH("\"", 1);
                break;
            case 'x':
            case 'u':
            case 'U':
            {
                size_t width = esc == 'x' ? 2 : esc == 'u' ? 4 : 8;
                size_t remaining = strlen(parser->input) - parser->position;
                if (remaining < width)
                {
                    set_error_at(parser, "expected hex escape");
                    free(buffer);
                    return NULL;
                }
                char hex[9];
                memcpy(hex, parser->input + parser->position, width);
                hex[width] = '\0';
                char *end = NULL;
                long value = strtol(hex, &end, 16);
                if (end != hex + width)
                {
                    set_error_at(parser, "expected hex escape");
                    free(buffer);
                    return NULL;
                }
                parser->position += width;
                // UTF-8 encode the code point
                unsigned int code = (unsigned int) value;
                if (code < 0x80)
                {
                    char single = (char) code;
                    PUSH(&single, 1);
                }
                else if (code < 0x800)
                {
                    char bytes[2] = {(char) (0xC0 | (code >> 6)), (char) (0x80 | (code & 0x3F))};
                    PUSH(bytes, 2);
                }
                else if (code < 0x10000)
                {
                    char bytes[3] = {(char) (0xE0 | (code >> 12)), (char) (0x80 | ((code >> 6) & 0x3F)),
                                     (char) (0x80 | (code & 0x3F))};
                    PUSH(bytes, 3);
                }
                else
                {
                    char bytes[4] = {(char) (0xF0 | (code >> 18)), (char) (0x80 | ((code >> 12) & 0x3F)),
                                     (char) (0x80 | ((code >> 6) & 0x3F)), (char) (0x80 | (code & 0x3F))};
                    PUSH(bytes, 4);
                }
                break;
            }
            default:
                set_error_at(parser, "invalid escape sequence");
                free(buffer);
                return NULL;
            }
        }
        else
        {
            // Java: the plain chunk accepts everything but the quotes and the backslash
            PUSH(&c, 1);
            parser->position++;
        }
    }
#undef PUSH

    set_error_at(parser, "unterminated string");
    free(buffer);
    return NULL;
}

// Java: the unquoted_string rule ([A-Za-z0-9._+-]+ not starting with a number character)
static LIBMATTI_MC_Nbt_Tag *parse_unquoted_string(Parser *parser)
{
    size_t start = parser->position;
    while (!at_end(parser))
    {
        char c = peek(parser);
        if (isalnum((unsigned char) c) || c == '.' || c == '_' || c == '+' || c == '-')
            parser->position++;
        else
            break;
    }
    size_t length = parser->position - start;
    if (length == 0)
        return NULL;
    char *text = malloc(length + 1);
    memcpy(text, parser->input + start, length);
    text[length] = '\0';
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_StringTag_Of(text);
    free(text);
    return tag;
}

// ---------------------------------------------------------------------------
// the compound/list/literal rules
// ---------------------------------------------------------------------------

static LIBMATTI_MC_Nbt_Tag *parse_tag(Parser *parser);

// Java: the map_entry rule + the empty key check
static int parse_map_entry(Parser *parser, char **key, LIBMATTI_MC_Nbt_Tag **value)
{
    skip_whitespace(parser);
    if (!at_end(parser) && (peek(parser) == '"' || peek(parser) == '\''))
    {
        LIBMATTI_MC_Nbt_Tag *keyTag = parse_quoted_string(parser);
        if (keyTag == NULL)
            return 0;
        *key = strdup(keyTag->as.stringValue);
        LIBMATTI_MC_Nbt_Tag_Free(keyTag);
    }
    else
    {
        LIBMATTI_MC_Nbt_Tag *keyTag = parse_unquoted_string(parser);
        if (keyTag == NULL)
        {
            set_error_at(parser, "expected a key");
            return 0;
        }
        *key = strdup(keyTag->as.stringValue);
        LIBMATTI_MC_Nbt_Tag_Free(keyTag);
    }

    skip_whitespace(parser);
    if (!read_char(parser, ':'))
    {
        set_error_at(parser, "expected ':' after key");
        free(*key);
        return 0;
    }
    skip_whitespace(parser);
    LIBMATTI_MC_Nbt_Tag *parsed = parse_tag(parser);
    if (parsed == NULL)
    {
        free(*key);
        return 0;
    }
    *value = parsed;
    return 1;
}

static LIBMATTI_MC_Nbt_Tag *parse_compound(Parser *parser)
{
    parser->position++; // '{'
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_CompoundTag_New();
    skip_whitespace(parser);
    if (read_char(parser, '}'))
        return (LIBMATTI_MC_Nbt_Tag *) compound;

    while (!at_end(parser))
    {
        char *key;
        LIBMATTI_MC_Nbt_Tag *value;
        if (!parse_map_entry(parser, &key, &value))
        {
            LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
            free(compound);
            return NULL;
        }
        if (key[0] == '\0')
        {
            set_error_at(parser, "empty key");
            free(key);
            LIBMATTI_MC_Nbt_Tag_Free(value);
            LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
            free(compound);
            return NULL;
        }
        LIBMATTI_MC_Nbt_Tag *replaced = LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, value);
        LIBMATTI_MC_Nbt_Tag_Free(replaced);
        free(key);

        skip_whitespace(parser);
        if (read_char(parser, ','))
        {
            skip_whitespace(parser);
            // Java: repeatedWithTrailingSeparator allows a trailing comma
            if (peek(parser) == '}')
                break;
            continue;
        }
        if (read_char(parser, '}'))
            return (LIBMATTI_MC_Nbt_Tag *) compound;
        set_error_at(parser, "expected ',' or '}'");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
        free(compound);
        return NULL;
    }
    set_error_at(parser, "unterminated compound");
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
    free(compound);
    return NULL;
}

static LIBMATTI_MC_Nbt_Tag *parse_list_or_array(Parser *parser)
{
    parser->position++; // '['
    skip_whitespace(parser);

    // Java: the array_prefix rule - [B;..] / [I;..] / [L;..]
    char prefix = 0;
    if (!at_end(parser) && (peek(parser) == 'B' || peek(parser) == 'I' || peek(parser) == 'L'))
    {
        if (peek_at(parser, 1) == ';')
        {
            prefix = peek(parser);
            parser->position += 2;
            skip_whitespace(parser);
        }
    }

    if (prefix != 0)
    {
        if (read_char(parser, ']'))
        {
            // the empty array (Java: ArrayPrefix.create(ops))
            if (prefix == 'B')
                return LIBMATTI_MC_Nbt_ByteArrayTag_Of(NULL, 0);
            if (prefix == 'I')
                return LIBMATTI_MC_Nbt_IntArrayTag_Of(NULL, 0);
            return LIBMATTI_MC_Nbt_LongArrayTag_Of(NULL, 0);
        }

        long long *longs = NULL;
        size_t count = 0;
        size_t capacity = 0;

        while (!at_end(parser) && peek(parser) != ']')
        {
            // Java: the element is scanned as an IntegerLiteral and evaluated against the
            // target type - the prefix type when the literal carries no suffix
            IntegerLiteral literal;
            if (!scan_integer(parser, &literal))
            {
                free(longs);
                return NULL;
            }
            // Java: ArrayPrefix.computeType - an explicit suffix must be allowed for the
            // array kind (B: byte; I: int/byte/short; L: long/byte/short/int)
            int targetType;
            if (literal.suffixType == 0)
            {
                targetType = prefix == 'B'   ? LIBMATTI_MC_Nbt_TAG_BYTE
                             : prefix == 'I' ? LIBMATTI_MC_Nbt_TAG_INT
                                             : LIBMATTI_MC_Nbt_TAG_LONG;
            }
            else
            {
                int kind = literal.suffixType;
                int allowed = prefix == 'B' ? kind == LIBMATTI_MC_Nbt_TAG_BYTE
                              : prefix == 'I' ? (kind == LIBMATTI_MC_Nbt_TAG_INT || kind == LIBMATTI_MC_Nbt_TAG_BYTE ||
                                                 kind == LIBMATTI_MC_Nbt_TAG_SHORT)
                                              : (kind == LIBMATTI_MC_Nbt_TAG_LONG || kind == LIBMATTI_MC_Nbt_TAG_BYTE ||
                                                 kind == LIBMATTI_MC_Nbt_TAG_SHORT || kind == LIBMATTI_MC_Nbt_TAG_INT);
                if (!allowed)
                {
                    set_error_at(parser, "invalid array element type");
                    literal_free(&literal);
                    free(longs);
                    return NULL;
                }
                targetType = kind;
            }
            LIBMATTI_MC_Nbt_Tag *element = literal_create(parser, &literal, targetType);
            literal_free(&literal);
            if (element == NULL)
            {
                free(longs);
                return NULL;
            }
            if (count == capacity)
            {
                capacity = capacity > 0 ? capacity * 2 : 8;
                longs = realloc(longs, sizeof(long long) * capacity);
            }
            longs[count++] = element->as.longValue;
            LIBMATTI_MC_Nbt_Tag_Free(element);

            skip_whitespace(parser);
            if (read_char(parser, ','))
            {
                skip_whitespace(parser);
                if (peek(parser) == ']')
                    break;
                continue;
            }
            break;
        }
        if (!read_char(parser, ']'))
        {
            set_error_at(parser, "unterminated array");
            free(longs);
            return NULL;
        }

        LIBMATTI_MC_Nbt_Tag *tag;
        if (prefix == 'B')
        {
            int8_t *bytes = malloc(count > 0 ? count : 1);
            for (size_t i = 0; i < count; i++)
                bytes[i] = (int8_t) longs[i];
            tag = LIBMATTI_MC_Nbt_ByteArrayTag_Of(bytes, count);
            free(bytes);
        }
        else if (prefix == 'I')
        {
            int32_t *ints = malloc(count > 0 ? count * sizeof(int32_t) : sizeof(int32_t));
            for (size_t i = 0; i < count; i++)
                ints[i] = (int32_t) longs[i];
            tag = LIBMATTI_MC_Nbt_IntArrayTag_Of(ints, count);
            free(ints);
        }
        else
        {
            int64_t *longValues = malloc(count > 0 ? count * sizeof(int64_t) : sizeof(int64_t));
            for (size_t i = 0; i < count; i++)
                longValues[i] = longs[i];
            tag = LIBMATTI_MC_Nbt_LongArrayTag_Of(longValues, count);
            free(longValues);
        }
        free(longs);
        return tag;
    }

    // the plain list
    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    if (read_char(parser, ']'))
        return (LIBMATTI_MC_Nbt_Tag *) list;

    while (!at_end(parser))
    {
        LIBMATTI_MC_Nbt_Tag *element = parse_tag(parser);
        if (element == NULL)
        {
            LIBMATTI_MC_Nbt_ListTag_FreeEntries(list);
            free(list);
            return NULL;
        }
        LIBMATTI_MC_Nbt_ListTag_Add(list, element);
        skip_whitespace(parser);
        if (read_char(parser, ','))
        {
            skip_whitespace(parser);
            if (peek(parser) == ']')
                break;
            continue;
        }
        if (read_char(parser, ']'))
            return (LIBMATTI_MC_Nbt_Tag *) list;
        set_error_at(parser, "expected ',' or ']'");
        LIBMATTI_MC_Nbt_ListTag_FreeEntries(list);
        free(list);
        return NULL;
    }
    set_error_at(parser, "unterminated list");
    LIBMATTI_MC_Nbt_ListTag_FreeEntries(list);
    free(list);
    return NULL;
}

// Java: the unquoted_string_or_builtin rule - "true"/"false" collapse to ByteTag
// through NbtOps.createBoolean
static LIBMATTI_MC_Nbt_Tag *parse_unquoted_or_builtin(Parser *parser)
{
    LIBMATTI_MC_Nbt_Tag *tag = parse_unquoted_string(parser);
    if (tag == NULL)
        return NULL;
    const char *text = tag->as.stringValue;
    if (string_equals_ignore_case(text, "true") || string_equals_ignore_case(text, "false"))
    {
        int isTrue = tolower((unsigned char) text[0]) == 't';
        LIBMATTI_MC_Nbt_Tag_Free(tag);
        return LIBMATTI_MC_Nbt_ByteTag_Of(isTrue ? 1 : 0);
    }
    return tag;
}

static int can_start_number(char c)
{
    return c == '+' || c == '-' || c == '.' || is_digit(c);
}

static LIBMATTI_MC_Nbt_Tag *parse_tag(Parser *parser)
{
    if (parser->hasError)
        return NULL;
    if (++parser->depth > MAX_DEPTH)
    {
        set_error(parser, "Tried to read NBT tag with too high complexity, depth > 512");
        parser->depth--;
        return NULL;
    }

    skip_whitespace(parser);
    LIBMATTI_MC_Nbt_Tag *tag = NULL;
    if (at_end(parser))
    {
        set_error_at(parser, "expected a value");
    }
    else if (can_start_number(peek(parser)))
    {
        // Java: the float literal wins on '.', otherwise the integer literal - the port
        // parses the integer first and hands over to the float rule when '.'/'e' follows
        if (peek(parser) == '.')
        {
            tag = parse_float_tag(parser);
        }
        else
        {
            size_t save = parser->position;
            int savedError = parser->hasError;
            char savedErrorText[256];
            memcpy(savedErrorText, parser->error, sizeof(savedErrorText));
            int savedCut = parser->cut;
            tag = parse_integer_tag(parser);
            if (tag != NULL && !at_end(parser) &&
                (peek(parser) == '.' || peek(parser) == 'e' || peek(parser) == 'E'))
            {
                // "1.5"/"1e3" - the float rule takes the whole span
                parser->position = save;
                LIBMATTI_MC_Nbt_Tag_Free(tag);
                tag = parse_float_tag(parser);
            }
            else if (tag == NULL && !parser->cut)
            {
                // Java tries the float literal before the integer one - the port retries
                // as float unless the integer hit a cut (leading zeros, underscores);
                // this covers "+.5" where the integer scan finds no digits
                parser->position = save;
                parser->hasError = savedError;
                memcpy(parser->error, savedErrorText, sizeof(parser->error));
                parser->cut = savedCut;
                tag = parse_float_tag(parser);
            }
        }
    }
    else if (peek(parser) == '"' || peek(parser) == '\'')
    {
        tag = parse_quoted_string(parser);
    }
    else if (peek(parser) == '{')
    {
        tag = parse_compound(parser);
    }
    else if (peek(parser) == '[')
    {
        tag = parse_list_or_array(parser);
    }
    else
    {
        tag = parse_unquoted_or_builtin(parser);
    }

    parser->depth--;
    return tag;
}

// Java: public T parseFully(String) - the grammar root plus the trailing-data check
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_TagParser_ParseTagFully(const char *input)
{
    Parser parser = {input, 0, 0, "", 0};
    LIBMATTI_MC_Nbt_Tag *tag = parse_tag(&parser);
    if (tag != NULL)
    {
        skip_whitespace(&parser);
        if (!at_end(&parser))
        {
            set_error_at(&parser, "trailing data");
            LIBMATTI_MC_Nbt_Tag_Free(tag);
            tag = NULL;
        }
    }
    if (tag == NULL)
    {
        store_error(parser.hasError ? parser.error : "invalid SNBT");
        return NULL;
    }
    hasLastError = 0;
    return tag;
}

// Java: public static CompoundTag parseCompoundFully(String) - the compound cast
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully(const char *input)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_TagParser_ParseTagFully(input);
    if (tag == NULL)
        return NULL;
    if (tag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
    {
        // Java: ERROR_EXPECTED_COMPOUND - "Expected compound tag, got <tag>"
        char *printed = LIBMATTI_MC_Nbt_StringTagVisitor_Print(tag);
        char message[300];
        snprintf(message, sizeof(message), "Expected compound tag, got %s", printed != NULL ? printed : "?");
        free(printed);
        LIBMATTI_MC_Nbt_Tag_Free(tag);
        store_error(message);
        return NULL;
    }
    hasLastError = 0;
    return (LIBMATTI_MC_Nbt_CompoundTag *) tag;
}
