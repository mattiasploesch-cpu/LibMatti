// Brigadier harness: registers a small command tree, checks parsing, execution,
// arguments, usage and the redirect behaviour against the Java semantics.

#include "libmatti/com/mojang/brigadier/CommandDispatcher.h"
#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"
#include "libmatti/com/mojang/brigadier/suggestion/Suggestions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(condition, name)                                                                                          \
    do                                                                                                                  \
    {                                                                                                                   \
        checks++;                                                                                                       \
        if (!(condition))                                                                                               \
        {                                                                                                               \
            failures++;                                                                                                 \
            printf("FAIL: %s\n", name);                                                                                 \
        }                                                                                                               \
    } while (0)

static int ran_command = 0;
static int last_int_arg = 0;
static char last_string_arg[128] = {0};
static int last_bool_arg = -1;

static int command_int_run(LIBMATTI_BR_CommandContext *context, void *userdata,
                           LIBMATTI_BR_CommandSyntaxException **outException)
{
    (void) userdata;
    (void) outException;
    const LIBMATTI_BR_ParsedArgument *value =
        LIBMATTI_BR_CommandContext_GetArgument(context, "value", LIBMATTI_BR_ARG_INTEGER);
    last_int_arg = value != NULL ? value->result.intValue : -1;
    ran_command = 1;
    return last_int_arg;
}

static int command_string_run(LIBMATTI_BR_CommandContext *context, void *userdata,
                              LIBMATTI_BR_CommandSyntaxException **outException)
{
    (void) userdata;
    (void) outException;
    const LIBMATTI_BR_ParsedArgument *value =
        LIBMATTI_BR_CommandContext_GetArgument(context, "text", LIBMATTI_BR_ARG_STRING);
    if (value != NULL)
    {
        snprintf(last_string_arg, sizeof(last_string_arg), "%s", value->result.stringValue);
    }
    ran_command = 1;
    return 1;
}

static int command_bool_run(LIBMATTI_BR_CommandContext *context, void *userdata,
                            LIBMATTI_BR_CommandSyntaxException **outException)
{
    (void) userdata;
    (void) outException;
    const LIBMATTI_BR_ParsedArgument *value =
        LIBMATTI_BR_CommandContext_GetArgument(context, "flag", LIBMATTI_BR_ARG_BOOL);
    last_bool_arg = value != NULL ? value->result.boolValue : -1;
    ran_command = 1;
    return 1;
}

static int command_simple_run(LIBMATTI_BR_CommandContext *context, void *userdata,
                              LIBMATTI_BR_CommandSyntaxException **outException)
{
    (void) context;
    (void) userdata;
    (void) outException;
    ran_command = 1;
    return 42;
}

int main(void)
{
    LIBMATTI_BR_CommandDispatcher *dispatcher = LIBMATTI_BR_CommandDispatcher_New();

    // register("int", argument("value", integer(0, 100)).executes(...))
    LIBMATTI_BR_ArgumentBuilder *intCommand = LIBMATTI_BR_LiteralArgumentBuilder_Literal("int");
    LIBMATTI_BR_ArgumentBuilder *intArg =
        LIBMATTI_BR_RequiredArgumentBuilder_Argument("value", LIBMATTI_BR_IntegerArgumentType_IntegerMinMax(0, 100));
    LIBMATTI_BR_ArgumentBuilder_Executes(intArg, command_int_run, NULL);
    LIBMATTI_BR_ArgumentBuilder_ThenBuilder(intCommand, intArg);
    LIBMATTI_BR_CommandDispatcher_Register(dispatcher, intCommand);

    // register("say", argument("text", greedyString()).executes(...))
    LIBMATTI_BR_ArgumentBuilder *sayCommand = LIBMATTI_BR_LiteralArgumentBuilder_Literal("say");
    LIBMATTI_BR_ArgumentBuilder *sayArg =
        LIBMATTI_BR_RequiredArgumentBuilder_Argument("text", LIBMATTI_BR_StringArgumentType_GreedyString());
    LIBMATTI_BR_ArgumentBuilder_Executes(sayArg, command_string_run, NULL);
    LIBMATTI_BR_ArgumentBuilder_ThenBuilder(sayCommand, sayArg);
    LIBMATTI_BR_CommandDispatcher_Register(dispatcher, sayCommand);

    // register("flag", argument("flag", bool()).executes(...))
    LIBMATTI_BR_ArgumentBuilder *flagCommand = LIBMATTI_BR_LiteralArgumentBuilder_Literal("flag");
    LIBMATTI_BR_ArgumentBuilder *flagArg =
        LIBMATTI_BR_RequiredArgumentBuilder_Argument("flag", LIBMATTI_BR_BoolArgumentType_Bool());
    LIBMATTI_BR_ArgumentBuilder_Executes(flagArg, command_bool_run, NULL);
    LIBMATTI_BR_ArgumentBuilder_ThenBuilder(flagCommand, flagArg);
    LIBMATTI_BR_CommandDispatcher_Register(dispatcher, flagCommand);

    // register("ping").executes(...)
    LIBMATTI_BR_ArgumentBuilder *pingCommand = LIBMATTI_BR_LiteralArgumentBuilder_Literal("ping");
    LIBMATTI_BR_ArgumentBuilder_Executes(pingCommand, command_simple_run, NULL);
    LIBMATTI_BR_CommandDispatcher_Register(dispatcher, pingCommand);

    int source = 0;
    LIBMATTI_BR_CommandSyntaxException *exception = NULL;

    // --- literal execution
    ran_command = 0;
    int result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "ping", &source, &exception);
    CHECK(ran_command, "literal executes");
    CHECK(exception == NULL, "literal no exception");
    CHECK(result == 42, "literal result");

    // --- integer argument
    ran_command = 0;
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "int 57", &source, &exception);
    CHECK(ran_command, "int executes");
    CHECK(exception == NULL, "int no exception");
    CHECK(result == 57, "int result");
    CHECK(last_int_arg == 57, "int argument value");

    // --- integer out of range (Java: throws IntegerTooHigh)
    ran_command = 0;
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "int 999", &source, &exception);
    CHECK(!ran_command, "int out of range does not execute");
    CHECK(exception != NULL, "int out of range throws");
    if (exception != NULL)
    {
        char *message = LIBMATTI_BR_CommandSyntaxException_GetMessage(exception);
        CHECK(strstr(message, "Integer must not be more than 100") != NULL, "int too high message");
        free(message);
        LIBMATTI_BR_CommandSyntaxException_Free(exception);
        exception = NULL;
    }

    // --- greedy string argument with quotes
    ran_command = 0;
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "say hello world", &source, &exception);
    CHECK(ran_command, "greedy string executes");
    CHECK(strcmp(last_string_arg, "hello world") == 0, "greedy string value");

    // --- quoted string: GREEDY_PHRASE takes the remaining input RAW (Java
    // StringArgumentType.parse), so the quotes stay - like Java's own behaviour
    ran_command = 0;
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "say \"quoted text\"", &source, &exception);
    CHECK(exception == NULL, "quoted string no exception");
    CHECK(strcmp(last_string_arg, "\"quoted text\"") == 0, "greedy string keeps quotes raw");

    // --- bool argument
    ran_command = 0;
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "flag true", &source, &exception);
    CHECK(ran_command && exception == NULL, "bool executes");
    CHECK(last_bool_arg == 1, "bool value true");

    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "flag maybe", &source, &exception);
    CHECK(exception != NULL, "invalid bool throws");
    if (exception != NULL)
    {
        LIBMATTI_BR_CommandSyntaxException_Free(exception);
        exception = NULL;
    }

    // --- unknown command (Java: dispatcherUnknownCommand)
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "frob", &source, &exception);
    CHECK(result == 0 && exception != NULL, "unknown command throws");
    if (exception != NULL)
    {
        char *message = LIBMATTI_BR_CommandSyntaxException_GetMessage(exception);
        CHECK(strstr(message, "Unknown command") == message, "unknown command message");
        free(message);
        LIBMATTI_BR_CommandSyntaxException_Free(exception);
        exception = NULL;
    }

    // --- trailing data: "pingx" never enters the separator check - the literal fast
    // path (getRelevantNodes) misses, so Java throws dispatcherUnknownCommand
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "pingx", &source, &exception);
    CHECK(exception != NULL, "trailing data throws");
    if (exception != NULL)
    {
        char *message = LIBMATTI_BR_CommandSyntaxException_GetMessage(exception);
        CHECK(strstr(message, "Unknown command") == message, "unknown command message");
        free(message);
        LIBMATTI_BR_CommandSyntaxException_Free(exception);
        exception = NULL;
    }

    // --- trailing data: "pingx" never enters the separator check - the literal fast
    // path (getRelevantNodes) misses, so Java throws dispatcherUnknownCommand

    // --- parse results: a valid parse leaves the reader exhausted
    LIBMATTI_BR_ParseResults *parse = LIBMATTI_BR_CommandDispatcher_ParseString(dispatcher, "ping", &source);
    CHECK(!LIBMATTI_BR_StringReader_CanRead(&parse->reader), "valid parse consumes input");
    LIBMATTI_BR_ParseResults_Free(parse);

    // --- trailing whitespace: Java stops at the literal end and reports
    // dispatcherUnknownArgument at position 4
    exception = NULL;
    result = LIBMATTI_BR_CommandDispatcher_ExecuteString(dispatcher, "ping ", &source, &exception);
    CHECK(result == 0 && exception != NULL, "trailing whitespace throws");
    if (exception != NULL)
    {
        char *message = LIBMATTI_BR_CommandSyntaxException_GetMessage(exception);
        CHECK(strstr(message, "Incorrect argument for command") != NULL, "unknown argument message");
        free(message);
        LIBMATTI_BR_CommandSyntaxException_Free(exception);
        exception = NULL;
    }

    // --- usage strings
    size_t usageCount = 0;
    char **usage = LIBMATTI_BR_CommandDispatcher_GetAllUsage(dispatcher,
                                                             LIBMATTI_BR_CommandDispatcher_GetRoot(dispatcher), &source,
                                                             1, &usageCount);
    // Java: getAllUsage(root) lists one line per executable path:
    // int <value>, say <text>, flag <flag>, ping
    CHECK(usageCount == 4, "usage lists every executable path");
    for (size_t i = 0; i < usageCount; i++)
    {
        free(usage[i]);
    }
    free(usage);

    LIBMATTI_BR_CommandDispatcher_Free(dispatcher);

    printf("brigadier: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
