// Port of com.google.gson.Gson.
// Only the JsonElement tree path is used by the ported code
// (Gson.fromJson(Reader, JsonObject.class)); the reflective type adapters are not ported.

#ifndef MATTICRAFT_GSON_GSON_H
#define MATTICRAFT_GSON_GSON_H

#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/com/google/gson/JsonReader.h"

#include <stddef.h>

// Java: public final class Gson
typedef struct LIBMATTI_GSON_Gson
{
    // Java: private final boolean lenient
    int lenient;
} LIBMATTI_GSON_Gson;

// Java: public Gson()
LIBMATTI_GSON_Gson *LIBMATTI_GSON_Gson_New(void);
void LIBMATTI_GSON_Gson_Free(LIBMATTI_GSON_Gson *gson);

// Java: public JsonReader newJsonReader(Reader reader)
LIBMATTI_GSON_JsonReader *LIBMATTI_GSON_Gson_NewJsonReader(const LIBMATTI_GSON_Gson *gson, const char *text,
                                                           size_t length);

// Java: public <T> T fromJson(JsonReader reader, Type typeOfT)
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_Gson_FromJsonReader(LIBMATTI_GSON_Gson *gson,
                                                             LIBMATTI_GSON_JsonReader *reader);

// Java: public <T> T fromJson(Reader json, Type typeOfT) - the JsonElement/JsonObject tree path
LIBMATTI_GSON_JsonElement *LIBMATTI_GSON_Gson_FromJson(LIBMATTI_GSON_Gson *gson, const char *text, size_t length);

#endif //MATTICRAFT_GSON_GSON_H
