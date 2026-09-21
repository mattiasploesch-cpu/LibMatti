// Port of com.mojang.brigadier.StringReader.

#include "libmatti/com/mojang/brigadier/StringReader.h"

#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_BR_StringReader LIBMATTI_BR_StringReader_Of(const char *string)
{
    LIBMATTI_BR_StringReader reader = {string, 0};
    return reader;
}

LIBMATTI_BR_StringReader LIBMATTI_BR_StringReader_Copy(const LIBMATTI_BR_StringReader *other)
{
    return *other;
}

const char *LIBMATTI_BR_StringReader_GetString(const LIBMATTI_BR_StringReader *reader)
{
    return reader->string;
}

void LIBMATTI_BR_StringReader_SetCursor(LIBMATTI_BR_StringReader *reader, int cursor)
{
    reader->cursor = cursor;
}

int LIBMATTI_BR_StringReader_GetRemainingLength(const LIBMATTI_BR_StringReader *reader)
{
    return (int) strlen(reader->string) - reader->cursor;
}

int LIBMATTI_BR_StringReader_GetTotalLength(const LIBMATTI_BR_StringReader *reader)
{
    return (int) strlen(reader->string);
}

int LIBMATTI_BR_StringReader_GetCursor(const LIBMATTI_BR_StringReader *reader)
{
    return reader->cursor;
}

char *LIBMATTI_BR_StringReader_GetRead(const LIBMATTI_BR_StringReader *reader)
{
    size_t length = (size_t) reader->cursor;
    char *result = malloc(length + 1);
    memcpy(result, reader->string, length);
    result[length] = '\0';
    return result;
}

char *LIBMATTI_BR_StringReader_GetRemaining(const LIBMATTI_BR_StringReader *reader)
{
    return strdup(reader->string + reader->cursor);
}

int LIBMATTI_BR_StringReader_CanReadLength(const LIBMATTI_BR_StringReader *reader, int length)
{
    return reader->cursor + length <= (int) strlen(reader->string);
}

int LIBMATTI_BR_StringReader_CanRead(const LIBMATTI_BR_StringReader *reader)
{
    return LIBMATTI_BR_StringReader_CanReadLength(reader, 1);
}

char LIBMATTI_BR_StringReader_Peek(const LIBMATTI_BR_StringReader *reader)
{
    return reader->string[reader->cursor];
}

char LIBMATTI_BR_StringReader_PeekOffset(const LIBMATTI_BR_StringReader *reader, int offset)
{
    return reader->string[reader->cursor + offset];
}

char LIBMATTI_BR_StringReader_Read(LIBMATTI_BR_StringReader *reader)
{
    return reader->string[reader->cursor++];
}

void LIBMATTI_BR_StringReader_Skip(LIBMATTI_BR_StringReader *reader)
{
    reader->cursor++;
}

int LIBMATTI_BR_StringReader_IsAllowedNumber(char c)
{
    return (c >= '0' && c <= '9') || c == '.' || c == '-';
}

int LIBMATTI_BR_StringReader_IsQuotedStringStart(char c)
{
    return c == LIBMATTI_BR_StringReader_SYNTAX_DOUBLE_QUOTE || c == LIBMATTI_BR_StringReader_SYNTAX_SINGLE_QUOTE;
}

void LIBMATTI_BR_StringReader_SkipWhitespace(LIBMATTI_BR_StringReader *reader)
{
    // Java: Character.isWhitespace(peek())
    while (LIBMATTI_BR_StringReader_CanRead(reader) &&
           isspace((unsigned char) LIBMATTI_BR_StringReader_Peek(reader)))
    {
        LIBMATTI_BR_StringReader_Skip(reader);
    }
}

// Java: the numeric readers share the "read the allowed chars, parse" shape
static char *read_number_text(LIBMATTI_BR_StringReader *reader, int *outStart)
{
    int start = reader->cursor;
    while (LIBMATTI_BR_StringReader_CanRead(reader) &&
           LIBMATTI_BR_StringReader_IsAllowedNumber(LIBMATTI_BR_StringReader_Peek(reader)))
    {
        LIBMATTI_BR_StringReader_Skip(reader);
    }
    *outStart = start;
    size_t length = (size_t) (reader->cursor - start);
    char *number = malloc(length + 1);
    memcpy(number, reader->string + start, length);
    number[length] = '\0';
    return number;
}

int LIBMATTI_BR_StringReader_ReadInt(LIBMATTI_BR_StringReader *reader,
                                     LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    int start;
    char *number = read_number_text(reader, &start);
    if (number[0] == '\0')
    {
        free(number);
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedInt(), reader->string, reader->cursor);
        return 0;
    }
    // Java: Integer.parseInt - the port validates the int range explicitly
    errno = 0;
    char *end = NULL;
    long long value = strtoll(number, &end, 10);
    if (errno != 0 || end == number || *end != '\0' || value < INT_MIN || value > INT_MAX)
    {
        reader->cursor = start;
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderInvalidInt(), reader->string, reader->cursor, number);
        free(number);
        return 0;
    }
    free(number);
    return (int) value;
}

long long LIBMATTI_BR_StringReader_ReadLong(LIBMATTI_BR_StringReader *reader,
                                            LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    int start;
    char *number = read_number_text(reader, &start);
    if (number[0] == '\0')
    {
        free(number);
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedLong(), reader->string, reader->cursor);
        return 0;
    }
    errno = 0;
    char *end = NULL;
    long long value = strtoll(number, &end, 10);
    if (errno != 0 || end == number || *end != '\0')
    {
        reader->cursor = start;
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderInvalidLong(), reader->string, reader->cursor, number);
        free(number);
        return 0;
    }
    free(number);
    return value;
}

double LIBMATTI_BR_StringReader_ReadDouble(LIBMATTI_BR_StringReader *reader,
                                           LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    int start;
    char *number = read_number_text(reader, &start);
    if (number[0] == '\0')
    {
        free(number);
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedDouble(), reader->string, reader->cursor);
        return 0.0;
    }
    char *end = NULL;
    double value = strtod(number, &end);
    if (end == number || *end != '\0')
    {
        reader->cursor = start;
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderInvalidDouble(), reader->string, reader->cursor, number);
        free(number);
        return 0.0;
    }
    free(number);
    return value;
}

float LIBMATTI_BR_StringReader_ReadFloat(LIBMATTI_BR_StringReader *reader,
                                         LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    int start;
    char *number = read_number_text(reader, &start);
    if (number[0] == '\0')
    {
        free(number);
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedFloat(), reader->string, reader->cursor);
        return 0.0f;
    }
    char *end = NULL;
    float value = strtof(number, &end);
    if (end == number || *end != '\0')
    {
        reader->cursor = start;
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderInvalidFloat(), reader->string, reader->cursor, number);
        free(number);
        return 0.0f;
    }
    free(number);
    return value;
}

int LIBMATTI_BR_StringReader_IsAllowedInUnquotedString(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-' ||
           c == '.' || c == '+';
}

char *LIBMATTI_BR_StringReader_ReadUnquotedString(LIBMATTI_BR_StringReader *reader)
{
    int start = reader->cursor;
    while (LIBMATTI_BR_StringReader_CanRead(reader) &&
           LIBMATTI_BR_StringReader_IsAllowedInUnquotedString(LIBMATTI_BR_StringReader_Peek(reader)))
    {
        LIBMATTI_BR_StringReader_Skip(reader);
    }
    size_t length = (size_t) (reader->cursor - start);
    char *result = malloc(length + 1);
    memcpy(result, reader->string + start, length);
    result[length] = '\0';
    return result;
}

char *LIBMATTI_BR_StringReader_ReadQuotedString(LIBMATTI_BR_StringReader *reader,
                                                LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    if (!LIBMATTI_BR_StringReader_CanRead(reader))
    {
        return strdup("");
    }
    char next = LIBMATTI_BR_StringReader_Peek(reader);
    if (!LIBMATTI_BR_StringReader_IsQuotedStringStart(next))
    {
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedStartOfQuote(), reader->string, reader->cursor);
        return NULL;
    }
    LIBMATTI_BR_StringReader_Skip(reader);
    return LIBMATTI_BR_StringReader_ReadStringUntil(reader, next, outException);
}

char *LIBMATTI_BR_StringReader_ReadStringUntil(LIBMATTI_BR_StringReader *reader, char terminator,
                                               LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    // Java: StringBuilder result
    size_t capacity = 16;
    char *result = malloc(capacity);
    size_t length = 0;
    int escaped = 0;
    while (LIBMATTI_BR_StringReader_CanRead(reader))
    {
        char c = LIBMATTI_BR_StringReader_Read(reader);
        if (escaped)
        {
            if (c == terminator || c == LIBMATTI_BR_StringReader_SYNTAX_ESCAPE)
            {
                if (length + 1 >= capacity)
                {
                    capacity *= 2;
                    result = realloc(result, capacity);
                }
                result[length++] = c;
                escaped = 0;
            }
            else
            {
                LIBMATTI_BR_StringReader_SetCursor(reader, LIBMATTI_BR_StringReader_GetCursor(reader) - 1);
                char symbol[2] = {c, '\0'};
                *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
                    LIBMATTI_BR_BuiltInExceptions_ReaderInvalidEscape(), reader->string, reader->cursor, symbol);
                free(result);
                return NULL;
            }
        }
        else if (c == LIBMATTI_BR_StringReader_SYNTAX_ESCAPE)
        {
            escaped = 1;
        }
        else if (c == terminator)
        {
            result[length] = '\0';
            return result;
        }
        else
        {
            if (length + 1 >= capacity)
            {
                capacity *= 2;
                result = realloc(result, capacity);
            }
            result[length++] = c;
        }
    }

    *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
        LIBMATTI_BR_BuiltInExceptions_ReaderExpectedEndOfQuote(), reader->string, reader->cursor);
    free(result);
    return NULL;
}

char *LIBMATTI_BR_StringReader_ReadString(LIBMATTI_BR_StringReader *reader,
                                          LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    if (!LIBMATTI_BR_StringReader_CanRead(reader))
    {
        return strdup("");
    }
    char next = LIBMATTI_BR_StringReader_Peek(reader);
    if (LIBMATTI_BR_StringReader_IsQuotedStringStart(next))
    {
        LIBMATTI_BR_StringReader_Skip(reader);
        return LIBMATTI_BR_StringReader_ReadStringUntil(reader, next, outException);
    }
    return LIBMATTI_BR_StringReader_ReadUnquotedString(reader);
}

int LIBMATTI_BR_StringReader_ReadBoolean(LIBMATTI_BR_StringReader *reader,
                                         LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    int start = reader->cursor;
    char *value = LIBMATTI_BR_StringReader_ReadString(reader, outException);
    if (*outException != NULL) return 0;
    if (value[0] == '\0')
    {
        free(value);
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedBool(), reader->string, reader->cursor);
        return 0;
    }
    if (strcmp(value, "true") == 0)
    {
        free(value);
        return 1;
    }
    if (strcmp(value, "false") == 0)
    {
        free(value);
        return 0;
    }
    reader->cursor = start;
    *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
        LIBMATTI_BR_BuiltInExceptions_ReaderInvalidBool(), reader->string, reader->cursor, value);
    free(value);
    return 0;
}

void LIBMATTI_BR_StringReader_Expect(LIBMATTI_BR_StringReader *reader, char c,
                                     LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    if (!LIBMATTI_BR_StringReader_CanRead(reader) || LIBMATTI_BR_StringReader_Peek(reader) != c)
    {
        char symbol[2] = {c, '\0'};
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_ReaderExpectedSymbol(), reader->string, reader->cursor, symbol);
        return;
    }
    LIBMATTI_BR_StringReader_Skip(reader);
}
