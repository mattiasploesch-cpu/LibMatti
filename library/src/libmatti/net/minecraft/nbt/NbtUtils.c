// Port of net.minecraft.nbt.StringTagVisitor (plain) and net.minecraft.nbt
// .TextComponentTagVisitor (pretty, without the chat colors).

#include "libmatti/net/minecraft/nbt/NbtUtils.h"

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// shared low-level appenders
// ---------------------------------------------------------------------------

typedef struct
{
    char *data;
    size_t length;
    size_t capacity;
} Builder;

static void builder_init(Builder *builder)
{
    builder->capacity = 256;
    builder->data = malloc(builder->capacity);
    builder->data[0] = '\0';
    builder->length = 0;
}

static void builder_append_n(Builder *builder, const char *text, size_t length)
{
    if (builder->length + length + 1 > builder->capacity)
    {
        while (builder->length + length + 1 > builder->capacity)
            builder->capacity *= 2;
        builder->data = realloc(builder->data, builder->capacity);
    }
    memcpy(builder->data + builder->length, text, length);
    builder->length += length;
    builder->data[builder->length] = '\0';
}

static void builder_append(Builder *builder, const char *text)
{
    builder_append_n(builder, text, strlen(text));
}

static void builder_append_char(Builder *builder, char c)
{
    builder_append_n(builder, &c, 1);
}

static void builder_append_long(Builder *builder, long long value)
{
    char buffer[32];
    int length = snprintf(buffer, sizeof(buffer), "%lld", value);
    builder_append_n(builder, buffer, (size_t) length);
}

static void builder_append_double(Builder *builder, double value)
{
    // Java: StringBuilder.append(double) - the shortest repr; %g comes close for the
    // game's value ranges
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%g", value);
    builder_append_n(builder, buffer, (size_t) length);
}

static void builder_append_float(Builder *builder, float value)
{
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%g", (double) value);
    builder_append_n(builder, buffer, (size_t) length);
}

static void builder_append_owned(Builder *builder, char *text)
{
    builder_append(builder, text);
    free(text);
}

// Java: number formatting of ByteTag/ShortTag values goes through the signed byte
// printing (StringBuilder.append(byte) prints the signed value)
static void builder_append_byte(Builder *builder, int8_t value)
{
    builder_append_long(builder, value);
}

// ---------------------------------------------------------------------------
// StringTagVisitor (the plain toString form)
// ---------------------------------------------------------------------------

// Java: private static final Pattern UNQUOTED_KEY_MATCH =
//       Pattern.compile("[A-Za-z._]+[A-Za-z0-9._+-]*")
static int unquoted_key_matches(const char *key)
{
    // first part: one or more of [A-Za-z._]
    const char *c = key;
    while (*c != '\0' && (isalpha((unsigned char) *c) || *c == '.' || *c == '_'))
        c++;
    if (c == key)
        return 0;
    // rest: zero or more of [A-Za-z0-9._+-]
    while (*c != '\0' && (isalnum((unsigned char) *c) || *c == '.' || *c == '_' || *c == '+' || *c == '-'))
        c++;
    return *c == '\0';
}

static int string_equals_ignore_case(const char *a, const char *b)
{
    for (; *a != '\0' && *b != '\0'; a++, b++)
        if (tolower((unsigned char) *a) != tolower((unsigned char) *b))
            return 0;
    return *a == '\0' && *b == '\0';
}

// Java: private void handleKeyEscape(String)
static void visit_key_plain(Builder *builder, const char *key)
{
    if (!string_equals_ignore_case(key, "true") && !string_equals_ignore_case(key, "false") &&
        unquoted_key_matches(key))
    {
        builder_append(builder, key);
    }
    else
    {
        builder_append_owned(builder, LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(key));
    }
}

static void visit_plain(Builder *builder, const LIBMATTI_MC_Nbt_Tag *tag)
{
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        builder_append_byte(builder, tag->as.byteValue);
        builder_append_char(builder, 'b');
        break;
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        builder_append_long(builder, tag->as.shortValue);
        builder_append_char(builder, 's');
        break;
    case LIBMATTI_MC_Nbt_TAG_INT:
        builder_append_long(builder, tag->as.intValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_LONG:
        builder_append_long(builder, tag->as.longValue);
        builder_append_char(builder, 'L');
        break;
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        builder_append_float(builder, tag->as.floatValue);
        builder_append_char(builder, 'f');
        break;
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        builder_append_double(builder, tag->as.doubleValue);
        builder_append_char(builder, 'd');
        break;
    case LIBMATTI_MC_Nbt_TAG_STRING:
        builder_append_owned(builder, LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(tag->as.stringValue));
        break;
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
    {
        builder_append(builder, "[B;");
        for (size_t i = 0; i < tag->as.byteArray.length; i++)
        {
            if (i != 0)
                builder_append_char(builder, ',');
            builder_append_byte(builder, tag->as.byteArray.data[i]);
            builder_append_char(builder, 'B');
        }
        builder_append_char(builder, ']');
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
    {
        builder_append(builder, "[I;");
        for (size_t i = 0; i < tag->as.intArray.length; i++)
        {
            if (i != 0)
                builder_append_char(builder, ',');
            builder_append_long(builder, tag->as.intArray.data[i]);
        }
        builder_append_char(builder, ']');
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
    {
        builder_append(builder, "[L;");
        for (size_t i = 0; i < tag->as.longArray.length; i++)
        {
            if (i != 0)
                builder_append_char(builder, ',');
            builder_append_long(builder, tag->as.longArray.data[i]);
            builder_append_char(builder, 'L');
        }
        builder_append_char(builder, ']');
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_LIST:
    {
        const LIBMATTI_MC_Nbt_ListTag *list = (const LIBMATTI_MC_Nbt_ListTag *) tag;
        builder_append_char(builder, '[');
        for (size_t i = 0; i < list->count; i++)
        {
            if (i != 0)
                builder_append_char(builder, ',');
            visit_plain(builder, list->elements[i]);
        }
        builder_append_char(builder, ']');
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
    {
        const LIBMATTI_MC_Nbt_CompoundTag *compound = (const LIBMATTI_MC_Nbt_CompoundTag *) tag;
        builder_append_char(builder, '{');
        // Java: new ArrayList<>(entrySet()) sorted with Entry.comparingByKey()
        size_t *order = malloc(sizeof(size_t) * (compound->count > 0 ? compound->count : 1));
        for (size_t i = 0; i < compound->count; i++)
            order[i] = i;
        for (size_t i = 1; i < compound->count; i++)
        {
            size_t current = order[i];
            size_t j = i;
            while (j > 0 && strcmp(compound->keys[order[j - 1]], compound->keys[current]) > 0)
            {
                order[j] = order[j - 1];
                j--;
            }
            order[j] = current;
        }
        for (size_t i = 0; i < compound->count; i++)
        {
            if (i != 0)
                builder_append_char(builder, ',');
            visit_key_plain(builder, compound->keys[order[i]]);
            builder_append_char(builder, ':');
            visit_plain(builder, compound->values[order[i]]);
        }
        free(order);
        builder_append_char(builder, '}');
        break;
    }
    default:
        builder_append(builder, "END");
        break;
    }
}

char *LIBMATTI_MC_Nbt_StringTagVisitor_Visit(const LIBMATTI_MC_Nbt_Tag *tag)
{
    Builder builder;
    builder_init(&builder);
    visit_plain(&builder, tag);
    return builder.data;
}

// Java: CompoundTag.toString() goes through StringTagVisitor as well
char *LIBMATTI_MC_Nbt_StringTagVisitor_Print(const LIBMATTI_MC_Nbt_Tag *tag)
{
    return LIBMATTI_MC_Nbt_StringTagVisitor_Visit(tag);
}

// ---------------------------------------------------------------------------
// TextComponentTagVisitor (the pretty form)
// ---------------------------------------------------------------------------

#define PRETTY_MAX_DEPTH 64
#define PRETTY_MAX_LENGTH 128

// Java: private static final Pattern SIMPLE_VALUE = Pattern.compile("[A-Za-z0-9._+-]+")
static int simple_value_matches(const char *key)
{
    if (*key == '\0')
        return 0;
    for (const char *c = key; *c != '\0'; c++)
    {
        if (!(isalnum((unsigned char) *c) || *c == '.' || *c == '_' || *c == '+' || *c == '-'))
            return 0;
    }
    return 1;
}

typedef struct
{
    Builder builder;
    const char *indentation;
    int indentDepth;
    int depth;
} PrettyVisitor;

static void repeat_indent(PrettyVisitor *visitor, int times)
{
    for (int i = 0; i < times; i++)
        builder_append(&visitor->builder, visitor->indentation);
}

// Java: protected static Component handleEscapePretty(String)
static void visit_key_pretty(PrettyVisitor *visitor, const char *key)
{
    if (simple_value_matches(key))
    {
        builder_append(&visitor->builder, key);
    }
    else
    {
        char *quoted = LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(key);
        // the quoted form is "content" or 'content'; the port prints it as-is
        builder_append(&visitor->builder, quoted);
        free(quoted);
    }
}

static void visit_pretty(PrettyVisitor *visitor, const LIBMATTI_MC_Nbt_Tag *tag);

// Java: private void appendSubTag(Tag, boolean)
static void append_sub_tag(PrettyVisitor *visitor, const LIBMATTI_MC_Nbt_Tag *tag, int increment)
{
    if (increment)
        visitor->indentDepth++;
    visitor->depth++;
    visit_pretty(visitor, tag);
    if (increment)
        visitor->indentDepth--;
    visitor->depth--;
}

// Java: private static boolean shouldWrapListElements(ListTag)
static int should_wrap_list_elements(const LIBMATTI_MC_Nbt_ListTag *list)
{
    if (list->count >= 8)
        return 0;
    for (size_t i = 0; i < list->count; i++)
    {
        uint8_t id = list->elements[i]->id;
        if (id < LIBMATTI_MC_Nbt_TAG_BYTE || id > LIBMATTI_MC_Nbt_TAG_DOUBLE)
            return 1;
    }
    return 0;
}

static void visit_pretty(PrettyVisitor *visitor, const LIBMATTI_MC_Nbt_Tag *tag)
{
    Builder *builder = &visitor->builder;
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        builder_append_byte(builder, tag->as.byteValue);
        builder_append(builder, "b");
        break;
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        builder_append_long(builder, tag->as.shortValue);
        builder_append(builder, "s");
        break;
    case LIBMATTI_MC_Nbt_TAG_INT:
        builder_append_long(builder, tag->as.intValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_LONG:
        builder_append_long(builder, tag->as.longValue);
        builder_append(builder, "L");
        break;
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        builder_append_float(builder, tag->as.floatValue);
        builder_append(builder, "f");
        break;
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        builder_append_double(builder, tag->as.doubleValue);
        builder_append(builder, "d");
        break;
    case LIBMATTI_MC_Nbt_TAG_STRING:
        builder_append_owned(builder, LIBMATTI_MC_Nbt_StringTag_QuoteAndEscape(tag->as.stringValue));
        break;
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
    {
        builder_append(builder, "[B;");
        for (size_t i = 0; i < tag->as.byteArray.length; i++)
        {
            if (i != 0)
                builder_append(builder, ", ");
            builder_append(builder, " ");
            builder_append_byte(builder, tag->as.byteArray.data[i]);
            builder_append(builder, "B");
        }
        builder_append(builder, "]");
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
    {
        builder_append(builder, "[I;");
        for (size_t i = 0; i < tag->as.intArray.length; i++)
        {
            if (i != 0)
                builder_append(builder, ", ");
            builder_append(builder, " ");
            builder_append_long(builder, tag->as.intArray.data[i]);
        }
        builder_append(builder, "]");
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
    {
        builder_append(builder, "[L;");
        for (size_t i = 0; i < tag->as.longArray.length; i++)
        {
            if (i != 0)
                builder_append(builder, ", ");
            builder_append(builder, " ");
            builder_append_long(builder, tag->as.longArray.data[i]);
            builder_append(builder, "L");
        }
        builder_append(builder, "]");
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_LIST:
    {
        const LIBMATTI_MC_Nbt_ListTag *list = (const LIBMATTI_MC_Nbt_ListTag *) tag;
        if (list->count == 0)
        {
            builder_append(builder, "[]");
        }
        else if (visitor->depth >= PRETTY_MAX_DEPTH)
        {
            builder_append(builder, "[<...>]");
        }
        else if (!should_wrap_list_elements(list))
        {
            builder_append(builder, "[");
            for (size_t i = 0; i < list->count; i++)
            {
                if (i != 0)
                    builder_append(builder, ", ");
                append_sub_tag(visitor, list->elements[i], 0);
            }
            builder_append(builder, "]");
        }
        else
        {
            builder_append(builder, "[");
            int indented = visitor->indentation[0] != '\0';
            if (indented)
                builder_append(builder, "\n");
            size_t printed = list->count < PRETTY_MAX_LENGTH ? list->count : PRETTY_MAX_LENGTH;
            for (size_t i = 0; i < printed; i++)
            {
                repeat_indent(visitor, visitor->indentDepth + 1);
                append_sub_tag(visitor, list->elements[i], 1);
                if (i != list->count - 1)
                    builder_append(builder, indented ? ",\n" : ", ");
            }
            if (list->count > PRETTY_MAX_LENGTH)
            {
                repeat_indent(visitor, visitor->indentDepth + 1);
                builder_append(builder, "<...>");
            }
            if (indented)
            {
                builder_append(builder, "\n");
                repeat_indent(visitor, visitor->indentDepth);
            }
            builder_append(builder, "]");
        }
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
    {
        const LIBMATTI_MC_Nbt_CompoundTag *compound = (const LIBMATTI_MC_Nbt_CompoundTag *) tag;
        if (compound->count == 0)
        {
            builder_append(builder, "{}");
        }
        else if (visitor->depth >= PRETTY_MAX_DEPTH)
        {
            builder_append(builder, "{<...>}");
        }
        else
        {
            builder_append(builder, "{");
            int indented = visitor->indentation[0] != '\0';
            if (indented)
                builder_append(builder, "\n");
            // Java iterates the map's key order; the port keeps the insertion order
            for (size_t i = 0; i < compound->count; i++)
            {
                repeat_indent(visitor, visitor->indentDepth + 1);
                visit_key_pretty(visitor, compound->keys[i]);
                builder_append(builder, ": ");
                append_sub_tag(visitor, compound->values[i], 1);
                if (i != compound->count - 1)
                    builder_append(builder, indented ? ",\n" : ", ");
            }
            if (indented)
            {
                builder_append(builder, "\n");
                repeat_indent(visitor, visitor->indentDepth);
            }
            builder_append(builder, "}");
        }
        break;
    }
    default:
        builder_append(builder, "END");
        break;
    }
}

char *LIBMATTI_MC_Nbt_TextComponentTagVisitor_Visit(const char *indentation, const LIBMATTI_MC_Nbt_Tag *tag)
{
    PrettyVisitor visitor;
    builder_init(&visitor.builder);
    visitor.indentation = indentation;
    visitor.indentDepth = 0;
    visitor.depth = 0;
    visit_pretty(&visitor, tag);
    return visitor.builder.data;
}
