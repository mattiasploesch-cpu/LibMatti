// Port of com.mojang.brigadier.tree.* (CommandNode, LiteralCommandNode,
// ArgumentCommandNode, RootCommandNode) and com.mojang.brigadier.builder.*
// (ArgumentBuilder, LiteralArgumentBuilder, RequiredArgumentBuilder).
//
// Java's class hierarchy becomes one struct with a kind discriminator; the
// builder mirrors it 1:1 (then/execute/requires/forward/suggests).

#ifndef MATTICRAFT_BR_TREE_COMMANDNODE_H
#define MATTICRAFT_BR_TREE_COMMANDNODE_H

#include "libmatti/com/mojang/brigadier/arguments/ArgumentType.h"
#include "libmatti/com/mojang/brigadier/context/StringRange.h"

#include <stddef.h>

// Java: Command<S> - int run(CommandContext) with the C error channel
typedef struct LIBMATTI_BR_CommandContext LIBMATTI_BR_CommandContext;
typedef struct LIBMATTI_BR_CommandSyntaxException LIBMATTI_BR_CommandSyntaxException;
typedef struct LIBMATTI_BR_CommandNode LIBMATTI_BR_CommandNode;

// Java: public interface Command<S> { int run(CommandContext<S> context) throws }
typedef int (*LIBMATTI_BR_Command)(LIBMATTI_BR_CommandContext *context, void *userdata,
                                   LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public interface Predicate<S> { boolean test(S source) }
typedef int (*LIBMATTI_BR_Requirement)(const void *source, void *userdata);

// Java: RedirectModifier<S> - Collection<S> apply(CommandContext<S>) through arrays
typedef int (*LIBMATTI_BR_RedirectModifier)(LIBMATTI_BR_CommandContext *context, void *userdata,
                                            const void ***outSources, size_t *outCount,
                                            LIBMATTI_BR_CommandSyntaxException **outException);

typedef struct LIBMATTI_BR_CommandNodeChild
{
    LIBMATTI_BR_CommandNode *node;
    struct LIBMATTI_BR_CommandNodeChild *next;
} LIBMATTI_BR_CommandNodeChild;

// Java: public abstract class CommandNode<S>
typedef struct LIBMATTI_BR_CommandNode
{
    // Java: the runtime type (LiteralCommandNode / ArgumentCommandNode / RootCommandNode)
    int isLiteral;
    int isArgument;
    int isRoot;

    // Java: private final Map<String, CommandNode<S>> children (insertion order kept)
    LIBMATTI_BR_CommandNodeChild *childrenHead;
    LIBMATTI_BR_CommandNodeChild *childrenTail;
    size_t childCount;

    // Java: private final Predicate<S> requirement (NULL = s -> true)
    LIBMATTI_BR_Requirement requirement;
    void *requirementUserdata;
    // Java: private final CommandNode<S> redirect
    LIBMATTI_BR_CommandNode *redirect;
    // Java: private final RedirectModifier<S> modifier
    LIBMATTI_BR_RedirectModifier modifier;
    void *modifierUserdata;
    // Java: private final boolean forks
    int forks;
    // Java: private Command<S> command
    LIBMATTI_BR_Command command;
    void *commandUserdata;

    // LiteralCommandNode: private final String literal
    char *literal;
    // ArgumentCommandNode: private final String name / ArgumentType<T> type
    char *name;
    LIBMATTI_BR_ArgumentType type;
} LIBMATTI_BR_CommandNode;

// Java: public static RootCommandNode / LiteralCommandNode / ArgumentCommandNode constructors
LIBMATTI_BR_CommandNode *LIBMATTI_BR_RootCommandNode_New(void);
LIBMATTI_BR_CommandNode *LIBMATTI_BR_LiteralCommandNode_New(const char *literal, LIBMATTI_BR_Command command,
                                                            void *commandUserdata, LIBMATTI_BR_Requirement requirement,
                                                            void *requirementUserdata, LIBMATTI_BR_CommandNode *redirect,
                                                            LIBMATTI_BR_RedirectModifier modifier,
                                                            void *modifierUserdata, int forks);
LIBMATTI_BR_CommandNode *LIBMATTI_BR_ArgumentCommandNode_New(const char *name, LIBMATTI_BR_ArgumentType type,
                                                             LIBMATTI_BR_Command command, void *commandUserdata,
                                                             LIBMATTI_BR_Requirement requirement,
                                                             void *requirementUserdata,
                                                             LIBMATTI_BR_CommandNode *redirect,
                                                             LIBMATTI_BR_RedirectModifier modifier,
                                                             void *modifierUserdata, int forks);

// Java: public void addChild(final CommandNode<S> node) - merges onto an existing child
void LIBMATTI_BR_CommandNode_AddChild(LIBMATTI_BR_CommandNode *self, LIBMATTI_BR_CommandNode *node);
// Java: public CommandNode<S> getChild(final String name)
LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandNode_GetChild(const LIBMATTI_BR_CommandNode *self, const char *name);
// Java: public Collection<CommandNode<S>> getChildren()
LIBMATTI_BR_CommandNodeChild *LIBMATTI_BR_CommandNode_GetChildren(const LIBMATTI_BR_CommandNode *self);
size_t LIBMATTI_BR_CommandNode_GetChildCount(const LIBMATTI_BR_CommandNode *self);
// Java: public boolean canUse(final S source)
int LIBMATTI_BR_CommandNode_CanUse(const LIBMATTI_BR_CommandNode *self, const void *source);
// Java: public Command<S> getCommand()
LIBMATTI_BR_Command LIBMATTI_BR_CommandNode_GetCommand(const LIBMATTI_BR_CommandNode *self);
// Java: public abstract String getName()
const char *LIBMATTI_BR_CommandNode_GetName(const LIBMATTI_BR_CommandNode *self);
// Java: public abstract String getUsageText() - the caller frees
char *LIBMATTI_BR_CommandNode_GetUsageText(const LIBMATTI_BR_CommandNode *self);
// Java: getRelevantNodes(input) - the literals fast path; returns the ordered child list
LIBMATTI_BR_CommandNodeChild *LIBMATTI_BR_CommandNode_GetRelevantNodes(LIBMATTI_BR_CommandNode *self,
                                                                       LIBMATTI_BR_StringReader *input);
// Java: public int compareTo(final CommandNode<S> o) - literals before arguments, then sorted key
int LIBMATTI_BR_CommandNode_CompareTo(const LIBMATTI_BR_CommandNode *self, const LIBMATTI_BR_CommandNode *other);

void LIBMATTI_BR_CommandNode_Free(LIBMATTI_BR_CommandNode *node);

// ---------------------------------------------------------------------------
// Java: com.mojang.brigadier.builder.ArgumentBuilder / LiteralArgumentBuilder /
// RequiredArgumentBuilder - one struct, the kind decided at build time
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_BR_ArgumentBuilder LIBMATTI_BR_ArgumentBuilder;

// The builder accumulates children in a temporary RootCommandNode, like Java's
// `private final RootCommandNode<S> arguments = new RootCommandNode<>()`.
struct LIBMATTI_BR_ArgumentBuilder
{
    int isLiteral;
    // LiteralArgumentBuilder: private final String literal
    char *literal;
    // RequiredArgumentBuilder: private final String name / ArgumentType<T> type
    char *name;
    LIBMATTI_BR_ArgumentType type;

    LIBMATTI_BR_CommandNode *arguments; // a RootCommandNode holding the built children
    LIBMATTI_BR_Command command;
    void *commandUserdata;
    LIBMATTI_BR_Requirement requirement;
    void *requirementUserdata;
    LIBMATTI_BR_CommandNode *target;
    LIBMATTI_BR_RedirectModifier modifier;
    void *modifierUserdata;
    int forks;
};

LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_LiteralArgumentBuilder_Literal(const char *name);
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_RequiredArgumentBuilder_Argument(const char *name,
                                                                          LIBMATTI_BR_ArgumentType type);
// Java: public T then(final ArgumentBuilder<S, ?> argument) / then(final CommandNode<S>)
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_ThenBuilder(LIBMATTI_BR_ArgumentBuilder *self,
                                                                     LIBMATTI_BR_ArgumentBuilder *argument);
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_ThenNode(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_CommandNode *argument);
// Java: public T executes(final Command<S> command)
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Executes(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_Command command, void *userdata);
// Java: public T requires(final Predicate<S> requirement)
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Requires(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_Requirement requirement,
                                                                  void *userdata);
// Java: public T redirect(final CommandNode<S> target) / fork(...) / forward(...)
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Redirect(LIBMATTI_BR_ArgumentBuilder *self,
                                                                  LIBMATTI_BR_CommandNode *target);
LIBMATTI_BR_ArgumentBuilder *LIBMATTI_BR_ArgumentBuilder_Forward(LIBMATTI_BR_ArgumentBuilder *self,
                                                                 LIBMATTI_BR_CommandNode *target,
                                                                 LIBMATTI_BR_RedirectModifier modifier,
                                                                 void *modifierUserdata, int fork);
// Java: public abstract CommandNode<S> build()
LIBMATTI_BR_CommandNode *LIBMATTI_BR_ArgumentBuilder_Build(LIBMATTI_BR_ArgumentBuilder *self);
void LIBMATTI_BR_ArgumentBuilder_Free(LIBMATTI_BR_ArgumentBuilder *self);

#endif
