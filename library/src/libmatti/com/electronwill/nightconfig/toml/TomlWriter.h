// Port of com.electronwill.nightconfig.toml.TomlWriter.
// Java: public final class TomlWriter implements ConfigWriter - with the default settings
// (writeTableInlinePredicate = isEmpty, hideRedundantLevels = true, tab indent, system newline).
// write(config, file, WritingMode.REPLACE_ATOMIC) becomes a fopen/fclose pair; Java's Writer
// overload goes through LIBMATTI_NC_TomlWriter_WriteToString.

#ifndef MATTICRAFT_NC_TOML_TOMLWRITER_H
#define MATTICRAFT_NC_TOML_TOMLWRITER_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"

#include <stddef.h>

// Java: public final class TomlWriter
typedef struct LIBMATTI_NC_TomlWriter
{
    // Java: the writer's settings
    int lenientBareKeys;
    // Java: Predicate<UnmodifiableConfig> writeTableInlinePredicate = UnmodifiableConfig::isEmpty
    //       - the port keeps only the default (empty tables inline)
    // Java: writeStringLiteralPredicate = str -> false, multiline per default predicate, no array
    //       indentation - all defaults, no setters used by the callers
    int hideRedundantLevels;
    const char *indent;
    const char *newline;

    // Java: private int currentIndentLevel
    int currentIndentLevel;
} LIBMATTI_NC_TomlWriter;

// Java: public TomlWriter()
LIBMATTI_NC_TomlWriter *LIBMATTI_NC_TomlWriter_New(void);
void LIBMATTI_NC_TomlWriter_Free(LIBMATTI_NC_TomlWriter *writer);

// Java: public void write(UnmodifiableConfig config, Writer writer) - the caller frees the string
char *LIBMATTI_NC_TomlWriter_WriteToString(LIBMATTI_NC_TomlWriter *writer, const LIBMATTI_NC_Config *config);

// Java: static void writeConfig(Path file, UnmodifiableCommentedConfig config) {
//           new TomlWriter().write(config, file, WritingMode.REPLACE_ATOMIC) }
// Returns 0 on failure (Java throws a WritingException).
int LIBMATTI_NC_TomlWriter_WriteFile(LIBMATTI_NC_TomlWriter *writer, const LIBMATTI_NC_Config *config,
                                     const char *file);

#endif //MATTICRAFT_NC_TOML_TOMLWRITER_H
