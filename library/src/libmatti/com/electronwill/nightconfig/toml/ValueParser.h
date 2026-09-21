// Port of com.electronwill.nightconfig.toml.ValueParser.
// Java throws a ParsingException and returns Object; the port returns a LIBMATTI_NC_ValueResult
// whose ok flag reports the failure instead.

#ifndef MATTICRAFT_NC_TOML_VALUEPARSER_H
#define MATTICRAFT_NC_TOML_VALUEPARSER_H

#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

// Java: static Object parse(CharacterInput input, char firstChar, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueParser_Parse(LIBMATTI_NC_ReaderInput *input, char firstChar,
                                                      LIBMATTI_NC_TomlParser *parser,
                                                      LIBMATTI_NC_Config *parentConfig);
// Java: static Object parse(CharacterInput input, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ValueParser_ParseValue(LIBMATTI_NC_ReaderInput *input,
                                                           LIBMATTI_NC_TomlParser *parser,
                                                           LIBMATTI_NC_Config *parentConfig);

#endif //MATTICRAFT_NC_TOML_VALUEPARSER_H
