// Port of org.apache.maven.artifact.versioning.ComparableVersion.

#include "libmatti/org/apache/maven/artifact/versioning/ComparableVersion.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final int MAX_INTITEM_LENGTH = 9 / MAX_LONGITEM_LENGTH = 18 / MAX_VERSION_LENGTH = 256
#define MAX_INTITEM_LENGTH 9
#define MAX_LONGITEM_LENGTH 18
#define MAX_VERSION_LENGTH 256

struct LIBMATTI_MAVEN_ComparableVersion
{
    // Java: private String value
    char *value;
    // Java: private String canonical
    char *canonical;
    // Java: private ListItem items
    LIBMATTI_MAVEN_Item *items;
};

// ---------------------------------------------------------------------------
// StringBuilder (the port's own, the JDK one only joins strings)
// ---------------------------------------------------------------------------

typedef struct
{
    char *data;
    size_t length;
    size_t capacity;
} Buffer;

static void buffer_append(Buffer *buffer, const char *value)
{
    size_t length = strlen(value);
    if (buffer->length + length + 1 > buffer->capacity)
    {
        buffer->capacity = (buffer->length + length + 1) * 2;
        buffer->data = realloc(buffer->data, buffer->capacity);
    }
    memcpy(buffer->data + buffer->length, value, length + 1);
    buffer->length += length;
}

static void buffer_append_char(Buffer *buffer, char value)
{
    char text[2] = {value, '\0'};
    buffer_append(buffer, text);
}

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------

static LIBMATTI_MAVEN_Item *string_item(const char *value, int followedByDigit);
static char *local_substring(const char *value, size_t start, size_t end);
static LIBMATTI_MAVEN_Item *parse_item(int isCombination, int isDigit, const char *buf);

static LIBMATTI_MAVEN_Item *item_new(LIBMATTI_MAVEN_ItemType type)
{
    LIBMATTI_MAVEN_Item *item = calloc(1, sizeof(LIBMATTI_MAVEN_Item));
    item->type = type;
    return item;
}

static LIBMATTI_MAVEN_Item *int_item(int value)
{
    LIBMATTI_MAVEN_Item *item = item_new(LIBMATTI_MAVEN_Item_INT);
    item->intValue = value;
    return item;
}

static void item_free(LIBMATTI_MAVEN_Item *item)
{
    if (item == NULL) return;

    free(item->digits);
    free(item->stringValue);
    if (item->type == LIBMATTI_MAVEN_Item_COMBINATION)
    {
        item_free(item->stringPart);
        item_free(item->digitPart);
    }
    else if (item->type == LIBMATTI_MAVEN_Item_LIST)
    {
        for (size_t i = 0; i < item->count; i++)
            item_free(item->items[i]);
        free(item->items);
    }
    free(item);
}

static void list_add(LIBMATTI_MAVEN_Item *list, LIBMATTI_MAVEN_Item *item)
{
    if (list->count + 1 > list->capacity)
    {
        list->capacity = list->capacity > 0 ? list->capacity * 2 : 4;
        list->items = realloc(list->items, sizeof(*list->items) * list->capacity);
    }
    list->items[list->count++] = item;
}

static void list_remove_at(LIBMATTI_MAVEN_Item *list, size_t index)
{
    item_free(list->items[index]);
    for (size_t i = index + 1; i < list->count; i++)
        list->items[i - 1] = list->items[i];
    list->count--;
}

// Java: Item.isNull()
static int item_is_null(const LIBMATTI_MAVEN_Item *item)
{
    switch (item->type)
    {
    case LIBMATTI_MAVEN_Item_INT: return item->intValue == 0;
    case LIBMATTI_MAVEN_Item_LONG: return item->longValue == 0;
    // Java: BigInteger.ZERO.equals(value)
    case LIBMATTI_MAVEN_Item_BIG_INTEGER: return strcmp(item->digits, "0") == 0;
    case LIBMATTI_MAVEN_Item_STRING: return item->stringValue == NULL || item->stringValue[0] == '\0';
    case LIBMATTI_MAVEN_Item_COMBINATION: return 0;
    case LIBMATTI_MAVEN_Item_LIST: return item->count == 0;
    }
    return 0;
}

// Java: StringItem.QUALIFIERS / RELEASE_QUALIFIERS / ALIASES / RELEASE_VERSION_INDEX
static const char *QUALIFIERS[] = {"alpha", "beta", "milestone", "rc", "snapshot", "", "sp"};
#define QUALIFIER_COUNT 7
static const char *RELEASE_QUALIFIERS[] = {"ga", "final", "release"};
#define RELEASE_VERSION_INDEX "5"

// Java: public static String comparableQualifier(String qualifier)
static char *comparable_qualifier(const char *qualifier)
{
    for (size_t i = 0; i < sizeof(RELEASE_QUALIFIERS) / sizeof(*RELEASE_QUALIFIERS); i++)
        if (strcmp(RELEASE_QUALIFIERS[i], qualifier) == 0) return strdup(RELEASE_VERSION_INDEX);

    for (int i = 0; i < QUALIFIER_COUNT; i++)
        if (strcmp(QUALIFIERS[i], qualifier) == 0)
        {
            char *result = malloc(8);
            snprintf(result, 8, "%d", i);
            return result;
        }

    char *result = malloc(QUALIFIER_COUNT + 1 + strlen(qualifier) + 1);
    sprintf(result, "%d-%s", QUALIFIER_COUNT, qualifier);
    return result;
}

// Java: String.compareTo of two comparableQualifier values
static int compare_qualifiers(const char *a, const char *b)
{
    char *left = comparable_qualifier(a);
    char *right = comparable_qualifier(b);
    int result = strcmp(left, right);
    free(left);
    free(right);
    return result;
}

// Java: comparableQualifier(value).compareTo(RELEASE_VERSION_INDEX)
static int string_item_compare_to_null(const char *value)
{
    char *self = comparable_qualifier(value);
    int result = strcmp(self, RELEASE_VERSION_INDEX);
    free(self);
    return result;
}

// Java: BigInteger.compareTo on two stripped digit strings
static int compare_digits(const char *a, const char *b)
{
    size_t aLength = strlen(a);
    size_t bLength = strlen(b);
    if (aLength != bLength) return aLength < bLength ? -1 : 1;
    return strcmp(a, b);
}

static int item_compare(const LIBMATTI_MAVEN_Item *item, const LIBMATTI_MAVEN_Item *other);

// Java: StringItem.compareTo(item)
static int string_item_compare(const LIBMATTI_MAVEN_Item *item, const LIBMATTI_MAVEN_Item *other)
{
    if (other == NULL) return string_item_compare_to_null(item->stringValue);

    switch (other->type)
    {
    // Java: 1.any < 1.1
    case LIBMATTI_MAVEN_Item_INT:
    case LIBMATTI_MAVEN_Item_LONG:
    case LIBMATTI_MAVEN_Item_BIG_INTEGER: return -1;

    case LIBMATTI_MAVEN_Item_STRING:
        return compare_qualifiers(item->stringValue, other->stringValue);

    case LIBMATTI_MAVEN_Item_COMBINATION:
    {
        int result = string_item_compare(item, other->stringPart);
        if (result == 0)
        {
            if (string_item_compare(item, NULL) == 0) return -item_compare(other->digitPart, NULL);
            return -1;
        }
        return result;
    }

    case LIBMATTI_MAVEN_Item_LIST: return -item_compare(other, item);
    }
    return 0;
}

// Java: CombinationItem.compareTo(item)
static int combination_item_compare(const LIBMATTI_MAVEN_Item *item, const LIBMATTI_MAVEN_Item *other)
{
    if (other == NULL)
    {
        int result = item_compare(item->stringPart, NULL);
        if (result == 0) return item_compare(item->digitPart, NULL);
        return result;
    }

    switch (other->type)
    {
    case LIBMATTI_MAVEN_Item_INT:
    case LIBMATTI_MAVEN_Item_LONG:
    case LIBMATTI_MAVEN_Item_BIG_INTEGER: return -1;

    case LIBMATTI_MAVEN_Item_STRING:
    {
        int result = item_compare(item->stringPart, other);
        if (result == 0)
        {
            if (item_compare(item->stringPart, NULL) == 0) return item_compare(item->digitPart, NULL);
            // Java: X1 > X
            return 1;
        }
        return result;
    }

    case LIBMATTI_MAVEN_Item_LIST: return -item_compare(other, item);

    case LIBMATTI_MAVEN_Item_COMBINATION:
    {
        int result = item_compare(item->stringPart, other->stringPart);
        if (result == 0) return item_compare(item->digitPart, other->digitPart);
        return result;
    }
    }
    return 0;
}

// Java: ListItem.compareTo(item)
static int list_item_compare(const LIBMATTI_MAVEN_Item *item, const LIBMATTI_MAVEN_Item *other)
{
    if (other == NULL)
    {
        if (item->count == 0) return 0; // Java: 1-0 = 1- (normalize) = 1
        for (size_t i = 0; i < item->count; i++)
        {
            int result = item_compare(item->items[i], NULL);
            if (result != 0) return result;
        }
        return 0;
    }

    switch (other->type)
    {
    // Java: 1-1 < 1.0.x
    case LIBMATTI_MAVEN_Item_INT:
    case LIBMATTI_MAVEN_Item_LONG:
    case LIBMATTI_MAVEN_Item_BIG_INTEGER: return -1;

    case LIBMATTI_MAVEN_Item_STRING:
    case LIBMATTI_MAVEN_Item_COMBINATION:
    {
        int scalarResult = item->count == 0 ? -item_compare(other, NULL) : item_compare(item->items[0], other);
        for (size_t i = 1; scalarResult == 0 && i < item->count; i++)
            scalarResult = item_compare(item->items[i], NULL);
        return scalarResult;
    }

    case LIBMATTI_MAVEN_Item_LIST:
    {
        size_t i = 0;
        while (i < item->count || i < other->count)
        {
            const LIBMATTI_MAVEN_Item *left = i < item->count ? item->items[i] : NULL;
            const LIBMATTI_MAVEN_Item *right = i < other->count ? other->items[i] : NULL;

            // Java: if this is shorter, then invert the compare and mul with -1
            int result = left == NULL ? (right == NULL ? 0 : -1 * item_compare(right, left))
                                      : item_compare(left, right);
            if (result != 0) return result;
            i++;
        }
        return 0;
    }
    }
    return 0;
}

// Java: Item.hashCode() per implementation class
static int item_hash_code(const LIBMATTI_MAVEN_Item *item)
{
    switch (item->type)
    {
    // Java: IntItem.hashCode() { return value; }
    case LIBMATTI_MAVEN_Item_INT: return item->intValue;
    // Java: LongItem.hashCode() { return (int) (value ^ (value >>> 32)); }
    case LIBMATTI_MAVEN_Item_LONG: return (int) ((uint32_t) item->longValue ^ (uint32_t) ((uint64_t) item->longValue >> 32));
    // Java: BigIntegerItem.hashCode() { return value.hashCode(); }
    case LIBMATTI_MAVEN_Item_BIG_INTEGER:
    {
        // Java: String.hashCode
        uint32_t hash = 0;
        for (const unsigned char *p = (const unsigned char *) item->digits; *p != '\0'; p++)
            hash = 31 * hash + *p;
        return (int) hash;
    }
    // Java: StringItem.hashCode() { return value.hashCode(); }
    case LIBMATTI_MAVEN_Item_STRING:
    {
        uint32_t hash = 0;
        for (const unsigned char *p = (const unsigned char *) item->stringValue; *p != '\0'; p++)
            hash = 31 * hash + *p;
        return (int) hash;
    }
    // Java: CombinationItem.hashCode() { return Objects.hash(stringPart, digitPart); }
    case LIBMATTI_MAVEN_Item_COMBINATION:
    {
        int hash = 1;
        hash = 31 * hash + item_hash_code(item->stringPart);
        hash = 31 * hash + item_hash_code(item->digitPart);
        return hash;
    }
    // Java: ListItem.hashCode() { return super.hashCode(); } - AbstractList of the items
    case LIBMATTI_MAVEN_Item_LIST:
    {
        int hash = 1;
        for (size_t i = 0; i < item->count; i++)
            hash = 31 * hash + item_hash_code(item->items[i]);
        return hash;
    }
    }
    return 0;
}

// Java: String.hashCode of a comparableQualifier value
static int qualifier_hash_code(const char *qualifier)
{
    char *comparable = comparable_qualifier(qualifier);
    uint32_t hash = 0;
    for (const unsigned char *p = (const unsigned char *) comparable; *p != '\0'; p++)
        hash = 31 * hash + *p;
    free(comparable);
    return (int) hash;
}

// Java: int orderingHash(Item item) - hash consistent with compareTo, null-comparing items hash alike
static int ordering_hash(const LIBMATTI_MAVEN_Item *item)
{
    switch (item->type)
    {
    case LIBMATTI_MAVEN_Item_LIST:
    {
        size_t end = item->count;
        // Java: trailing items that compare as equal to null do not affect ordering: 1-ga == 1
        while (end > 0 && item_compare(item->items[end - 1], NULL) == 0)
            end--;
        if (end == 1) return ordering_hash(item->items[0]);

        int hash = 1;
        for (size_t i = 0; i < end; i++)
            hash = 31 * hash + ordering_hash(item->items[i]);
        return hash;
    }
    // Java: qualifiers that compare as equal ("ga", "final", "release" and "") must hash alike
    case LIBMATTI_MAVEN_Item_STRING:
        return qualifier_hash_code(item->stringValue);
    case LIBMATTI_MAVEN_Item_COMBINATION:
        if (item_compare(item->stringPart, NULL) == 0 && item_compare(item->digitPart, NULL) == 0)
            return ordering_hash(item->stringPart);
        return 31 * qualifier_hash_code(item->stringPart->stringValue) + ordering_hash(item->digitPart);
    // Java: numeric items only compare as equal to items of the same type with the same value
    default: return item_hash_code(item);
    }
}

// Java: item.compareTo(other) - other may be null
static int item_compare(const LIBMATTI_MAVEN_Item *item, const LIBMATTI_MAVEN_Item *other)
{
    switch (item->type)
    {
    case LIBMATTI_MAVEN_Item_INT:
        if (other == NULL) return item->intValue == 0 ? 0 : 1; // Java: 1.0 == 1, 1.1 > 1
        switch (other->type)
        {
        case LIBMATTI_MAVEN_Item_INT:
            return item->intValue < other->intValue ? -1 : (item->intValue > other->intValue ? 1 : 0);
        case LIBMATTI_MAVEN_Item_LONG:
        case LIBMATTI_MAVEN_Item_BIG_INTEGER: return -1;
        case LIBMATTI_MAVEN_Item_STRING:
        case LIBMATTI_MAVEN_Item_COMBINATION:
        case LIBMATTI_MAVEN_Item_LIST: return 1;
        }
        return 0;

    case LIBMATTI_MAVEN_Item_LONG:
        if (other == NULL) return item->longValue == 0 ? 0 : 1;
        switch (other->type)
        {
        case LIBMATTI_MAVEN_Item_INT: return 1;
        case LIBMATTI_MAVEN_Item_LONG:
            return item->longValue < other->longValue ? -1 : (item->longValue > other->longValue ? 1 : 0);
        case LIBMATTI_MAVEN_Item_BIG_INTEGER: return -1;
        case LIBMATTI_MAVEN_Item_STRING:
        case LIBMATTI_MAVEN_Item_COMBINATION:
        case LIBMATTI_MAVEN_Item_LIST: return 1;
        }
        return 0;

    case LIBMATTI_MAVEN_Item_BIG_INTEGER:
        if (other == NULL) return strcmp(item->digits, "0") == 0 ? 0 : 1;
        switch (other->type)
        {
        case LIBMATTI_MAVEN_Item_INT:
        case LIBMATTI_MAVEN_Item_LONG: return 1;
        case LIBMATTI_MAVEN_Item_BIG_INTEGER: return compare_digits(item->digits, other->digits);
        case LIBMATTI_MAVEN_Item_STRING:
        case LIBMATTI_MAVEN_Item_COMBINATION:
        case LIBMATTI_MAVEN_Item_LIST: return 1;
        }
        return 0;

    case LIBMATTI_MAVEN_Item_STRING: return string_item_compare(item, other);
    case LIBMATTI_MAVEN_Item_COMBINATION: return combination_item_compare(item, other);
    case LIBMATTI_MAVEN_Item_LIST: return list_item_compare(item, other);
    }
    return 0;
}

// Java: Item.toString()
static void item_to_string(const LIBMATTI_MAVEN_Item *item, Buffer *buffer)
{
    char text[32];
    switch (item->type)
    {
    case LIBMATTI_MAVEN_Item_INT:
        snprintf(text, sizeof(text), "%d", item->intValue);
        buffer_append(buffer, text);
        break;
    case LIBMATTI_MAVEN_Item_LONG:
        snprintf(text, sizeof(text), "%ld", item->longValue);
        buffer_append(buffer, text);
        break;
    case LIBMATTI_MAVEN_Item_BIG_INTEGER:
    case LIBMATTI_MAVEN_Item_STRING:
        buffer_append(buffer, item->type == LIBMATTI_MAVEN_Item_STRING ? item->stringValue : item->digits);
        break;
    case LIBMATTI_MAVEN_Item_COMBINATION:
        item_to_string(item->stringPart, buffer);
        item_to_string(item->digitPart, buffer);
        break;
    case LIBMATTI_MAVEN_Item_LIST:
    {
        // Java: ListItem.toString() builds its own StringBuilder, so the separators are per list
        Buffer inner = {0};
        for (size_t i = 0; i < item->count; i++)
        {
            // Java: if (!buffer.isEmpty()) - an item that renders as nothing must not add a separator
            if (inner.length > 0)
                buffer_append_char(&inner, item->items[i]->type == LIBMATTI_MAVEN_Item_LIST ? '-' : '.');
            item_to_string(item->items[i], &inner);
        }
        if (inner.data != NULL)
        {
            buffer_append(buffer, inner.data);
            free(inner.data);
        }
        break;
    }
    }
}

// Java: void normalize()
static void list_normalize(LIBMATTI_MAVEN_Item *list)
{
    for (size_t i = list->count; i-- > 0;)
    {
        if (!item_is_null(list->items[i])) continue;
        if (i == list->count - 1 || list->items[i + 1]->type == LIBMATTI_MAVEN_Item_STRING)
        {
            list_remove_at(list, i);
        }
        else if (list->items[i + 1]->type == LIBMATTI_MAVEN_Item_LIST && list->items[i + 1]->count > 0)
        {
            LIBMATTI_MAVEN_ItemType type = list->items[i + 1]->items[0]->type;
            if (type == LIBMATTI_MAVEN_Item_COMBINATION || type == LIBMATTI_MAVEN_Item_STRING) list_remove_at(list, i);
        }
    }

    // Java: if (size() == 1 && get(0) instanceof ListItem list) { clear(); addAll(list); }
    if (list->count == 1 && list->items[0]->type == LIBMATTI_MAVEN_Item_LIST)
    {
        LIBMATTI_MAVEN_Item *inner = list->items[0];
        list->items[0] = NULL;
        list->count = 0;
        for (size_t i = 0; i < inner->count; i++)
            list_add(list, inner->items[i]);
        inner->count = 0;
        item_free(inner);
    }
}

// Java: private static String stripLeadingZeroes(String buf)
static char *strip_leading_zeroes(const char *buf)
{
    if (buf == NULL || buf[0] == '\0') return strdup("0");
    for (size_t i = 0; buf[i] != '\0'; i++)
        if (buf[i] != '0') return strdup(buf + i);
    return strdup("0");
}

// Java: parseItem(isCombination, isDigit, version.substring(start, end)) - the substring is not garbage
// collected in C, so it is released here
static LIBMATTI_MAVEN_Item *parse_item_range(int isCombination, int isDigit, const char *value, size_t start,
                                            size_t end)
{
    char *text = local_substring(value, start, end);
    LIBMATTI_MAVEN_Item *item = parse_item(isCombination, isDigit, text);
    free(text);
    return item;
}

// Java: private static Item parseItem(boolean isCombination, boolean isDigit, String buf)
static LIBMATTI_MAVEN_Item *parse_item(int isCombination, int isDigit, const char *buf)
{
    if (isCombination)
    {
        // Java: new CombinationItem(buf.replace("-", ""))
        char *value = malloc(strlen(buf) + 1);
        size_t length = 0;
        for (const char *c = buf; *c != '\0'; c++)
            if (*c != '-') value[length++] = *c;
        value[length] = '\0';

        size_t index = 0;
        for (size_t i = 0; value[i] != '\0'; i++)
            if (isdigit((unsigned char)value[i]))
            {
                index = i;
                break;
            }

        LIBMATTI_MAVEN_Item *item = item_new(LIBMATTI_MAVEN_Item_COMBINATION);
        char *stringPart = malloc(index + 1);
        memcpy(stringPart, value, index);
        stringPart[index] = '\0';
        item->stringPart = string_item(stringPart, 1);
        free(stringPart);
        // Java: parseItem(true, value.substring(index)) - the two-argument overload adds isCombination = false
        item->digitPart = parse_item(0, 1, value + index);
        free(value);
        return item;
    }

    if (isDigit)
    {
        char *stripped = strip_leading_zeroes(buf);
        LIBMATTI_MAVEN_Item *item;
        size_t length = strlen(stripped);

        if (length <= MAX_INTITEM_LENGTH)
        {
            item = int_item(atoi(stripped));
        }
        else if (length <= MAX_LONGITEM_LENGTH)
        {
            item = item_new(LIBMATTI_MAVEN_Item_LONG);
            item->longValue = strtol(stripped, NULL, 10);
        }
        else
        {
            item = item_new(LIBMATTI_MAVEN_Item_BIG_INTEGER);
            item->digits = stripped;
            stripped = NULL;
        }

        free(stripped);
        return item;
    }

    // Java: return new StringItem(buf, false);
    return string_item(buf, 0);
}

// Java: StringItem(String value, boolean followedByDigit)
static LIBMATTI_MAVEN_Item *string_item(const char *value, int followedByDigit)
{
    char *resolved = NULL;
    if (followedByDigit && strlen(value) == 1)
    {
        // Java: a1 = alpha-1, b1 = beta-1, m1 = milestone-1
        switch (value[0])
        {
        case 'a': resolved = strdup("alpha");
            break;
        case 'b': resolved = strdup("beta");
            break;
        case 'm': resolved = strdup("milestone");
            break;
        default: break;
        }
    }

    if (resolved == NULL) resolved = strdup(value);
    // Java: this.value = ALIASES.getProperty(value, value) - cr is the only alias
    if (strcmp(resolved, "cr") == 0)
    {
        free(resolved);
        resolved = strdup("rc");
    }

    LIBMATTI_MAVEN_Item *item = item_new(LIBMATTI_MAVEN_Item_STRING);
    item->stringValue = resolved;
    return item;
}

// Java: public final void parseVersion(String version)
static void parse_version(LIBMATTI_MAVEN_ComparableVersion *version, const char *versionString)
{
    // Java: if (version.length() > MAX_VERSION_LENGTH) throw new IllegalArgumentException(...)
    if (strlen(versionString) > MAX_VERSION_LENGTH)
    {
        // the port aborts like the uncaught exception it would be
        char message[300];
        snprintf(message, sizeof(message), "Version string is too long (%zu > %d characters)",
                 strlen(versionString), (int) MAX_VERSION_LENGTH);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "IllegalArgumentException: {}", message);
        abort();
    }

    version->value = strdup(versionString);

    version->items = item_new(LIBMATTI_MAVEN_Item_LIST);
    LIBMATTI_MAVEN_Item *list = version->items;

    // Java: version = version.toLowerCase(Locale.ENGLISH)
    char *lower = strdup(versionString);
    size_t length = strlen(lower);
    for (size_t i = 0; i < length; i++)
        lower[i] = (char)tolower((unsigned char)lower[i]);

    // Java: Deque<Item> stack = new ArrayDeque<>(); stack.push(list)
    LIBMATTI_MAVEN_Item **stack = NULL;
    size_t stackCount = 0;
    stack = realloc(stack, sizeof(*stack) * (stackCount + 1));
    stack[stackCount++] = list;

    int isDigit = 0;
    int isCombination = 0;
    size_t startIndex = 0;

    for (size_t i = 0; i < length; i++)
    {
        char c = lower[i];
        // Java decodes a surrogate pair into one code point; the port skips the UTF-8 continuation bytes
        if ((c & 0xC0) == 0x80) continue;

        if (c == '.')
        {
            if (i == startIndex)
                list_add(list, int_item(0)); // Java: IntItem.ZERO
            else
                list_add(list, parse_item_range(isCombination, isDigit, lower, startIndex, i));

            isCombination = 0;
            startIndex = i + 1;
        }
        else if (c == '-')
        {
            if (i == startIndex)
            {
                list_add(list, int_item(0));
            }
            else
            {
                // Java: X-1 is going to be treated as X1
                if (!isDigit && i != length - 1 && isdigit((unsigned char)lower[i + 1]))
                {
                    isCombination = 1;
                    continue;
                }
                list_add(list, parse_item_range(isCombination, isDigit, lower, startIndex, i));
            }
            startIndex = i + 1;

            if (list->count > 0)
            {
                LIBMATTI_MAVEN_Item *nested = item_new(LIBMATTI_MAVEN_Item_LIST);
                list_add(list, nested);
                list = nested;
                stack = realloc(stack, sizeof(*stack) * (stackCount + 1));
                stack[stackCount++] = list;
            }
            isCombination = 0;
        }
        else if (c >= '0' && c <= '9')
        {
            if (!isDigit && i > startIndex)
            {
                // Java: X1
                isCombination = 1;

                if (list->count > 0)
                {
                    LIBMATTI_MAVEN_Item *nested = item_new(LIBMATTI_MAVEN_Item_LIST);
                    list_add(list, nested);
                    list = nested;
                    stack = realloc(stack, sizeof(*stack) * (stackCount + 1));
                    stack[stackCount++] = list;
                }
            }

            isDigit = 1;
        }
        else
        {
            if (isDigit && i > startIndex)
            {
                list_add(list, parse_item_range(isCombination, 1, lower, startIndex, i));
                startIndex = i;

                LIBMATTI_MAVEN_Item *nested = item_new(LIBMATTI_MAVEN_Item_LIST);
                list_add(list, nested);
                list = nested;
                stack = realloc(stack, sizeof(*stack) * (stackCount + 1));
                stack[stackCount++] = list;
                isCombination = 0;
            }

            isDigit = 0;
        }
    }

    if (length > startIndex)
    {
        // Java: 1.0.0.X1 < 1.0.0-X2 - treat .X as -X for any string qualifier X
        if (!isDigit && list->count > 0)
        {
            LIBMATTI_MAVEN_Item *nested = item_new(LIBMATTI_MAVEN_Item_LIST);
            list_add(list, nested);
            list = nested;
            stack = realloc(stack, sizeof(*stack) * (stackCount + 1));
            stack[stackCount++] = list;
        }

        list_add(list, parse_item(isCombination, isDigit, lower + startIndex));
    }

    while (stackCount > 0)
    {
        list = stack[--stackCount];
        list_normalize(list);
    }

    free(stack);
    free(lower);
}

// Java: version.substring(startIndex, end)
static char *local_substring(const char *value, size_t start, size_t end)
{
    size_t length = end - start;
    char *result = malloc(length + 1);
    memcpy(result, value + start, length);
    result[length] = '\0';
    return result;
}

// Java: public ComparableVersion(String version)
LIBMATTI_MAVEN_ComparableVersion *LIBMATTI_MAVEN_ComparableVersion_New(const char *version)
{
    LIBMATTI_MAVEN_ComparableVersion *comparable = calloc(1, sizeof(LIBMATTI_MAVEN_ComparableVersion));
    parse_version(comparable, version);
    return comparable;
}

void LIBMATTI_MAVEN_ComparableVersion_Free(LIBMATTI_MAVEN_ComparableVersion *version)
{
    if (version == NULL) return;

    free(version->value);
    free(version->canonical);
    item_free(version->items);
    free(version);
}

// Java: public int compareTo(ComparableVersion o)
int LIBMATTI_MAVEN_ComparableVersion_Compare(const LIBMATTI_MAVEN_ComparableVersion *a,
                                            const LIBMATTI_MAVEN_ComparableVersion *b)
{
    return item_compare(a->items, b->items);
}

// Java: public int hashCode() { return items.hashCode(); }
int LIBMATTI_MAVEN_ComparableVersion_HashCode(const LIBMATTI_MAVEN_ComparableVersion *version)
{
    return item_hash_code(version->items);
}

// Java: public int orderingHashCode() { return orderingHash(items); }
int LIBMATTI_MAVEN_ComparableVersion_OrderingHashCode(const LIBMATTI_MAVEN_ComparableVersion *version)
{
    return ordering_hash(version->items);
}

// Java: public String toString()
const char *LIBMATTI_MAVEN_ComparableVersion_ToString(const LIBMATTI_MAVEN_ComparableVersion *version)
{
    return version->value;
}

// Java: public String getCanonical()
char *LIBMATTI_MAVEN_ComparableVersion_GetCanonical(LIBMATTI_MAVEN_ComparableVersion *version)
{
    if (version->canonical == NULL)
    {
        Buffer buffer = {0};
        item_to_string(version->items, &buffer);
        version->canonical = buffer.data != NULL ? buffer.data : strdup("");
    }
    return strdup(version->canonical);
}
