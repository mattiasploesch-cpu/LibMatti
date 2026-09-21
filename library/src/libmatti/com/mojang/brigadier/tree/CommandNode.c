// Port of com.mojang.brigadier.tree.* and com.mojang.brigadier.builder.*.

#include "libmatti/com/mojang/brigadier/tree/CommandNode.h"

#include "libmatti/com/mojang/brigadier/exceptions/CommandSyntaxException.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

static LIBMATTI_BR_CommandNode *node_base(void)
{
    LIBMATTI_BR_CommandNode *node = calloc(1, sizeof(LIBMATTI_BR_CommandNode));
    return node;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_RootCommandNode_New(void)
{
    LIBMATTI_BR_CommandNode *node = node_base();
    node->isRoot = 1;
    return node;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_LiteralCommandNode_New(const char *literal, LIBMATTI_BR_Command command,
                                                            void *commandUserdata, LIBMATTI_BR_Requirement requirement,
                                                            void *requirementUserdata, LIBMATTI_BR_CommandNode *redirect,
                                                            LIBMATTI_BR_RedirectModifier modifier,
                                                            void *modifierUserdata, int forks)
{
    LIBMATTI_BR_CommandNode *node = node_base();
    node->isLiteral = 1;
    node->literal = strdup(literal);
    node->command = command;
    node->commandUserdata = commandUserdata;
    node->requirement = requirement;
    node->requirementUserdata = requirementUserdata;
    node->redirect = redirect;
    node->modifier = modifier;
    node->modifierUserdata = modifierUserdata;
    node->forks = forks;
    return node;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_ArgumentCommandNode_New(const char *name, LIBMATTI_BR_ArgumentType type,
                                                             LIBMATTI_BR_Command command, void *commandUserdata,
                                                             LIBMATTI_BR_Requirement requirement,
                                                             void *requirementUserdata,
                                                             LIBMATTI_BR_CommandNode *redirect,
                                                             LIBMATTI_BR_RedirectModifier modifier,
                                                             void *modifierUserdata, int forks)
{
    LIBMATTI_BR_CommandNode *node = node_base();
    node->isArgument = 1;
    node->name = strdup(name);
    node->type = type;
    node->command = command;
    node->commandUserdata = commandUserdata;
    node->requirement = requirement;
    node->requirementUserdata = requirementUserdata;
    node->redirect = redirect;
    node->modifier = modifier;
    node->modifierUserdata = modifierUserdata;
    node->forks = forks;
    return node;
}

// ---------------------------------------------------------------------------
// Children
// ---------------------------------------------------------------------------

void LIBMATTI_BR_CommandNode_AddChild(LIBMATTI_BR_CommandNode *self, LIBMATTI_BR_CommandNode *node)
{
    // Java: if (node instanceof RootCommandNode) throw new UnsupportedOperationException(...)
    if (node->isRoot) return;

    // Java: final CommandNode<S> child = children.get(node.getName()) - merge when present
    LIBMATTI_BR_CommandNode *child = LIBMATTI_BR_CommandNode_GetChild(self, LIBMATTI_BR_CommandNode_GetName(node));
    if (child != NULL)
    {
        // Java: We've found something to merge onto
        if (LIBMATTI_BR_CommandNode_GetCommand(node) != NULL)
        {
            child->command = node->command;
            child->commandUserdata = node->commandUserdata;
        }
        for (LIBMATTI_BR_CommandNodeChild *grandchild = node->childrenHead; grandchild != NULL;
             grandchild = grandchild->next)
        {
            LIBMATTI_BR_CommandNode_AddChild(child, grandchild->node);
        }
        return;
    }

    LIBMATTI_BR_CommandNodeChild *entry = malloc(sizeof(LIBMATTI_BR_CommandNodeChild));
    entry->node = node;
    entry->next = NULL;
    if (self->childrenTail != NULL) self->childrenTail->next = entry;
    else self->childrenHead = entry;
    self->childrenTail = entry;
    self->childCount++;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandNode_GetChild(const LIBMATTI_BR_CommandNode *self, const char *name)
{
    for (LIBMATTI_BR_CommandNodeChild *child = self->childrenHead; child != NULL; child = child->next)
    {
        if (strcmp(LIBMATTI_BR_CommandNode_GetName(child->node), name) == 0) return child->node;
    }
    return NULL;
}

LIBMATTI_BR_CommandNodeChild *LIBMATTI_BR_CommandNode_GetChildren(const LIBMATTI_BR_CommandNode *self)
{
    return self->childrenHead;
}

size_t LIBMATTI_BR_CommandNode_GetChildCount(const LIBMATTI_BR_CommandNode *self)
{
    return self->childCount;
}

int LIBMATTI_BR_CommandNode_CanUse(const LIBMATTI_BR_CommandNode *self, const void *source)
{
    // Java: requirement.test(source) with the s -> true default
    if (self->requirement == NULL) return 1;
    return self->requirement(source, self->requirementUserdata);
}

LIBMATTI_BR_Command LIBMATTI_BR_CommandNode_GetCommand(const LIBMATTI_BR_CommandNode *self)
{
    return self->command;
}

const char *LIBMATTI_BR_CommandNode_GetName(const LIBMATTI_BR_CommandNode *self)
{
    if (self->isLiteral) return self->literal;
    if (self->isArgument) return self->name;
    return "";
}

char *LIBMATTI_BR_CommandNode_GetUsageText(const LIBMATTI_BR_CommandNode *self)
{
    if (self->isArgument)
    {
        // Java: USAGE_ARGUMENT_OPEN + name + USAGE_ARGUMENT_CLOSE
        size_t length = strlen(self->name) + 3;
        char *usage = malloc(length);
        snprintf(usage, length, "<%s>", self->name);
        return usage;
    }
    return strdup(LIBMATTI_BR_CommandNode_GetName(self));
}

LIBMATTI_BR_CommandNodeChild *LIBMATTI_BR_CommandNode_GetRelevantNodes(LIBMATTI_BR_CommandNode *self,
                                                                       LIBMATTI_BR_StringReader *input)
{
    if (self->childCount == 0) return NULL;

    // Java: the literals fast path - read the next word, look it up
    int cursor = LIBMATTI_BR_StringReader_GetCursor(input);
    while (LIBMATTI_BR_StringReader_CanRead(input) && LIBMATTI_BR_StringReader_Peek(input) != ' ')
    {
        LIBMATTI_BR_StringReader_Skip(input);
    }
    size_t length = (size_t) (LIBMATTI_BR_StringReader_GetCursor(input) - cursor);
    char text[length + 1];
    memcpy(text, LIBMATTI_BR_StringReader_GetString(input) + cursor, length);
    text[length] = '\0';
    LIBMATTI_BR_StringReader_SetCursor(input, cursor);

    // Java: literals.get(text) - the port walks the children and compares
    for (LIBMATTI_BR_CommandNodeChild *child = self->childrenHead; child != NULL; child = child->next)
    {
        if (child->node->isLiteral && strcmp(child->node->literal, text) == 0) return child;
    }
    // Java: arguments.values() -> the first argument child stands in for the group
    for (LIBMATTI_BR_CommandNodeChild *child = self->childrenHead; child != NULL; child = child->next)
    {
        if (child->node->isArgument) return child;
    }
    return NULL;
}

int LIBMATTI_BR_CommandNode_CompareTo(const LIBMATTI_BR_CommandNode *self, const LIBMATTI_BR_CommandNode *other)
{
    // Java: if (this instanceof LiteralCommandNode == o instanceof LiteralCommandNode)
    //           return getSortedKey().compareTo(o.getSortedKey());
    if (self->isLiteral == other->isLiteral)
    {
        return strcmp(LIBMATTI_BR_CommandNode_GetName(self), LIBMATTI_BR_CommandNode_GetName(other));
    }
    // Java: return (o instanceof LiteralCommandNode) ? 1 : -1 - literals sort first
    return other->isLiteral ? 1 : -1;
}

void LIBMATTI_BR_CommandNode_Free(LIBMATTI_BR_CommandNode *node)
{
    LIBMATTI_BR_CommandNodeChild *child = node->childrenHead;
    while (child != NULL)
    {
        LIBMATTI_BR_CommandNodeChild *next = child->next;
        LIBMATTI_BR_CommandNode_Free(child->node);
        free(child);
        child = next;
    }
    free(node->literal);
    free(node->name);
    free(node);
}

// ---------------------------------------------------------------------------
// Builders
// ---------------------------------------------------------------------------

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_LiteralArgumentBuilder_Literal(const char *name)
{
    LIBMATTI_BR_ArgumentBuilder *builder = calloc(1, sizeof(LIBMATTI_BR_ArgumentBuilder));
    builder->isLiteral = 1;
    builder->literal = strdup(name);
    builder->arguments = LIBMATTI_BR_RootCommandNode_New();
    return builder;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_RequiredArgumentBuilder_Argument(const char *name,
                                                                          LIBMATTI_BR_ArgumentType type)
{
    LIBMATTI_BR_ArgumentBuilder *builder = calloc(1, sizeof(LIBMATTI_BR_ArgumentBuilder));
    builder->isLiteral = 0;
    builder->name = strdup(name);
    builder->type = type;
    builder->arguments = LIBMATTI_BR_RootCommandNode_New();
    return builder;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_ThenBuilder(LIBMATTI_BR_ArgumentBuilder *self,
                                                                     LIBMATTI_BR_ArgumentBuilder *argument)
{
    // Java: if (target != null) throw new IllegalStateException("Cannot add children to a redirected node")
    if (self->target != NULL) return self;
    LIBMATTI_BR_CommandNode_AddChild(self->arguments, LIBMATTI_BR_ArgumentBuilder_Build(argument));
    LIBMATTI_BR_ArgumentBuilder_Free(argument);
    return self;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_ThenNode(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_CommandNode *argument)
{
    if (self->target != NULL) return self;
    LIBMATTI_BR_CommandNode_AddChild(self->arguments, argument);
    return self;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Executes(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_Command command, void *userdata)
{
    self->command = command;
    self->commandUserdata = userdata;
    return self;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Requires(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_Requirement requirement,
                                                                  void *userdata)
{
    self->requirement = requirement;
    self->requirementUserdata = userdata;
    return self;
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Redirect(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_CommandNode *target)
{
    return LIBMATTI_BR_ArgumentBuilder_Forward(self, target, NULL, NULL, 0);
}

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Forward(LIBMATTI_BR_ArgumentBuilder *self,
                                                                 LIBMATTI_BR_CommandNode *target,
                                                                 LIBMATTI_BR_RedirectModifier modifier,
                                                                 void *modifierUserdata, int fork)
{
    // Java: if (!arguments.getChildren().isEmpty()) throw new IllegalStateException("Cannot forward a node with children")
    if (self->arguments->childCount > 0) return self;
    self->target = target;
    self->modifier = modifier;
    self->modifierUserdata = modifierUserdata;
    self->forks = fork;
    return self;
}

LIBMATTI_BR_CommandNode *LIBMATTI_BR_ArgumentBuilder_Build(LIBMATTI_BR_ArgumentBuilder *self)
{
    LIBMATTI_BR_CommandNode *result;
    if (self->isLiteral)
    {
        // Java: LiteralArgumentBuilder.build()
        result = LIBMATTI_BR_LiteralCommandNode_New(self->literal, self->command, self->commandUserdata,
                                                    self->requirement, self->requirementUserdata, self->target,
                                                    self->modifier, self->modifierUserdata, self->forks);
    }
    else
    {
        // Java: RequiredArgumentBuilder.build()
        result = LIBMATTI_BR_ArgumentCommandNode_New(self->name, self->type, self->command, self->commandUserdata,
                                                     self->requirement, self->requirementUserdata, self->target,
                                                     self->modifier, self->modifierUserdata, self->forks);
    }
    // Java: for (final CommandNode<S> argument : getArguments()) result.addChild(argument)
    for (LIBMATTI_BR_CommandNodeChild *child = self->arguments->childrenHead; child != NULL; child = child->next)
    {
        LIBMATTI_BR_CommandNode_AddChild(result, child->node);
    }
    // the nodes moved to the result; detach the temporary root's list
    self->arguments->childrenHead = NULL;
    self->arguments->childrenTail = NULL;
    self->arguments->childCount = 0;
    return result;
}

void LIBMATTI_BR_ArgumentBuilder_Free(LIBMATTI_BR_ArgumentBuilder *self)
{
    LIBMATTI_BR_CommandNode_Free(self->arguments);
    free(self->literal);
    free(self->name);
    free(self);
}
