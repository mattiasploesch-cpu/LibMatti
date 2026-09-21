#ifndef MATTICRAFT_L4J_THROWABLEPATTERNCONVERTER_H
#define MATTICRAFT_L4J_THROWABLEPATTERNCONVERTER_H

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/org/apache/logging/log4j/core/LogEvent.h"
#include "libmatti/org/apache/logging/log4j/core/config/Configuration.h"
#include "libmatti/org/apache/logging/log4j/core/pattern/TextRenderer.h"

#include <stddef.h>

typedef struct
{
    LIBMATTI_L4J_TextRenderer *textRenderer;
    char **ignorePackages;
    size_t ignorePackageCount;
    const char *separator;
    int anyLines;
} LIBMATTI_L4J_Options;

typedef struct LIBMATTI_L4J_ThrowablePatternConverter LIBMATTI_L4J_ThrowablePatternConverter;

struct LIBMATTI_L4J_ThrowablePatternConverter
{
    const char *name;
    const char *style;
    LIBMATTI_L4J_Options options;
    LIBMATTI_L4J_Configuration *config;
    void (*formatThrowable)(LIBMATTI_L4J_ThrowablePatternConverter *self, LIBMATTI_L4J_LogEvent *event,
                            LIBMATTI_JL_StringBuilder *toAppendTo);
    void (*format)(LIBMATTI_L4J_ThrowablePatternConverter *self, LIBMATTI_L4J_LogEvent *event,
                   LIBMATTI_JL_StringBuilder *toAppendTo);
};

LIBMATTI_L4J_ThrowablePatternConverter *LIBMATTI_L4J_ThrowablePatternConverter_New(
    const char *name, const char *style, const LIBMATTI_L4J_Options *options, LIBMATTI_L4J_Configuration *config);
void LIBMATTI_L4J_ThrowablePatternConverter_Free(LIBMATTI_L4J_ThrowablePatternConverter *converter);

void LIBMATTI_L4J_ThrowablePatternConverter_Format(LIBMATTI_L4J_ThrowablePatternConverter *converter,
                                                   LIBMATTI_L4J_LogEvent *event, LIBMATTI_JL_StringBuilder *toAppendTo);
const LIBMATTI_L4J_Options *LIBMATTI_L4J_ThrowablePatternConverter_GetOptions(
    const LIBMATTI_L4J_ThrowablePatternConverter *converter);

int LIBMATTI_L4J_Options_AnyLines(const LIBMATTI_L4J_Options *options);
LIBMATTI_L4J_TextRenderer *LIBMATTI_L4J_Options_GetTextRenderer(const LIBMATTI_L4J_Options *options);
char **LIBMATTI_L4J_Options_GetIgnorePackages(const LIBMATTI_L4J_Options *options, size_t *count);
const char *LIBMATTI_L4J_Options_GetSeparator(const LIBMATTI_L4J_Options *options);

#endif //MATTICRAFT_L4J_THROWABLEPATTERNCONVERTER_H
