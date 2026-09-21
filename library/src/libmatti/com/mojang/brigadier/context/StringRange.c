// Port of com.mojang.brigadier.context.StringRange.

#include "libmatti/com/mojang/brigadier/context/StringRange.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_At(int pos)
{
    LIBMATTI_BR_StringRange range = {pos, pos};
    return range;
}

LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_Between(int start, int end)
{
    LIBMATTI_BR_StringRange range = {start, end};
    return range;
}

LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_Encompassing(const LIBMATTI_BR_StringRange *a,
                                                             const LIBMATTI_BR_StringRange *b)
{
    int start = a->start < b->start ? a->start : b->start;
    int end = a->end > b->end ? a->end : b->end;
    return LIBMATTI_BR_StringRange_Between(start, end);
}

char *LIBMATTI_BR_StringRange_Get(const LIBMATTI_BR_StringRange *range, const char *string)
{
    size_t length = (size_t) (range->end - range->start);
    char *result = malloc(length + 1);
    memcpy(result, string + range->start, length);
    result[length] = '\0';
    return result;
}

int LIBMATTI_BR_StringRange_IsEmpty(const LIBMATTI_BR_StringRange *range)
{
    return range->start == range->end;
}

int LIBMATTI_BR_StringRange_GetLength(const LIBMATTI_BR_StringRange *range)
{
    return range->end - range->start;
}
