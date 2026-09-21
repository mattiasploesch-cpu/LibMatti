// Port of net.minecraft.nbt.ListTag.

#include "libmatti/net/minecraft/nbt/ListTag.h"

#include "libmatti/net/minecraft/nbt/CompoundTag.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_ListTag_New(void)
{
    LIBMATTI_MC_Nbt_ListTag *list = calloc(1, sizeof(LIBMATTI_MC_Nbt_ListTag));
    list->id = LIBMATTI_MC_Nbt_TAG_LIST;
    return list;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_CopyTag(const LIBMATTI_MC_Nbt_ListTag *list)
{
    LIBMATTI_MC_Nbt_ListTag *copy = LIBMATTI_MC_Nbt_ListTag_New();
    for (size_t i = 0; i < list->count; i++)
        LIBMATTI_MC_Nbt_ListTag_Add(copy, LIBMATTI_MC_Nbt_Tag_Copy(list->elements[i]));
    return (LIBMATTI_MC_Nbt_Tag *) copy;
}

void LIBMATTI_MC_Nbt_ListTag_FreeEntries(LIBMATTI_MC_Nbt_ListTag *list)
{
    if (list == NULL)
        return;
    for (size_t i = 0; i < list->count; i++)
        LIBMATTI_MC_Nbt_Tag_Free(list->elements[i]);
    free(list->elements);
    list->elements = NULL;
    list->count = 0;
    list->capacity = 0;
}

int LIBMATTI_MC_Nbt_ListTag_Size(const LIBMATTI_MC_Nbt_ListTag *list)
{
    return (int) list->count;
}

int LIBMATTI_MC_Nbt_ListTag_IsEmpty(const LIBMATTI_MC_Nbt_ListTag *list)
{
    return list->count == 0;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_Get(const LIBMATTI_MC_Nbt_ListTag *list, int index)
{
    if (index < 0 || (size_t) index >= list->count)
        return NULL;
    return list->elements[index];
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_Set(LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_Tag *tag)
{
    if (index < 0 || (size_t) index >= list->count)
    {
        LIBMATTI_MC_Nbt_Tag_Free(tag);
        return NULL;
    }
    LIBMATTI_MC_Nbt_Tag *replaced = list->elements[index];
    list->elements[index] = tag;
    return replaced;
}

static void ensure_capacity(LIBMATTI_MC_Nbt_ListTag *list, size_t needed)
{
    if (needed <= list->capacity)
        return;
    list->capacity = list->capacity > 0 ? list->capacity * 2 : 8;
    if (list->capacity < needed)
        list->capacity = needed;
    list->elements = realloc(list->elements, sizeof(LIBMATTI_MC_Nbt_Tag *) * list->capacity);
}

void LIBMATTI_MC_Nbt_ListTag_AddAt(LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_Tag *tag)
{
    if (index < 0 || (size_t) index > list->count)
    {
        LIBMATTI_MC_Nbt_Tag_Free(tag);
        return;
    }
    ensure_capacity(list, list->count + 1);
    for (size_t i = list->count; i > (size_t) index; i--)
        list->elements[i] = list->elements[i - 1];
    list->elements[index] = tag;
    list->count++;
}

void LIBMATTI_MC_Nbt_ListTag_Add(LIBMATTI_MC_Nbt_ListTag *list, LIBMATTI_MC_Nbt_Tag *tag)
{
    ensure_capacity(list, list->count + 1);
    list->elements[list->count++] = tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_RemoveAt(LIBMATTI_MC_Nbt_ListTag *list, int index)
{
    if (index < 0 || (size_t) index >= list->count)
        return NULL;
    LIBMATTI_MC_Nbt_Tag *removed = list->elements[index];
    for (size_t i = index; i + 1 < list->count; i++)
        list->elements[i] = list->elements[i + 1];
    list->count--;
    return removed;
}

void LIBMATTI_MC_Nbt_ListTag_Clear(LIBMATTI_MC_Nbt_ListTag *list)
{
    for (size_t i = 0; i < list->count; i++)
        LIBMATTI_MC_Nbt_Tag_Free(list->elements[i]);
    list->count = 0;
}

// Java: private static boolean isWrapper(CompoundTag tag) - size 1 and the empty-string key
static int is_wrapper(const LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    return compound->count == 1 && compound->keys[0][0] == '\0';
}

// Java: private static Tag tryUnwrap(CompoundTag tag)
static LIBMATTI_MC_Nbt_Tag *try_unwrap(LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    if (compound->count == 1)
    {
        LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_CompoundTag_Get(compound, "");
        if (tag != NULL)
            return tag;
    }
    return (LIBMATTI_MC_Nbt_Tag *) compound;
}

// Java: private static CompoundTag wrapElement(Tag tag) - new CompoundTag(Map.of("", tag))
static LIBMATTI_MC_Nbt_CompoundTag *wrap_element(LIBMATTI_MC_Nbt_Tag *tag)
{
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, "", tag);
    return compound;
}

// Java: private static Tag wrapIfNeeded(byte type, Tag tag) - lists of compounds keep
// compounds; every other list wraps the (first) non-compound element kinds
static LIBMATTI_MC_Nbt_Tag *wrap_if_needed(uint8_t elementType, LIBMATTI_MC_Nbt_Tag *tag)
{
    if (elementType != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return tag;
    if (tag->id == LIBMATTI_MC_Nbt_TAG_COMPOUND && !is_wrapper((LIBMATTI_MC_Nbt_CompoundTag *) tag))
        return tag;
    return (LIBMATTI_MC_Nbt_Tag *) wrap_element(tag);
}

void LIBMATTI_MC_Nbt_ListTag_AddAndUnwrap(LIBMATTI_MC_Nbt_ListTag *list, LIBMATTI_MC_Nbt_Tag *tag)
{
    if (tag->id == LIBMATTI_MC_Nbt_TAG_COMPOUND)
        LIBMATTI_MC_Nbt_ListTag_Add(list, try_unwrap((LIBMATTI_MC_Nbt_CompoundTag *) tag));
    else
        LIBMATTI_MC_Nbt_ListTag_Add(list, tag);
}

// Java: public byte identifyRawElementType() - 0 for the empty list, 10 when mixed
uint8_t LIBMATTI_MC_Nbt_ListTag_IdentifyRawElementType(const LIBMATTI_MC_Nbt_ListTag *list)
{
    uint8_t type = 0;
    for (size_t i = 0; i < list->count; i++)
    {
        uint8_t id = list->elements[i]->id;
        if (type == 0)
            type = id;
        else if (type != id)
            return LIBMATTI_MC_Nbt_TAG_COMPOUND;
    }
    return type;
}

int LIBMATTI_MC_Nbt_ListTag_GetCompound(const LIBMATTI_MC_Nbt_ListTag *list, int index,
                                        LIBMATTI_MC_Nbt_CompoundTag **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return 0;
    *out = (LIBMATTI_MC_Nbt_CompoundTag *) tag;
    return 1;
}

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_ListTag_GetCompoundOrEmpty(const LIBMATTI_MC_Nbt_ListTag *list, int index)
{
    LIBMATTI_MC_Nbt_CompoundTag *found;
    if (LIBMATTI_MC_Nbt_ListTag_GetCompound(list, index, &found))
        return found;
    return LIBMATTI_MC_Nbt_CompoundTag_New();
}

int LIBMATTI_MC_Nbt_ListTag_GetList(const LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_ListTag **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL || tag->id != LIBMATTI_MC_Nbt_TAG_LIST)
        return 0;
    *out = (LIBMATTI_MC_Nbt_ListTag *) tag;
    return 1;
}

LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_ListTag_GetListOrEmpty(const LIBMATTI_MC_Nbt_ListTag *list, int index)
{
    LIBMATTI_MC_Nbt_ListTag *found;
    if (LIBMATTI_MC_Nbt_ListTag_GetList(list, index, &found))
        return found;
    return LIBMATTI_MC_Nbt_ListTag_New();
}

int LIBMATTI_MC_Nbt_ListTag_GetShort(const LIBMATTI_MC_Nbt_ListTag *list, int index, int16_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_ShortValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_ListTag_GetInt(const LIBMATTI_MC_Nbt_ListTag *list, int index, int32_t *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_IntValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_ListTag_GetDouble(const LIBMATTI_MC_Nbt_ListTag *list, int index, double *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_ListTag_GetFloat(const LIBMATTI_MC_Nbt_ListTag *list, int index, float *out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_FloatValue(tag);
    return 1;
}

int LIBMATTI_MC_Nbt_ListTag_GetString(const LIBMATTI_MC_Nbt_ListTag *list, int index, const char **out)
{
    LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ListTag_Get(list, index);
    if (tag == NULL)
        return 0;
    *out = LIBMATTI_MC_Nbt_Tag_AsString(tag);
    return *out != NULL;
}

int16_t LIBMATTI_MC_Nbt_ListTag_GetShortOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, int16_t fallback)
{
    int16_t value;
    return LIBMATTI_MC_Nbt_ListTag_GetShort(list, index, &value) ? value : fallback;
}

int32_t LIBMATTI_MC_Nbt_ListTag_GetIntOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, int32_t fallback)
{
    int32_t value;
    return LIBMATTI_MC_Nbt_ListTag_GetInt(list, index, &value) ? value : fallback;
}

double LIBMATTI_MC_Nbt_ListTag_GetDoubleOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, double fallback)
{
    double value;
    return LIBMATTI_MC_Nbt_ListTag_GetDouble(list, index, &value) ? value : fallback;
}

float LIBMATTI_MC_Nbt_ListTag_GetFloatOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, float fallback)
{
    float value;
    return LIBMATTI_MC_Nbt_ListTag_GetFloat(list, index, &value) ? value : fallback;
}

const char *LIBMATTI_MC_Nbt_ListTag_GetStringOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, const char *fallback)
{
    const char *value;
    return LIBMATTI_MC_Nbt_ListTag_GetString(list, index, &value) ? value : fallback;
}
