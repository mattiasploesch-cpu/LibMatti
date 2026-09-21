// Port of com.electronwill.nightconfig.toml.TomlParser.
// Java throws a ParsingException on invalid data; the port records the first failure in the parser
// (LIBMATTI_NC_TomlParser_Failed) and returns NULL, because C has no exception channel.

#ifndef MATTICRAFT_NC_TOML_TOMLPARSER_H
#define MATTICRAFT_NC_TOML_TOMLPARSER_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"
#include "libmatti/com/electronwill/nightconfig/core/io/CharacterInput.h"
#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"
#include "libmatti/com/electronwill/nightconfig/core/io/ParsingMode.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlVersion.h"

#include <stddef.h>

// Java: the parsers hand around the Object of a value; the port adds the failure flag
typedef struct
{
    int ok;
    LIBMATTI_NC_Value value;
} LIBMATTI_NC_ValueResult;

// Java: public final class TomlParser implements ConfigParser<CommentedConfig>
typedef struct LIBMATTI_NC_TomlParser
{
    // Java: the parser's settings
    int initialStringBuilderCapacity;
    int initialListCapacity;
    int lenientBareKeys;
    int lenientSeparators;
    int configWasEmpty;
    LIBMATTI_NC_ParsingMode parsingMode;
    LIBMATTI_NC_TomlVersion tomlVersion;

    // Java: private final Set<Config> inlineTables - an identity set
    LIBMATTI_NC_Config **inlineTables;
    size_t inlineTableCount;

    // Java: private String currentComment
    char *currentComment;

    // Java: the ParsingException the parser would have thrown
    int failed;
    char *error;
} LIBMATTI_NC_TomlParser;

// the Ok(value) / failed result of the parsers
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueResult_Ok(LIBMATTI_NC_Value value);
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueResult_Fail(void);

// Java: public TomlParser()
LIBMATTI_NC_TomlParser *LIBMATTI_NC_TomlParser_New(void);
void LIBMATTI_NC_TomlParser_Free(LIBMATTI_NC_TomlParser *parser);

// Java: throw new ParsingException(...)
void LIBMATTI_NC_TomlParser_Error(LIBMATTI_NC_TomlParser *parser, const char *message);
int LIBMATTI_NC_TomlParser_Failed(const LIBMATTI_NC_TomlParser *parser);
const char *LIBMATTI_NC_TomlParser_ErrorMessage(const LIBMATTI_NC_TomlParser *parser);

// Java: void registerInlineTable(Config table) / boolean isInlineTable(Config table)
void LIBMATTI_NC_TomlParser_RegisterInlineTable(LIBMATTI_NC_TomlParser *parser, LIBMATTI_NC_Config *table);
int LIBMATTI_NC_TomlParser_IsInlineTable(const LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_Config *table);

// Java: the getters/setters of the parser's settings
int LIBMATTI_NC_TomlParser_IsLenientWithBareKeys(const LIBMATTI_NC_TomlParser *parser);
int LIBMATTI_NC_TomlParser_IsLenientWithSeparators(const LIBMATTI_NC_TomlParser *parser);
LIBMATTI_NC_TomlVersion LIBMATTI_NC_TomlParser_GetTomlVersion(const LIBMATTI_NC_TomlParser *parser);
LIBMATTI_NC_ParsingMode LIBMATTI_NC_TomlParser_GetParsingMode(const LIBMATTI_NC_TomlParser *parser);
int LIBMATTI_NC_TomlParser_ConfigWasEmpty(const LIBMATTI_NC_TomlParser *parser);

// Java: <T> List<T> createList()
LIBMATTI_NC_Value *LIBMATTI_NC_TomlParser_CreateList(size_t *count);
// Java: CharsWrapper.Builder createBuilder()
LIBMATTI_NC_CharsWrapper_Builder *LIBMATTI_NC_TomlParser_CreateBuilder(LIBMATTI_NC_TomlParser *parser);

// Java: private String currentComment / hasPendingComment() / consumeComment() / setComment(...)
int LIBMATTI_NC_TomlParser_HasPendingComment(const LIBMATTI_NC_TomlParser *parser);
char *LIBMATTI_NC_TomlParser_ConsumeComment(LIBMATTI_NC_TomlParser *parser);
void LIBMATTI_NC_TomlParser_SetComment(LIBMATTI_NC_TomlParser *parser, const char *comment);
void LIBMATTI_NC_TomlParser_SetComments(LIBMATTI_NC_TomlParser *parser, const LIBMATTI_NC_CharsWrapper *comments,
                                        size_t commentCount);

// Java: public CommentedConfig parse(Reader reader) - NULL when the data is invalid
LIBMATTI_NC_Config *LIBMATTI_NC_TomlParser_Parse(LIBMATTI_NC_TomlParser *parser, const char *data, size_t length);

#endif //MATTICRAFT_NC_TOML_TOMLPARSER_H
