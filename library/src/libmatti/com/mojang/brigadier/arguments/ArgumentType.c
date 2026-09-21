// Port of com.mojang.brigadier.arguments.*: the built-in argument types.

#include "libmatti/com/mojang/brigadier/arguments/ArgumentType.h"

#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_BR_ArgumentType LIBMATTI_BR_BoolArgumentType_Bool(void)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_BOOL;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_IntegerMinMax(int min, int max)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_INTEGER;
    type.minimum = min;
    type.maximum = max;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_IntegerMin(int min)
{
    return LIBMATTI_BR_IntegerArgumentType_IntegerMinMax(min, INT_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_Integer(void)
{
    return LIBMATTI_BR_IntegerArgumentType_IntegerMinMax(INT_MIN, INT_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_LongMinMax(long long min, long long max)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_LONG;
    type.minimumLong = min;
    type.maximumLong = max;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_LongMin(long long min)
{
    return LIBMATTI_BR_LongArgumentType_LongMinMax(min, LLONG_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_Long(void)
{
    return LIBMATTI_BR_LongArgumentType_LongMinMax(LLONG_MIN, LLONG_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_FloatMinMax(float min, float max)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_FLOAT;
    type.minimumDouble = min;
    type.maximumDouble = max;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_FloatMin(float min)
{
    return LIBMATTI_BR_FloatArgumentType_FloatMinMax(min, FLT_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_Float(void)
{
    return LIBMATTI_BR_FloatArgumentType_FloatMinMax(-FLT_MAX, FLT_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_DoubleMinMax(double min, double max)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_DOUBLE;
    type.minimumDouble = min;
    type.maximumDouble = max;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_DoubleMin(double min)
{
    return LIBMATTI_BR_DoubleArgumentType_DoubleMinMax(min, DBL_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_Double(void)
{
    return LIBMATTI_BR_DoubleArgumentType_DoubleMinMax(-DBL_MAX, DBL_MAX);
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_Word(void)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_STRING;
    type.stringType = LIBMATTI_BR_StringType_SINGLE_WORD;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_String(void)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_STRING;
    type.stringType = LIBMATTI_BR_StringType_QUOTABLE_PHRASE;
    return type;
}

LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_GreedyString(void)
{
    LIBMATTI_BR_ArgumentType type = {0};
    type.kind = LIBMATTI_BR_ARG_STRING;
    type.stringType = LIBMATTI_BR_StringType_GREEDY_PHRASE;
    return type;
}

int LIBMATTI_BR_ArgumentType_ParseBool(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                       LIBMATTI_BR_CommandSyntaxException **outException)
{
    (void) type;
    return LIBMATTI_BR_StringReader_ReadBoolean(reader, outException);
}

int LIBMATTI_BR_ArgumentType_ParseInt(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                      LIBMATTI_BR_CommandSyntaxException **outException)
{
    // Java: final int start = reader.getCursor(); final int result = reader.readInt();
    int start = LIBMATTI_BR_StringReader_GetCursor(reader);
    int result = LIBMATTI_BR_StringReader_ReadInt(reader, outException);
    if (*outException != NULL) return 0;
    if (result < type->minimum)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[16];
        char min[16];
        snprintf(found, sizeof(found), "%d", result);
        snprintf(min, sizeof(min), "%d", type->minimum);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_IntegerTooLow(), reader->string, reader->cursor, min, found);
        return 0;
    }
    if (result > type->maximum)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[16];
        char max[16];
        snprintf(found, sizeof(found), "%d", result);
        snprintf(max, sizeof(max), "%d", type->maximum);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_IntegerTooHigh(), reader->string, reader->cursor, max, found);
        return 0;
    }
    return result;
}

long long LIBMATTI_BR_ArgumentType_ParseLong(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                             LIBMATTI_BR_CommandSyntaxException **outException)
{
    int start = LIBMATTI_BR_StringReader_GetCursor(reader);
    long long result = LIBMATTI_BR_StringReader_ReadLong(reader, outException);
    if (*outException != NULL) return 0;
    if (result < type->minimumLong)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[24];
        char min[24];
        snprintf(found, sizeof(found), "%lld", result);
        snprintf(min, sizeof(min), "%lld", type->minimumLong);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_LongTooLow(), reader->string, reader->cursor, min, found);
        return 0;
    }
    if (result > type->maximumLong)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[24];
        char max[24];
        snprintf(found, sizeof(found), "%lld", result);
        snprintf(max, sizeof(max), "%lld", type->maximumLong);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_LongTooHigh(), reader->string, reader->cursor, max, found);
        return 0;
    }
    return result;
}

float LIBMATTI_BR_ArgumentType_ParseFloat(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                          LIBMATTI_BR_CommandSyntaxException **outException)
{
    int start = LIBMATTI_BR_StringReader_GetCursor(reader);
    float result = LIBMATTI_BR_StringReader_ReadFloat(reader, outException);
    if (*outException != NULL) return 0.0f;
    float minimum = (float) type->minimumDouble;
    float maximum = (float) type->maximumDouble;
    if (result < minimum)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[32];
        char min[32];
        snprintf(found, sizeof(found), "%g", (double) result);
        snprintf(min, sizeof(min), "%g", (double) minimum);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_FloatTooLow(), reader->string, reader->cursor, min, found);
        return 0.0f;
    }
    if (result > maximum)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[32];
        char max[32];
        snprintf(found, sizeof(found), "%g", (double) result);
        snprintf(max, sizeof(max), "%g", (double) maximum);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_FloatTooHigh(), reader->string, reader->cursor, max, found);
        return 0.0f;
    }
    return result;
}

double LIBMATTI_BR_ArgumentType_ParseDouble(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                            LIBMATTI_BR_CommandSyntaxException **outException)
{
    int start = LIBMATTI_BR_StringReader_GetCursor(reader);
    double result = LIBMATTI_BR_StringReader_ReadDouble(reader, outException);
    if (*outException != NULL) return 0.0;
    if (result < type->minimumDouble)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[40];
        char min[40];
        snprintf(found, sizeof(found), "%g", result);
        snprintf(min, sizeof(min), "%g", type->minimumDouble);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_DoubleTooLow(), reader->string, reader->cursor, min, found);
        return 0.0;
    }
    if (result > type->maximumDouble)
    {
        LIBMATTI_BR_StringReader_SetCursor(reader, start);
        char found[40];
        char max[40];
        snprintf(found, sizeof(found), "%g", result);
        snprintf(max, sizeof(max), "%g", type->maximumDouble);
        *outException = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
            LIBMATTI_BR_BuiltInExceptions_DoubleTooHigh(), reader->string, reader->cursor, max, found);
        return 0.0;
    }
    return result;
}

char *LIBMATTI_BR_ArgumentType_ParseString(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                           LIBMATTI_BR_CommandSyntaxException **outException)
{
    // Java: StringArgumentType.parse
    if (type->stringType == LIBMATTI_BR_StringType_GREEDY_PHRASE)
    {
        char *text = LIBMATTI_BR_StringReader_GetRemaining(reader);
        LIBMATTI_BR_StringReader_SetCursor(reader, LIBMATTI_BR_StringReader_GetTotalLength(reader));
        return text;
    }
    if (type->stringType == LIBMATTI_BR_StringType_SINGLE_WORD)
    {
        return LIBMATTI_BR_StringReader_ReadUnquotedString(reader);
    }
    return LIBMATTI_BR_StringReader_ReadString(reader, outException);
}

char *LIBMATTI_BR_StringArgumentType_EscapeIfRequired(const char *input)
{
    for (const char *p = input; *p != '\0'; p++)
    {
        if (!LIBMATTI_BR_StringReader_IsAllowedInUnquotedString(*p))
        {
            // Java: escape(input) -> "\"" + input.replace("\\", "\\\\").replace("\"", "\\\"") + "\""
            size_t extra = 0;
            for (const char *q = input; *q != '\0'; q++)
            {
                if (*q == '\\' || *q == '"') extra++;
            }
            size_t length = strlen(input) + extra + 3;
            char *escaped = malloc(length);
            char *writer = escaped;
            *writer++ = '"';
            for (const char *q = input; *q != '\0'; q++)
            {
                if (*q == '\\' || *q == '"') *writer++ = '\\';
                *writer++ = *q;
            }
            *writer++ = '"';
            *writer = '\0';
            return escaped;
        }
    }
    return strdup(input);
}
