// Port of com.electronwill.nightconfig.toml.TomlParser.
// Comments are dropped (FML never reads them); everything else follows the original control flow.

#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

#include "libmatti/com/electronwill/nightconfig/toml/TableParser.h"
#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value value)
{
    LIBMATTI_NC_ValueResult result;
    result.ok = 1;
    result.value = value;
    return result;
}

LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueResult_Fail(void)
{
    LIBMATTI_NC_ValueResult result = {0};
    return result;
}

// Java: public TomlParser()
LIBMATTI_NC_TomlParser *LIBMATTI_NC_TomlParser_New(void)
{
    LIBMATTI_NC_TomlParser *parser = calloc(1, sizeof(*parser));
    parser->initialStringBuilderCapacity = 16;
    parser->initialListCapacity = 10;
    parser->tomlVersion = LIBMATTI_NC_TOMLVERSION_V1_0;
    return parser;
}

void LIBMATTI_NC_TomlParser_Free(LIBMATTI_NC_TomlParser *parser)
{
    if (parser == NULL) return;
    free(parser->inlineTables);
    free(parser->error);
    free(parser->currentComment);
    free(parser);
}

// Java: throw new ParsingException(...) - the port keeps the first failure
void LIBMATTI_NC_TomlParser_Error(LIBMATTI_NC_TomlParser *parser, const char *message)
{
    if (parser->failed) return;
    parser->failed = 1;
    parser->error = strdup(message);
}

int LIBMATTI_NC_TomlParser_Failed(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->failed;
}

const char *LIBMATTI_NC_TomlParser_ErrorMessage(const LIBMATTI_NC_TomlParser *parser)
{
    return parser != NULL ? parser->error : NULL;
}

// Java: void registerInlineTable(Config table)
void LIBMATTI_NC_TomlParser_RegisterInlineTable(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *table)
{
    parser->inlineTables =
        realloc(parser->inlineTables, sizeof(*parser->inlineTables) * (parser->inlineTableCount + 1));
    parser->inlineTables[parser->inlineTableCount++] = table;
}

// Java: boolean isInlineTable(Config table)
int LIBMATTI_NC_TomlParser_IsInlineTable(const LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_Config *table)
{
    for (size_t i = 0; i < parser->inlineTableCount; i++)
        if (parser->inlineTables[i] == table) return 1;
    return 0;
}

// Java: private void clearParsingState() { inlineTables.clear(); }
static void clearParsingState(LIBMATTI_NC_TomlParser *parser)
{
    free(parser->inlineTables);
    parser->inlineTables = NULL;
    parser->inlineTableCount = 0;
}

int LIBMATTI_NC_TomlParser_IsLenientWithBareKeys(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->lenientBareKeys;
}

int LIBMATTI_NC_TomlParser_IsLenientWithSeparators(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->lenientSeparators;
}

LIBMATTI_NC_TomlVersion LIBMATTI_NC_TomlParser_GetTomlVersion(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->tomlVersion;
}

LIBMATTI_NC_ParsingMode LIBMATTI_NC_TomlParser_GetParsingMode(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->parsingMode;
}

int LIBMATTI_NC_TomlParser_ConfigWasEmpty(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->configWasEmpty;
}

// Java: <T> List<T> createList() - the port's lists are (pointer, count) pairs, so this is empty
LIBMATTI_NC_Value *LIBMATTI_NC_TomlParser_CreateList(size_t *count)
{
    *count = 0;
    return NULL;
}

// Java: CharsWrapper.Builder createBuilder()
LIBMATTI_NC_CharsWrapper_Builder *LIBMATTI_NC_TomlParser_CreateBuilder(LIBMATTI_NC_TomlParser *parser)
{
    return LIBMATTI_NC_CharsWrapper_Builder_New(parser->initialStringBuilderCapacity);
}

// Java: boolean hasPendingComment()
int LIBMATTI_NC_TomlParser_HasPendingComment(const LIBMATTI_NC_TomlParser *parser)
{
    return parser->currentComment != NULL;
}

// Java: String consumeComment()
char *LIBMATTI_NC_TomlParser_ConsumeComment(LIBMATTI_NC_TomlParser *parser)
{
    char *comment = parser->currentComment;
    parser->currentComment = NULL;
    return comment;
}

// Java: void setComment(CharsWrapper comment)
void LIBMATTI_NC_TomlParser_SetComment(LIBMATTI_NC_TomlParser *parser, const char *comment)
{
    if (comment == NULL) return;

    // control characters other than tab are not permitted in comments
    for (const char *c = comment; *c != '\0'; c++)
    {
        unsigned char value = (unsigned char) *c;
        if (value == '\t' || value == '\n') continue;
        if (value <= 0x1F || value == 0x7F)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid control character in comment");
            return;
        }
    }

    if (parser->currentComment == NULL)
    {
        parser->currentComment = strdup(comment);
    }
    else
    {
        size_t length = strlen(parser->currentComment) + 1 + strlen(comment) + 1;
        char *combined = malloc(length);
        snprintf(combined, length, "%s\n%s", parser->currentComment, comment);
        free(parser->currentComment);
        parser->currentComment = combined;
    }
}

// Java: void setComment(List<CharsWrapper> commentsList)
void LIBMATTI_NC_TomlParser_SetComments(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_CharsWrapper *comments,
                                        size_t commentCount)
{
    if (commentCount == 0) return;

    LIBMATTI_NC_CharsWrapper_Builder *builder = LIBMATTI_NC_CharsWrapper_Builder_New(32);
    for (size_t i = 0; i < commentCount; i++)
    {
        if (i > 0) LIBMATTI_NC_CharsWrapper_Builder_AppendChar(builder, '\n');
        LIBMATTI_NC_CharsWrapper_Builder_Append(builder, &comments[i]);
    }
    char *comment = LIBMATTI_NC_CharsWrapper_Builder_ToString(builder);
    LIBMATTI_NC_CharsWrapper_Builder_Free(builder);
    LIBMATTI_NC_TomlParser_SetComment(parser, comment);
    free(comment);
}

// Java: private Config getSubTable(Config parentTable, List<String> path)
static LIBMATTI_NC_Config *getSubTable(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *parentTable, char **path,
                                       size_t pathLength)
{
    if (pathLength == 0) return parentTable;

    LIBMATTI_NC_Config *currentConfig = parentTable;
    for (size_t i = 0; i < pathLength; i++)
    {
        const char *singleKey[1] = {path[i]};
        LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_Get(currentConfig, singleKey, 1);
        if (value == NULL)
        {
            LIBMATTI_NC_Config *sub = LIBMATTI_NC_Config_CreateSubConfig(parentTable);
            LIBMATTI_NC_Value subValue = LIBMATTI_NC_Value_OfConfig(sub);
            LIBMATTI_NC_Config_Set(currentConfig, singleKey, 1, &subValue);
            currentConfig = sub;
        }
        else if (value->type == LIBMATTI_NC_VALUE_CONFIG)
        {
            currentConfig = value->config;
        }
        else if (value->type == LIBMATTI_NC_VALUE_LIST)
        {
            // Arrays of tables resolve to their last element
            int allConfigs = value->listCount > 0;
            for (size_t j = 0; j < value->listCount; j++)
                if (value->list[j].type != LIBMATTI_NC_VALUE_CONFIG)
                {
                    allConfigs = 0;
                    break;
                }
            if (!allConfigs) return NULL;
            currentConfig = value->list[value->listCount - 1].config;
        }
        else
        {
            return NULL;
        }

        if (LIBMATTI_NC_TomlParser_IsInlineTable(parser, currentConfig))
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Cannot modify an inline table after its creation.");
            return NULL;
        }
    }
    return currentConfig;
}

// Java: private void checkContainsOnlySubtables(Config table, List<String> path) - 0 means the ParsingException was thrown
static int checkContainsOnlySubtables(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_Config *table)
{
    for (size_t i = 0; i < table->count; i++)
    {
        const LIBMATTI_NC_Value *value = &table->values[i];
        int onlySubtables =
            value->type == LIBMATTI_NC_VALUE_CONFIG ||
            (value->type == LIBMATTI_NC_VALUE_LIST && value->listCount > 0 &&
             value->list[0].type == LIBMATTI_NC_VALUE_CONFIG);
        if (!onlySubtables)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Table has been declared twice.");
            return 0;
        }
    }
    return 1;
}

// Java: private <T extends Config> T parse(CharacterInput input, T destination, ParsingMode parsingMode)
static LIBMATTI_NC_Config *parse(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_ReaderInput *input,
                                 LIBMATTI_NC_Config *destination, LIBMATTI_NC_ParsingMode parsingMode)
{
    parser->parsingMode = parsingMode;
    LIBMATTI_NC_ParsingMode_PrepareParsing(parsingMode, destination);
    // Java: CommentedConfig commentedConfig = CommentedConfig.fake(destination) - the same map
    LIBMATTI_NC_Config *rootTable = destination;

    if (!LIBMATTI_NC_TableParser_ParseNormal(input, parser, rootTable))
    {
        clearParsingState(parser);
        return NULL;
    }

    int next;
    while ((next = LIBMATTI_NC_ReaderInput_Peek(input)) != -1)
    {
        const int isArray = (next == '[');
        if (isArray)
        {
            LIBMATTI_NC_ReaderInput_SkipPeeks(input);
        }

        LIBMATTI_NC_KeyList *path = LIBMATTI_NC_TableParser_ParseTableName(input, parser, isArray);
        if (path == NULL)
        {
            clearParsingState(parser);
            return NULL;
        }

        const size_t lastIndex = path->count - 1;
        LIBMATTI_NC_Config *parentConfig = getSubTable(parser, rootTable, path->items, lastIndex);
        const char *lastKey[1] = {path->items[lastIndex]}; // Java: List<String> lastPath

        if (isArray) // It's an element of an array of tables
        {
            if (parentConfig == NULL)
            {
                LIBMATTI_NC_TomlParser_Error(
                    parser, "Cannot create entry because of an invalid parent that isn't a table.");
                LIBMATTI_NC_KeyList_Free(path);
                clearParsingState(parser);
                return NULL;
            }

            LIBMATTI_NC_Config *table = LIBMATTI_NC_Config_CreateSubConfig(parentConfig);
            if (!LIBMATTI_NC_TableParser_ParseNormal(input, parser, table))
            {
                LIBMATTI_NC_Config_Free(table);
                LIBMATTI_NC_KeyList_Free(path);
                clearParsingState(parser);
                return NULL;
            }

            LIBMATTI_NC_Value *shouldBeArrayOfTables = LIBMATTI_NC_Config_Get(parentConfig, lastKey, 1);
            if (shouldBeArrayOfTables != NULL && shouldBeArrayOfTables->type == LIBMATTI_NC_VALUE_LIST)
            {
                LIBMATTI_NC_Value_ListAppend(shouldBeArrayOfTables, LIBMATTI_NC_Value_OfConfig(table));
            }
            else if (shouldBeArrayOfTables == NULL)
            {
                // fill the array of tables before put because put could convert the value
                LIBMATTI_NC_Value *arrayOfTables = malloc(sizeof(LIBMATTI_NC_Value));
                arrayOfTables[0] = LIBMATTI_NC_Value_OfConfig(table);
                LIBMATTI_NC_Value arrayValue = LIBMATTI_NC_Value_OfList(arrayOfTables, 1);
                LIBMATTI_NC_ParsingMode_Put(parser->parsingMode, parentConfig, lastKey, 1, &arrayValue);
            }
            else
            {
                LIBMATTI_NC_TomlParser_Error(
                    parser, "Cannot create entry because of an invalid parent that is not an array of tables");
                LIBMATTI_NC_Config_Free(table);
                LIBMATTI_NC_KeyList_Free(path);
                clearParsingState(parser);
                return NULL;
            }
        }
        else // It's a table
        {
            if (parentConfig == NULL)
            {
                LIBMATTI_NC_TomlParser_Error(
                    parser, "Cannot create entry because of an invalid parent that isn't a table.");
                LIBMATTI_NC_KeyList_Free(path);
                clearParsingState(parser);
                return NULL;
            }

            LIBMATTI_NC_Value *alreadyDeclared = LIBMATTI_NC_Config_Get(parentConfig, lastKey, 1);
            if (alreadyDeclared == NULL)
            {
                LIBMATTI_NC_Config *table = LIBMATTI_NC_Config_CreateSubConfig(parentConfig);
                if (!LIBMATTI_NC_TableParser_ParseNormal(input, parser, table))
                {
                    LIBMATTI_NC_Config_Free(table);
                    LIBMATTI_NC_KeyList_Free(path);
                    clearParsingState(parser);
                    return NULL;
                }
                LIBMATTI_NC_Value tableValue = LIBMATTI_NC_Value_OfConfig(table);
                LIBMATTI_NC_ParsingMode_Put(parser->parsingMode, parentConfig, lastKey, 1, &tableValue);
            }
            else
            {
                if (alreadyDeclared->type == LIBMATTI_NC_VALUE_CONFIG)
                {
                    // check that there is no conflict with the existing declaration
                    LIBMATTI_NC_Config *table = alreadyDeclared->config;
                    if (!checkContainsOnlySubtables(parser, table))
                    {
                        LIBMATTI_NC_KeyList_Free(path);
                        clearParsingState(parser);
                        return NULL;
                    }
                    if (!LIBMATTI_NC_TableParser_ParseNormal(input, parser, table))
                    {
                        LIBMATTI_NC_KeyList_Free(path);
                        clearParsingState(parser);
                        return NULL;
                    }
                }
                else if (parser->configWasEmpty)
                {
                    LIBMATTI_NC_TomlParser_Error(parser, "Entry has been defined twice.");
                    LIBMATTI_NC_KeyList_Free(path);
                    clearParsingState(parser);
                    return NULL;
                }
            }
        }

        LIBMATTI_NC_KeyList_Free(path);
    }

    clearParsingState(parser);
    return destination;
}

// Java: public CommentedConfig parse(Reader reader)
LIBMATTI_NC_Config *LIBMATTI_NC_TomlParser_Parse(LIBMATTI_NC_TomlParser *parser, const char *data, size_t length)
{
    parser->failed = 0;
    free(parser->error);
    parser->error = NULL;
    parser->configWasEmpty = 1;

    LIBMATTI_NC_ReaderInput *input = LIBMATTI_NC_ReaderInput_New(data, length);
    LIBMATTI_NC_Config *destination = LIBMATTI_NC_Config_New();
    LIBMATTI_NC_Config *result = parse(parser, input, destination, LIBMATTI_NC_PARSINGMODE_MERGE);
    LIBMATTI_NC_ReaderInput_Free(input);

    // Java: any ParsingException aborts the parse, so a recorded failure rejects the document
    if (result == NULL || parser->failed)
    {
        LIBMATTI_NC_Config_Free(destination);
        return NULL;
    }
    return destination;
}
