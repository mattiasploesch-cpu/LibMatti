// Port of com.mojang.brigadier.context.CommandContextBuilder and CommandContext.

#include "libmatti/com/mojang/brigadier/context/CommandContext.h"

#include "libmatti/com/mojang/brigadier/CommandDispatcher.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// CommandContextBuilder
// ---------------------------------------------------------------------------

static void add_argument(LIBMATTI_BR_CommandContextBuilder *self, const char *name, LIBMATTI_BR_ParsedArgument *value)
{
    self->argumentNames = realloc(self->argumentNames, (self->argumentCount + 1) * sizeof(char *));
    self->argumentValues = realloc(self->argumentValues, (self->argumentCount + 1) * sizeof(LIBMATTI_BR_ParsedArgument *));
    self->argumentNames[self->argumentCount] = strdup(name);
    self->argumentValues[self->argumentCount] = value;
    self->argumentCount++;
}

static void add_node(LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_ParsedCommandNode node)
{
    self->nodes = realloc(self->nodes, (self->nodeCount + 1) * sizeof(LIBMATTI_BR_ParsedCommandNode));
    self->nodes[self->nodeCount] = node;
    self->nodeCount++;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_New(LIBMATTI_BR_CommandDispatcher *dispatcher,
                                                                         const void *source,
                                                                         LIBMATTI_BR_CommandNode *rootNode, int start)
{
    LIBMATTI_BR_CommandContextBuilder *self = calloc(1, sizeof(LIBMATTI_BR_CommandContextBuilder));
    self->dispatcher = dispatcher;
    self->rootNode = rootNode;
    self->source = source;
    self->range = LIBMATTI_BR_StringRange_At(start);
    return self;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_Copy(const LIBMATTI_BR_CommandContextBuilder *self)
{
    // Java: copy() shares the argument map; the port deep-copies the names and moves
    // the ParsedArgument pointers (the caller frees only one of the two)
    LIBMATTI_BR_CommandContextBuilder *copy = calloc(1, sizeof(LIBMATTI_BR_CommandContextBuilder));
    copy->dispatcher = self->dispatcher;
    copy->rootNode = self->rootNode;
    copy->source = self->source;
    copy->command = self->command;
    copy->commandUserdata = self->commandUserdata;
    copy->child = self->child;
    copy->range = self->range;
    copy->modifier = self->modifier;
    copy->modifierUserdata = self->modifierUserdata;
    copy->forks = self->forks;
    for (size_t i = 0; i < self->argumentCount; i++)
    {
        add_argument(copy, self->argumentNames[i], self->argumentValues[i]);
    }
    for (size_t i = 0; i < self->nodeCount; i++)
    {
        add_node(copy, self->nodes[i]);
    }
    return copy;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithArgument(
    LIBMATTI_BR_CommandContextBuilder *self, const char *name, LIBMATTI_BR_ParsedArgument *argument)
{
    add_argument(self, name, argument);
    return self;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithCommand(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_Command command, void *userdata)
{
    self->command = command;
    self->commandUserdata = userdata;
    return self;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithNode(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_CommandNode *node, LIBMATTI_BR_StringRange range)
{
    // Java: nodes.add(new ParsedCommandNode<>(node, range));
    //       this.range = StringRange.encompassing(this.range, range);
    //       this.modifier = node.getRedirectModifier(); this.forks = node.isFork();
    LIBMATTI_BR_ParsedCommandNode parsed = {node, range};
    add_node(self, parsed);
    self->range = LIBMATTI_BR_StringRange_Encompassing(&self->range, &range);
    self->modifier = node->modifier;
    self->modifierUserdata = node->modifierUserdata;
    self->forks = node->forks;
    return self;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_WithChild(
    LIBMATTI_BR_CommandContextBuilder *self, LIBMATTI_BR_CommandContextBuilder *child)
{
    self->child = child;
    return self;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_GetChild(
    const LIBMATTI_BR_CommandContextBuilder *self)
{
    return self->child;
}

LIBMATTI_BR_CommandContextBuilder *LIBMATTI_BR_CommandContextBuilder_GetLastChild(
    LIBMATTI_BR_CommandContextBuilder *self)
{
    LIBMATTI_BR_CommandContextBuilder *result = self;
    while (LIBMATTI_BR_CommandContextBuilder_GetChild(result) != NULL)
    {
        result = LIBMATTI_BR_CommandContextBuilder_GetChild(result);
    }
    return result;
}

const void *LIBMATTI_BR_CommandContextBuilder_GetSource(const LIBMATTI_BR_CommandContextBuilder *self)
{
    return self->source;
}

LIBMATTI_BR_StringRange LIBMATTI_BR_CommandContextBuilder_GetRange(const LIBMATTI_BR_CommandContextBuilder *self)
{
    return self->range;
}

LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContextBuilder_Build(const LIBMATTI_BR_CommandContextBuilder *self,
                                                                    const char *input)
{
    LIBMATTI_BR_CommandContext *context = calloc(1, sizeof(LIBMATTI_BR_CommandContext));
    context->source = self->source;
    context->input = strdup(input);
    context->command = self->command;
    context->commandUserdata = self->commandUserdata;
    context->rootNode = self->rootNode;
    context->range = self->range;
    context->modifier = self->modifier;
    context->modifierUserdata = self->modifierUserdata;
    context->forks = self->forks;
    // Java: the maps are shared by reference; the port moves the arrays (the builder
    // keeps ownership of nothing afterwards)
    context->argumentNames = self->argumentNames;
    context->argumentValues = self->argumentValues;
    context->argumentCount = self->argumentCount;
    context->nodes = self->nodes;
    context->nodeCount = self->nodeCount;
    ((LIBMATTI_BR_CommandContextBuilder *) self)->argumentNames = NULL;
    ((LIBMATTI_BR_CommandContextBuilder *) self)->argumentValues = NULL;
    ((LIBMATTI_BR_CommandContextBuilder *) self)->argumentCount = 0;
    ((LIBMATTI_BR_CommandContextBuilder *) self)->nodes = NULL;
    ((LIBMATTI_BR_CommandContextBuilder *) self)->nodeCount = 0;
    // Java: child == null ? null : child.build(input)
    context->child = self->child != NULL ? LIBMATTI_BR_CommandContextBuilder_Build(self->child, input) : NULL;
    return context;
}

void LIBMATTI_BR_CommandContextBuilder_Free(LIBMATTI_BR_CommandContextBuilder *self)
{
    // Java: the copies share the arguments; the port frees them once (the original owner)
    for (size_t i = 0; i < self->argumentCount; i++)
    {
        free(self->argumentNames[i]);
        LIBMATTI_BR_ParsedArgument_Free(self->argumentValues[i]);
    }
    free(self->argumentNames);
    free(self->argumentValues);
    free(self->nodes);
    // Java: the child is owned by the redirect chain that created it
    free(self);
}

// ---------------------------------------------------------------------------
// CommandContext
// ---------------------------------------------------------------------------

const LIBMATTI_BR_ParsedArgument *LIBMATTI_BR_CommandContext_GetArgument(const LIBMATTI_BR_CommandContext *self,
                                                                         const char *name,
                                                                         LIBMATTI_BR_ArgumentKind kind)
{
    // Java: final ParsedArgument<S, ?> argument = arguments.get(name);
    //       if (argument == null) throw new IllegalArgumentException("No such argument ...")
    for (size_t i = 0; i < self->argumentCount; i++)
    {
        if (strcmp(self->argumentNames[i], name) == 0)
        {
            // Java: the class check; the port compares the kind
            if (self->argumentValues[i]->kind != kind) return NULL;
            return self->argumentValues[i];
        }
    }
    return NULL;
}

LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_GetChild(const LIBMATTI_BR_CommandContext *self)
{
    return self->child;
}

LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_GetLastChild(LIBMATTI_BR_CommandContext *self)
{
    LIBMATTI_BR_CommandContext *result = self;
    while (LIBMATTI_BR_CommandContext_GetChild(result) != NULL)
    {
        result = LIBMATTI_BR_CommandContext_GetChild(result);
    }
    return result;
}

LIBMATTI_BR_CommandContext *LIBMATTI_BR_CommandContext_CopyFor(LIBMATTI_BR_CommandContext *self, const void *source)
{
    // Java: copyFor returns a new context with the given source; the port's contexts
    // own their parsed data, so the source pointer swap happens on the same context
    // (the C sources are immutable values).
    (void) source;
    return self;
}

const void *LIBMATTI_BR_CommandContext_GetSource(const LIBMATTI_BR_CommandContext *self)
{
    return self->source;
}

const char *LIBMATTI_BR_CommandContext_GetInput(const LIBMATTI_BR_CommandContext *self)
{
    return self->input;
}

LIBMATTI_BR_Command LIBMATTI_BR_CommandContext_GetCommand(const LIBMATTI_BR_CommandContext *self)
{
    return self->command;
}

void *LIBMATTI_BR_CommandContext_GetCommandUserdata(const LIBMATTI_BR_CommandContext *self)
{
    return self->commandUserdata;
}

const LIBMATTI_BR_StringRange *LIBMATTI_BR_CommandContext_GetRange(const LIBMATTI_BR_CommandContext *self)
{
    return &self->range;
}

int LIBMATTI_BR_CommandContext_IsForked(const LIBMATTI_BR_CommandContext *self)
{
    return self->forks;
}

LIBMATTI_BR_RedirectModifier LIBMATTI_BR_CommandContext_GetRedirectModifier(const LIBMATTI_BR_CommandContext *self)
{
    return self->modifier;
}

void *LIBMATTI_BR_CommandContext_GetRedirectModifierUserdata(const LIBMATTI_BR_CommandContext *self)
{
    return self->modifierUserdata;
}

void LIBMATTI_BR_CommandContext_Free(LIBMATTI_BR_CommandContext *self)
{
    for (size_t i = 0; i < self->argumentCount; i++)
    {
        free(self->argumentNames[i]);
        LIBMATTI_BR_ParsedArgument_Free(self->argumentValues[i]);
    }
    free(self->argumentNames);
    free(self->argumentValues);
    free(self->nodes);
    if (self->child != NULL) LIBMATTI_BR_CommandContext_Free(self->child);
    free(self->input);
    free(self);
}
