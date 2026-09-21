// Port of com.electronwill.nightconfig.core.io.CharsWrapper.

#ifndef MATTICRAFT_NC_CORE_IO_CHARSWRAPPER_H
#define MATTICRAFT_NC_CORE_IO_CHARSWRAPPER_H

#include <stddef.h>

// Java: public class CharsWrapper implements CharSequence
typedef struct
{
    char *data;
    size_t length;
} LIBMATTI_NC_CharsWrapper;

// Java: public static class Builder
typedef struct
{
    char *data;
    size_t length;
    size_t capacity;
} LIBMATTI_NC_CharsWrapper_Builder;

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_Of(const char *chars, size_t length);

// Java: public int length()
size_t LIBMATTI_NC_CharsWrapper_Length(const LIBMATTI_NC_CharsWrapper *wrapper);
// Java: public char charAt(int index) / get(int index)
char LIBMATTI_NC_CharsWrapper_Get(const LIBMATTI_NC_CharsWrapper *wrapper, size_t index);
// Java: public boolean isEmpty()
int LIBMATTI_NC_CharsWrapper_IsEmpty(const LIBMATTI_NC_CharsWrapper *wrapper);
// Java: public CharsWrapper subView(int from)
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_SubView(const LIBMATTI_NC_CharsWrapper *wrapper, size_t from);
// Java: public CharsWrapper subView(int from, int to)
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_SubView2(const LIBMATTI_NC_CharsWrapper *wrapper, size_t from,
                                                           size_t to);
// Java: public CharsWrapper trimmedView()
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_TrimmedView(const LIBMATTI_NC_CharsWrapper *wrapper);
// Java: public boolean contentEquals(char[] chars)
int LIBMATTI_NC_CharsWrapper_ContentEquals(const LIBMATTI_NC_CharsWrapper *wrapper, const char *chars, size_t length);
// Java: public int indexOfFirst(char[] chars) - -1 when absent
long LIBMATTI_NC_CharsWrapper_IndexOfFirst(const LIBMATTI_NC_CharsWrapper *wrapper, const char *chars, size_t length);
// Java: public boolean startsWith(String prefix)
int LIBMATTI_NC_CharsWrapper_StartsWith(const LIBMATTI_NC_CharsWrapper *wrapper, const char *prefix);
// Java: public String toString()
char *LIBMATTI_NC_CharsWrapper_ToString(const LIBMATTI_NC_CharsWrapper *wrapper);

// Java: public Builder(int initialCapacity)
LIBMATTI_NC_CharsWrapper_Builder *LIBMATTI_NC_CharsWrapper_Builder_New(size_t initialCapacity);
// Java: public Builder append(char c)
void LIBMATTI_NC_CharsWrapper_Builder_AppendChar(LIBMATTI_NC_CharsWrapper_Builder *builder, char c);
// Java: public Builder append(CharsWrapper wrapper)
void LIBMATTI_NC_CharsWrapper_Builder_Append(LIBMATTI_NC_CharsWrapper_Builder *builder,
                                             const LIBMATTI_NC_CharsWrapper *wrapper);
// Java: public Builder append(String str)
void LIBMATTI_NC_CharsWrapper_Builder_AppendString(LIBMATTI_NC_CharsWrapper_Builder *builder, const char *str);
// Java: public Builder write(char c)
void LIBMATTI_NC_CharsWrapper_Builder_Write(LIBMATTI_NC_CharsWrapper_Builder *builder, char c);
// Java: public char get(int index)
char LIBMATTI_NC_CharsWrapper_Builder_Get(const LIBMATTI_NC_CharsWrapper_Builder *builder, size_t index);
// Java: public String toString()
char *LIBMATTI_NC_CharsWrapper_Builder_ToString(const LIBMATTI_NC_CharsWrapper_Builder *builder);
// Java: public String toString(int from)
char *LIBMATTI_NC_CharsWrapper_Builder_ToStringFrom(const LIBMATTI_NC_CharsWrapper_Builder *builder, size_t from);
void LIBMATTI_NC_CharsWrapper_Builder_Free(LIBMATTI_NC_CharsWrapper_Builder *builder);

#endif //MATTICRAFT_NC_CORE_IO_CHARSWRAPPER_H
