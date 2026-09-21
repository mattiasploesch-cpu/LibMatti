// Port of com.google.gson.stream.JsonReader.
// Java reads from a Reader; the port is handed the complete text.
// The C port emits SyntaxError as NULL / (Java: throws MalformedJsonException, IOException,
// IllegalStateException).

#ifndef MATTICRAFT_GSON_JSONREADER_H
#define MATTICRAFT_GSON_JSONREADER_H

#include <stddef.h>

// Java: public enum JsonToken
typedef enum
{
    LIBMATTI_GSON_JSON_TOKEN_BEGIN_ARRAY = 0,
    LIBMATTI_GSON_JSON_TOKEN_END_ARRAY,
    LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT,
    LIBMATTI_GSON_JSON_TOKEN_END_OBJECT,
    LIBMATTI_GSON_JSON_TOKEN_NAME,
    LIBMATTI_GSON_JSON_TOKEN_STRING,
    LIBMATTI_GSON_JSON_TOKEN_NUMBER,
    LIBMATTI_GSON_JSON_TOKEN_BOOLEAN,
    LIBMATTI_GSON_JSON_TOKEN_NULL,
    LIBMATTI_GSON_JSON_TOKEN_END_DOCUMENT
} LIBMATTI_GSON_JsonToken;

typedef struct LIBMATTI_GSON_JsonReader
{
    // Java: private final Reader in / private final char[] buffer
    const char *text;
    size_t length;
    size_t pos;
    // Java: private boolean lenient
    int lenient;
    // Java: private int[] stack - PEEKED_NONE / PEEKED_* of the enclosing scopes
    int *stack;
    size_t stackSize;
    size_t stackCapacity;
    // Java: private String peeked, peekedString
    int peeked;
    char *peekedText;
    int hasPeekedText;
    // Java: private int peekedNumber / the peeked literal
    char *peekedNumber;
    int hasPeekedNumber;
} LIBMATTI_GSON_JsonReader;

// Java: public JsonReader(Reader in)
LIBMATTI_GSON_JsonReader *LIBMATTI_GSON_JsonReader_New(const char *text, size_t length);
void LIBMATTI_GSON_JsonReader_Free(LIBMATTI_GSON_JsonReader *reader);

// Java: public void setLenient(boolean lenient)
void LIBMATTI_GSON_JsonReader_SetLenient(LIBMATTI_GSON_JsonReader *reader, int lenient);

// Java: public JsonToken peek()
int LIBMATTI_GSON_JsonReader_Peek(LIBMATTI_GSON_JsonReader *reader);
// Java: public void beginArray()
void LIBMATTI_GSON_JsonReader_BeginArray(LIBMATTI_GSON_JsonReader *reader);
// Java: public void endArray()
void LIBMATTI_GSON_JsonReader_EndArray(LIBMATTI_GSON_JsonReader *reader);
// Java: public void beginObject()
void LIBMATTI_GSON_JsonReader_BeginObject(LIBMATTI_GSON_JsonReader *reader);
// Java: public void endObject()
void LIBMATTI_GSON_JsonReader_EndObject(LIBMATTI_GSON_JsonReader *reader);
// Java: public boolean hasNext()
int LIBMATTI_GSON_JsonReader_HasNext(LIBMATTI_GSON_JsonReader *reader);
// Java: public String nextName(); caller frees; NULL at the end of the document
char *LIBMATTI_GSON_JsonReader_NextName(LIBMATTI_GSON_JsonReader *reader);
// Java: public String nextString(); caller frees; NULL at the end of the document
char *LIBMATTI_GSON_JsonReader_NextString(LIBMATTI_GSON_JsonReader *reader);
// Java: public boolean nextBoolean()
int LIBMATTI_GSON_JsonReader_NextBoolean(LIBMATTI_GSON_JsonReader *reader);
// Java: public void nextNull()
void LIBMATTI_GSON_JsonReader_NextNull(LIBMATTI_GSON_JsonReader *reader);
// Java: public void skipValue()
void LIBMATTI_GSON_JsonReader_SkipValue(LIBMATTI_GSON_JsonReader *reader);

#endif //MATTICRAFT_GSON_JSONREADER_H
