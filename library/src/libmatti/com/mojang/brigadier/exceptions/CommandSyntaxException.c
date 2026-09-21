// Port of com.mojang.brigadier.exceptions.*: CommandSyntaxException,
// SimpleCommandExceptionType, DynamicCommandExceptionType, BuiltInExceptions.

#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// CommandSyntaxException
// ---------------------------------------------------------------------------

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_CommandSyntaxException_New(LIBMATTI_BR_CommandExceptionType *type,
                                                                           LIBMATTI_BR_Message *message)
{
    LIBMATTI_BR_CommandSyntaxException *exception = calloc(1, sizeof(LIBMATTI_BR_CommandSyntaxException));
    exception->type = type;
    exception->message = message;
    exception->input = NULL;
    exception->cursor = -1;
    return exception;
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_CommandSyntaxException_NewWithContext(
    LIBMATTI_BR_CommandExceptionType *type, LIBMATTI_BR_Message *message, const char *input, int cursor)
{
    LIBMATTI_BR_CommandSyntaxException *exception = LIBMATTI_BR_CommandSyntaxException_New(type, message);
    exception->input = strdup(input);
    exception->cursor = cursor;
    return exception;
}

char *LIBMATTI_BR_CommandSyntaxException_GetContext(const LIBMATTI_BR_CommandSyntaxException *exception)
{
    if (exception->input == NULL || exception->cursor < 0) return NULL;

    // Java: cursor = Math.min(input.length(), this.cursor)
    int length = (int) strlen(exception->input);
    int cursor = exception->cursor < length ? exception->cursor : length;

    // Java: builder over [max(0, cursor - CONTEXT_AMOUNT), cursor) + "<--[HERE]"
    int start = cursor - LIBMATTI_BR_CommandSyntaxException_CONTEXT_AMOUNT;
    if (start < 0) start = 0;
    size_t contextLength = (size_t) (cursor - start);
    size_t total = contextLength + strlen("<--[HERE]") + 4;
    char *context = malloc(total);
    char *writer = context;
    if (cursor > LIBMATTI_BR_CommandSyntaxException_CONTEXT_AMOUNT)
    {
        memcpy(writer, "...", 3);
        writer += 3;
    }
    memcpy(writer, exception->input + start, contextLength);
    writer += contextLength;
    strcpy(writer, "<--[HERE]");
    return context;
}

char *LIBMATTI_BR_CommandSyntaxException_GetMessage(const LIBMATTI_BR_CommandSyntaxException *exception)
{
    char *message = strdup(exception->message->string);
    char *context = LIBMATTI_BR_CommandSyntaxException_GetContext(exception);
    if (context != NULL)
    {
        size_t length = strlen(message) + strlen(" at position ") + 12 + strlen(": ") + strlen(context) + 1;
        char *full = malloc(length);
        snprintf(full, length, "%s at position %d: %s", message, exception->cursor, context);
        free(message);
        free(context);
        return full;
    }
    return message;
}

void LIBMATTI_BR_CommandSyntaxException_Free(LIBMATTI_BR_CommandSyntaxException *exception)
{
    if (exception->type != NULL && exception->type->name == NULL) free(exception->type);
    free(exception->input);
    // Java: the message is owned by the exception type's template; simple types keep
    // their own. The port frees the per-exception copy only when detached.
    LIBMATTI_BR_Message_Free(exception->message);
    free(exception);
}

// ---------------------------------------------------------------------------
// SimpleCommandExceptionType
// ---------------------------------------------------------------------------

LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_SimpleCommandExceptionType_New(const char *message)
{
    LIBMATTI_BR_SimpleCommandExceptionType *type = calloc(1, sizeof(LIBMATTI_BR_SimpleCommandExceptionType));
    type->type.name = NULL; // unnamed; freed with the exception
    type->message = LIBMATTI_BR_LiteralMessage_New(message);
    return type;
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_SimpleCommandExceptionType_Create(
    const LIBMATTI_BR_SimpleCommandExceptionType *type)
{
    // Java: new CommandSyntaxException(this, message)
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    LIBMATTI_BR_Message *message = LIBMATTI_BR_LiteralMessage_New(type->message->string);
    return LIBMATTI_BR_CommandSyntaxException_New(exceptionType, message);
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
    const LIBMATTI_BR_SimpleCommandExceptionType *type, const char *input, int cursor)
{
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    LIBMATTI_BR_Message *message = LIBMATTI_BR_LiteralMessage_New(type->message->string);
    return LIBMATTI_BR_CommandSyntaxException_NewWithContext(exceptionType, message, input, cursor);
}

// ---------------------------------------------------------------------------
// DynamicCommandExceptionType
// ---------------------------------------------------------------------------

LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_DynamicCommandExceptionType_New(const char *format)
{
    LIBMATTI_BR_DynamicCommandExceptionType *type = calloc(1, sizeof(LIBMATTI_BR_DynamicCommandExceptionType));
    type->format = strdup(format);
    return type;
}

static LIBMATTI_BR_Message *apply_format(const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *arg)
{
    // Java: function.apply(arg); the port formats the single %s
    size_t length = strlen(type->format) - 2 + strlen(arg) + 1;
    char *text = malloc(length);
    snprintf(text, length, type->format, arg);
    LIBMATTI_BR_Message *message = LIBMATTI_BR_LiteralMessage_New(text);
    free(text);
    return message;
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_Create(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *arg)
{
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    return LIBMATTI_BR_CommandSyntaxException_New(exceptionType, apply_format(type, arg));
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *input, int cursor, const char *arg)
{
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    return LIBMATTI_BR_CommandSyntaxException_NewWithContext(exceptionType, apply_format(type, arg), input, cursor);
}

// Java: Dynamic2CommandExceptionType - two substitutions; the built-in messages use
// a "%s, found %s" tail, so the port takes two strings and fills both
static LIBMATTI_BR_Message *apply_format2(const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *a,
                                          const char *b)
{
    // the format carries two %s; render in two passes
    size_t length = strlen(type->format) - 4 + strlen(a) + strlen(b) + 1;
    char *text = malloc(length);
    int written = snprintf(text, length, type->format, a, b);
    (void) written;
    LIBMATTI_BR_Message *message = LIBMATTI_BR_LiteralMessage_New(text);
    free(text);
    return message;
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_Create2(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *a, const char *b)
{
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    return LIBMATTI_BR_CommandSyntaxException_New(exceptionType, apply_format2(type, a, b));
}

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *input, int cursor, const char *a, const char *b)
{
    LIBMATTI_BR_CommandExceptionType *exceptionType = malloc(sizeof(LIBMATTI_BR_CommandExceptionType));
    exceptionType->name = NULL;
    return LIBMATTI_BR_CommandSyntaxException_NewWithContext(exceptionType, apply_format2(type, a, b), input, cursor);
}

// ---------------------------------------------------------------------------
// BuiltInExceptions (lazily built singletons with the exact Java messages)
// ---------------------------------------------------------------------------

#define SIMPLE_BUILTIN(accessor, text)                                                                                 \
    static LIBMATTI_BR_SimpleCommandExceptionType *accessor##_instance = NULL;                                         \
    LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_##accessor(void)                             \
    {                                                                                                                  \
        if (accessor##_instance == NULL) accessor##_instance = LIBMATTI_BR_SimpleCommandExceptionType_New(text);       \
        return accessor##_instance;                                                                                    \
    }

#define DYNAMIC_BUILTIN(accessor, text)                                                                                \
    static LIBMATTI_BR_DynamicCommandExceptionType *accessor##_instance = NULL;                                        \
    LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_##accessor(void)                            \
    {                                                                                                                  \
        if (accessor##_instance == NULL) accessor##_instance = LIBMATTI_BR_DynamicCommandExceptionType_New(text);      \
        return accessor##_instance;                                                                                    \
    }

DYNAMIC_BUILTIN(DoubleTooLow, "Double must not be less than %s, found %s")
DYNAMIC_BUILTIN(DoubleTooHigh, "Double must not be more than %s, found %s")
DYNAMIC_BUILTIN(FloatTooLow, "Float must not be less than %s, found %s")
DYNAMIC_BUILTIN(FloatTooHigh, "Float must not be more than %s, found %s")
DYNAMIC_BUILTIN(IntegerTooLow, "Integer must not be less than %s, found %s")
DYNAMIC_BUILTIN(IntegerTooHigh, "Integer must not be more than %s, found %s")
DYNAMIC_BUILTIN(LongTooLow, "Long must not be less than %s, found %s")
DYNAMIC_BUILTIN(LongTooHigh, "Long must not be more than %s, found %s")
DYNAMIC_BUILTIN(LiteralIncorrect, "Expected literal %s")
SIMPLE_BUILTIN(ReaderExpectedStartOfQuote, "Expected quote to start a string")
SIMPLE_BUILTIN(ReaderExpectedEndOfQuote, "Unclosed quoted string")
DYNAMIC_BUILTIN(ReaderInvalidEscape, "Invalid escape sequence '%s' in quoted string")
DYNAMIC_BUILTIN(ReaderInvalidBool, "Invalid bool, expected true or false but found '%s'")
DYNAMIC_BUILTIN(ReaderInvalidInt, "Invalid integer '%s'")
SIMPLE_BUILTIN(ReaderExpectedInt, "Expected integer")
DYNAMIC_BUILTIN(ReaderInvalidLong, "Invalid long '%s'")
SIMPLE_BUILTIN(ReaderExpectedLong, "Expected long")
DYNAMIC_BUILTIN(ReaderInvalidDouble, "Invalid double '%s'")
SIMPLE_BUILTIN(ReaderExpectedDouble, "Expected double")
DYNAMIC_BUILTIN(ReaderInvalidFloat, "Invalid float '%s'")
SIMPLE_BUILTIN(ReaderExpectedFloat, "Expected float")
SIMPLE_BUILTIN(ReaderExpectedBool, "Expected bool")
DYNAMIC_BUILTIN(ReaderExpectedSymbol, "Expected '%s'")
SIMPLE_BUILTIN(DispatcherUnknownCommand, "Unknown command")
SIMPLE_BUILTIN(DispatcherUnknownArgument, "Incorrect argument for command")
SIMPLE_BUILTIN(DispatcherExpectedArgumentSeparator,
               "Expected whitespace to end one argument, but found trailing data")
DYNAMIC_BUILTIN(DispatcherParseException, "Could not parse command: %s")
