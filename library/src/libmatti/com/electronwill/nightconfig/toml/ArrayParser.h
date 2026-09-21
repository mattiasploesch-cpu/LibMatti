// Port of com.electronwill.nightconfig.toml.ArrayParser.
// Java throws a ParsingException; the port reports the failure in the LIBMATTI_NC_ValueResult.

#ifndef MATTICRAFT_NC_TOML_ARRAYPARSER_H
#define MATTICRAFT_NC_TOML_ARRAYPARSER_H

#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

// Java: static List<?> parse(CharacterInput input, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ArrayParser_Parse(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                                      LIBMATTI_NC_Config *parentConfig);

#endif //MATTICRAFT_NC_TOML_ARRAYPARSER_H
