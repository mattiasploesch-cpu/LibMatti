// Port of com.google.gson.stream.JsonReader.

#include "libmatti/com/google/gson/JsonReader.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final class JsonScope - the scopes on the stack
enum
{
    SCOPE_EMPTY_ARRAY = 0,
    SCOPE_NONEMPTY_ARRAY,
    SCOPE_EMPTY_OBJECT,
    SCOPE_NONEMPTY_OBJECT,
    SCOPE_EXPECT_VALUE,
    SCOPE_EMPTY_DOCUMENT,
    SCOPE_NONEMPTY_DOCUMENT
};

// Java: private int peeked = PEEKED_NONE
enum
{
    PEEKED_NONE = 0,
    PEEKED_BEGIN_ARRAY,
    PEEKED_END_ARRAY,
    PEEKED_BEGIN_OBJECT,
    PEEKED_END_OBJECT,
    PEEKED_NAME,
    PEEKED_STRING,
    PEEKED_NUMBER,
    PEEKED_BOOLEAN,
    PEEKED_NULL,
    PEEKED_EOF
};

// Java: private char peekedBoolean
static int peekedBoolean;

static void push(LIBMATTI_GSON_JsonReader *reader, int scope)
{
    if (reader->stackSize == reader->stackCapacity)
    {
        reader->stackCapacity = reader->stackCapacity == 0 ? 8 : reader->stackCapacity * 2;
        reader->stack = realloc(reader->stack, sizeof(int) * reader->stackCapacity);
    }
    reader->stack[reader->stackSize++] = scope;
}

static void pop(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->stackSize > 0) reader->stackSize--;
}

static int currentScope(const LIBMATTI_GSON_JsonReader *reader)
{
    return reader->stackSize > 0 ? reader->stack[reader->stackSize - 1] : SCOPE_NONEMPTY_DOCUMENT;
}

static void setScope(LIBMATTI_GSON_JsonReader *reader, int scope)
{
    if (reader->stackSize > 0) reader->stack[reader->stackSize - 1] = scope;
}

// Java: private int nextNonWhitespace(boolean throwOnEof)
static int nextNonWhitespace(LIBMATTI_GSON_JsonReader *reader)
{
    while (reader->pos < reader->length)
    {
        char c = reader->text[reader->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { reader->pos++; continue; }
        reader->pos++;
        return (unsigned char) c;
    }
    return -1;
}

static void clearPeeked(LIBMATTI_GSON_JsonReader *reader)
{
    reader->peeked = PEEKED_NONE;
    free(reader->peekedText);
    reader->peekedText = NULL;
}

// Java: private String nextQuotedValue(char quote)
static char *nextQuotedValue(LIBMATTI_GSON_JsonReader *reader, char quote)
{
    size_t capacity = 32;
    size_t length = 0;
    char *value = malloc(capacity);

    while (reader->pos < reader->length)
    {
        char c = reader->text[reader->pos++];
        if (c == quote)
        {
            value[length] = '\0';
            return value;
        }

        if (c != '\\')
        {
            if (length + 1 >= capacity) { capacity *= 2; value = realloc(value, capacity); }
            value[length++] = c;
            continue;
        }

        if (reader->pos >= reader->length) break;

        char escape = reader->text[reader->pos++];
        if (length + 4 >= capacity) { capacity *= 2; value = realloc(value, capacity); }

        switch (escape)
        {
            case 'u':
            {
                if (reader->pos + 4 > reader->length) break;
                char hex[5] = {reader->text[reader->pos], reader->text[reader->pos + 1],
                               reader->text[reader->pos + 2], reader->text[reader->pos + 3], '\0'};
                reader->pos += 4;
                value[length++] = (char) strtol(hex, NULL, 16);
                break;
            }
            case 't': value[length++] = '\t'; break;
            case 'b': value[length++] = '\b'; break;
            case 'n': value[length++] = '\n'; break;
            case 'r': value[length++] = '\r'; break;
            case 'f': value[length++] = '\f'; break;
            default: value[length++] = escape; break;
        }
    }

    value[length] = '\0';
    return value;
}

// Java: private String nextUnquotedValue()
static char *nextUnquotedValue(LIBMATTI_GSON_JsonReader *reader)
{
    size_t start = reader->pos;
    while (reader->pos < reader->length)
    {
        char c = reader->text[reader->pos];
        if (c == ',' || c == '}' || c == ']' || c == ' ' || c == '\t' || c == '\n' || c == '\r') break;
        reader->pos++;
    }

    size_t length = reader->pos - start;
    char *value = malloc(length + 1);
    memcpy(value, reader->text + start, length);
    value[length] = '\0';
    return value;
}

static int matchesKeyword(LIBMATTI_GSON_JsonReader *reader, size_t start, const char *keyword)
{
    size_t keywordLength = strlen(keyword);
    if (start + keywordLength > reader->length) return 0;
    return memcmp(reader->text + start, keyword, keywordLength) == 0;
}

// Java: private int peekKeyword()
static int peekKeyword(LIBMATTI_GSON_JsonReader *reader, int *token)
{
    char c = reader->text[reader->pos];
    const char *keyword = NULL;
    int peekedValue = PEEKED_NONE;

    if (c == 't' || c == 'T')
    {
        keyword = "true";
        peekedValue = PEEKED_BOOLEAN;
        peekedBoolean = 1;
    }
    else if (c == 'f' || c == 'F')
    {
        keyword = "false";
        peekedValue = PEEKED_BOOLEAN;
        peekedBoolean = 0;
    }
    else if (c == 'n' || c == 'N')
    {
        keyword = "null";
        peekedValue = PEEKED_NULL;
    }

    if (keyword == NULL) return 0;

    size_t start = reader->pos;
    size_t keywordLength = strlen(keyword);
    if (!matchesKeyword(reader, start, keyword))
    {
        // Java: the keyword match is case-insensitive in lenient mode only
        if (!reader->lenient) return 0;
    }

    reader->pos += keywordLength;
    *token = peekedValue;
    return 1;
}

// Java: private int doPeek()
static int doPeek(LIBMATTI_GSON_JsonReader *reader)
{
    int scope = currentScope(reader);

    if (scope == SCOPE_EXPECT_VALUE)
    {
        setScope(reader, SCOPE_NONEMPTY_OBJECT);
        scope = SCOPE_NONEMPTY_OBJECT;
        goto read_value;
    }

    if (scope == SCOPE_EMPTY_ARRAY)
    {
        int c = nextNonWhitespace(reader);
        if (c == ']') return PEEKED_END_ARRAY;
        if (c != -1) reader->pos--;
        setScope(reader, SCOPE_NONEMPTY_ARRAY);
        goto read_value;
    }

    if (scope == SCOPE_NONEMPTY_ARRAY)
    {
        int c = nextNonWhitespace(reader);
        if (c == ']') return PEEKED_END_ARRAY;
        if (c == ',') goto read_value;
        return PEEKED_EOF; // Java: throw syntaxError("Unterminated array")
    }

    if (scope == SCOPE_EMPTY_OBJECT || scope == SCOPE_NONEMPTY_OBJECT)
    {
        if (scope == SCOPE_NONEMPTY_OBJECT)
        {
            int c = nextNonWhitespace(reader);
            if (c == '}') return PEEKED_END_OBJECT;
            if (c != ',') return PEEKED_EOF; // Java: throw syntaxError("Unterminated object")
        }
        else
        {
            int c = nextNonWhitespace(reader);
            if (c == '}') return PEEKED_END_OBJECT;
            if (c != -1) reader->pos--;
        }

        int c = nextNonWhitespace(reader);
        if (c == '"' || (reader->lenient && c == '\''))
            reader->peekedText = nextQuotedValue(reader, (char) c);
        else if (reader->lenient)
        {
            if (c != -1) reader->pos--;
            reader->peekedText = nextUnquotedValue(reader);
        }
        else
            return PEEKED_EOF; // Java: throw syntaxError("Names must be double-quoted")

        // Java: the ':' separating the name from its value
        int colon = nextNonWhitespace(reader);
        if (colon != ':') return PEEKED_EOF; // Java: throw syntaxError("Expected ':'")

        setScope(reader, SCOPE_EXPECT_VALUE);
        return PEEKED_NAME;
    }

    if (scope == SCOPE_EMPTY_DOCUMENT)
    {
        setScope(reader, SCOPE_NONEMPTY_DOCUMENT);
    }
    else if (scope == SCOPE_NONEMPTY_DOCUMENT)
    {
        int c = nextNonWhitespace(reader);
        if (c == -1) return PEEKED_EOF;
        reader->pos--; // Java: throw syntaxError("Expected EOF")
        return PEEKED_EOF;
    }

read_value:
    {
        int c = nextNonWhitespace(reader);
        if (c == -1) return PEEKED_EOF;

        switch (c)
        {
            case '{':
                return PEEKED_BEGIN_OBJECT;
            case '[':
                return PEEKED_BEGIN_ARRAY;
            case '"':
                reader->peekedText = nextQuotedValue(reader, '"');
                return PEEKED_STRING;
            case '\'':
                if (!reader->lenient) return PEEKED_EOF;
                reader->peekedText = nextQuotedValue(reader, '\'');
                return PEEKED_STRING;
            case 't':
            case 'T':
            case 'f':
            case 'F':
            case 'n':
            case 'N':
            {
                reader->pos--;
                int token = PEEKED_NONE;
                if (peekKeyword(reader, &token)) return token;
                return PEEKED_EOF;
            }
            default:
                reader->pos--;
                break;
        }

        // Java: a number (or an unquoted value in lenient mode)
        char first = reader->text[reader->pos];
        if (first == '-' || (first >= '0' && first <= '9'))
        {
            reader->peekedText = nextUnquotedValue(reader);
            return PEEKED_NUMBER;
        }

        if (reader->lenient)
        {
            reader->peekedText = nextUnquotedValue(reader);
            return PEEKED_STRING;
        }

        return PEEKED_EOF; // Java: throw syntaxError("Expected value")
    }
}

// Java: public void setLenient(boolean lenient)
void LIBMATTI_GSON_JsonReader_SetLenient(LIBMATTI_GSON_JsonReader *reader, int lenient)
{
    reader->lenient = lenient;
}

// Java: public JsonToken peek()
int LIBMATTI_GSON_JsonReader_Peek(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);

    switch (reader->peeked)
    {
        case PEEKED_BEGIN_ARRAY: return LIBMATTI_GSON_JSON_TOKEN_BEGIN_ARRAY;
        case PEEKED_END_ARRAY: return LIBMATTI_GSON_JSON_TOKEN_END_ARRAY;
        case PEEKED_BEGIN_OBJECT: return LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT;
        case PEEKED_END_OBJECT: return LIBMATTI_GSON_JSON_TOKEN_END_OBJECT;
        case PEEKED_NAME: return LIBMATTI_GSON_JSON_TOKEN_NAME;
        case PEEKED_STRING: return LIBMATTI_GSON_JSON_TOKEN_STRING;
        case PEEKED_NUMBER: return LIBMATTI_GSON_JSON_TOKEN_NUMBER;
        case PEEKED_BOOLEAN: return LIBMATTI_GSON_JSON_TOKEN_BOOLEAN;
        case PEEKED_NULL: return LIBMATTI_GSON_JSON_TOKEN_NULL;
        default: return LIBMATTI_GSON_JSON_TOKEN_END_DOCUMENT;
    }
}

// Java: public void beginArray()
void LIBMATTI_GSON_JsonReader_BeginArray(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked == PEEKED_BEGIN_ARRAY)
    {
        push(reader, SCOPE_EMPTY_ARRAY);
        clearPeeked(reader);
    }
}

// Java: public void endArray()
void LIBMATTI_GSON_JsonReader_EndArray(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked == PEEKED_END_ARRAY)
    {
        pop(reader);
        clearPeeked(reader);
    }
}

// Java: public void beginObject()
void LIBMATTI_GSON_JsonReader_BeginObject(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked == PEEKED_BEGIN_OBJECT)
    {
        push(reader, SCOPE_EMPTY_OBJECT);
        clearPeeked(reader);
    }
}

// Java: public void endObject()
void LIBMATTI_GSON_JsonReader_EndObject(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked == PEEKED_END_OBJECT)
    {
        pop(reader);
        clearPeeked(reader);
    }
}

// Java: public boolean hasNext()
int LIBMATTI_GSON_JsonReader_HasNext(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);

    return reader->peeked != PEEKED_END_OBJECT && reader->peeked != PEEKED_END_ARRAY &&
           reader->peeked != PEEKED_EOF;
}

// Java: public String nextName()
char *LIBMATTI_GSON_JsonReader_NextName(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked != PEEKED_NAME) return NULL;

    char *name = reader->peekedText;
    reader->peekedText = NULL;
    reader->peeked = PEEKED_NONE;
    return name;
}

// Java: public String nextString()
char *LIBMATTI_GSON_JsonReader_NextString(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);

    char *value = NULL;
    switch (reader->peeked)
    {
        case PEEKED_STRING:
        case PEEKED_NUMBER:
            value = reader->peekedText;
            reader->peekedText = NULL;
            break;
        case PEEKED_BOOLEAN:
            value = strdup(peekedBoolean ? "true" : "false");
            break;
        case PEEKED_NULL:
            value = strdup("null");
            break;
        default:
            return NULL;
    }

    clearPeeked(reader);
    return value;
}

// Java: public boolean nextBoolean()
int LIBMATTI_GSON_JsonReader_NextBoolean(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked != PEEKED_BOOLEAN) return 0;

    clearPeeked(reader);
    return peekedBoolean;
}

// Java: public void nextNull()
void LIBMATTI_GSON_JsonReader_NextNull(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader->peeked == PEEKED_NONE) reader->peeked = doPeek(reader);
    if (reader->peeked == PEEKED_NULL) clearPeeked(reader);
}

// Java: public void skipValue()
void LIBMATTI_GSON_JsonReader_SkipValue(LIBMATTI_GSON_JsonReader *reader)
{
    int token = LIBMATTI_GSON_JsonReader_Peek(reader);

    if (token == LIBMATTI_GSON_JSON_TOKEN_BEGIN_ARRAY)
    {
        LIBMATTI_GSON_JsonReader_BeginArray(reader);
        while (LIBMATTI_GSON_JsonReader_HasNext(reader)) LIBMATTI_GSON_JsonReader_SkipValue(reader);
        LIBMATTI_GSON_JsonReader_EndArray(reader);
        return;
    }

    if (token == LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
    {
        LIBMATTI_GSON_JsonReader_BeginObject(reader);
        while (LIBMATTI_GSON_JsonReader_HasNext(reader))
        {
            free(LIBMATTI_GSON_JsonReader_NextName(reader));
            LIBMATTI_GSON_JsonReader_SkipValue(reader);
        }
        LIBMATTI_GSON_JsonReader_EndObject(reader);
        return;
    }

    if (token == LIBMATTI_GSON_JSON_TOKEN_NULL)
        LIBMATTI_GSON_JsonReader_NextNull(reader);
    else
        free(LIBMATTI_GSON_JsonReader_NextString(reader));
}

// Java: public JsonReader(Reader in)
LIBMATTI_GSON_JsonReader *LIBMATTI_GSON_JsonReader_New(const char *text, size_t length)
{
    LIBMATTI_GSON_JsonReader *reader = calloc(1, sizeof(LIBMATTI_GSON_JsonReader));
    reader->text = text;
    reader->length = length;
    reader->peeked = PEEKED_NONE;

    // Java: stack[stackSize++] = JsonScope.EMPTY_DOCUMENT;
    push(reader, SCOPE_EMPTY_DOCUMENT);
    return reader;
}

void LIBMATTI_GSON_JsonReader_Free(LIBMATTI_GSON_JsonReader *reader)
{
    if (reader == NULL) return;
    free(reader->stack);
    free(reader->peekedText);
    free(reader->peekedNumber);
    free(reader);
}
