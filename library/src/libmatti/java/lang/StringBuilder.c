#include "StringBuilder.h"

#include <stdlib.h>
#include <string.h>

static void ensure_capacity(LIBMATTI_JL_StringBuilder *builder, size_t needed)
{
    if (builder->capacity >= needed) return;

    size_t capacity = builder->capacity == 0 ? 16 : builder->capacity;
    while (capacity < needed) capacity *= 2;

    builder->value = realloc(builder->value, capacity);
    builder->capacity = capacity;
}

static void set_value(LIBMATTI_JL_StringBuilder *builder, const char *value)
{
    size_t length = strlen(value);
    ensure_capacity(builder, length + 1);
    memcpy(builder->value, value, length + 1);
    builder->length = length;
}

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_New(void)
{
    LIBMATTI_JL_StringBuilder *builder = calloc(1, sizeof(LIBMATTI_JL_StringBuilder));
    ensure_capacity(builder, 1);
    builder->value[0] = '\0';
    return builder;
}

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_NewFromString(const char *value)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_New();
    set_value(builder, value);
    return builder;
}

void LIBMATTI_JL_StringBuilder_Free(LIBMATTI_JL_StringBuilder *builder)
{
    if (builder == NULL) return;
    free(builder->value);
    free(builder);
}

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_Append(LIBMATTI_JL_StringBuilder *builder, const char *value)
{
    size_t length = strlen(value);
    ensure_capacity(builder, builder->length + length + 1);
    memcpy(builder->value + builder->length, value, length + 1);
    builder->length += length;
    return builder;
}

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_AppendChar(LIBMATTI_JL_StringBuilder *builder, char value)
{
    ensure_capacity(builder, builder->length + 2);
    builder->value[builder->length++] = value;
    builder->value[builder->length] = '\0';
    return builder;
}

size_t LIBMATTI_JL_StringBuilder_Length(const LIBMATTI_JL_StringBuilder *builder)
{
    return builder->length;
}

char LIBMATTI_JL_StringBuilder_CharAt(const LIBMATTI_JL_StringBuilder *builder, size_t index)
{
    return builder->value[index];
}

const char *LIBMATTI_JL_StringBuilder_ToString(const LIBMATTI_JL_StringBuilder *builder)
{
    return builder->value;
}
