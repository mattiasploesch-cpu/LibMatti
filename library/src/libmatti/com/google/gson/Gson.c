// Port of com.google.gson.Gson.

#include "libmatti/com/google/gson/Gson.h"

#include <stdlib.h>
#include <string.h>

// Java: public Gson()
LIBMATTI_GSON_Gson *LIBMATTI_GSON_Gson_New(void)
{
    LIBMATTI_GSON_Gson *gson = calloc(1, sizeof(LIBMATTI_GSON_Gson));
    // Java: private final boolean lenient = false
    gson->lenient = 0;
    return gson;
}

void LIBMATTI_GSON_Gson_Free(LIBMATTI_GSON_Gson *gson)
{
    free(gson);
}

// Java: public JsonReader newJsonReader(Reader reader)
LIBMATTI_GSON_JsonReader *LIBMATTI_GSON_Gson_NewJsonReader(const LIBMATTI_GSON_Gson *gson, const char *text,
                                                           size_t length)
{
    LIBMATTI_GSON_JsonReader *reader = LIBMATTI_GSON_JsonReader_New(text, length);
    LIBMATTI_GSON_JsonReader_SetLenient(reader, gson->lenient);
    return reader;
}

// Java: TypeAdapters.JSON_ELEMENT.read(JsonReader in)
static LIBMATTI_GSON_JsonElement *readJsonElement(LIBMATTI_GSON_JsonReader *reader)
{
    switch (LIBMATTI_GSON_JsonReader_Peek(reader))
    {
        case LIBMATTI_GSON_JSON_TOKEN_STRING:
        {
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_JsonElement_NewPrimitiveString(value);
            free(value);
            return element;
        }
        case LIBMATTI_GSON_JSON_TOKEN_NUMBER:
        {
            // Java: String number = in.nextString(); return new JsonPrimitive(new LazilyParsedNumber(number));
            char *number = LIBMATTI_GSON_JsonReader_NextString(reader);
            LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_JsonElement_NewPrimitiveNumber(number);
            free(number);
            return element;
        }
        case LIBMATTI_GSON_JSON_TOKEN_BOOLEAN:
            return LIBMATTI_GSON_JsonElement_NewPrimitiveBoolean(
                LIBMATTI_GSON_JsonReader_NextBoolean(reader));

        case LIBMATTI_GSON_JSON_TOKEN_NULL:
            LIBMATTI_GSON_JsonReader_NextNull(reader);
            return LIBMATTI_GSON_JsonNull_Instance();

        case LIBMATTI_GSON_JSON_TOKEN_BEGIN_ARRAY:
        {
            // Java: JsonArray array = new JsonArray(); in.beginArray(); while (in.hasNext()) array.add(read(in)); in.endArray();
            LIBMATTI_GSON_JsonElement *array = LIBMATTI_GSON_JsonElement_NewArray();
            LIBMATTI_GSON_JsonReader_BeginArray(reader);
            while (LIBMATTI_GSON_JsonReader_HasNext(reader))
                LIBMATTI_GSON_JsonElement_AddElement(array, readJsonElement(reader));
            LIBMATTI_GSON_JsonReader_EndArray(reader);
            return array;
        }

        case LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT:
        {
            // Java: JsonObject object = new JsonObject(); in.beginObject(); while (in.hasNext()) object.add(in.nextName(), read(in)); in.endObject();
            LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_JsonElement_NewObject();
            LIBMATTI_GSON_JsonReader_BeginObject(reader);
            while (LIBMATTI_GSON_JsonReader_HasNext(reader))
            {
                char *name = LIBMATTI_GSON_JsonReader_NextName(reader);
                LIBMATTI_GSON_JsonElement_AddMember(object, name, readJsonElement(reader));
                free(name);
            }
            LIBMATTI_GSON_JsonReader_EndObject(reader);
            return object;
        }

        case LIBMATTI_GSON_JSON_TOKEN_END_DOCUMENT:
            // Java: return null;
            return NULL;

        default:
            // Java: throw new IllegalStateException()
            return NULL;
    }
}

// Java: public <T> T fromJson(JsonReader reader, Type typeOfT)
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_Gson_FromJsonReader(LIBMATTI_GSON_Gson *gson,
                                                             LIBMATTI_GSON_JsonReader *reader)
{
    (void) gson;

    // Java: boolean oldLenient = reader.isLenient(); reader.setLenient(true);
    int oldLenient = reader->lenient;
    LIBMATTI_GSON_JsonReader_SetLenient(reader, 1);

    // Java: boolean isEmpty = true; reader.peek(); isEmpty = false;
    LIBMATTI_GSON_JsonReader_Peek(reader);

    LIBMATTI_GSON_JsonElement *object = readJsonElement(reader);

    LIBMATTI_GSON_JsonReader_SetLenient(reader, oldLenient);
    return object;
}

// Java: private static void assertFullConsumption(Object obj, JsonReader reader)
static int assertFullConsumption(LIBMATTI_GSON_JsonElement *object, LIBMATTI_GSON_JsonReader *reader)
{
    // Java: throw new JsonIOException("JSON document was not fully consumed.")
    return object != NULL && LIBMATTI_GSON_JsonReader_Peek(reader) != LIBMATTI_GSON_JSON_TOKEN_END_DOCUMENT;
}

// Java: public <T> T fromJson(Reader json, Type typeOfT)
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_Gson_FromJson(LIBMATTI_GSON_Gson *gson, const char *text, size_t length)
{
    // Java: JsonReader jsonReader = newJsonReader(json);
    LIBMATTI_GSON_JsonReader *reader = LIBMATTI_GSON_Gson_NewJsonReader(gson, text, length);

    LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_Gson_FromJsonReader(gson, reader);

    // Java: assertFullConsumption(object, jsonReader);
    assertFullConsumption(object, reader);

    LIBMATTI_GSON_JsonReader_Free(reader);
    return object;
}
