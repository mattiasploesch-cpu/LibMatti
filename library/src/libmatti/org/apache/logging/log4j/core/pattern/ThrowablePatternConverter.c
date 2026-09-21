#include "ThrowablePatternConverter.h"

#include "libmatti/org/apache/logging/log4j/core/impl/ThrowableProxy.h"

#include <stdlib.h>

static void throwable_format(LIBMATTI_L4J_ThrowablePatternConverter *self, LIBMATTI_L4J_LogEvent *event,
                             LIBMATTI_JL_StringBuilder *toAppendTo)
{
    LIBMATTI_L4J_ThrowableProxy *proxy = LIBMATTI_L4J_LogEvent_GetThrownProxy(event);
    if (proxy == NULL) return;

    const char *name = LIBMATTI_L4J_ThrowableProxy_GetName(proxy);
    const char *message = LIBMATTI_L4J_ThrowableProxy_GetMessage(proxy);

    LIBMATTI_JL_StringBuilder_Append(toAppendTo, name != NULL ? name : "");
    if (message != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(toAppendTo, ": ");
        LIBMATTI_JL_StringBuilder_Append(toAppendTo, message);
    }
    LIBMATTI_JL_StringBuilder_Append(toAppendTo, self->options.separator != NULL ? self->options.separator : "\n");

    LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(proxy, toAppendTo, self->options.ignorePackages,
                                                           self->options.ignorePackageCount,
                                                           self->options.textRenderer, "",
                                                           self->options.separator);
}

LIBMATTI_L4J_ThrowablePatternConverter *LIBMATTI_L4J_ThrowablePatternConverter_New(
    const char *name, const char *style, const LIBMATTI_L4J_Options *options, LIBMATTI_L4J_Configuration *config)
{
    LIBMATTI_L4J_ThrowablePatternConverter *converter = calloc(1, sizeof(LIBMATTI_L4J_ThrowablePatternConverter));
    converter->name = name;
    converter->style = style;
    converter->config = config;
    converter->formatThrowable = throwable_format;
    converter->format = throwable_format;

    if (options != NULL) converter->options = *options;

    return converter;
}

void LIBMATTI_L4J_ThrowablePatternConverter_Free(LIBMATTI_L4J_ThrowablePatternConverter *converter)
{
    free(converter);
}

void LIBMATTI_L4J_ThrowablePatternConverter_Format(LIBMATTI_L4J_ThrowablePatternConverter *converter,
                                                  LIBMATTI_L4J_LogEvent *event,
                                                  LIBMATTI_JL_StringBuilder *toAppendTo)
{
    converter->format(converter, event, toAppendTo);
}

const LIBMATTI_L4J_Options *LIBMATTI_L4J_ThrowablePatternConverter_GetOptions(
    const LIBMATTI_L4J_ThrowablePatternConverter *converter)
{
    return &converter->options;
}

int LIBMATTI_L4J_Options_AnyLines(const LIBMATTI_L4J_Options *options)
{
    if (options == NULL) return 0;
    return options->anyLines;
}

LIBMATTI_L4J_TextRenderer *LIBMATTI_L4J_Options_GetTextRenderer(const LIBMATTI_L4J_Options *options)
{
    if (options == NULL) return NULL;
    return options->textRenderer;
}

char **LIBMATTI_L4J_Options_GetIgnorePackages(const LIBMATTI_L4J_Options *options, size_t *count)
{
    if (options == NULL)
    {
        *count = 0;
        return NULL;
    }

    *count = options->ignorePackageCount;
    return options->ignorePackages;
}

const char *LIBMATTI_L4J_Options_GetSeparator(const LIBMATTI_L4J_Options *options)
{
    if (options == NULL) return NULL;
    return options->separator;
}
