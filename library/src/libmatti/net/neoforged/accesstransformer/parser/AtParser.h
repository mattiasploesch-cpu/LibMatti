// Port of net.neoforged.accesstransformer.parser.AtParser.
// Parses accesstransformer.cfg lines into (Target, Transformation) pairs.

#ifndef MATTICRAFT_AT_ATPARSER_H
#define MATTICRAFT_AT_ATPARSER_H

#include "libmatti/net/neoforged/accesstransformer/parser/Target.h"

#include <stddef.h>

// Java: public record AtEntry(Target target, Transformation transformation)
typedef struct
{
    LIBMATTI_AT_Target *target;
    LIBMATTI_AT_Transformation *transformation;
} LIBMATTI_AT_AtEntry;

// Java: public static void parse(Reader wrappedReader, String originName, BiConsumer<Target, Transformation> consumer)
// The port returns the parsed entries instead (caller frees each entry and the
// array). Returns NULL on a parse error; *errorLine gets the 1-based line.
LIBMATTI_AT_AtEntry **LIBMATTI_AT_AtParser_Parse(const char *text, const char *originName, size_t *count,
                                                 int *errorLine);

void LIBMATTI_AT_AtParser_FreeEntries(LIBMATTI_AT_AtEntry **entries, size_t count);

#endif //MATTICRAFT_AT_ATPARSER_H
