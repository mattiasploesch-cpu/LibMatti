// Port of net.minecraft.nbt.CompoundTag.
// Java backs the tag with a HashMap<String, Tag>; the C port keeps a growable
// key/value array with string keys owned by the compound. The put/get surface
// follows the Java methods 1:1 (putByte..putBoolean, getByte/getByteOr, ...);
// the Codec/store/read parts are the DFU port's business.

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_COMPOUNDTAG_H
#define MATTICRAFT_NET_MINECRAFT_NBT_COMPOUNDTAG_H

#include "libmatti/net/minecraft/nbt/Tag.h"

// Java: public final class CompoundTag implements Tag - id 10
typedef struct LIBMATTI_MC_Nbt_CompoundTag
{
    // the Tag header (id = TAG_COMPOUND)
    uint8_t id;
    // Java: private final Map<String, Tag> tags - keys owned by the compound, values owned too
    char **keys;
    LIBMATTI_MC_Nbt_Tag **values;
    size_t count;
    size_t capacity;
} LIBMATTI_MC_Nbt_CompoundTag;

// Java: public CompoundTag()
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_CompoundTag_New(void);
// Java: CompoundTag copy() as a Tag
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_CopyTag(const LIBMATTI_MC_Nbt_CompoundTag *compound);
// free the compound and every entry (the LIBMATTI_MC_Nbt_Tag_Free dispatch)
void LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(LIBMATTI_MC_Nbt_CompoundTag *compound);

// Java: public Set<String> keySet()
char **LIBMATTI_MC_Nbt_CompoundTag_KeySet(const LIBMATTI_MC_Nbt_CompoundTag *compound, size_t *count);
// Java: public int size()
int LIBMATTI_MC_Nbt_CompoundTag_Size(const LIBMATTI_MC_Nbt_CompoundTag *compound);
// Java: public boolean isEmpty()
int LIBMATTI_MC_Nbt_CompoundTag_IsEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound);

// Java: public @Nullable Tag put(String key, Tag tag) - the compound takes ownership of the
// tag; the replaced tag is returned for the caller to free (Java hands it back too)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Put(LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                     const char *key, LIBMATTI_MC_Nbt_Tag *tag);
// Java: the typed puts - each wraps the value into the matching tag
void LIBMATTI_MC_Nbt_CompoundTag_PutByte(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t value);
void LIBMATTI_MC_Nbt_CompoundTag_PutShort(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t value);
void LIBMATTI_MC_Nbt_CompoundTag_PutInt(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t value);
void LIBMATTI_MC_Nbt_CompoundTag_PutLong(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t value);
void LIBMATTI_MC_Nbt_CompoundTag_PutFloat(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float value);
void LIBMATTI_MC_Nbt_CompoundTag_PutDouble(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double value);
void LIBMATTI_MC_Nbt_CompoundTag_PutString(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, const char *value);
void LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                              const int8_t *value, size_t length);
void LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int32_t *value, size_t length);
void LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                              const int64_t *value, size_t length);
// Java: public void putBoolean(String key, boolean value) - ByteTag.valueOf(value)
void LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int value);

// Java: public @Nullable Tag get(String key)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Get(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key);
// Java: public boolean contains(String key)
int LIBMATTI_MC_Nbt_CompoundTag_Contains(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key);

// Java: public Optional<Byte> getByte(String) - 1/0 found flag, out only when found
int LIBMATTI_MC_Nbt_CompoundTag_GetByte(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t *out);
int LIBMATTI_MC_Nbt_CompoundTag_GetShort(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t *out);
int LIBMATTI_MC_Nbt_CompoundTag_GetInt(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t *out);
int LIBMATTI_MC_Nbt_CompoundTag_GetLong(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t *out);
int LIBMATTI_MC_Nbt_CompoundTag_GetFloat(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float *out);
int LIBMATTI_MC_Nbt_CompoundTag_GetDouble(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double *out);
// Java: public Optional<String> getString(String)
int LIBMATTI_MC_Nbt_CompoundTag_GetString(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, const char **out);

// Java: the *Or fallbacks - the default when the tag is missing or of another kind
int8_t LIBMATTI_MC_Nbt_CompoundTag_GetByteOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int8_t fallback);
int16_t LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int16_t fallback);
int32_t LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int32_t fallback);
int64_t LIBMATTI_MC_Nbt_CompoundTag_GetLongOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int64_t fallback);
float LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, float fallback);
double LIBMATTI_MC_Nbt_CompoundTag_GetDoubleOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, double fallback);
// Java: public String getStringOr(String, String) - the fallback string is returned directly
const char *LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                                    const char *fallback);

// Java: public Optional<Boolean> getBoolean(String)
int LIBMATTI_MC_Nbt_CompoundTag_GetBoolean(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int *out);
// Java: public boolean getBooleanOr(String, boolean)
int LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key, int fallback);

// Java: the array getters - found flag + out pointer into the tag (valid while the tag lives)
int LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int8_t **out, size_t *length);
int LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                            const int32_t **out, size_t *length);
int LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                             const int64_t **out, size_t *length);

// Java: public Optional<CompoundTag> getCompound(String)
int LIBMATTI_MC_Nbt_CompoundTag_GetCompound(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                            LIBMATTI_MC_Nbt_CompoundTag **out);
// Java: public CompoundTag getCompoundOrEmpty(String) - a fresh empty compound when missing
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_CompoundTag_GetCompoundOrEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                                            const char *key);
// Java: public Optional<ListTag> getList(String)
int LIBMATTI_MC_Nbt_CompoundTag_GetList(const LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key,
                                        LIBMATTI_MC_Nbt_ListTag **out);
// Java: public ListTag getListOrEmpty(String)
LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_Nbt_CompoundTag_GetListOrEmpty(const LIBMATTI_MC_Nbt_CompoundTag *compound,
                                                                    const char *key);

// Java: public @Nullable Tag remove(String key) - the caller frees the returned tag
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_CompoundTag_Remove(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *key);

// Java: public CompoundTag merge(CompoundTag other) - deep for compound values, copy otherwise
void LIBMATTI_MC_Nbt_CompoundTag_Merge(LIBMATTI_MC_Nbt_CompoundTag *compound,
                                       const LIBMATTI_MC_Nbt_CompoundTag *other);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_COMPOUNDTAG_H
