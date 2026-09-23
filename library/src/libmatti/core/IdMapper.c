// Port of net.minecraft.core.IdMapper (implementation).

#include "libmatti/core/IdMapper.h"

#include <stdlib.h>

LIBMATTI_JL_IdMapper *LIBMATTI_JL_IdMapper_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JL_IdMapper));
}

void LIBMATTI_JL_IdMapper_Free(LIBMATTI_JL_IdMapper *mapper)
{
    if (mapper == NULL)
        return;
    free(mapper->values);
    free(mapper->ids);
    free(mapper);
}

void LIBMATTI_JL_IdMapper_AddMapping(LIBMATTI_JL_IdMapper *mapper, void *value, int id)
{
    if (mapper->size == mapper->capacity)
    {
        int capacity = mapper->capacity == 0 ? 32 : mapper->capacity * 2;
        mapper->values = realloc(mapper->values, (size_t) capacity * sizeof(void *));
        mapper->ids = realloc(mapper->ids, (size_t) capacity * sizeof(int));
        mapper->capacity = capacity;
    }
    mapper->values[mapper->size] = value;
    mapper->ids[mapper->size] = id;
    mapper->size++;
}

void *LIBMATTI_JL_IdMapper_ById(const LIBMATTI_JL_IdMapper *mapper, int id)
{
    for (int i = 0; i < mapper->size; i++)
    {
        if (mapper->ids[i] == id)
            return mapper->values[i];
    }
    return NULL;
}

int LIBMATTI_JL_IdMapper_GetId(const LIBMATTI_JL_IdMapper *mapper, const void *value)
{
    for (int i = 0; i < mapper->size; i++)
    {
        if (mapper->values[i] == value)
            return mapper->ids[i];
    }
    return -1;
}

int LIBMATTI_JL_IdMapper_Size(const LIBMATTI_JL_IdMapper *mapper)
{
    return mapper->size;
}
