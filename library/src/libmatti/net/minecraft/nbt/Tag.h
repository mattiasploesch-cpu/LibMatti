// Port of net.minecraft.nbt.Tag (the id constants) and the concrete tag types the
// C port keeps in one tagged union per tag object: EndTag, the primitive tags
// (ByteTag/ShortTag/IntTag/LongTag/FloatTag/DoubleTag/StringTag) and the array tags
// (ByteArrayTag/IntArrayTag/LongArrayTag). CompoundTag and ListTag are separate
// headers (they carry the container APIs); they embed the same header so every tag
// flows through the shared binary reader/writer.

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_TAG_H
#define MATTICRAFT_NET_MINECRAFT_NBT_TAG_H

#include <stddef.h>
#include <stdint.h>

// Java: the constants on the Tag interface
#define LIBMATTI_MC_Nbt_TAG_END          0
#define LIBMATTI_MC_Nbt_TAG_BYTE         1
#define LIBMATTI_MC_Nbt_TAG_SHORT        2
#define LIBMATTI_MC_Nbt_TAG_INT          3
#define LIBMATTI_MC_Nbt_TAG_LONG         4
#define LIBMATTI_MC_Nbt_TAG_FLOAT        5
#define LIBMATTI_MC_Nbt_TAG_DOUBLE       6
#define LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY   7
#define LIBMATTI_MC_Nbt_TAG_STRING       8
#define LIBMATTI_MC_Nbt_TAG_LIST         9
#define LIBMATTI_MC_Nbt_TAG_COMPOUND    10
#define LIBMATTI_MC_Nbt_TAG_INT_ARRAY   11
#define LIBMATTI_MC_Nbt_TAG_LONG_ARRAY  12

// Java: the tag records as a union; `id` is the discriminator
typedef struct LIBMATTI_MC_Nbt_Tag
{
    uint8_t id;
    union
    {
        // Java: ByteTag(byte value) - id 1
        int8_t byteValue;
        // Java: ShortTag(short value) - id 2
        int16_t shortValue;
        // Java: IntTag(int value) - id 3
        int32_t intValue;
        // Java: LongTag(long value) - id 4
        int64_t longValue;
        // Java: FloatTag(float value) - id 5
        float floatValue;
        // Java: DoubleTag(double value) - id 6
        double doubleValue;
        // Java: StringTag(String value) - id 8; owned by the tag
        char *stringValue;
        // Java: ByteArrayTag(byte[] value) - id 7; owned by the tag
        struct
        {
            int8_t *data;
            size_t length;
        } byteArray;
        // Java: IntArrayTag(int[] value) - id 11; owned by the tag
        struct
        {
            int32_t *data;
            size_t length;
        } intArray;
        // Java: LongArrayTag(long[] value) - id 12; owned by the tag
        struct
        {
            int64_t *data;
            size_t length;
        } longArray;
    } as;
} LIBMATTI_MC_Nbt_Tag;

// Java: the valueOf factories - the caller owns the returned tag and frees it
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ByteTag_Of(int8_t value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ShortTag_Of(int16_t value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_IntTag_Of(int32_t value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_LongTag_Of(int64_t value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_FloatTag_Of(float value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_DoubleTag_Of(double value);
// Java: StringTag.valueOf(String) - the string is copied
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_StringTag_Of(const char *value);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ByteArrayTag_Of(const int8_t *data, size_t length);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_IntArrayTag_Of(const int32_t *data, size_t length);
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_LongArrayTag_Of(const int64_t *data, size_t length);

// Java: EndTag.INSTANCE - a shared singleton (never freed)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_EndTag_Instance(void);

// allocate an empty tag of the primitive/array kinds
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_New(uint8_t id);
// deep copy (Java: Tag.copy()); compounds/lists dispatch into their own copy
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_Copy(const LIBMATTI_MC_Nbt_Tag *tag);
// free a tag owned by the caller (compound/list free their children)
void LIBMATTI_MC_Nbt_Tag_Free(LIBMATTI_MC_Nbt_Tag *tag);

// Java: NumericTag.byteValue()/shortValue()/... - the widening casts of Java; -1/0/NaN when the
// tag is not numeric (Java: Optional.empty on the Tag defaults)
int8_t LIBMATTI_MC_Nbt_Tag_ByteValue(const LIBMATTI_MC_Nbt_Tag *tag);
int16_t LIBMATTI_MC_Nbt_Tag_ShortValue(const LIBMATTI_MC_Nbt_Tag *tag);
int32_t LIBMATTI_MC_Nbt_Tag_IntValue(const LIBMATTI_MC_Nbt_Tag *tag);
int64_t LIBMATTI_MC_Nbt_Tag_LongValue(const LIBMATTI_MC_Nbt_Tag *tag);
float LIBMATTI_MC_Nbt_Tag_FloatValue(const LIBMATTI_MC_Nbt_Tag *tag);
double LIBMATTI_MC_Nbt_Tag_DoubleValue(const LIBMATTI_MC_Nbt_Tag *tag);
// Java: Tag.asString() - the value of a StringTag, NULL otherwise
const char *LIBMATTI_MC_Nbt_Tag_AsString(const LIBMATTI_MC_Nbt_Tag *tag);
// Java: Tag.asNumber() as double - 0.0 when not numeric (the caller checks the id)
double LIBMATTI_MC_Nbt_Tag_AsNumber(const LIBMATTI_MC_Nbt_Tag *tag);
// Java: Tag.asBoolean() - asByte() != 0
int LIBMATTI_MC_Nbt_Tag_AsBoolean(const LIBMATTI_MC_Nbt_Tag *tag);

// Java: Tag.equals semantics for the value tags (compound/list dispatch deeper)
int LIBMATTI_MC_Nbt_Tag_Equals(const LIBMATTI_MC_Nbt_Tag *a, const LIBMATTI_MC_Nbt_Tag *b);

// Java: StringTag.quoteAndEscape(String) - the caller frees the string
char *LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(const char *value);
// Java: StringTag.escapeWithoutQuotes(String) - the caller frees the string
char *LIBMATTI_MC_Nbt_StringTag_EscapeWithoutQuotes(const char *value);

// The container dispatch Tag.c needs; CompoundTag.h/ListTag.h provide the bodies and the
// full struct definitions
typedef struct LIBMATTI_MC_Nbt_CompoundTag LIBMATTI_MC_Nbt_CompoundTag;
typedef struct LIBMATTI_MC_Nbt_ListTag LIBMATTI_MC_Nbt_ListTag;

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_CopyContainer(const LIBMATTI_MC_Nbt_Tag *tag);
int LIBMATTI_MC_Nbt_Tag_EqualsContainer(const LIBMATTI_MC_Nbt_Tag *a, const LIBMATTI_MC_Nbt_Tag *b);
void LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(LIBMATTI_MC_Nbt_CompoundTag *compound);
void LIBMATTI_MC_Nbt_ListTag_FreeEntries(LIBMATTI_MC_Nbt_ListTag *list);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_TAG_H
