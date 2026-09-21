// Port of com.mojang.brigadier.context.ParsedArgument and ParsedCommandNode.

#ifndef MATTICRAFT_BR_CONTEXT_PARSEDARGUMENT_H
#define MATTICRAFT_BR_CONTEXT_PARSEDARGUMENT_H

#include "libmatti/com/mojang/brigadier/arguments/ArgumentType.h"
#include "libmatti/com/mojang/brigadier/context/StringRange.h"

#include <stddef.h>

typedef struct LIBMATTI_BR_CommandNode LIBMATTI_BR_CommandNode;

// Java: public class ParsedArgument<S, T>
typedef struct LIBMATTI_BR_ParsedArgument
{
    LIBMATTI_BR_StringRange range;
    LIBMATTI_BR_ArgumentKind kind;
    LIBMATTI_BR_ArgumentValue result;
} LIBMATTI_BR_ParsedArgument;

// Java: public ParsedArgument(final int start, final int end, final T result)
// For a string result the union owns the malloc'd text.
LIBMATTI_BR_ParsedArgument *LIBMATTI_BR_ParsedArgument_New(int start, int end, LIBMATTI_BR_ArgumentKind kind,
                                                           LIBMATTI_BR_ArgumentValue result);
// Java: public T getResult()
const LIBMATTI_BR_ArgumentValue *LIBMATTI_BR_ParsedArgument_GetResult(const LIBMATTI_BR_ParsedArgument *argument);
// Java: public StringRange getRange()
const LIBMATTI_BR_StringRange *LIBMATTI_BR_ParsedArgument_GetRange(const LIBMATTI_BR_ParsedArgument *argument);
void LIBMATTI_BR_ParsedArgument_Free(LIBMATTI_BR_ParsedArgument *argument);

// Java: public class ParsedCommandNode<S>
typedef struct LIBMATTI_BR_ParsedCommandNode
{
    LIBMATTI_BR_CommandNode *node;
    LIBMATTI_BR_StringRange range;
} LIBMATTI_BR_ParsedCommandNode;

#endif
