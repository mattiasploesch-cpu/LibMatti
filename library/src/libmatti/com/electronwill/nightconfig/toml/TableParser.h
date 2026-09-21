// Port of com.electronwill.nightconfig.toml.TableParser.
// Java throws a ParsingException; the port returns NULL / 0 and records the failure in the parser.
// Java's List<String> keys become LIBMATTI_NC_KeyList.

#ifndef MATTICRAFT_NC_TOML_TABLEPARSER_H
#define MATTICRAFT_NC_TOML_TABLEPARSER_H

#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

// Java: List<String> - the components of a (dotted) key or table name
typedef struct
{
    char **items;
    size_t count;
    size_t capacity;
} LIBMATTI_NC_KeyList;

// Java: new ArrayList<>()
LIBMATTI_NC_KeyList *LIBMATTI_NC_KeyList_New(void);
// Java: List.add(element) - the list takes ownership of the string
void LIBMATTI_NC_KeyList_Add(LIBMATTI_NC_KeyList *list, char *item);
void LIBMATTI_NC_KeyList_Free(LIBMATTI_NC_KeyList *list);

// Java: static <T extends CommentedConfig> T parseInline(CharacterInput input, TomlParser parser, T config)
int LIBMATTI_NC_TableParser_ParseInline(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                        LIBMATTI_NC_Config *config);
// Java: static <T extends CommentedConfig> T parseNormal(CharacterInput input, TomlParser parser, T config)
int LIBMATTI_NC_TableParser_ParseNormal(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                        LIBMATTI_NC_Config *config);
// Java: static CommentedConfig parseNormal(CommentedConfig parentConfig, CharacterInput input, TomlParser parser)
LIBMATTI_NC_Config *LIBMATTI_NC_TableParser_ParseNormalSubConfig(LIBMATTI_NC_Config *parentConfig,
                                                                LIBMATTI_NC_ReaderInput *input,
                                                                LIBMATTI_NC_TomlParser *parser);
// Java: static List<String> parseTableName(CharacterInput input, TomlParser parser, boolean array)
LIBMATTI_NC_KeyList *LIBMATTI_NC_TableParser_ParseTableName(LIBMATTI_NC_ReaderInput *input,
                                                           LIBMATTI_NC_TomlParser *parser, int array);
// Java: static List<String> parseDottedKey(CharacterInput input, char firstChar, TomlParser parser)
LIBMATTI_NC_KeyList *LIBMATTI_NC_TableParser_ParseDottedKey(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                                           LIBMATTI_NC_TomlParser *parser);
// Java: static String parseNonDottedKey(CharacterInput input, char firstChar, TomlParser parser)
char *LIBMATTI_NC_TableParser_ParseNonDottedKey(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                               LIBMATTI_NC_TomlParser *parser);

#endif //MATTICRAFT_NC_TOML_TABLEPARSER_H
