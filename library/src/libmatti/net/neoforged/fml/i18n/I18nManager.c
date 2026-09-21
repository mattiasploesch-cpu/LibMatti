// Port of net.neoforged.fml.i18n.I18nManager over the Gson port.
// The translation JSONs are {key: value} objects at /lang/<locale>.json.

#include "libmatti/net/neoforged/fml/i18n/I18nManager.h"

#include "libmatti/com/google/gson/JsonReader.h"
#include "libmatti/java/nio/file/Files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_FML_I18nManager_TranslationMap *defaultTranslations = NULL;
static LIBMATTI_FML_I18nManager_TranslationMap *currentLocale = NULL;

// parses a {key: value} JSON object into the map
static LIBMATTI_FML_I18nManager_TranslationMap *parse_translations(const char *text)
{
    LIBMATTI_FML_I18nManager_TranslationMap *map = calloc(1, sizeof(LIBMATTI_FML_I18nManager_TranslationMap));
    if (text == NULL)
        return map;

    LIBMATTI_GSON_JsonReader *reader = LIBMATTI_GSON_JsonReader_New(text, strlen(text));
    if (reader == NULL)
        return map;

    if (LIBMATTI_GSON_JsonReader_Peek(reader) == LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
    {
        LIBMATTI_GSON_JsonReader_BeginObject(reader);
        while (LIBMATTI_GSON_JsonReader_HasNext(reader))
        {
            char *key = LIBMATTI_GSON_JsonReader_NextName(reader);
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (key != NULL && value != NULL)
            {
                map->keys = realloc(map->keys, sizeof(char *) * (map->count + 1));
                map->values = realloc(map->values, sizeof(char *) * (map->count + 1));
                map->keys[map->count] = key;
                map->values[map->count] = value;
                map->count++;
            }
            else
            {
                free(key);
                free(value);
            }
        }
        LIBMATTI_GSON_JsonReader_EndObject(reader);
    }
    LIBMATTI_GSON_JsonReader_Free(reader);
    return map;
}

// Java: FMLTranslations.class.getResourceAsStream("/lang/" + language + ".json")
// The port reads from the fml lang folder next to the library (the loader's
// resources are files on disk here).
static char *read_lang_resource(const char *language)
{
    // the lang folder ships next to the FML loader ("lang/en_us.json")
    char *paths[] = {"lang/en_us.json", NULL};
    (void) paths;

    char path[256];
    snprintf(path, sizeof(path), "lang/%s.json", language);
    if (!LIBMATTI_JNF_Files_Exists(path))
        return NULL;
    return LIBMATTI_JNF_Files_ReadString(path);
}

LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_DefaultTranslations(void)
{
    if (defaultTranslations == NULL)
    {
        // Java: Collections.unmodifiableMap(loadTranslations(DEFAULT_LOCALE))
        char *text = read_lang_resource(LIBMATTI_FML_I18nManager_DEFAULT_LOCALE);
        defaultTranslations = parse_translations(text);
        free(text);
    }
    return defaultTranslations;
}

LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_CurrentLocale(void)
{
    return currentLocale != NULL ? currentLocale : LIBMATTI_FML_I18nManager_DefaultTranslations();
}

void LIBMATTI_FML_I18nManager_InjectTranslations(LIBMATTI_FML_I18nManager_TranslationMap *translations)
{
    // Java: currentLocale = Collections.unmodifiableMap(translations)
    currentLocale = translations;
}

LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_LoadTranslations(const char *language)
{
    char *text = read_lang_resource(language);
    LIBMATTI_FML_I18nManager_TranslationMap *map = parse_translations(text);
    free(text);
    return map;
}

// Java: the JSON parse of a loaded language file buffer (Language.loadFromJson counterpart)
LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_LoadTranslationsBuffer(const char *text, size_t length)
{
    return parse_translations(text);
}

const char *LIBMATTI_FML_I18nManager_Get(const LIBMATTI_FML_I18nManager_TranslationMap *map, const char *key)
{
    if (map == NULL)
        return NULL;
    for (size_t i = 0; i < map->count; i++)
        if (strcmp(map->keys[i], key) == 0)
            return map->values[i];
    return NULL;
}

void LIBMATTI_FML_I18nManager_FreeMap(LIBMATTI_FML_I18nManager_TranslationMap *map)
{
    if (map == NULL)
        return;
    for (size_t i = 0; i < map->count; i++)
    {
        free(map->keys[i]);
        free(map->values[i]);
    }
    free(map->keys);
    free(map->values);
    free(map);
}
