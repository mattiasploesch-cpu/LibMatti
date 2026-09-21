// Port of net.minecraft.nbt.ListTag.
// Java: public final class ListTag extends AbstractList<Tag> implements CollectionTag - the
// element tags are owned by the list. The wrapper-compound logic (lists of non-compound tags
// wrap each element into {"" : tag} on the wire) is carried over 1:1.

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_LISTTAG_H
#define MATTICRAFT_NET_MINECRAFT_NBT_LISTTAG_H

#include "libmatti/net/minecraft/nbt/Tag.h"

// Java: public final class ListTag - id 9
typedef struct LIBMATTI_MC_Nbt_ListTag
{
    uint8_t id;
    // Java: private final List<Tag> list - owned by the list
    LIBMATTI_MC_Nbt_Tag **elements;
    size_t count;
    size_t capacity;
} LIBMATTI_MC_Nbt_ListTag;

// Java: public ListTag()
LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_ListTag_New(void);
// Java: public ListTag copy() as a Tag
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_CopyTag(const LIBMATTI_MC_Nbt_ListTag *list);
// the LIBMATTI_MC_Nbt_Tag_Free dispatch for TAG_LIST
void LIBMATTI_MC_Nbt_ListTag_FreeEntries(LIBMATTI_MC_Nbt_ListTag *list);

// Java: public int size()
int LIBMATTI_MC_Nbt_ListTag_Size(const LIBMATTI_MC_Nbt_ListTag *list);
// Java: public boolean isEmpty()
int LIBMATTI_MC_Nbt_ListTag_IsEmpty(const LIBMATTI_MC_Nbt_ListTag *list);
// Java: public Tag get(int index) - NULL when out of bounds (Java throws)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_Get(const LIBMATTI_MC_Nbt_ListTag *list, int index);
// Java: public Tag set(int index, Tag tag) - returns the replaced tag (caller frees)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_Set(LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_Tag *tag);
// Java: public void add(int index, Tag tag) / add(Tag) - the list takes ownership
void LIBMATTI_MC_Nbt_ListTag_Add(LIBMATTI_MC_Nbt_ListTag *list, LIBMATTI_MC_Nbt_Tag *tag);
void LIBMATTI_MC_Nbt_ListTag_AddAt(LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_Tag *tag);
// Java: public Tag remove(int index) - the caller frees the returned tag
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ListTag_RemoveAt(LIBMATTI_MC_Nbt_ListTag *list, int index);
// Java: public void clear()
void LIBMATTI_MC_Nbt_ListTag_Clear(LIBMATTI_MC_Nbt_ListTag *list);

// Java: public void addAndUnwrap(Tag tag) - unwraps single-entry {"" : tag} compounds
void LIBMATTI_MC_Nbt_ListTag_AddAndUnwrap(LIBMATTI_MC_Nbt_ListTag *list, LIBMATTI_MC_Nbt_Tag *tag);
// Java: byte identifyRawElementType() - visible for testing in Java
uint8_t LIBMATTI_MC_Nbt_ListTag_IdentifyRawElementType(const LIBMATTI_MC_Nbt_ListTag *list);

// Java: public Optional<CompoundTag> getCompound(int)
int LIBMATTI_MC_Nbt_ListTag_GetCompound(const LIBMATTI_MC_Nbt_ListTag *list, int index,
                                        LIBMATTI_MC_Nbt_CompoundTag **out);
// Java: public CompoundTag getCompoundOrEmpty(int)
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_ListTag_GetCompoundOrEmpty(const LIBMATTI_MC_Nbt_ListTag *list, int index);
// Java: public Optional<ListTag> getList(int)
int LIBMATTI_MC_Nbt_ListTag_GetList(const LIBMATTI_MC_Nbt_ListTag *list, int index, LIBMATTI_MC_Nbt_ListTag **out);
// Java: public ListTag getListOrEmpty(int)
LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_ListTag_GetListOrEmpty(const LIBMATTI_MC_Nbt_ListTag *list, int index);
// Java: public Optional<Short> getShort(int) etc.
int LIBMATTI_MC_Nbt_ListTag_GetShort(const LIBMATTI_MC_Nbt_ListTag *list, int index, int16_t *out);
int LIBMATTI_MC_Nbt_ListTag_GetInt(const LIBMATTI_MC_Nbt_ListTag *list, int index, int32_t *out);
int LIBMATTI_MC_Nbt_ListTag_GetDouble(const LIBMATTI_MC_Nbt_ListTag *list, int index, double *out);
int LIBMATTI_MC_Nbt_ListTag_GetFloat(const LIBMATTI_MC_Nbt_ListTag *list, int index, float *out);
int LIBMATTI_MC_Nbt_ListTag_GetString(const LIBMATTI_MC_Nbt_ListTag *list, int index, const char **out);
// the *Or fallbacks
int16_t LIBMATTI_MC_Nbt_ListTag_GetShortOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, int16_t fallback);
int32_t LIBMATTI_MC_Nbt_ListTag_GetIntOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, int32_t fallback);
double LIBMATTI_MC_Nbt_ListTag_GetDoubleOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, double fallback);
float LIBMATTI_MC_Nbt_ListTag_GetFloatOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, float fallback);
const char *LIBMATTI_MC_Nbt_ListTag_GetStringOr(const LIBMATTI_MC_Nbt_ListTag *list, int index, const char *fallback);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_LISTTAG_H
