// Port of com.mojang.brigadier.context.ParsedArgument.

#include "libmatti/com/mojang/brigadier/context/ParsedArgument.h"

#include <stdlib.h>

LIBMATTI_BR_ParsedArgument *LIBMATTI_BR_ParsedArgument_New(int start, int end, LIBMATTI_BR_ArgumentKind kind,
                                                           LIBMATTI_BR_ArgumentValue result)
{
    LIBMATTI_BR_ParsedArgument *argument = calloc(1, sizeof(LIBMATTI_BR_ParsedArgument));
    argument->range = LIBMATTI_BR_StringRange_Between(start, end);
    argument->kind = kind;
    argument->result = result;
    return argument;
}

const LIBMATTI_BR_ArgumentValue *LIBMATTI_BR_ParsedArgument_GetResult(const LIBMATTI_BR_ParsedArgument *argument)
{
    return &argument->result;
}

const LIBMATTI_BR_StringRange *LIBMATTI_BR_ParsedArgument_GetRange(const LIBMATTI_BR_ParsedArgument *argument)
{
    return &argument->range;
}

void LIBMATTI_BR_ParsedArgument_Free(LIBMATTI_BR_ParsedArgument *argument)
{
    if (argument->kind == LIBMATTI_BR_ARG_STRING) free(argument->result.stringValue);
    free(argument);
}
