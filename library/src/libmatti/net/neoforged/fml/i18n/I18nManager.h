// Port of net.neoforged.fml.i18n.I18nManager.
// Loads the FML translation JSONs (/lang/<locale>.json) and keeps the
// current-locale map.

#ifndef MATTICRAFT_FML_I18N_I18NMANAGER_H
#define MATTICRAFT_FML_I18N_I18NMANAGER_H

#include <stddef.h>

// Java: private static final String DEFAULT_LOCALE = "en_us"
#define LIBMATTI_FML_I18nManager_DEFAULT_LOCALE "en_us"

typedef struct
{
    char **keys;   // owned
    char **values; // owned
    size_t count;
} LIBMATTI_FML_I18nManager_TranslationMap;

// Java: static final Map<String,String> DEFAULT_TRANSLATIONS - loaded on first use
LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_DefaultTranslations(void);
// Java: static Map<String, String> currentLocale
LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_CurrentLocale(void);

// Java: public static void injectTranslations(Map<String, String> translations)
void LIBMATTI_FML_I18nManager_InjectTranslations(LIBMATTI_FML_I18nManager_TranslationMap *translations);
// Java: public static Map<String, String> loadTranslations(String language) -
// parses /lang/<language>.json from the classpath root; empty map when absent
LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_LoadTranslations(const char *language);
// Java: the JSON parse of a loaded language file buffer (Language.loadFromJson counterpart)
LIBMATTI_FML_I18nManager_TranslationMap *LIBMATTI_FML_I18nManager_LoadTranslationsBuffer(const char *text, size_t length);

// Java: Map.get(key) - NULL when absent
const char *LIBMATTI_FML_I18nManager_Get(const LIBMATTI_FML_I18nManager_TranslationMap *map, const char *key);

// frees a translation map
void LIBMATTI_FML_I18nManager_FreeMap(LIBMATTI_FML_I18nManager_TranslationMap *map);

#endif //MATTICRAFT_FML_I18N_I18NMANAGER_H
