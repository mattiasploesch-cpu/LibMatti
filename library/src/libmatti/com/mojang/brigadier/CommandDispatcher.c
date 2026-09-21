// Port of com.mojang.brigadier.CommandDispatcher.

#include "libmatti/com/mojang/brigadier/CommandDispatcher.h"

#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"
#include "libmatti/com/mojang/brigadier/suggestion/Suggestions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: ARGUMENT_SEPARATOR_CHAR
#define ARGUMENT_SEPARATOR_CHAR ' '
#define USAGE_OPTIONAL_OPEN "["
#define USAGE_OPTIONAL_CLOSE "]"
#define USAGE_REQUIRED_OPEN "("
#define USAGE_REQUIRED_CLOSE ")"
#define USAGE_OR "|"

LIBMATTI_BR_CommandDispatcher *LIBMATTI_BR_CommandDispatcher_NewWithRoot(LIBMATTI_BR_CommandNode *root)
{
    LIBMATTI_BR_CommandDispatcher *self = calloc(1, sizeof(LIBMATTI_BR_CommandDispatcher));
    self->root = root;
    return self;
}

LIBMATTI_BR_CommandDispatcher *LIBMATTI_BR_CommandDispatcher_New(void)
{
    return LIBMATTI_BR_CommandDispatcher_NewWithRoot(LIBMATTI_BR_RootCommandNode_New());
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandDispatcher_Register(LIBMATTI_BR_CommandDispatcher *self,
                                                                LIBMATTI_BR_ArgumentBuilder *command)
{
    // Java: final LiteralCommandNode<S> build = command.build(); root.addChild(build);
    LIBMATTI_BR_CommandNode *build = LIBMATTI_BR_ArgumentBuilder_Build(command);
    LIBMATTI_BR_CommandNode_AddChild(self->root, build);
    LIBMATTI_BR_ArgumentBuilder_Free(command);
    return build;
}

void LIBMATTI_BR_CommandDispatcher_SetConsumer(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_ResultConsumer consumer,
                                               void *userdata)
{
    self->consumer = consumer;
    self->consumerUserdata = userdata;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandDispatcher_GetRoot(const LIBMATTI_BR_CommandDispatcher *self)
{
    return self->root;
}

// Java: private ParseResults<S> parseNodes(node, originalReader, contextSoFar)
static LIBMATTI_BR_ParseResults *parse_nodes(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node,
                                             LIBMATTI_BR_StringReader *originalReader,
                                             LIBMATTI_BR_CommandContextBuilder *contextSoFar)
{
    const void *source = LIBMATTI_BR_CommandContextBuilder_GetSource(contextSoFar);
    LIBMATTI_BR_CommandNode **errorNodes = NULL;
    LIBMATTI_BR_CommandSyntaxException **errors = NULL;
    size_t errorCount = 0;

    // Java: List<ParseResults<S>> potentials = null
    LIBMATTI_BR_ParseResults *potentials[64];
    size_t potentialCount = 0;
    const int cursor = LIBMATTI_BR_StringReader_GetCursor(originalReader);

    // Java: for (final CommandNode<S> child : node.getRelevantNodes(originalReader))
    for (LIBMATTI_BR_CommandNodeChild *childEntry =
             LIBMATTI_BR_CommandNode_GetRelevantNodes(node, originalReader);
         childEntry != NULL; childEntry = childEntry->next)
    {
        LIBMATTI_BR_CommandNode *child = childEntry->node;
        if (!LIBMATTI_BR_CommandNode_CanUse(child, source)) continue;

        LIBMATTI_BR_CommandContextBuilder *context = LIBMATTI_BR_CommandContextBuilder_Copy(contextSoFar);
        LIBMATTI_BR_StringReader reader = LIBMATTI_BR_StringReader_Copy(originalReader);
        LIBMATTI_BR_CommandSyntaxException *exception = NULL;

        // Java: child.parse(reader, context) wrapped in the RuntimeException guard
        if (child->isLiteral)
        {
            // LiteralCommandNode.parse
            int start = LIBMATTI_BR_StringReader_GetCursor(&reader);
            int matched = -1;
            int literalLength = (int) strlen(child->literal);
            if (LIBMATTI_BR_StringReader_CanReadLength(&reader, literalLength))
            {
                int end = start + literalLength;
                if (strncmp(LIBMATTI_BR_StringReader_GetString(&reader) + start, child->literal, literalLength) == 0)
                {
                    LIBMATTI_BR_StringReader_SetCursor(&reader, end);
                    if (!LIBMATTI_BR_StringReader_CanRead(&reader) ||
                        LIBMATTI_BR_StringReader_Peek(&reader) == ' ')
                    {
                        matched = end;
                    }
                    else
                    {
                        LIBMATTI_BR_StringReader_SetCursor(&reader, start);
                    }
                }
            }
            if (matched > -1)
            {
                LIBMATTI_BR_CommandContextBuilder_WithNode(context, child,
                                                           LIBMATTI_BR_StringRange_Between(start, matched));
            }
            else
            {
                char literal[128];
                snprintf(literal, sizeof(literal), "%s", child->literal);
                exception = LIBMATTI_BR_DynamicCommandExceptionType_CreateWithContext(
                    LIBMATTI_BR_BuiltInExceptions_LiteralIncorrect(), reader.string, reader.cursor, literal);
            }
        }
        else
        {
            // ArgumentCommandNode.parse
            int start = LIBMATTI_BR_StringReader_GetCursor(&reader);
            LIBMATTI_BR_ArgumentKind kind = child->type.kind;
            LIBMATTI_BR_ArgumentValue value;
            memset(&value, 0, sizeof(value));
            int parseOk = 1;
            switch (kind)
            {
                case LIBMATTI_BR_ARG_BOOL:
                    value.boolValue = LIBMATTI_BR_ArgumentType_ParseBool(&child->type, &reader, &exception);
                    break;
                case LIBMATTI_BR_ARG_INTEGER:
                    value.intValue = LIBMATTI_BR_ArgumentType_ParseInt(&child->type, &reader, &exception);
                    break;
                case LIBMATTI_BR_ARG_LONG:
                    value.longValue = LIBMATTI_BR_ArgumentType_ParseLong(&child->type, &reader, &exception);
                    break;
                case LIBMATTI_BR_ARG_FLOAT:
                    value.floatValue = LIBMATTI_BR_ArgumentType_ParseFloat(&child->type, &reader, &exception);
                    break;
                case LIBMATTI_BR_ARG_DOUBLE:
                    value.doubleValue = LIBMATTI_BR_ArgumentType_ParseDouble(&child->type, &reader, &exception);
                    break;
                case LIBMATTI_BR_ARG_STRING:
                {
                    char *text = LIBMATTI_BR_ArgumentType_ParseString(&child->type, &reader, &exception);
                    value.stringValue = text;
                    break;
                }
            }
            if (exception == NULL)
            {
                LIBMATTI_BR_ParsedArgument *parsed =
                    LIBMATTI_BR_ParsedArgument_New(start, LIBMATTI_BR_StringReader_GetCursor(&reader), kind, value);
                LIBMATTI_BR_CommandContextBuilder_WithArgument(context, child->name, parsed);
                LIBMATTI_BR_CommandContextBuilder_WithNode(context, child,
                                                           *LIBMATTI_BR_ParsedArgument_GetRange(parsed));
            }
            else
            {
                parseOk = 0;
                if (kind == LIBMATTI_BR_ARG_STRING && value.stringValue != NULL)
                {
                    // the parse partially consumed an owned string
                    free(value.stringValue);
                }
            }
            (void) parseOk;
        }

        if (exception == NULL && LIBMATTI_BR_StringReader_CanRead(&reader))
        {
            if (LIBMATTI_BR_StringReader_Peek(&reader) != ARGUMENT_SEPARATOR_CHAR)
            {
                exception = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
                    LIBMATTI_BR_BuiltInExceptions_DispatcherExpectedArgumentSeparator(), reader.string, reader.cursor);
            }
        }

        if (exception != NULL)
        {
            // Java: errors.put(child, ex); reader.setCursor(cursor); continue;
            errorNodes = realloc(errorNodes, (errorCount + 1) * sizeof(LIBMATTI_BR_CommandNode *));
            errors = realloc(errors, (errorCount + 1) * sizeof(LIBMATTI_BR_CommandSyntaxException *));
            errorNodes[errorCount] = child;
            errors[errorCount] = exception;
            errorCount++;
            LIBMATTI_BR_StringReader_SetCursor(&reader, cursor);
            // the copied context is dropped (Java lets it go out of scope)
            // the moved parsed arguments belong to the copy; free it
            for (size_t i = 0; i < context->argumentCount; i++)
            {
                free(context->argumentNames[i]);
                context->argumentNames[i] = NULL;
            }
            free(context->argumentNames);
            free(context->argumentValues);
            free(context->nodes);
            context->argumentNames = NULL;
            context->argumentValues = NULL;
            context->nodes = NULL;
            context->argumentCount = 0;
            context->nodeCount = 0;
            LIBMATTI_BR_CommandContextBuilder_Free(context);
            continue;
        }

        LIBMATTI_BR_CommandContextBuilder_WithCommand(context, LIBMATTI_BR_CommandNode_GetCommand(child),
                                                      child->commandUserdata);

        // Java: if (reader.canRead(child.getRedirect() == null ? 2 : 1))
        if (LIBMATTI_BR_StringReader_CanReadLength(&reader, child->redirect == NULL ? 2 : 1))
        {
            LIBMATTI_BR_StringReader_Skip(&reader);
            if (child->redirect != NULL)
            {
                LIBMATTI_BR_CommandContextBuilder *childContext = LIBMATTI_BR_CommandContextBuilder_New(
                    self, source, child->redirect, LIBMATTI_BR_StringReader_GetCursor(&reader));
                LIBMATTI_BR_ParseResults *parse = parse_nodes(self, child->redirect, &reader, childContext);
                LIBMATTI_BR_CommandContextBuilder_WithChild(context, parse->context);
                // the port moves the parse's context into this one and returns
                parse->context = context;
                LIBMATTI_BR_ParseResults_Free(parse);
                LIBMATTI_BR_ParseResults *result = calloc(1, sizeof(LIBMATTI_BR_ParseResults));
                result->context = context;
                result->reader = reader;
                return result;
            }
            LIBMATTI_BR_ParseResults *parse = parse_nodes(self, child, &reader, context);
            if (potentialCount < sizeof(potentials) / sizeof(potentials[0]))
            {
                potentials[potentialCount++] = parse;
            }
        }
        else
        {
            LIBMATTI_BR_ParseResults *parse = calloc(1, sizeof(LIBMATTI_BR_ParseResults));
            parse->context = context;
            parse->reader = reader;
            if (potentialCount < sizeof(potentials) / sizeof(potentials[0]))
            {
                potentials[potentialCount++] = parse;
            }
        }
    }

    // Java: if (potentials != null) { sort; return potentials.get(0); }
    if (potentialCount > 0)
    {
        if (potentialCount > 1)
        {
            // Java: the comparator - complete parses first, then exception-free parses
            for (size_t i = 0; i + 1 < potentialCount; i++)
            {
                for (size_t j = 0; j + 1 < potentialCount - i; j++)
                {
                    LIBMATTI_BR_ParseResults *a = potentials[j];
                    LIBMATTI_BR_ParseResults *b = potentials[j + 1];
                    int aCanRead = LIBMATTI_BR_StringReader_CanRead(&a->reader);
                    int bCanRead = LIBMATTI_BR_StringReader_CanRead(&b->reader);
                    int weight = 0;
                    if (!aCanRead && bCanRead) weight = -1;
                    else if (aCanRead && !bCanRead) weight = 1;
                    else if (a->errorCount == 0 && b->errorCount > 0) weight = -1;
                    else if (a->errorCount > 0 && b->errorCount == 0) weight = 1;
                    if (weight > 0)
                    {
                        LIBMATTI_BR_ParseResults *swap = potentials[j];
                        potentials[j] = potentials[j + 1];
                        potentials[j + 1] = swap;
                    }
                }
            }
        }
        LIBMATTI_BR_ParseResults *best = potentials[0];
        for (size_t i = 1; i < potentialCount; i++)
        {
            LIBMATTI_BR_ParseResults_Free(potentials[i]);
        }
        // free the error arrays the winning parse does not carry
        for (size_t i = 0; i < errorCount; i++)
        {
            LIBMATTI_BR_CommandSyntaxException_Free(errors[i]);
        }
        free(errors);
        free(errorNodes);
        return best;
    }

    LIBMATTI_BR_ParseResults *result = calloc(1, sizeof(LIBMATTI_BR_ParseResults));
    result->context = contextSoFar;
    result->reader = *originalReader;
    result->errorNodes = errorNodes;
    result->errors = errors;
    result->errorCount = errorCount;
    return result;
}

LIBMATTI_BR_ParseResults *LIBMATTI_BR_CommandDispatcher_Parse(LIBMATTI_BR_CommandDispatcher *self,
                                                              LIBMATTI_BR_StringReader *command, const void *source)
{
    LIBMATTI_BR_CommandContextBuilder *context =
        LIBMATTI_BR_CommandContextBuilder_New(self, source, self->root, LIBMATTI_BR_StringReader_GetCursor(command));
    return parse_nodes(self, self->root, command, context);
}

LIBMATTI_BR_ParseResults *LIBMATTI_BR_CommandDispatcher_ParseString(LIBMATTI_BR_CommandDispatcher *self,
                                                                    const char *command, const void *source)
{
    LIBMATTI_BR_StringReader reader = LIBMATTI_BR_StringReader_Of(command);
    return LIBMATTI_BR_CommandDispatcher_Parse(self, &reader, source);
}

int LIBMATTI_BR_CommandDispatcher_Execute(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_ParseResults *parse,
                                          LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    // Java: if (parse.getReader().canRead()) { ... }
    if (LIBMATTI_BR_StringReader_CanRead(&parse->reader))
    {
        if (parse->errorCount == 1)
        {
            *outException = parse->errors[0];
            parse->errors[0] = NULL;
            parse->errorCount = 0;
            return 0;
        }
        if (LIBMATTI_BR_StringRange_IsEmpty(&parse->context->range))
        {
            *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
                LIBMATTI_BR_BuiltInExceptions_DispatcherUnknownCommand(), parse->reader.string,
                parse->reader.cursor);
        }
        else
        {
            *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
                LIBMATTI_BR_BuiltInExceptions_DispatcherUnknownArgument(), parse->reader.string,
                parse->reader.cursor);
        }
        return 0;
    }

    // Java: final String command = parse.getReader().getString();
    //       final CommandContext<S> original = parse.getContext().build(command);
    LIBMATTI_BR_CommandContext *original =
        LIBMATTI_BR_CommandContextBuilder_Build(parse->context, LIBMATTI_BR_StringReader_GetString(&parse->reader));

    // Java: ContextChain.tryFlatten(original) - walk the child chain; the last entry
    // must be executable
    int result = 0;
    // collect the modifiers (contexts with a child) and the executable
    LIBMATTI_BR_CommandContext *modifiers[64];
    size_t modifierCount = 0;
    LIBMATTI_BR_CommandContext *current = original;
    int flattenOk = 0;
    while (1)
    {
        LIBMATTI_BR_CommandContext *child = LIBMATTI_BR_CommandContext_GetChild(current);
        if (child == NULL)
        {
            if (LIBMATTI_BR_CommandContext_GetCommand(current) != NULL) flattenOk = 1;
            break;
        }
        if (modifierCount < sizeof(modifiers) / sizeof(modifiers[0]))
        {
            modifiers[modifierCount++] = current;
        }
        current = child;
    }

    if (!flattenOk)
    {
        if (self->consumer != NULL)
        {
            self->consumer(original, 0, 0, self->consumerUserdata);
        }
        *outException = LIBMATTI_BR_SimpleCommandExceptionType_CreateWithContext(
            LIBMATTI_BR_BuiltInExceptions_DispatcherUnknownCommand(), parse->reader.string, parse->reader.cursor);
        LIBMATTI_BR_CommandContext_Free(original);
        return 0;
    }

    // Java: executeAll - the port runs the executable (the modifier stages carry no
    // source lists in the C port, so a redirect without modifier just propagates)
    (void) modifiers;
    (void) modifierCount;
    LIBMATTI_BR_CommandContext *executable = LIBMATTI_BR_CommandContext_GetLastChild(original);
    LIBMATTI_BR_Command command = LIBMATTI_BR_CommandContext_GetCommand(executable);
    if (command != NULL)
    {
        result = command(executable, LIBMATTI_BR_CommandContext_GetCommandUserdata(executable), outException);
        if (self->consumer != NULL)
        {
            self->consumer(executable, *outException == NULL, result, self->consumerUserdata);
        }
    }

    LIBMATTI_BR_CommandContext_Free(original);
    return result;
}

int LIBMATTI_BR_CommandDispatcher_ExecuteReader(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_StringReader *input,
                                                const void *source, LIBMATTI_BR_CommandSyntaxException **outException)
{
    *outException = NULL;
    LIBMATTI_BR_ParseResults *parse = LIBMATTI_BR_CommandDispatcher_Parse(self, input, source);
    int result = LIBMATTI_BR_CommandDispatcher_Execute(self, parse, outException);
    LIBMATTI_BR_ParseResults_Free(parse);
    return result;
}

int LIBMATTI_BR_CommandDispatcher_ExecuteString(LIBMATTI_BR_CommandDispatcher *self, const char *input,
                                                const void *source, LIBMATTI_BR_CommandSyntaxException **outException)
{
    LIBMATTI_BR_StringReader reader = LIBMATTI_BR_StringReader_Of(input);
    return LIBMATTI_BR_CommandDispatcher_ExecuteReader(self, &reader, source, outException);
}

// Java: private void getAllUsage(node, source, result, prefix, restricted)
static void get_all_usage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node, const void *source,
                          char ***result, size_t *count, const char *prefix, int restricted)
{
    if (restricted && !LIBMATTI_BR_CommandNode_CanUse(node, source)) return;

    if (LIBMATTI_BR_CommandNode_GetCommand(node) != NULL)
    {
        *result = realloc(*result, (*count + 1) * sizeof(char *));
        (*result)[(*count)++] = strdup(prefix);
    }

    if (node->redirect != NULL)
    {
        // Java: node.getRedirect() == root ? "..." : "-> " + node.getRedirect().getUsageText()
        char *redirect;
        if (node->redirect == self->root)
        {
            redirect = strdup("...");
        }
        else
        {
            char *usageText = LIBMATTI_BR_CommandNode_GetUsageText(node->redirect);
            redirect = malloc(strlen("-> ") + strlen(usageText) + 1);
            sprintf(redirect, "-> %s", usageText);
            free(usageText);
        }
        char *usageText = LIBMATTI_BR_CommandNode_GetUsageText(node);
        size_t length = strlen(prefix) + strlen(usageText) + strlen(redirect) + 3;
        char *line = malloc(length);
        if (prefix[0] == '\0')
        {
            snprintf(line, length, "%s %s", usageText, redirect);
        }
        else
        {
            snprintf(line, length, "%s %s", prefix, redirect);
        }
        *result = realloc(*result, (*count + 1) * sizeof(char *));
        (*result)[(*count)++] = line;
        free(usageText);
        free(redirect);
    }
    else if (node->childCount > 0)
    {
        for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
             child = child->next)
        {
            char *childUsage = LIBMATTI_BR_CommandNode_GetUsageText(child->node);
            size_t length = strlen(prefix) + strlen(childUsage) + 2;
            char *nextPrefix = malloc(length);
            if (prefix[0] == '\0')
            {
                snprintf(nextPrefix, length, "%s", childUsage);
            }
            else
            {
                snprintf(nextPrefix, length, "%s %s", prefix, childUsage);
            }
            get_all_usage(self, child->node, source, result, count, nextPrefix, restricted);
            free(nextPrefix);
            free(childUsage);
        }
    }
}

char **LIBMATTI_BR_CommandDispatcher_GetAllUsage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node,
                                                 const void *source, int restricted, size_t *outCount)
{
    char **result = NULL;
    size_t count = 0;
    get_all_usage(self, node, source, &result, &count, "", restricted);
    result = realloc(result, (count + 1) * sizeof(char *));
    result[count] = NULL;
    *outCount = count;
    return result;
}

// Java: private String getSmartUsage(node, source, optional, deep)
static char *get_smart_usage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node, const void *source,
                             int optional, int deep)
{
    if (!LIBMATTI_BR_CommandNode_CanUse(node, source)) return NULL;

    char *nodeUsage = LIBMATTI_BR_CommandNode_GetUsageText(node);
    char *selfUsage;
    if (optional)
    {
        size_t length = strlen(nodeUsage) + 3;
        selfUsage = malloc(length);
        snprintf(selfUsage, length, "[%s]", nodeUsage);
        free(nodeUsage);
    }
    else
    {
        selfUsage = nodeUsage;
    }

    int childOptional = LIBMATTI_BR_CommandNode_GetCommand(node) != NULL;
    const char *open = childOptional ? USAGE_OPTIONAL_OPEN : USAGE_REQUIRED_OPEN;
    const char *close = childOptional ? USAGE_OPTIONAL_CLOSE : USAGE_REQUIRED_CLOSE;

    if (!deep)
    {
        if (node->redirect != NULL)
        {
            char *redirect;
            if (node->redirect == self->root)
            {
                redirect = strdup("...");
            }
            else
            {
                char *usageText = LIBMATTI_BR_CommandNode_GetUsageText(node->redirect);
                redirect = malloc(strlen("-> ") + strlen(usageText) + 1);
                sprintf(redirect, "-> %s", usageText);
                free(usageText);
            }
            size_t length = strlen(selfUsage) + strlen(redirect) + 2;
            char *result = malloc(length);
            snprintf(result, length, "%s %s", selfUsage, redirect);
            free(selfUsage);
            free(redirect);
            return result;
        }

        // count the usable children
        size_t usableCount = 0;
        for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
             child = child->next)
        {
            if (LIBMATTI_BR_CommandNode_CanUse(child->node, source)) usableCount++;
        }
        if (usableCount == 1)
        {
            for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
                 child = child->next)
            {
                if (!LIBMATTI_BR_CommandNode_CanUse(child->node, source)) continue;
                char *usage = get_smart_usage(self, child->node, source, childOptional, childOptional);
                if (usage != NULL)
                {
                    size_t length = strlen(selfUsage) + strlen(usage) + 2;
                    char *result = malloc(length);
                    snprintf(result, length, "%s %s", selfUsage, usage);
                    free(selfUsage);
                    free(usage);
                    return result;
                }
                free(selfUsage);
                return NULL;
            }
        }
        else if (usableCount > 1)
        {
            // Java: collect the child usages; equal usages collapse into one entry
            char **childUsages = NULL;
            size_t childUsageCount = 0;
            for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
                 child = child->next)
            {
                if (!LIBMATTI_BR_CommandNode_CanUse(child->node, source)) continue;
                char *usage = get_smart_usage(self, child->node, source, childOptional, 1);
                if (usage == NULL) continue;
                int duplicate = 0;
                for (size_t i = 0; i < childUsageCount; i++)
                {
                    if (strcmp(childUsages[i], usage) == 0)
                    {
                        duplicate = 1;
                        break;
                    }
                }
                if (duplicate)
                {
                    free(usage);
                    continue;
                }
                childUsages = realloc(childUsages, (childUsageCount + 1) * sizeof(char *));
                childUsages[childUsageCount++] = usage;
            }
            if (childUsageCount == 1)
            {
                size_t length = strlen(selfUsage) + strlen(childUsages[0]) + 4;
                char *result = malloc(length);
                if (childOptional)
                {
                    snprintf(result, length, "%s [%s]", selfUsage, childUsages[0]);
                }
                else
                {
                    snprintf(result, length, "%s %s", selfUsage, childUsages[0]);
                }
                for (size_t i = 0; i < childUsageCount; i++) free(childUsages[i]);
                free(childUsages);
                free(selfUsage);
                return result;
            }
            if (childUsageCount > 1)
            {
                size_t length = strlen(selfUsage) + strlen(open) + strlen(close) + 2;
                for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
                     child = child->next)
                {
                    if (!LIBMATTI_BR_CommandNode_CanUse(child->node, source)) continue;
                    char *usageText = LIBMATTI_BR_CommandNode_GetUsageText(child->node);
                    length += strlen(usageText) + strlen(USAGE_OR);
                    free(usageText);
                }
                char *result = malloc(length);
                char *writer = result;
                writer += sprintf(writer, "%s %s", selfUsage, open);
                int usageIndex = 0;
                for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
                     child = child->next)
                {
                    if (!LIBMATTI_BR_CommandNode_CanUse(child->node, source)) continue;
                    if (usageIndex > 0) writer += sprintf(writer, "%s", USAGE_OR);
                    char *usageText = LIBMATTI_BR_CommandNode_GetUsageText(child->node);
                    writer += sprintf(writer, "%s", usageText);
                    free(usageText);
                    usageIndex++;
                }
                sprintf(writer, "%s", close);
                for (size_t i = 0; i < childUsageCount; i++) free(childUsages[i]);
                free(childUsages);
                free(selfUsage);
                return result;
            }
            for (size_t i = 0; i < childUsageCount; i++) free(childUsages[i]);
            free(childUsages);
        }
    }

    return selfUsage;
}

char *LIBMATTI_BR_CommandDispatcher_GetSmartUsage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node,
                                                  const void *source)
{
    // Java: public Map<CommandNode<S>, String> getSmartUsage - the port exposes one
    // entry; the game port iterates children itself
    int optional = LIBMATTI_BR_CommandNode_GetCommand(node) != NULL;
    for (LIBMATTI_BR_CommandNodeChild *child = LIBMATTI_BR_CommandNode_GetChildren(node); child != NULL;
         child = child->next)
    {
        char *usage = get_smart_usage(self, child->node, source, optional, 0);
        if (usage != NULL) return usage;
    }
    return NULL;
}

void LIBMATTI_BR_ParseResults_Free(LIBMATTI_BR_ParseResults *results)
{
    for (size_t i = 0; i < results->errorCount; i++)
    {
        LIBMATTI_BR_CommandSyntaxException_Free(results->errors[i]);
    }
    free(results->errors);
    free(results->errorNodes);
    if (results->context != NULL) LIBMATTI_BR_CommandContextBuilder_Free(results->context);
    free(results);
}

void LIBMATTI_BR_CommandDispatcher_Free(LIBMATTI_BR_CommandDispatcher *self)
{
    LIBMATTI_BR_CommandNode_Free(self->root);
    free(self);
}
