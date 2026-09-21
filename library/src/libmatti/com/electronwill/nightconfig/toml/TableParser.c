// Port of com.electronwill.nightconfig.toml.TableParser.

#include "libmatti/com/electronwill/nightconfig/toml/TableParser.h"

#include "libmatti/com/electronwill/nightconfig/toml/StringParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"
#include "libmatti/com/electronwill/nightconfig/toml/ValueParser.h"

#include <stdlib.h>
#include <string.h>

static int parseInlineV1_0(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *config);
static int parseInlineV1_1(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *config);

// Java: private static final char[] KEY_END = {'\t',' ','=','.','\n','\r',']',':'}
static const char KEY_END[] = "\t =.\n\r]:";

LIBMATTI_NC_KeyList *LIBMATTI_NC_KeyList_New(void)
{
    return calloc(1, sizeof(LIBMATTI_NC_KeyList));
}

void LIBMATTI_NC_KeyList_Add(LIBMATTI_NC_KeyList *list, char *item)
{
    if (list->count == list->capacity)
    {
        list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = realloc(list->items, sizeof(*list->items) * list->capacity);
    }
    list->items[list->count++] = item;
}

void LIBMATTI_NC_KeyList_Free(LIBMATTI_NC_KeyList *list)
{
    if (list == NULL) return;
    for (size_t i = 0; i < list->count; i++) free(list->items[i]);
    free(list->items);
    free(list);
}

// Java: private static void checkDuplicateKeyBecauseOfParents(...) - 0 means the ParsingException was thrown
static int checkDuplicateKeyBecauseOfParents(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_KeyList *key,
                                            LIBMATTI_NC_Config *config, int emptyConfig)
{
    if (!emptyConfig || key->count <= 1) return 1;

    LIBMATTI_NC_Config *current = config;
    for (size_t i = 0; i + 1 < key->count; i++)
    {
        const char *singleKey[1] = {key->items[i]};
        LIBMATTI_NC_Value *sub = LIBMATTI_NC_Config_Get(current, singleKey, 1);
        if (sub == NULL) return 1;
        if (sub->type != LIBMATTI_NC_VALUE_CONFIG)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Wrong duplicate key: intermediary level already exists");
            return 0;
        }
        current = sub->config;
    }
    return 1;
}

// Java: private static void checkDuplicateKey(...) - 0 means the ParsingException was thrown
static int checkDuplicateKey(LIBMATTI_NC_TomlParser *parser, int hadPrevious, int emptyConfig)
{
    if (hadPrevious && emptyConfig)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid TOML data: an entry is defined twice in its table.");
        return 0;
    }
    return 1;
}

// Java: static <T extends CommentedConfig> T parseInline(CharacterInput input, TomlParser parser, T config)
int LIBMATTI_NC_TableParser_ParseInline(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                        LIBMATTI_NC_Config *config)
{
    // Java: switch on the TOML version; the port implements the two variants
    if (LIBMATTI_NC_TomlParser_GetTomlVersion(parser) == LIBMATTI_NC_TOMLVERSION_V1_1)
        return parseInlineV1_1(input, parser, config);
    return parseInlineV1_0(input, parser, config);
}

// Java: static <T extends CommentedConfig> T parseInlineV1_0(CharacterInput input, TomlParser parser, T config)
static int parseInlineV1_0(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *config)
{
    LIBMATTI_NC_TomlParser_RegisterInlineTable(parser, config);
    int configWasInitiallyEmpty = LIBMATTI_NC_Config_IsEmpty(config);
    int expectNextElement = 0;
    while (1)
    {
        int keyFirst = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (keyFirst == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid inline table: missing closing brace }");
            return 0;
        }
        if (keyFirst == '}') // handles {} and {k1=v1,... ,}
        {
            if (expectNextElement)
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid trailing comma in inline table.");
            return 1;
        }

        LIBMATTI_NC_KeyList *key = LIBMATTI_NC_TableParser_ParseDottedKey(input, (char) keyFirst, parser);
        if (key == NULL) return 0;
        if (!checkDuplicateKeyBecauseOfParents(parser, key, config, configWasInitiallyEmpty))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        LIBMATTI_NC_ValueResult value = LIBMATTI_NC_ValueParser_ParseValue(input, parser, config);
        if (!value.ok)
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        int hadPrevious =
            LIBMATTI_NC_Config_Contains(config, (const char **) key->items, key->count);
        LIBMATTI_NC_ParsingMode_Put(parser->parsingMode, config, (const char **) key->items, key->count, &value.value);
        if (!checkDuplicateKey(parser, hadPrevious, 1))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }
        LIBMATTI_NC_KeyList_Free(key);

        int after = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (after == '}') return 1;
        if (after != ',')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid entry separator in inline table.");
            return 0;
        }
        expectNextElement = 1;
    }
}

// Java: static <T extends CommentedConfig> T parseInlineV1_1(CharacterInput input, TomlParser parser, T config)
static int parseInlineV1_1(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *config)
{
    LIBMATTI_NC_TomlParser_RegisterInlineTable(parser, config);
    int configWasInitiallyEmpty = LIBMATTI_NC_Config_IsEmpty(config);
    int expectNextElement = 0;
    while (1)
    {
        int keyFirst = LIBMATTI_NC_Toml_ReadUseful(input, parser);
        if (keyFirst == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid inline table: missing closing brace }");
            return 0;
        }
        else if (keyFirst == '}') // handles {} and {k1=v1,... ,}
        {
            // Java accepts trailing commas (the flag is set but not checked here)
            (void) expectNextElement;
            return 1;
        }

        LIBMATTI_NC_KeyList *key = LIBMATTI_NC_TableParser_ParseDottedKey(input, (char) keyFirst, parser);
        if (key == NULL) return 0;
        if (!checkDuplicateKeyBecauseOfParents(parser, key, config, configWasInitiallyEmpty))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        LIBMATTI_NC_ValueResult value = LIBMATTI_NC_ValueParser_ParseValue(input, parser, config);
        if (!value.ok)
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        int hadPrevious =
            LIBMATTI_NC_Config_Contains(config, (const char **) key->items, key->count);
        LIBMATTI_NC_ParsingMode_Put(parser->parsingMode, config, (const char **) key->items, key->count, &value.value);
        if (!checkDuplicateKey(parser, hadPrevious, 1))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }
        LIBMATTI_NC_KeyList_Free(key);

        int after = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        int expectComma = 0;
        if (after == '}') // end of the inline config table
        {
            return 1;
        }
        else if (after == '#')
        {
            // Java keeps the comment; the port drops it (FML does not read comments)
            LIBMATTI_NC_Toml_ReadLine(input);
        }
        else if (after != ',')
        {
            if (after == '\n' || after == '\r')
            {
                expectComma = 1;
            }
            else
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid entry separator in inline table.");
                return 0;
            }
        }

        if (expectComma)
        {
            after = LIBMATTI_NC_Toml_ReadUsefulChar(input);
            if (after == '}') // end of the table
            {
                return 1;
            }
            else if (after != ',')
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid entry separator in inline table.");
                return 0;
            }
        }
        expectNextElement = 1;
    }
}

// Java: static <T extends CommentedConfig> T parseNormal(CharacterInput input, TomlParser parser, T config)
int LIBMATTI_NC_TableParser_ParseNormal(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                        LIBMATTI_NC_Config *config)
{
    int configWasInitiallyEmpty = LIBMATTI_NC_Config_IsEmpty(config);
    while (1)
    {
        int keyFirst = LIBMATTI_NC_Toml_ReadUseful(input, parser);
        if (keyFirst == -1 || keyFirst == '[')
        {
            // No more data, or beginning of an other table
            return 1;
        }

        LIBMATTI_NC_KeyList *key = LIBMATTI_NC_TableParser_ParseDottedKey(input, (char) keyFirst, parser);
        if (key == NULL) return 0;
        if (!checkDuplicateKeyBecauseOfParents(parser, key, config, configWasInitiallyEmpty))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        LIBMATTI_NC_ValueResult value = LIBMATTI_NC_ValueParser_ParseValue(input, parser, config);
        if (!value.ok)
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }

        int hadPrevious =
            LIBMATTI_NC_Config_Contains(config, (const char **) key->items, key->count);
        LIBMATTI_NC_ParsingMode_Put(parser->parsingMode, config, (const char **) key->items, key->count, &value.value);
        if (!checkDuplicateKey(parser, hadPrevious, LIBMATTI_NC_TomlParser_ConfigWasEmpty(parser)))
        {
            LIBMATTI_NC_KeyList_Free(key);
            return 0;
        }
        LIBMATTI_NC_KeyList_Free(key);

        int after = LIBMATTI_NC_Toml_ReadNonSpace(input, 0);
        if (after == -1) // End of the stream
        {
            return 1;
        }
        if (after == '#')
        {
            // Java keeps the comment; the port drops it (FML does not read comments)
            LIBMATTI_NC_Toml_ReadLine(input);
        }
        else if (after != '\n' && after != '\r')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid character after a table entry");
            return 0;
        }
    }
}

// Java: static CommentedConfig parseNormal(CommentedConfig parentConfig, CharacterInput input, TomlParser parser)
LIBMATTI_NC_Config *LIBMATTI_NC_TableParser_ParseNormalSubConfig(LIBMATTI_NC_Config *parentConfig,
                                                                LIBMATTI_NC_ReaderInput *input,
                                                                LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_Config *config = LIBMATTI_NC_Config_CreateSubConfig(parentConfig);
    return LIBMATTI_NC_TableParser_ParseNormal(input, parser, config) ? config : NULL;
}

// Java: static List<String> parseTableName(CharacterInput input, TomlParser parser, boolean array)
LIBMATTI_NC_KeyList *LIBMATTI_NC_TableParser_ParseTableName(LIBMATTI_NC_ReaderInput *input,
                                                           LIBMATTI_NC_TomlParser *parser, int array)
{
    LIBMATTI_NC_KeyList *list = LIBMATTI_NC_KeyList_New();
    while (1)
    {
        int firstChar = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (firstChar == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Tables names must not be empty.");
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }
        if (firstChar == ']')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Tables names must not be empty.");
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }

        char *key = LIBMATTI_NC_TableParser_ParseNonDottedKey(input, (char) firstChar, parser);
        if (key == NULL)
        {
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }
        LIBMATTI_NC_KeyList_Add(list, key);

        int separator = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (separator == ']') // End of the declaration
        {
            if (array)
            {
                int after = LIBMATTI_NC_ReaderInput_ReadChar(input);
                if (after != ']')
                {
                    LIBMATTI_NC_TomlParser_Error(parser,
                                                 "Invalid declaration of an element of an array of tables");
                    LIBMATTI_NC_KeyList_Free(list);
                    return NULL;
                }
            }

            int after = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
            if (after == '#') // Comment
            {
                // Java keeps the comment; the port drops it (FML does not read comments)
                LIBMATTI_NC_Toml_ReadLine(input);
            }
            else if (after != '\n' && after != '\r')
            {
                LIBMATTI_NC_TomlParser_Error(parser, "Invalid character after a table declaration.");
                LIBMATTI_NC_KeyList_Free(list);
                return NULL;
            }
            return list;
        }
        else if (separator != '.')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid separator in table name.");
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }
    }
}

// Java: static List<String> parseDottedKey(CharacterInput input, char firstChar, TomlParser parser)
LIBMATTI_NC_KeyList *LIBMATTI_NC_TableParser_ParseDottedKey(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                                           LIBMATTI_NC_TomlParser *parser)
{
    LIBMATTI_NC_KeyList *list = LIBMATTI_NC_KeyList_New();
    char first = firstChar;
    while (1)
    {
        char *part = LIBMATTI_NC_TableParser_ParseNonDottedKey(input, first, parser);
        if (part == NULL)
        {
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }
        LIBMATTI_NC_KeyList_Add(list, part);

        int sep = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (LIBMATTI_NC_Toml_IsKeyValueSeparator((char) sep, LIBMATTI_NC_TomlParser_IsLenientWithSeparators(parser)))
        {
            return list;
        }
        else if (sep != '.')
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid character after key");
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }

        int next = LIBMATTI_NC_Toml_ReadNonSpaceChar(input, 0);
        if (next == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid key, the value is missing");
            LIBMATTI_NC_KeyList_Free(list);
            return NULL;
        }
        first = (char) next;
    }
}

// Java: static String parseNonDottedKey(CharacterInput input, char firstChar, TomlParser parser)
char *LIBMATTI_NC_TableParser_ParseNonDottedKey(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                               LIBMATTI_NC_TomlParser *parser)
{
    // Note that a key can't be multiline
    // Empty keys are allowed if and only if they are quoted (with double or single quotes)
    if (firstChar == '"') return LIBMATTI_NC_StringParser_ParseBasic(input, parser);
    if (firstChar == '\'') return LIBMATTI_NC_StringParser_ParseLiteral(input, parser);

    LIBMATTI_NC_CharsWrapper restOfKey =
        LIBMATTI_NC_ReaderInput_ReadCharsUntil(input, KEY_END, sizeof(KEY_END) - 1);
    LIBMATTI_NC_CharsWrapper_Builder *builder =
        LIBMATTI_NC_CharsWrapper_Builder_New(restOfKey.length + 1);
    LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, firstChar);
    LIBMATTI_NC_CharsWrapper_Builder_Append(builder, &restOfKey);
    char *bareKey = LIBMATTI_NC_CharsWrapper_Builder_ToString(builder);
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);

    // Checks that the bare key is conform to the specification
    if (bareKey[0] == '\0')
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Empty bare keys aren't allowed.");
        free(bareKey);
        return NULL;
    }

    LIBMATTI_NC_CharsWrapper bareKeyWrapper = LIBMATTI_NC_CharsWrapper_Of(bareKey, strlen(bareKey));
    if (!LIBMATTI_NC_Toml_IsValidBareKey(&bareKeyWrapper, LIBMATTI_NC_TomlParser_IsLenientWithBareKeys(parser)))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid bare key");
        free(bareKey);
        return NULL;
    }
    return bareKey;
}
