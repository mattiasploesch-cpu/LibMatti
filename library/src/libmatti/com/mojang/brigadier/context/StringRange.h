// Port of com.mojang.brigadier.context.StringRange.

#ifndef MATTICRAFT_BR_STRINGRANGE_H
#define MATTICRAFT_BR_STRINGRANGE_H

// Java: public class StringRange
typedef struct LIBMATTI_BR_StringRange
{
    int start;
    int end;
} LIBMATTI_BR_StringRange;

// Java: public static StringRange at(final int pos)
LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_At(int pos);
// Java: public static StringRange between(final int start, final int end)
LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_Between(int start, int end);
// Java: public static StringRange encompassing(final StringRange a, final StringRange b)
LIBMATTI_BR_StringRange LIBMATTI_BR_StringRange_Encompassing(const LIBMATTI_BR_StringRange *a,
                                                             const LIBMATTI_BR_StringRange *b);

// Java: public String get(final String string) - the caller frees
char *LIBMATTI_BR_StringRange_Get(const LIBMATTI_BR_StringRange *range, const char *string);
// Java: public boolean isEmpty()
int LIBMATTI_BR_StringRange_IsEmpty(const LIBMATTI_BR_StringRange *range);
// Java: public int getLength()
int LIBMATTI_BR_StringRange_GetLength(const LIBMATTI_BR_StringRange *range);

#endif
