// Port of com.electronwill.nightconfig.toml.StringParser.
// Java throws a ParsingException; the port returns NULL and records the failure in the parser.

#ifndef MATTICRAFT_NC_TOML_STRINGPARSER_H
#define MATTICRAFT_NC_TOML_STRINGPARSER_H

#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

// Java: static String parseBasic(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseBasic(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser);
// Java: static String parseLiteral(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseLiteral(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser);
// Java: static String parseMultiBasic(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseMultiBasic(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser);
// Java: static String parseMultiLiteral(CharacterInput input, TomlParser parser)
char *LIBMATTI_NC_StringParser_ParseMultiLiteral(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser);

#endif //MATTICRAFT_NC_TOML_STRINGPARSER_H
