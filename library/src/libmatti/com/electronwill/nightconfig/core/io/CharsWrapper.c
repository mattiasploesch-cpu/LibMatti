// Port of com.electronwill.nightconfig.core.io.CharsWrapper.

#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_Of(const char *chars, size_t length)
{
    LIBMATTI_NC_CharsWrapper wrapper;
    wrapper.data = (char *) chars;
    wrapper.length = length;
    return wrapper;
}

size_t LIBMATTI_NC_CharsWrapper_Length(const LIBMATTI_NC_CharsWrapper *wrapper)
{
    return wrapper->length;
}

char LIBMATTI_NC_CharsWrapper_Get(const LIBMATTI_NC_CharsWrapper *wrapper, size_t index)
{
    return wrapper->data[index];
}

int LIBMATTI_NC_CharsWrapper_IsEmpty(const LIBMATTI_NC_CharsWrapper *wrapper)
{
    return wrapper->length == 0;
}

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_SubView(const LIBMATTI_NC_CharsWrapper *wrapper, size_t from)
{
    return LIBMATTI_NC_CharsWrapper_SubView2(wrapper, from, wrapper->length);
}

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_SubView2(const LIBMATTI_NC_CharsWrapper *wrapper, size_t from,
                                                           size_t to)
{
    return LIBMATTI_NC_CharsWrapper_Of(wrapper->data + from, to - from);
}

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_CharsWrapper_TrimmedView(const LIBMATTI_NC_CharsWrapper *wrapper)
{
    size_t start = 0;
    size_t end = wrapper->length;
    while (start < end && wrapper->data[start] <= ' ') start++;
    while (end > start && wrapper->data[end - 1] <= ' ') end--;
    return LIBMATTI_NC_CharsWrapper_Of(wrapper->data + start, end - start);
}

int LIBMATTI_NC_CharsWrapper_ContentEquals(const LIBMATTI_NC_CharsWrapper *wrapper, const char *chars, size_t length)
{
    if (wrapper->length != length) return 0;
    return memcmp(wrapper->data, chars, length) == 0;
}

long LIBMATTI_NC_CharsWrapper_IndexOfFirst(const LIBMATTI_NC_CharsWrapper *wrapper, const char *chars, size_t length)
{
    for (size_t i = 0; i < wrapper->length; i++)
        for (size_t j = 0; j < length; j++)
            if (wrapper->data[i] == chars[j]) return (long) i;
    return -1;
}

int LIBMATTI_NC_CharsWrapper_StartsWith(const LIBMATTI_NC_CharsWrapper *wrapper, const char *prefix)
{
    size_t length = strlen(prefix);
    if (wrapper->length < length) return 0;
    return memcmp(wrapper->data, prefix, length) == 0;
}

char *LIBMATTI_NC_CharsWrapper_ToString(const LIBMATTI_NC_CharsWrapper *wrapper)
{
    char *result = malloc(wrapper->length + 1);
    memcpy(result, wrapper->data, wrapper->length);
    result[wrapper->length] = '\0';
    return result;
}

LIBMATTI_NC_CharsWrapper_Builder *LIBMATTI_NC_CharsWrapper_Builder_New(size_t initialCapacity)
{
    LIBMATTI_NC_CharsWrapper_Builder *builder = calloc(1, sizeof(*builder));
    builder->capacity = initialCapacity > 0 ? initialCapacity : 1;
    builder->data = malloc(builder->capacity + 1);
    builder->length = 0;
    builder->data[0] = '\0';
    return builder;
}

static void ensureCapacity(LIBMATTI_NC_CharsWrapper_Builder *builder, size_t extra)
{
    if (builder->length + extra <= builder->capacity) return;

    while (builder->capacity < builder->length + extra) builder->capacity *= 2;
    builder->data = realloc(builder->data, builder->capacity + 1);
}

void LIBMATTI_NC_CharsWrapper_Builder_AppendChar(LIBMATTI_NC_CharsWrapper_Builder *builder, char c)
{
    ensureCapacity(builder, 1);
    builder->data[builder->length++] = c;
    builder->data[builder->length] = '\0';
}

void LIBMATTI_NC_CharsWrapper_Builder_Append(LIBMATTI_NC_CharsWrapper_Builder *builder,
                                             const LIBMATTI_NC_CharsWrapper *wrapper)
{
    ensureCapacity(builder, wrapper->length);
    memcpy(builder->data + builder->length, wrapper->data, wrapper->length);
    builder->length += wrapper->length;
    builder->data[builder->length] = '\0';
}

// Java: public Builder append(String str)
void LIBMATTI_NC_CharsWrapper_Builder_AppendString(LIBMATTI_NC_CharsWrapper_Builder *builder, const char *str)
{
    size_t length = strlen(str);
    ensureCapacity(builder, length);
    memcpy(builder->data + builder->length, str, length);
    builder->length += length;
    builder->data[builder->length] = '\0';
}

// Java: write(char) writes the escaped character
void LIBMATTI_NC_CharsWrapper_Builder_Write(LIBMATTI_NC_CharsWrapper_Builder *builder, char c)
{
    LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, c);
}

char LIBMATTI_NC_CharsWrapper_Builder_Get(const LIBMATTI_NC_CharsWrapper_Builder *builder, size_t index)
{
    return builder->data[index];
}

char *LIBMATTI_NC_CharsWrapper_Builder_ToString(const LIBMATTI_NC_CharsWrapper_Builder *builder)
{
    return LIBMATTI_NC_CharsWrapper_Builder_ToStringFrom(builder, 0);
}

char *LIBMATTI_NC_CharsWrapper_Builder_ToStringFrom(const LIBMATTI_NC_CharsWrapper_Builder *builder, size_t from)
{
    char *result = malloc(builder->length - from + 1);
    memcpy(result, builder->data + from, builder->length - from);
    result[builder->length - from] = '\0';
    return result;
}

void LIBMATTI_NC_CharsWrapper_Builder_Free(LIBMATTI_NC_CharsWrapper_Builder *builder)
{
    if (builder == NULL) return;
    free(builder->data);
    free(builder);
}
