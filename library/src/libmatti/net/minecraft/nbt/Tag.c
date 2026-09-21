// Port of net.minecraft.nbt.Tag (value factories and NumericTag conversions) plus
// StringTag.quoteAndEscape/escapeWithoutQuotes and SnbtGrammar.escapeControlCharacters.

#include "libmatti/net/minecraft/nbt/Tag.h"

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// The container dispatch (CompoundTag.c / ListTag.c provide the real bodies)
// ---------------------------------------------------------------------------

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_CopyContainer(const LIBMATTI_MC_Nbt_Tag *tag)
{
    if (tag->id == LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return LIBMATTI_MC_Nbt_CompoundTag_CopyTag((const LIBMATTI_MC_Nbt_CompoundTag *) tag);
    return LIBMATTI_MC_Nbt_ListTag_CopyTag((const LIBMATTI_MC_Nbt_ListTag *) tag);
}

int LIBMATTI_MC_Nbt_Tag_EqualsContainer(const LIBMATTI_MC_Nbt_Tag *a, const LIBMATTI_MC_Nbt_Tag *b)
{
    if (a->id == LIBMATTI_MC_Nbt_TAG_COMPOUND)
    {
        const LIBMATTI_MC_Nbt_CompoundTag *left = (const LIBMATTI_MC_Nbt_CompoundTag *) a;
        const LIBMATTI_MC_Nbt_CompoundTag *right = (const LIBMATTI_MC_Nbt_CompoundTag *) b;
        if (left->count != right->count)
            return 0;
        // Java: Objects.equals(this.tags, other.tags) - order-independent map equality
        for (size_t i = 0; i < left->count; i++)
        {
            LIBMATTI_MC_Nbt_Tag *other = LIBMATTI_MC_Nbt_CompoundTag_Get(right, left->keys[i]);
            if (other == NULL || !LIBMATTI_MC_Nbt_Tag_Equals(left->values[i], other))
                return 0;
        }
        return 1;
    }
    const LIBMATTI_MC_Nbt_ListTag *left = (const LIBMATTI_MC_Nbt_ListTag *) a;
    const LIBMATTI_MC_Nbt_ListTag *right = (const LIBMATTI_MC_Nbt_ListTag *) b;
    if (left->count != right->count)
        return 0;
    for (size_t i = 0; i < left->count; i++)
        if (!LIBMATTI_MC_Nbt_Tag_Equals(left->elements[i], right->elements[i]))
            return 0;
    return 1;
}

static LIBMATTI_MC_Nbt_Tag *tag_new(uint8_t id)
{
    LIBMATTI_MC_Nbt_Tag *tag = calloc(1, sizeof(LIBMATTI_MC_Nbt_Tag));
    tag->id = id;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ByteTag_Of(int8_t value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_BYTE);
    tag->as.byteValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ShortTag_Of(int16_t value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_SHORT);
    tag->as.shortValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_IntTag_Of(int32_t value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_INT);
    tag->as.intValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_LongTag_Of(int64_t value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_LONG);
    tag->as.longValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_FloatTag_Of(float value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_FLOAT);
    tag->as.floatValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_DoubleTag_Of(double value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_DOUBLE);
    tag->as.doubleValue = value;
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_StringTag_Of(const char *value)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_STRING);
    tag->as.stringValue = strdup(value);
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_ByteArrayTag_Of(const int8_t *data, size_t length)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY);
    tag->as.byteArray.length = length;
    tag->as.byteArray.data = malloc(length > 0 ? length : 1);
    if (data != NULL)
        memcpy(tag->as.byteArray.data, data, length);
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_IntArrayTag_Of(const int32_t *data, size_t length)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_INT_ARRAY);
    tag->as.intArray.length = length;
    tag->as.intArray.data = malloc(length > 0 ? length * sizeof(int32_t) : sizeof(int32_t));
    if (data != NULL)
        memcpy(tag->as.intArray.data, data, length * sizeof(int32_t));
    return tag;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_LongArrayTag_Of(const int64_t *data, size_t length)
{
    LIBMATTI_MC_Nbt_Tag *tag = tag_new(LIBMATTI_MC_Nbt_TAG_LONG_ARRAY);
    tag->as.longArray.length = length;
    tag->as.longArray.data = malloc(length > 0 ? length * sizeof(int64_t) : sizeof(int64_t));
    if (data != NULL)
        memcpy(tag->as.longArray.data, data, length * sizeof(int64_t));
    return tag;
}

// Java: EndTag.INSTANCE
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_EndTag_Instance(void)
{
    static LIBMATTI_MC_Nbt_Tag end = {LIBMATTI_MC_Nbt_TAG_END, {0}};
    return &end;
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_New(uint8_t id)
{
    return tag_new(id);
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_Tag_Copy(const LIBMATTI_MC_Nbt_Tag *tag)
{
    if (tag == NULL || tag->id == LIBMATTI_MC_Nbt_TAG_END)
        return LIBMATTI_MC_Nbt_EndTag_Instance();
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        return LIBMATTI_MC_Nbt_ByteTag_Of(tag->as.byteValue);
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        return LIBMATTI_MC_Nbt_ShortTag_Of(tag->as.shortValue);
    case LIBMATTI_MC_Nbt_TAG_INT:
        return LIBMATTI_MC_Nbt_IntTag_Of(tag->as.intValue);
    case LIBMATTI_MC_Nbt_TAG_LONG:
        return LIBMATTI_MC_Nbt_LongTag_Of(tag->as.longValue);
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        return LIBMATTI_MC_Nbt_FloatTag_Of(tag->as.floatValue);
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        return LIBMATTI_MC_Nbt_DoubleTag_Of(tag->as.doubleValue);
    case LIBMATTI_MC_Nbt_TAG_STRING:
        return LIBMATTI_MC_Nbt_StringTag_Of(tag->as.stringValue);
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
        return LIBMATTI_MC_Nbt_ByteArrayTag_Of(tag->as.byteArray.data, tag->as.byteArray.length);
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
        return LIBMATTI_MC_Nbt_IntArrayTag_Of(tag->as.intArray.data, tag->as.intArray.length);
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
        return LIBMATTI_MC_Nbt_LongArrayTag_Of(tag->as.longArray.data, tag->as.longArray.length);
    default:
        // Java: CompoundTag.copy() / ListTag.copy() - the containers dispatch through their
        // own copy functions; Tag.h forward-declares the compound/list copies
        return LIBMATTI_MC_Nbt_Tag_CopyContainer(tag);
    }
}

void LIBMATTI_MC_Nbt_Tag_Free(LIBMATTI_MC_Nbt_Tag *tag)
{
    if (tag == NULL || tag == LIBMATTI_MC_Nbt_EndTag_Instance())
        return;
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_STRING:
        free(tag->as.stringValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
        free(tag->as.byteArray.data);
        break;
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
        free(tag->as.intArray.data);
        break;
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
        free(tag->as.longArray.data);
        break;
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries((struct LIBMATTI_MC_Nbt_CompoundTag *) tag);
        break;
    case LIBMATTI_MC_Nbt_TAG_LIST:
        LIBMATTI_MC_Nbt_ListTag_FreeEntries((struct LIBMATTI_MC_Nbt_ListTag *) tag);
        break;
    default:
        break;
    }
    free(tag);
}

// Java: NumericTag.byteValue() - every numeric widens like Java's implicit casts; non-numeric
// tags fall back to 0 (Java: Optional.empty on the Tag default methods)
int8_t LIBMATTI_MC_Nbt_Tag_ByteValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return (int8_t) LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

int16_t LIBMATTI_MC_Nbt_Tag_ShortValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return (int16_t) LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

int32_t LIBMATTI_MC_Nbt_Tag_IntValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return (int32_t) LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

int64_t LIBMATTI_MC_Nbt_Tag_LongValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return (int64_t) LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

float LIBMATTI_MC_Nbt_Tag_FloatValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return (float) LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

double LIBMATTI_MC_Nbt_Tag_DoubleValue(const LIBMATTI_MC_Nbt_Tag *tag)
{
    if (tag == NULL)
        return 0.0;
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        return tag->as.byteValue;
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        return tag->as.shortValue;
    case LIBMATTI_MC_Nbt_TAG_INT:
        return tag->as.intValue;
    case LIBMATTI_MC_Nbt_TAG_LONG:
        return (double) tag->as.longValue;
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        return tag->as.floatValue;
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        return tag->as.doubleValue;
    default:
        return 0.0;
    }
}

const char *LIBMATTI_MC_Nbt_Tag_AsString(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return tag != NULL && tag->id == LIBMATTI_MC_Nbt_TAG_STRING ? tag->as.stringValue : NULL;
}

double LIBMATTI_MC_Nbt_Tag_AsNumber(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return LIBMATTI_MC_Nbt_Tag_DoubleValue(tag);
}

int LIBMATTI_MC_Nbt_Tag_AsBoolean(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return LIBMATTI_MC_Nbt_Tag_ByteValue(tag) != 0;
}

int LIBMATTI_MC_Nbt_Tag_Equals(const LIBMATTI_MC_Nbt_Tag *a, const LIBMATTI_MC_Nbt_Tag *b)
{
    if (a == b)
        return 1;
    if (a == NULL || b == NULL || a->id != b->id)
        return 0;
    switch (a->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        return a->as.byteValue == b->as.byteValue;
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        return a->as.shortValue == b->as.shortValue;
    case LIBMATTI_MC_Nbt_TAG_INT:
        return a->as.intValue == b->as.intValue;
    case LIBMATTI_MC_Nbt_TAG_LONG:
        return a->as.longValue == b->as.longValue;
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        return a->as.floatValue == b->as.floatValue;
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        return a->as.doubleValue == b->as.doubleValue;
    case LIBMATTI_MC_Nbt_TAG_STRING:
        return strcmp(a->as.stringValue, b->as.stringValue) == 0;
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
        return a->as.byteArray.length == b->as.byteArray.length &&
               memcmp(a->as.byteArray.data, b->as.byteArray.data, a->as.byteArray.length) == 0;
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
        return a->as.intArray.length == b->as.intArray.length &&
               memcmp(a->as.intArray.data, b->as.intArray.data, a->as.intArray.length * sizeof(int32_t)) == 0;
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
        return a->as.longArray.length == b->as.longArray.length &&
               memcmp(a->as.longArray.data, b->as.longArray.data, a->as.longArray.length * sizeof(int64_t)) == 0;
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
    case LIBMATTI_MC_Nbt_TAG_LIST:
        return LIBMATTI_MC_Nbt_Tag_EqualsContainer(a, b);
    default:
        return 1; // EndTag
    }
}

// Java: SnbtGrammar.escapeControlCharacters(char) - the escape letter for control
// characters, NULL for printable ones
static const char *escape_control(char c)
{
    switch (c)
    {
    case '\b':
        return "b";
    case '\t':
        return "t";
    case '\n':
        return "n";
    case '\f':
        return "f";
    case '\r':
        return "r";
    default:
        return (unsigned char) c < ' ' ? "x" : NULL;
    }
}

// Java: StringTag.quoteAndEscape(String, StringBuilder) - picks " or ' as the quote
// character (the first quote occurring in the content), escapes \\, control characters
// and the active quote
char *LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(const char *value)
{
    size_t length = strlen(value);
    // worst case: every character escapes to two, plus the two quote characters
    char *out = malloc(length * 2 + 3);
    size_t outLength = 0;

    char quote = 0;
    out[outLength++] = ' '; // Java: the placeholder at the start position, replaced at the end
    for (const char *c = value; *c != '\0'; c++)
    {
        char ch = *c;
        if (ch == '\\')
        {
            out[outLength++] = '\\';
            out[outLength++] = '\\';
        }
        else if (ch != '"' && ch != '\'')
        {
            const char *escape = escape_control(ch);
            if (escape != NULL)
            {
                out[outLength++] = '\\';
                if (escape[1] == '\0')
                {
                    out[outLength++] = escape[0];
                }
                else
                {
                    // Java: "x" + HexFormat uppercase byte digits for the remaining controls
                    out[outLength++] = 'x';
                    out[outLength++] = "0123456789ABCDEF"[(unsigned char) ch >> 4];
                    out[outLength++] = "0123456789ABCDEF"[(unsigned char) ch & 0xF];
                }
            }
            else
            {
                out[outLength++] = ch;
            }
        }
        else
        {
            if (quote == 0)
                quote = ch == '"' ? '\'' : '"';
            if (quote == ch)
                out[outLength++] = '\\';
            out[outLength++] = ch;
        }
    }

    if (quote == 0)
        quote = '"';
    out[0] = quote;
    out[outLength++] = quote;
    out[outLength] = '\0';
    return out;
}

// Java: StringTag.escapeWithoutQuotes(String, StringBuilder)
char *LIBMATTI_MC_Nbt_StringTag_EscapeWithoutQuotes(const char *value)
{
    size_t length = strlen(value);
    char *out = malloc(length * 6 + 1); // worst case \xHH per character
    size_t outLength = 0;
    for (const char *c = value; *c != '\0'; c++)
    {
        char ch = *c;
        if (ch == '"' || ch == '\'' || ch == '\\')
        {
            out[outLength++] = '\\';
            out[outLength++] = ch;
        }
        else
        {
            const char *escape = escape_control(ch);
            if (escape != NULL)
            {
                out[outLength++] = '\\';
                if (escape[1] == '\0')
                {
                    out[outLength++] = escape[0];
                }
                else
                {
                    out[outLength++] = 'x';
                    out[outLength++] = "0123456789ABCDEF"[(unsigned char) ch >> 4];
                    out[outLength++] = "0123456789ABCDEF"[(unsigned char) ch & 0xF];
                }
            }
            else
            {
                out[outLength++] = ch;
            }
        }
    }
    out[outLength] = '\0';
    return out;
}
