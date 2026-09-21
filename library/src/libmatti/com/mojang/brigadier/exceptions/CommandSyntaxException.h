// Port of com.mojang.brigadier.exceptions.*:
// CommandExceptionType, CommandSyntaxException, SimpleCommandExceptionType,
// DynamicCommandExceptionType (the 2/3/4-arg variants fold into the same shape),
// and BuiltInExceptions.
//
// Java throws; the C port returns a malloc'd LIBMATTI_BR_CommandSyntaxException
// through an out-parameter and the caller frees it.

#ifndef MATTICRAFT_BR_EXCEPTIONS_COMMANDEXCEPTION_H
#define MATTICRAFT_BR_EXCEPTIONS_COMMANDEXCEPTION_H

#include "libmatti/com/mojang/brigadier/Message.h"

// Java: public interface CommandExceptionType
typedef struct LIBMATTI_BR_CommandExceptionType
{
    const char *name;
} LIBMATTI_BR_CommandExceptionType;

// Java: public class CommandSyntaxException extends Exception
typedef struct LIBMATTI_BR_CommandSyntaxException
{
    LIBMATTI_BR_CommandExceptionType *type;
    LIBMATTI_BR_Message *message;
    // Java: private final String input (NULL when created without context)
    char *input;
    int cursor;
} LIBMATTI_BR_CommandSyntaxException;

// Java: CONTEXT_AMOUNT
#define LIBMATTI_BR_CommandSyntaxException_CONTEXT_AMOUNT 10

LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_CommandSyntaxException_New(LIBMATTI_BR_CommandExceptionType *type,
                                                                           LIBMATTI_BR_Message *message);
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_CommandSyntaxException_NewWithContext(
    LIBMATTI_BR_CommandExceptionType *type, LIBMATTI_BR_Message *message, const char *input, int cursor);
// Java: public String getMessage() - the caller frees
char *LIBMATTI_BR_CommandSyntaxException_GetMessage(const LIBMATTI_BR_CommandSyntaxException *exception);
// Java: public String getContext() - the caller frees, NULL when no input
char *LIBMATTI_BR_CommandSyntaxException_GetContext(const LIBMATTI_BR_CommandSyntaxException *exception);
void LIBMATTI_BR_CommandSyntaxException_Free(LIBMATTI_BR_CommandSyntaxException *exception);

// Java: public class SimpleCommandExceptionType implements CommandExceptionType
typedef struct LIBMATTI_BR_SimpleCommandExceptionType
{
    LIBMATTI_BR_CommandExceptionType type;
    LIBMATTI_BR_Message *message;
} LIBMATTI_BR_SimpleCommandExceptionType;

LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_SimpleCommandExceptionType_New(const char *message);
// Java: public CommandSyntaxException create() / createWithContext(ImmutableStringReader)
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_SimpleCommandExceptionType_Create(
    const LIBMATTI_BR_SimpleCommandExceptionType *type);
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
    const LIBMATTI_BR_SimpleCommandExceptionType *type, const char *input, int cursor);

// Java: public class DynamicCommandExceptionType implements CommandExceptionType
// Java's function application becomes a printf-style format at create() time.
typedef struct LIBMATTI_BR_DynamicCommandExceptionType
{
    LIBMATTI_BR_CommandExceptionType type;
    // Java: Function<Object, Message> function - the port formats a single "%s"
    char *format;
} LIBMATTI_BR_DynamicCommandExceptionType;

LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_DynamicCommandExceptionType_New(const char *format);
// Java: public CommandSyntaxException create(Object arg) / createWithContext(reader, arg)
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_Create(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *arg);
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *input, int cursor, const char *arg);
// Java: Dynamic2CommandExceptionType create(a, b) - the port's two-argument format
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_Create2(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *a, const char *b);
LIBMATTI_BR_CommandSyntaxException *LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext2(
    const LIBMATTI_BR_DynamicCommandExceptionType *type, const char *input, int cursor, const char *a, const char *b);

// Java: public class BuiltInExceptions implements BuiltInExceptionProvider
// The port exposes the accessor functions directly; the exception types are
// lazily built singletons.
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DoubleTooLow(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DoubleTooHigh(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_FloatTooLow(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_FloatTooHigh(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_IntegerTooLow(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_IntegerTooHigh(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_LongTooLow(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_LongTooHigh(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_LiteralIncorrect(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedStartOfQuote(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedEndOfQuote(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidEscape(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidBool(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidInt(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedInt(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidLong(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedLong(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidDouble(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedDouble(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderInvalidFloat(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedFloat(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedBool(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_ReaderExpectedSymbol(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DispatcherUnknownCommand(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DispatcherUnknownArgument(void);
LIBMATTI_BR_SimpleCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DispatcherExpectedArgumentSeparator(void);
LIBMATTI_BR_DynamicCommandExceptionType *LIBMATTI_BR_BuiltInExceptions_DispatcherParseException(void);

#endif
