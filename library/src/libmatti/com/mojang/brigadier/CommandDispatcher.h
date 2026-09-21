// Port of com.mojang.brigadier.CommandDispatcher, ParseResults and ResultConsumer.

#ifndef MATTICRAFT_BR_COMMANDDISPATCHER_H
#define MATTICRAFT_BR_COMMANDDISPATCHER_H

#include "libmatti/com/mojang/brigadier/context/CommandContext.h"
#include "libmatti/com/mojang/brigadier/tree/CommandNode.h"

#include <stddef.h>

// Java: public interface ResultConsumer<S>
typedef void (*LIBMATTI_BR_ResultConsumer)(LIBMATTI_BR_CommandContext *context, int success, int result,
                                           void *userdata);

// Java: public class ParseResults<S>
typedef struct LIBMATTI_BR_ParseResults
{
    LIBMATTI_BR_CommandContextBuilder *context;
    LIBMATTI_BR_StringReader reader;
    // Java: private final Map<CommandNode<S>, CommandSyntaxException> errors
    LIBMATTI_BR_CommandNode **errorNodes;
    LIBMATTI_BR_CommandSyntaxException **errors;
    size_t errorCount;
} LIBMATTI_BR_ParseResults;

void LIBMATTI_BR_ParseResults_Free(LIBMATTI_BR_ParseResults *results);

// Java: public class CommandDispatcher<S>
typedef struct LIBMATTI_BR_CommandDispatcher
{
    // Java: private final RootCommandNode<S> root
    LIBMATTI_BR_CommandNode *root;
    // Java: private ResultConsumer<S> consumer
    LIBMATTI_BR_ResultConsumer consumer;
    void *consumerUserdata;
} LIBMATTI_BR_CommandDispatcher;

// Java: public CommandDispatcher() - an empty command tree
LIBMATTI_BR_CommandDispatcher *LIBMATTI_BR_CommandDispatcher_New(void);
// Java: public CommandDispatcher(final RootCommandNode<S> root)
LIBMATTI_BR_CommandDispatcher *LIBMATTI_BR_CommandDispatcher_NewWithRoot(LIBMATTI_BR_CommandNode *root);

// Java: public LiteralCommandNode<S> register(final LiteralArgumentBuilder<S> command)
LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandDispatcher_Register(LIBMATTI_BR_CommandDispatcher *self,
                                                                LIBMATTI_BR_ArgumentBuilder *command);
// Java: public void setConsumer(final ResultConsumer<S> consumer)
void LIBMATTI_BR_CommandDispatcher_SetConsumer(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_ResultConsumer consumer,
                                               void *userdata);

// Java: public int execute(final String input, final S source) throws
int LIBMATTI_BR_CommandDispatcher_ExecuteString(LIBMATTI_BR_CommandDispatcher *self, const char *input,
                                                const void *source, LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public int execute(final StringReader input, final S source) throws
int LIBMATTI_BR_CommandDispatcher_ExecuteReader(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_StringReader *input,
                                                const void *source, LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public int execute(final ParseResults<S> parse) throws
int LIBMATTI_BR_CommandDispatcher_Execute(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_ParseResults *parse,
                                          LIBMATTI_BR_CommandSyntaxException **outException);

// Java: public ParseResults<S> parse(final String command, final S source)
LIBMATTI_BR_ParseResults *LIBMATTI_BR_CommandDispatcher_ParseString(LIBMATTI_BR_CommandDispatcher *self,
                                                                    const char *command, const void *source);
// Java: public ParseResults<S> parse(final StringReader command, final S source)
LIBMATTI_BR_ParseResults *LIBMATTI_BR_CommandDispatcher_Parse(LIBMATTI_BR_CommandDispatcher *self,
                                                              LIBMATTI_BR_StringReader *command, const void *source);

// Java: public String[] getAllUsage(final CommandNode<S> node, final S source, final boolean restricted)
// Returns a NULL-terminated array of malloc'd strings.
char **LIBMATTI_BR_CommandDispatcher_GetAllUsage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node,
                                                 const void *source, int restricted, size_t *outCount);
// Java: public Map<CommandNode<S>, String> getSmartUsage(...) - the single usage string
// for one child, NULL when none
char *LIBMATTI_BR_CommandDispatcher_GetSmartUsage(LIBMATTI_BR_CommandDispatcher *self, LIBMATTI_BR_CommandNode *node,
                                                  const void *source);
// Java: public RootCommandNode<S> getRoot()
LIBMATTI_BR_CommandNode *LIBMATTI_BR_CommandDispatcher_GetRoot(const LIBMATTI_BR_CommandDispatcher *self);

void LIBMATTI_BR_CommandDispatcher_Free(LIBMATTI_BR_CommandDispatcher *self);

#endif
