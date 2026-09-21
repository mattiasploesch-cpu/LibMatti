// Port of com.google.gson.JsonElement / JsonObject / JsonArray / JsonPrimitive / JsonNull.

#include "libmatti/com/google/gson/JsonElement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: JsonNull.INSTANCE
static LIBMATTI_GSON_JsonElement *instance;

// Java: public JsonArray()
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewArray(void)
{
    LIBMATTI_GSON_JsonElement *element = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
    element->type = LIBMATTI_GSON_JSON_ARRAY;
    return element;
}

// Java: public JsonObject()
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewObject(void)
{
    LIBMATTI_GSON_JsonElement *element = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
    element->type = LIBMATTI_GSON_JSON_OBJECT;
    return element;
}

// Java: public JsonPrimitive(Boolean value) { super(); this.value = value; }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveBoolean(int value)
{
    LIBMATTI_GSON_JsonElement *element = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
    element->type = LIBMATTI_GSON_JSON_PRIMITIVE;
    element->isBoolean = 1;
    element->booleanValue = value;
    return element;
}

// Java: public JsonPrimitive(String value) { super(); this.value = value; }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveString(const char *value)
{
    LIBMATTI_GSON_JsonElement *element = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
    element->type = LIBMATTI_GSON_JSON_PRIMITIVE;
    element->isString = 1;
    element->value = value != NULL ? strdup(value) : NULL;
    return element;
}

// Java: public JsonPrimitive(Number value) - LazilyParsedNumber keeps the literal
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveNumber(const char *literal)
{
    LIBMATTI_GSON_JsonElement *element = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
    element->type = LIBMATTI_GSON_JSON_PRIMITIVE;
    element->isNumber = 1;
    element->value = literal != NULL ? strdup(literal) : strdup("0");
    return element;
}

// Java: JsonNull.INSTANCE
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonNull_Instance(void)
{
    if (instance == NULL)
    {
        instance = calloc(1, sizeof(LIBMATTI_GSON_JsonElement));
        instance->type = LIBMATTI_GSON_JSON_NULL;
    }
    return instance;
}

void LIBMATTI_GSON_JsonElement_Free(LIBMATTI_GSON_JsonElement *element)
{
    if (element == NULL || element == instance) return;

    free(element->value);

    for (size_t i = 0; i < element->elementCount; i++)
        LIBMATTI_GSON_JsonElement_Free(element->elements[i]);
    free(element->elements);

    for (size_t i = 0; i < element->memberCount; i++)
    {
        free(element->memberNames[i]);
        LIBMATTI_GSON_JsonElement_Free(element->members[i]);
    }
    free(element->memberNames);
    free(element->members);

    free(element);
}

// Java: public boolean isJsonNull() { return this instanceof JsonNull; }
int LIBMATTI_GSON_JsonElement_IsJsonNull(const LIBMATTI_GSON_JsonElement *element)
{
    return element != NULL && element->type == LIBMATTI_GSON_JSON_NULL;
}

// Java: public boolean isJsonPrimitive() { return this instanceof JsonPrimitive; }
int LIBMATTI_GSON_JsonElement_IsJsonPrimitive(const LIBMATTI_GSON_JsonElement *element)
{
    return element != NULL && element->type == LIBMATTI_GSON_JSON_PRIMITIVE;
}

// Java: public boolean isJsonArray() { return this instanceof JsonArray; }
int LIBMATTI_GSON_JsonElement_IsJsonArray(const LIBMATTI_GSON_JsonElement *element)
{
    return element != NULL && element->type == LIBMATTI_GSON_JSON_ARRAY;
}

// Java: public boolean isJsonObject() { return this instanceof JsonObject; }
int LIBMATTI_GSON_JsonElement_IsJsonObject(const LIBMATTI_GSON_JsonElement *element)
{
    return element != NULL && element->type == LIBMATTI_GSON_JSON_OBJECT;
}

// Java: public JsonObject getAsJsonObject() { if (isJsonObject()) return (JsonObject) this; throw new IllegalStateException(...); }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetAsJsonObject(LIBMATTI_GSON_JsonElement *element)
{
    if (LIBMATTI_GSON_JsonElement_IsJsonObject(element)) return element;
    return NULL;
}

// Java: public JsonArray getAsJsonArray() { if (isJsonArray()) return (JsonArray) this; throw new IllegalStateException(...); }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetAsJsonArray(LIBMATTI_GSON_JsonElement *element)
{
    if (LIBMATTI_GSON_JsonElement_IsJsonArray(element)) return element;
    return NULL;
}

// Java: public String getAsString()
char *LIBMATTI_GSON_JsonElement_GetAsString(const LIBMATTI_GSON_JsonElement *element)
{
    if (element == NULL) return NULL;

    // Java: JsonPrimitive.getAsString()
    if (element->type == LIBMATTI_GSON_JSON_PRIMITIVE)
    {
        // Java: return isNumber() ? getAsNumber().toString() : isBoolean() ? Boolean.toString(value) : (String) value;
        if (element->isBoolean)
            return strdup(element->booleanValue ? "true" : "false");
        return element->value != NULL ? strdup(element->value) : NULL;
    }

    // Java: the default throws UnsupportedOperationException; the port reports NULL
    return NULL;
}

// Java: public boolean getAsBoolean() { if (isJsonPrimitive()) return getAsJsonPrimitive().getAsBoolean(); return Boolean.parseBoolean(getAsString()); }
int LIBMATTI_GSON_JsonElement_GetAsBoolean(const LIBMATTI_GSON_JsonElement *element)
{
    if (element == NULL) return 0;
    if (element->type == LIBMATTI_GSON_JSON_PRIMITIVE)
    {
        // Java: return isBoolean() ? ((Boolean) value) : Boolean.parseBoolean(getAsString());
        if (element->isBoolean) return element->booleanValue;
        return element->value != NULL && strcasecmp(element->value, "true") == 0;
    }
    return 0;
}

// Java: public Number getAsNumber() - the port exposes the number literal
const char *LIBMATTI_GSON_JsonElement_GetAsNumber(const LIBMATTI_GSON_JsonElement *element)
{
    if (element == NULL || element->type != LIBMATTI_GSON_JSON_PRIMITIVE) return NULL;
    return element->value;
}

// Java: public byte getAsByte() { return isNumber() ? getAsNumber().byteValue() : Byte.parseByte(getAsString()); }
int LIBMATTI_GSON_JsonElement_GetAsByte(const LIBMATTI_GSON_JsonElement *element)
{
    return (int) (signed char) LIBMATTI_GSON_JsonElement_GetAsInt(element);
}

// Java: public short getAsShort() { return isNumber() ? getAsNumber().shortValue() : Short.parseShort(getAsString()); }
int LIBMATTI_GSON_JsonElement_GetAsShort(const LIBMATTI_GSON_JsonElement *element)
{
    return (int) (short) LIBMATTI_GSON_JsonElement_GetAsInt(element);
}

// Java: public int getAsInt() { return isNumber() ? getAsNumber().intValue() : Integer.parseInt(getAsString()); }
int LIBMATTI_GSON_JsonElement_GetAsInt(const LIBMATTI_GSON_JsonElement *element)
{
    const char *number = LIBMATTI_GSON_JsonElement_GetAsNumber(element);
    if (number == NULL)
    {
        char *text = LIBMATTI_GSON_JsonElement_GetAsString(element);
        int result = text != NULL ? atoi(text) : 0;
        free(text);
        return result;
    }
    return (int) strtol(number, NULL, 10);
}

// Java: public long getAsLong() { return isNumber() ? getAsNumber().longValue() : Long.parseLong(getAsString()); }
long long LIBMATTI_GSON_JsonElement_GetAsLong(const LIBMATTI_GSON_JsonElement *element)
{
    const char *number = LIBMATTI_GSON_JsonElement_GetAsNumber(element);
    if (number == NULL)
    {
        char *text = LIBMATTI_GSON_JsonElement_GetAsString(element);
        long long result = text != NULL ? strtoll(text, NULL, 10) : 0;
        free(text);
        return result;
    }
    return strtoll(number, NULL, 10);
}

// Java: public float getAsFloat() { return isNumber() ? getAsNumber().floatValue() : Float.parseFloat(getAsString()); }
float LIBMATTI_GSON_JsonElement_GetAsFloat(const LIBMATTI_GSON_JsonElement *element)
{
    const char *number = LIBMATTI_GSON_JsonElement_GetAsNumber(element);
    if (number == NULL)
    {
        char *text = LIBMATTI_GSON_JsonElement_GetAsString(element);
        float result = text != NULL ? strtof(text, NULL) : 0.0f;
        free(text);
        return result;
    }
    return strtof(number, NULL);
}

// Java: public double getAsDouble() { return isNumber() ? getAsNumber().doubleValue() : Double.parseDouble(getAsString()); }
double LIBMATTI_GSON_JsonElement_GetAsDouble(const LIBMATTI_GSON_JsonElement *element)
{
    const char *number = LIBMATTI_GSON_JsonElement_GetAsNumber(element);
    if (number == NULL)
    {
        char *text = LIBMATTI_GSON_JsonElement_GetAsString(element);
        double result = text != NULL ? strtod(text, NULL) : 0.0;
        free(text);
        return result;
    }
    return strtod(number, NULL);
}

// Java: JsonObject.get(String memberName) { return members.get(memberName); }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetMember(const LIBMATTI_GSON_JsonElement *object,
                                                               const char *memberName)
{
    if (object == NULL || object->type != LIBMATTI_GSON_JSON_OBJECT || memberName == NULL) return NULL;

    for (size_t i = 0; i < object->memberCount; i++)
        if (strcmp(object->memberNames[i], memberName) == 0)
            return object->members[i];

    return NULL;
}

// Java: JsonObject.has(String memberName) { return members.containsKey(memberName); }
int LIBMATTI_GSON_JsonElement_HasMember(const LIBMATTI_GSON_JsonElement *object, const char *memberName)
{
    return LIBMATTI_GSON_JsonElement_GetMember(object, memberName) != NULL;
}

// Java: JsonObject.add(String property, JsonElement value)
void LIBMATTI_GSON_JsonElement_AddMember(LIBMATTI_GSON_JsonElement *object, const char *memberName,
                                         LIBMATTI_GSON_JsonElement *value)
{
    if (object == NULL) return;

    // Java: members.put(property, value == null ? JsonNull.INSTANCE : value);
    for (size_t i = 0; i < object->memberCount; i++)
    {
        if (strcmp(object->memberNames[i], memberName) == 0)
        {
            object->members[i] = value != NULL ? value : LIBMATTI_GSON_JsonNull_Instance();
            return;
        }
    }

    object->memberNames = realloc(object->memberNames, sizeof(char *) * (object->memberCount + 1));
    object->members = realloc(object->members, sizeof(void *) * (object->memberCount + 1));
    object->memberNames[object->memberCount] = strdup(memberName);
    object->members[object->memberCount] = value != NULL ? value : LIBMATTI_GSON_JsonNull_Instance();
    object->memberCount++;
}

// Java: JsonObject.size() { return members.size(); }
size_t LIBMATTI_GSON_JsonElement_MemberCount(const LIBMATTI_GSON_JsonElement *object)
{
    return object != NULL ? object->memberCount : 0;
}

// Java: JsonObject.entrySet() - the key of the entry at the given index
const char *LIBMATTI_GSON_JsonElement_MemberNameAt(const LIBMATTI_GSON_JsonElement *object, size_t index)
{
    if (object == NULL || index >= object->memberCount) return NULL;
    return object->memberNames[index];
}

// Java: JsonObject.entrySet() - the value of the entry at the given index
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_MemberValueAt(const LIBMATTI_GSON_JsonElement *object,
                                                                  size_t index)
{
    if (object == NULL || index >= object->memberCount) return NULL;
    return object->members[index];
}

// Java: JsonArray.add(JsonElement element)
void LIBMATTI_GSON_JsonElement_AddElement(LIBMATTI_GSON_JsonElement *array, LIBMATTI_GSON_JsonElement *element)
{
    if (array == NULL) return;

    array->elements = realloc(array->elements, sizeof(void *) * (array->elementCount + 1));
    array->elements[array->elementCount++] = element != NULL ? element : LIBMATTI_GSON_JsonNull_Instance();
}

// Java: JsonArray.size() { return elements.size(); }
size_t LIBMATTI_GSON_JsonElement_ElementCount(const LIBMATTI_GSON_JsonElement *array)
{
    return array != NULL ? array->elementCount : 0;
}

// Java: JsonArray.get(int i) { return elements.get(i); }
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_ElementAt(const LIBMATTI_GSON_JsonElement *array, size_t index)
{
    if (array == NULL || index >= array->elementCount) return NULL;
    return array->elements[index];
}

// Java: the Streams.write(JsonWriter) implementation of each subclass
static void writeElement(const LIBMATTI_GSON_JsonElement *element, void (*append)(const char *text, void *userdata),
                         void *userdata)
{
    char buffer[64];

    switch (element->type)
    {
        case LIBMATTI_GSON_JSON_NULL:
            append("null", userdata);
            break;
        case LIBMATTI_GSON_JSON_PRIMITIVE:
            if (element->isBoolean)
                append(element->booleanValue ? "true" : "false", userdata);
            else if (element->isString)
            {
                append("\"", userdata);
                append(element->value != NULL ? element->value : "", userdata);
                append("\"", userdata);
            }
            else
                append(element->value != NULL ? element->value : "null", userdata);
            break;
        case LIBMATTI_GSON_JSON_ARRAY:
            append("[", userdata);
            for (size_t i = 0; i < element->elementCount; i++)
            {
                if (i > 0) append(",", userdata);
                writeElement(element->elements[i], append, userdata);
            }
            append("]", userdata);
            break;
        case LIBMATTI_GSON_JSON_OBJECT:
            append("{", userdata);
            for (size_t i = 0; i < element->memberCount; i++)
            {
                if (i > 0) append(",", userdata);
                snprintf(buffer, sizeof(buffer), "\"%s\":", element->memberNames[i]);
                append(buffer, userdata);
                writeElement(element->members[i], append, userdata);
            }
            append("}", userdata);
            break;
    }
}

typedef struct
{
    char *text;
    size_t length;
    size_t capacity;
} StringBuilder;

static void appendText(const char *text, void *userdata)
{
    StringBuilder *builder = userdata;
    size_t textLength = strlen(text);

    while (builder->length + textLength + 1 > builder->capacity)
    {
        builder->capacity = builder->capacity == 0 ? 64 : builder->capacity * 2;
        builder->text = realloc(builder->text, builder->capacity);
    }

    memcpy(builder->text + builder->length, text, textLength);
    builder->length += textLength;
    builder->text[builder->length] = '\0';
}

// Java: @Override public String toString()
char *LIBMATTI_GSON_JsonElement_ToString(const LIBMATTI_GSON_JsonElement *element)
{
    if (element == NULL) return strdup("null");

    StringBuilder builder = {0};
    writeElement(element, appendText, &builder);
    if (builder.text == NULL) return strdup("");
    return builder.text;
}
