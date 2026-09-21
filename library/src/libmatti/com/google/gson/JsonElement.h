// Port of com.google.gson.JsonElement and its subclasses
// (JsonObject, JsonArray, JsonPrimitive, JsonNull).
// The C port groups the small Java class family in one header/implementation pair,
// as the ASM tree node classes are grouped in tree/InsnNodes.
// Java: getAsNumber() returns a LazilyParsedNumber; the port keeps the raw literal.

#ifndef MATTICRAFT_GSON_JSONELEMENT_H
#define MATTICRAFT_GSON_JSONELEMENT_H

#include <stddef.h>

// Java: the concrete subclasses of JsonElement
typedef enum
{
    LIBMATTI_GSON_JSON_NULL = 0,
    LIBMATTI_GSON_JSON_PRIMITIVE,
    LIBMATTI_GSON_JSON_ARRAY,
    LIBMATTI_GSON_JSON_OBJECT
} LIBMATTI_GSON_JsonElementType;

// Java: public abstract class JsonElement
typedef struct LIBMATTI_GSON_JsonElement LIBMATTI_GSON_JsonElement;

struct LIBMATTI_GSON_JsonElement
{
    // Java: the runtime class of the element
    LIBMATTI_GSON_JsonElementType type;

    // Java: JsonPrimitive - a boolean, a number (LazilyParsedNumber) or a string
    int booleanValue;
    int isString; // Java: JsonPrimitive.isString()
    int isNumber; // Java: JsonPrimitive.isNumber()
    int isBoolean; // Java: JsonPrimitive.isBoolean()
    char *value; // Java: the string value / the number literal

    // Java: JsonArray.elements
    LIBMATTI_GSON_JsonElement **elements;
    size_t elementCount;

    // Java: JsonObject.members (a LinkedTreeMap, kept insertion-ordered)
    char **memberNames;
    LIBMATTI_GSON_JsonElement **members;
    size_t memberCount;
};

// Java: public JsonElement() - the port creates the subclass directly
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewObject(void);
// Java: public JsonArray()
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewArray(void);
// Java: public JsonPrimitive(Boolean/String/Number)
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveBoolean(int value);
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveString(const char *value);
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_NewPrimitiveNumber(const char *literal);
// Java: JsonNull.INSTANCE
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonNull_Instance(void);

void LIBMATTI_GSON_JsonElement_Free(LIBMATTI_GSON_JsonElement *element);

// Java: public boolean isJsonNull()
int LIBMATTI_GSON_JsonElement_IsJsonNull(const LIBMATTI_GSON_JsonElement *element);
// Java: public boolean isJsonPrimitive()
int LIBMATTI_GSON_JsonElement_IsJsonPrimitive(const LIBMATTI_GSON_JsonElement *element);
// Java: public boolean isJsonArray()
int LIBMATTI_GSON_JsonElement_IsJsonArray(const LIBMATTI_GSON_JsonElement *element);
// Java: public boolean isJsonObject()
int LIBMATTI_GSON_JsonElement_IsJsonObject(const LIBMATTI_GSON_JsonElement *element);

// Java: public JsonObject getAsJsonObject()
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetAsJsonObject(LIBMATTI_GSON_JsonElement *element);
// Java: public JsonArray getAsJsonArray()
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetAsJsonArray(LIBMATTI_GSON_JsonElement *element);

// Java: public String getAsString()
char *LIBMATTI_GSON_JsonElement_GetAsString(const LIBMATTI_GSON_JsonElement *element);
// Java: public boolean getAsBoolean()
int LIBMATTI_GSON_JsonElement_GetAsBoolean(const LIBMATTI_GSON_JsonElement *element);
// Java: public Number getAsNumber() - the port returns the number literal
const char *LIBMATTI_GSON_JsonElement_GetAsNumber(const LIBMATTI_GSON_JsonElement *element);
// Java: public byte getAsByte()
int LIBMATTI_GSON_JsonElement_GetAsByte(const LIBMATTI_GSON_JsonElement *element);
// Java: public short getAsShort()
int LIBMATTI_GSON_JsonElement_GetAsShort(const LIBMATTI_GSON_JsonElement *element);
// Java: public int getAsInt()
int LIBMATTI_GSON_JsonElement_GetAsInt(const LIBMATTI_GSON_JsonElement *element);
// Java: public long getAsLong()
long long LIBMATTI_GSON_JsonElement_GetAsLong(const LIBMATTI_GSON_JsonElement *element);
// Java: public float getAsFloat()
float LIBMATTI_GSON_JsonElement_GetAsFloat(const LIBMATTI_GSON_JsonElement *element);
// Java: public double getAsDouble()
double LIBMATTI_GSON_JsonElement_GetAsDouble(const LIBMATTI_GSON_JsonElement *element);

// Java: JsonObject.get(String) - NULL when the member is absent
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_GetMember(const LIBMATTI_GSON_JsonElement *object,
                                                               const char *memberName);
// Java: JsonObject.has(String)
int LIBMATTI_GSON_JsonElement_HasMember(const LIBMATTI_GSON_JsonElement *object, const char *memberName);
// Java: JsonObject.add(String, JsonElement)
void LIBMATTI_GSON_JsonElement_AddMember(LIBMATTI_GSON_JsonElement *object, const char *memberName,
                                         LIBMATTI_GSON_JsonElement *value);
// Java: JsonObject.size()
size_t LIBMATTI_GSON_JsonElement_MemberCount(const LIBMATTI_GSON_JsonElement *object);
// Java: JsonObject.entrySet() - the member name at the given index
const char *LIBMATTI_GSON_JsonElement_MemberNameAt(const LIBMATTI_GSON_JsonElement *object, size_t index);
// Java: JsonObject.entrySet() - the member value at the given index
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_MemberValueAt(const LIBMATTI_GSON_JsonElement *object,
                                                                  size_t index);

// Java: JsonArray.add(JsonElement)
void LIBMATTI_GSON_JsonElement_AddElement(LIBMATTI_GSON_JsonElement *array, LIBMATTI_GSON_JsonElement *element);
// Java: JsonArray.size()
size_t LIBMATTI_GSON_JsonElement_ElementCount(const LIBMATTI_GSON_JsonElement *array);
// Java: JsonArray.get(int)
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_JsonElement_ElementAt(const LIBMATTI_GSON_JsonElement *array, size_t index);

// Java: @Override public String toString(); caller frees
char *LIBMATTI_GSON_JsonElement_ToString(const LIBMATTI_GSON_JsonElement *element);

#endif //MATTICRAFT_GSON_JSONELEMENT_H
