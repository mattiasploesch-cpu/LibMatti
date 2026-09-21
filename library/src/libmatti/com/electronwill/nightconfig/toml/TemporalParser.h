// Port of com.electronwill.nightconfig.toml.TemporalParser.
// Java: static Temporal parse(CharsWrapper chars, TomlVersion version) - the port reports
// failures through the TomlParser error channel instead of throwing a ParsingException.

#ifndef MATTICRAFT_NC_TOML_TEMPORALPARSER_H
#define MATTICRAFT_NC_TOML_TEMPORALPARSER_H

#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"
#include "libmatti/java/time/Temporal.h"

// Java: static Temporal parse(CharsWrapper chars, TomlVersion version)
// 'chars' must be the raw value; the function trims it itself, like Java.
// Returns 0 on failure and records the ParsingException in the parser.
int LIBMATTI_NC_TemporalParser_Parse(const LIBMATTI_NC_CharsWrapper *chars, LIBMATTI_NC_TomlVersion version,
                                     LIBMATTI_NC_TomlParser *parser, LIBMATTI_JT_Temporal *out);

#endif //MATTICRAFT_NC_TOML_TEMPORALPARSER_H
