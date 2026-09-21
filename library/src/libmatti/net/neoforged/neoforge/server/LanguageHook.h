// Port of net.neoforged.neoforge.server.LanguageHook.

#ifndef MATTICRAFT_NEOFORGE_SERVER_LANGUAGEHOOK_H
#define MATTICRAFT_NEOFORGE_SERVER_LANGUAGEHOOK_H

#include <stddef.h>

// Java: private static Map<String, String> modTable - the port owns the merged table
typedef struct
{
    char **keys;
    char **values;
    size_t count;
} LIBMATTI_NEOFORGE_LanguageHook_LanguageTable;

// Java: public static void loadBuiltinLanguages()
void LIBMATTI_NEOFORGE_LanguageHook_LoadBuiltinLanguages(void);

// Java: static Map<String, String> getModTable() - the table the game merges into its language
const LIBMATTI_NEOFORGE_LanguageHook_LanguageTable *LIBMATTI_NEOFORGE_LanguageHook_GetModTable(void);

// Java: public static void captureLanguageMap(Map<String, String> table, Map<String, Component> componentTable)
// The game hands its default tables over; the mod entries are merged in like Java.
void LIBMATTI_NEOFORGE_LanguageHook_CaptureLanguageMap(void *table, void *componentTable);

#endif //MATTICRAFT_NEOFORGE_SERVER_LANGUAGEHOOK_H
