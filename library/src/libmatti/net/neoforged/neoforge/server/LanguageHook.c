// Port of net.neoforged.neoforge.server.LanguageHook.
// Java loads assets/<ns>/lang/en_us.json through the context class loader and merges the FML
// translations; the Component maps are the game port's part (no net.minecraft texts here).

#include "libmatti/net/neoforged/neoforge/server/LanguageHook.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/net/neoforged/fml/i18n/I18nManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static Map<String, String> defaultLanguageTable / modTable
static LIBMATTI_NEOFORGE_LanguageHook_LanguageTable defaultLanguageTable = {0};
static LIBMATTI_NEOFORGE_LanguageHook_LanguageTable modTable = {0};

static void table_put(LIBMATTI_NEOFORGE_LanguageHook_LanguageTable *table, char *key, char *value)
{
    table->keys = realloc(table->keys, sizeof(char *) * (table->count + 1));
    table->values = realloc(table->values, sizeof(char *) * (table->count + 1));
    table->keys[table->count] = key;
    table->values[table->count] = value;
    table->count++;
}

static void table_free(LIBMATTI_NEOFORGE_LanguageHook_LanguageTable *table)
{
    for (size_t i = 0; i < table->count; i++)
    {
        free(table->keys[i]);
        free(table->values[i]);
    }
    free(table->keys);
    free(table->values);
    table->keys = NULL;
    table->values = NULL;
    table->count = 0;
}

// Java: Language.loadFromJson(input, (key, value) -> modTable.put(key, value), ...)
static void load_language_file(const char *resourceName, const char *logName)
{
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();

    // Java: Thread.currentThread().getContextClassLoader().getResourceAsStream(...)
    LIBMATTI_JL_ClassLoader *classLoader = LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread());
    size_t length = 0;
    char *input = LIBMATTI_JL_ClassLoader_GetResourceAsStream(classLoader, resourceName, &length);
    if (input == NULL)
    {
        // Java: catch (Exception exception) { LOGGER.warn("Failed to load built-in language file for ...", exception) }
        LIBMATTI_ML_Logger_Warn(logger, NULL, "Failed to load built-in language file for {}", logName);
        return;
    }

    // Java: the JSON parse walks the {key: value} pairs into modTable; the port parses the
    // buffer through the I18nManager's Gson reader
    LIBMATTI_FML_I18nManager_TranslationMap *parsed = LIBMATTI_FML_I18nManager_LoadTranslationsBuffer(input, length);
    for (size_t i = 0; i < parsed->count; i++)
        table_put(&modTable, strdup(parsed->keys[i]), strdup(parsed->values[i]));
    LIBMATTI_FML_I18nManager_FreeMap(parsed);

    free(input);
}

// Java: public static void loadBuiltinLanguages()
void LIBMATTI_NEOFORGE_LanguageHook_LoadBuiltinLanguages(void)
{
    table_free(&modTable); // Java: modTable = new HashMap<>(5000)

    // Java: classLoader.getResourceAsStream("assets/minecraft/lang/en_us.json")
    load_language_file("assets/minecraft/lang/en_us.json", "Minecraft");
    // Java: classLoader.getResourceAsStream("assets/neoforge/lang/en_us.json")
    load_language_file("assets/neoforge/lang/en_us.json", "NeoForge");

    // Java: modTable.putAll(I18nManager.loadTranslations("en_us"))
    LIBMATTI_FML_I18nManager_TranslationMap *fmlTranslations = LIBMATTI_FML_I18nManager_LoadTranslations("en_us");
    for (size_t i = 0; i < fmlTranslations->count; i++)
        table_put(&modTable, strdup(fmlTranslations->keys[i]), strdup(fmlTranslations->values[i]));
    LIBMATTI_FML_I18nManager_FreeMap(fmlTranslations);

    // Java: defaultLanguageTable.putAll(modTable)
    for (size_t i = 0; i < modTable.count; i++)
        table_put(&defaultLanguageTable, strdup(modTable.keys[i]), strdup(modTable.values[i]));

    // Java: I18nManager.injectTranslations(modTable)
    LIBMATTI_FML_I18nManager_TranslationMap injected = {modTable.keys, modTable.values, modTable.count};
    LIBMATTI_FML_I18nManager_InjectTranslations(&injected);
}

// Java: static Map<String, String> getModTable()
const LIBMATTI_NEOFORGE_LanguageHook_LanguageTable *LIBMATTI_NEOFORGE_LanguageHook_GetModTable(void)
{
    return &modTable;
}

// Java: public static void captureLanguageMap(Map<String, String> table, Map<String, Component> componentTable)
void LIBMATTI_NEOFORGE_LanguageHook_CaptureLanguageMap(void *table, void *componentTable)
{
    (void) componentTable; // Java: defaultLanguageComponentTable - the Component model is the game port's part
    (void) table;
    // Java: defaultLanguageTable = table; the port's default table is owned here
}
