// Port of net.minecraft.nbt.CompoundTag.

#include "libmatti/net/minecraft/nbt/CompoundTag.h"

#include "libmatti/net/minecraft/nbt/ListTag.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_CompoundTag_New(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *compound = calloc(1, sizeof(LIBMATTI_MC_Nbt_CompoundTag));
    compound->id = LIBMATTI_MC_Nbt_TAG_COMPOUND;
    return compound;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_CopyTag(const LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    LIBMATTI_MC_Nbt_CompoundTag *copy = LIBMATTI_MC_Nbt_CompoundTag_New();
    for (size_t i = 0; i < compound->count; i++)
    {
        LIBMATTI_MC_Nbt_Tag *value = LIBMATTI_MC_Nbt_Tag_Copy(compound->values[i]);
        LIBMATTI_MC_Nbt_CompoundTag_Put(copy, compound->keys[i], value);
    }
    return (LIBMATTI_MC_Nbt_Tag *) copy;
}

// the LIBMATTI_MC_Nbt_Tag_Free dispatch for TAG_COMPOUND
void LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    if (compound == NULL)
        return;
    for (size_t i = 0; i < compound->count; i++)
    {
        free(compound->keys[i]);
        LIBMATTI_MC_Nbt_Tag_Free(compound->values[i]);
    }
    free(compound->keys);
    free(compound->values);
    compound->keys = NULL;
    compound->values = NULL;
    compound->count = 0;
    compound->capacity = 0;
}

char **LIBMATTI_MC_Nbt_CompoundTag_KeySet(const LIBMATTI_MC_Nbt_CompoundTag *compound, size_t *count)
{
    *count = compound->count;
    return compound->keys;
}

int LIBMATTI_MC_Nbt_CompoundTag_Size(const LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    return (int) compound->count;
}

int LIBMATTI_MC_Nbt_CompoundTag_IsEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    return compound->count == 0;
}

// Java: public @Nullable Tag put(String key, Tag tag)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Put(LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                     const char *key, LIBMATTI_MC_Nbt_Tag *tag)
{
    for (size_t i = 0; i < compound->count; i++)
    {
        if (strcmp(compound->keys[i], key) == 0)
        {
            LIBMATTI_MC_Nbt_Tag *replaced = compound->values[i];
            compound->values[i] = tag;
            return replaced;
        }
    }
    if (compound->count == compound->capacity)
    {
        compound->capacity = compound->capacity > 0 ? compound->capacity * 2 : 8;
        compound->keys = realloc(compound->keys, sizeof(char *) * compound->capacity);
        compound->values = realloc(compound->values, sizeof(LIBMATTI_MC_Nbt_Tag *) * compound->capacity);
    }
    compound->keys[compound->count] = strdup(key);
    compound->values[compound->count] = tag;
    compound->count++;
    return NULL;
}

void LIBMATTI_MC_Nbt_CompoundTag_PutByte(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_ByteTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutShort(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_ShortTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutInt(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_IntTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutLong(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_LongTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutFloat(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_FloatTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutDouble(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_DoubleTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutString(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, const char *value)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_StringTag_Of(value));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                              const int8_t *value, size_t length)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_ByteArrayTag_Of(value, length));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int32_t *value, size_t length)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_IntArrayTag_Of(value, length));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                              const int64_t *value, size_t length)
{
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, LIBMATTI_MC_Nbt_LongArrayTag_Of(value, length));
}

void LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int value)
{
    LIBMATTI_MC_Nbt_CompoundTag_PutByte(compound, key, value ? 1 : 0);
}

// Java: private static int hash(String key) - Java's String.hashCode to keep the iteration
// order comparable (not required for correctness of the map itself)
static size_t find_key(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key)
{
    for (size_t i = 0; i < compound->count; i++)
        if (strcmp(compound->keys[i], key) == 0)
            return i;
    return (size_t) -1;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Get(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key)
{
    size_t index = find_key(compound, key);
    return index == (size_t) -1 ? NULL : compound->values[index];
}

int LIBMATTI_MC_Nbt_CompoundTag_Contains(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key)
{
    return find_key(compound, key) != (size_t) -1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetByte(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_ByteValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetShort(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_ShortValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetInt(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_IntValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetLong(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_LongValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetFloat(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_FloatValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetDouble(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetString(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                          const char **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_AsString(tag);
    return *out != NULL;
}

int8_t LIBMATTI_MC_Nbt_CompoundTag_GetByteOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t fallback)
{
    int8_t value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetByte(compound, key, &value) ? value : fallback;
}

int16_t LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t fallback)
{
    int16_t value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetShort(compound, key, &value) ? value : fallback;
}

int32_t LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t fallback)
{
    int32_t value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetInt(compound, key, &value) ? value : fallback;
}

int64_t LIBMATTI_MC_Nbt_CompoundTag_GetLongOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t fallback)
{
    int64_t value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetLong(compound, key, &value) ? value : fallback;
}

float LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float fallback)
{
    float value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetFloat(compound, key, &value) ? value : fallback;
}

double LIBMATTI_MC_Nbt_CompoundTag_GetDoubleOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double fallback)
{
    double value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetDouble(compound, key, &value) ? value : fallback;
}

const char *LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                                    const char *fallback)
{
    const char *value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetString(compound, key, &value) ? value : fallback;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetBoolean(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_AsBoolean(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int fallback)
{
    int value;
    return LIBMATTI_MC_Nbt_CompoundTag_GetBoolean(compound, key, &value) ? value : fallback;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int8_t **out, size_t *length)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY)
        return 0;
    *out = tag->as.byteArray.data;
    *length = tag->as.byteArray.length;
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                            const int32_t **out, size_t *length)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_INT_ARRAY)
        return 0;
    *out = tag->as.intArray.data;
    *length = tag->as.intArray.length;
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int64_t **out, size_t *length)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_LONG_ARRAY)
        return 0;
    *out = tag->as.longArray.data;
    *length = tag->as.longArray.length;
    return 1;
}

int LIBMATTI_MC_Nbt_CompoundTag_GetCompound(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                            LIBMATTI_MC_Nbt_CompoundTag **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return 0;
    *out = (LIBMATTI_MC_Nbt_CompoundTag *) tag;
    return 1;
}

// Java: public CompoundTag getCompoundOrEmpty(String) - the empty fallback is a fresh compound
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_CompoundTag_GetCompoundOrEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                                            const char *key)
{
    LIBMATTI_MC_Nbt_CompoundTag *found;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetCompound(compound, key, &found))
        return found;
    return LIBMATTI_MC_Nbt_CompoundTag_New();
}

int LIBMATTI_MC_Nbt_CompoundTag_GetList(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                        LIBMATTI_MC_Nbt_ListTag **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, key);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_LIST)
        return 0;
    *out = (LIBMATTI_MC_Nbt_ListTag *) tag;
    return 1;
}

LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_CompoundTag_GetListOrEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                                    const char *key)
{
    LIBMATTI_MC_Nbt_ListTag *found;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(compound, key, &found))
        return found;
    return LIBMATTI_MC_Nbt_ListTag_New();
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Remove(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key)
{
    size_t index = find_key(compound, key);
    if (index == (size_t) -1)
        return NULL;
    LIBMATTI_MC_Nbt_Tag *removed = compound->values[index];
    free(compound->keys[index]);
    for (size_t i = index; i + 1 < compound->count; i++)
    {
        compound->keys[i] = compound->keys[i + 1];
        compound->values[i] = compound->values[i + 1];
    }
    compound->count--;
    return removed;
}

// Java: public CompoundTag merge(CompoundTag other)
void LIBMATTI_MC_Nbt_CompoundTag_Merge(LIBMATTI_MC_Nbt_CompoundTag *compound,
                                       const LIBMATTI_MC_Nbt_CompoundTag *other)
{
    for (size_t i = 0; i < other->count; i++)
    {
        LIBMATTI_MC_Nbt_Tag *tag = other->values[i];
        LIBMATTI_MC_Nbt_Tag *existing = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, other->keys[i]);
        if (tag->id == LIBMATTI_MC_Nbt_TAG_COMPOUND && existing != NULL &&
            existing->id == LIBMATTI_MC_Nbt_TAG_COMPOUND)
        {
            LIBMATTI_MC_Nbt_CompoundTag_Merge((LIBMATTI_MC_Nbt_CompoundTag *) existing,
                                              (const LIBMATTI_MC_Nbt_CompoundTag *) tag);
        }
        else
        {
            LIBMATTI_MC_Nbt_Tag *replaced =
                LIBMATTI_MC_Nbt_CompoundTag_Put(compound, other->keys[i], LIBMATTI_MC_Nbt_Tag_Copy(tag));
            LIBMATTI_MC_Nbt_Tag_Free(replaced);
        }
    }
}
