// Port of com.mojang.brigadier.context.CommandContextBuilder and CommandContext.

#ifndef MATTICRAFT_BR_CONTEXT_COMMANDCONTEXT_H
#define MATTICRAFT_BR_CONTEXT_COMMANDCONTEXT_H

#include "libmatti/com/mojang/brigadier/context/ParsedArgument.h"
#include "libmatti/com/mojang/brigadier/tree/CommandNode.h"

#include <stddef.h>

typedef struct LIBMATTI_BR_CommandDispatcher LIBMATTI_BR_CommandDispatcher;
typedef struct LIBMATTI_BR_CommandContextBuilder LIBMATTI_BR_CommandContextBuilder;

// Java: public class CommandContextBuilder<S>
struct LIBMATTI_BR_CommandContextBuilder
{
    LIBMATTI_BR_CommandDispatcher *dispatcher;
    const void *source;
    LIBMATTI_BR_CommandNode *rootNode;
    // Java: private final Map<String, ParsedArgument<S, ?>> arguments (ordered)
    char **argumentNames;
    LIBMATTI_BR_ParsedArgument **argumentValues;
    size_t argumentCount;
    // Java: private final List<ParsedCommandNode<S>> nodes
    LIBMATTI_BR_ParsedCommandNode *nodes;
    size_t nodeCount;
    // Java: private Command<S> command
    LIBMATTI_BR_Command command;
    void *commandUserdata;
    // Java: private CommandContextBuilder<S> child
    LIBMATTI_BR_CommandContextBuilder *child;
    // Java: private StringRange range
    LIBMATTI_BR_StringRange range;
    // Java: private RedirectModifier<S> modifier
    LIBMATTI_BR_RedirectModifier modifier;
    void *modifierUserdata;
    int forks;
};

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_New(LIBMATTI_BR_CommandDispatcher *dispatcher,
                                                                         const void *source,
                                                                         LIBMATTI_BR_CommandNode *rootNode, int start);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_Copy(const LIBMATTI_BR_CommandContextBuilder *self);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithArgument(
    LIBMATTI_BR_CommandContextBuilder *self, const char *name, LIBMATTI_BR_ParsedArgument *argument);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithCommand(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_Command command, void *userdata);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithNode(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_CommandNode *node, LIBMATTI_BR_StringRange range);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithChild(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_CommandContextBuilder *child);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_GetChild(
    const LIBMATTI_BR_CommandContextBuilder *self);
LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_GetLastChild(
    LIBMATTI_BR_CommandContextBuilder *self);
const void *LIBMATTI_BR_CommandContextBuilder_GetSource(const LIBMATTI_BR_CommandContextBuilder *self);
LIBMATTI_BR_StringRange LIBMATTI_BR_CommandContextBuilder_GetRange(const LIBMATTI_BR_CommandContextBuilder *self);
// Java: public CommandContext<S> build(final String input) - the caller owns the result
LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContextBuilder_Build(const LIBMATTI_BR_CommandContextBuilder *self,
                                                                    const char *input);
// The port's copies are deep (Java's copy() shares the argument map; the port's
// parsed arguments move with the copy, so the original must not be freed after a copy).
void LIBMATTI_BR_CommandContextBuilder_Free(LIBMATTI_BR_CommandContextBuilder *self);

// ---------------------------------------------------------------------------
// Java: public class CommandContext<S>
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_BR_CommandContext
{
    const void *source;
    char *input;
    LIBMATTI_BR_Command command;
    void *commandUserdata;
    // Java: private final Map<String, ParsedArgument<S, ?>> arguments
    char **argumentNames;
    LIBMATTI_BR_ParsedArgument **argumentValues;
    size_t argumentCount;
    LIBMATTI_BR_CommandNode *rootNode;
    LIBMATTI_BR_ParsedCommandNode *nodes;
    size_t nodeCount;
    LIBMATTI_BR_StringRange range;
    LIBMATTI_BR_CommandContext *child;
    LIBMATTI_BR_RedirectModifier modifier;
    void *modifierUserdata;
    int forks;
} LIBMATTI_BR_CommandContext;

// Java: public <V> V getArgument(final String name, final Class<V> clazz) - the port
// dispatches on the requested kind; NULL when absent (Java throws IllegalArgumentException)
const LIBMATTI_BR_ParsedArgument *LIBMATTI_BR_CommandContext_GetArgument(const LIBMATTI_BR_CommandContext *self,
                                                                         const char *name,
                                                                         LIBMATTI_BR_ArgumentKind kind);
// Java: public CommandContext<S> getChild() / getLastChild()
LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_GetChild(const LIBMATTI_BR_CommandContext *self);
LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_GetLastChild(LIBMATTI_BR_CommandContext *self);
// Java: public CommandContext<S> copyFor(final S source) - the port's contexts own their
// data, so copyFor returns the same context (the source is immutable in the port)
LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_CopyFor(LIBMATTI_BR_CommandContext *self, const void *source);
const void *LIBMATTI_BR_CommandContext_GetSource(const LIBMATTI_BR_CommandContext *self);
const char *LIBMATTI_BR_CommandContext_GetInput(const LIBMATTI_BR_CommandContext *self);
LIBMATTI_BR_Command LIBMATTI_BR_CommandContext_GetCommand(const LIBMATTI_BR_CommandContext *self);
void *LIBMATTI_BR_CommandContext_GetCommandUserdata(const LIBMATTI_BR_CommandContext *self);
const LIBMATTI_BR_StringRange *LIBMATTI_BR_CommandContext_GetRange(const LIBMATTI_BR_CommandContext *self);
int LIBMATTI_BR_CommandContext_IsForked(const LIBMATTI_BR_CommandContext *self);
LIBMATTI_BR_RedirectModifier LIBMATTI_BR_CommandContext_GetRedirectModifier(const LIBMATTI_BR_CommandContext *self);
void *LIBMATTI_BR_CommandContext_GetRedirectModifierUserdata(const LIBMATTI_BR_CommandContext *self);
void LIBMATTI_BR_CommandContext_Free(LIBMATTI_BR_CommandContext *self);

#endif
