#include "libmatti/cpw/modlauncher/log/TransformingThrowablePatternConverter.h"

#include "libmatti/cpw/modlauncher/log/ExtraDataTextRenderer.h"
#include "libmatti/joptsimple/internal/Strings.h"
#include "libmatti/org/apache/logging/log4j/core/impl/ThrowableProxy.h"
#include "libmatti/org/apache/logging/log4j/core/pattern/PlainTextRenderer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void transforming_format(LIBMATTI_L4J_ThrowablePatternConverter *self, LIBMATTI_L4J_LogEvent *event,
                                LIBMATTI_JL_StringBuilder *toAppendTo)
{
    LIBMATTI_L4J_ThrowableProxy *proxy = LIBMATTI_L4J_LogEvent_GetThrownProxy(event);
    LIBMATTI_JL_Throwable *throwable = LIBMATTI_L4J_LogEvent_GetThrown(event);

    if ((throwable == NULL && proxy == NULL) || !LIBMATTI_L4J_Options_AnyLines(&self->options)) return;

    if (proxy == NULL)
    {
        self->formatThrowable(self, event, toAppendTo);
        return;
    }

    size_t length = LIBMATTI_JL_StringBuilder_Length(toAppendTo);

    if (length > 0)
    {
        char last = LIBMATTI_JL_StringBuilder_CharAt(toAppendTo, length - 1);
        if (!isspace((unsigned char)last)) LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, ' ');
    }

    LIBMATTI_ML_ExtraDataTextRenderer *textRenderer =
        LIBMATTI_ML_ExtraDataTextRenderer_New(LIBMATTI_L4J_Options_GetTextRenderer(&self->options));

    size_t ignorePackageCount = 0;
    char **ignorePackages = LIBMATTI_L4J_Options_GetIgnorePackages(&self->options, &ignorePackageCount);

    LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(
        proxy, toAppendTo, ignorePackages, ignorePackageCount, &textRenderer->base,
        LIBMATTI_ML_TransformingThrowablePatternConverter_SUFFIXFLAG,
        LIBMATTI_L4J_Options_GetSeparator(&self->options));

    LIBMATTI_ML_ExtraDataTextRenderer_Free(textRenderer);
}

LIBMATTI_ML_TransformingThrowablePatternConverter *LIBMATTI_ML_TransformingThrowablePatternConverter_New(
    LIBMATTI_L4J_Configuration *config, const LIBMATTI_L4J_Options *options)
{
    LIBMATTI_ML_TransformingThrowablePatternConverter *converter =
        calloc(1, sizeof(LIBMATTI_ML_TransformingThrowablePatternConverter));

    LIBMATTI_L4J_ThrowablePatternConverter *base =
        LIBMATTI_L4J_ThrowablePatternConverter_New("TransformingThrowable", "throwable", options, config);
    converter->base = *base;
    free(base);

    converter->base.format = transforming_format;
    return converter;
}

void LIBMATTI_ML_TransformingThrowablePatternConverter_Free(
    LIBMATTI_ML_TransformingThrowablePatternConverter *converter)
{
    free(converter);
}

LIBMATTI_ML_TransformingThrowablePatternConverter *LIBMATTI_ML_TransformingThrowablePatternConverter_NewInstance(
    LIBMATTI_L4J_Configuration *config, const LIBMATTI_L4J_Options *options)
{
    return LIBMATTI_ML_TransformingThrowablePatternConverter_New(config, options);
}

char *LIBMATTI_ML_TransformingThrowablePatternConverter_GenerateEnhancedStackTrace(
    const LIBMATTI_JL_Throwable *throwable)
{
    LIBMATTI_L4J_ThrowableProxy *proxy = LIBMATTI_L4J_ThrowableProxy_New(throwable);
    LIBMATTI_JL_StringBuilder *buffer = LIBMATTI_JL_StringBuilder_New();
    LIBMATTI_ML_ExtraDataTextRenderer *textRenderer =
        LIBMATTI_ML_ExtraDataTextRenderer_New(LIBMATTI_L4J_PlainTextRenderer_GetInstance());

    LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(
        proxy, buffer, NULL, 0, &textRenderer->base,
        LIBMATTI_ML_TransformingThrowablePatternConverter_SUFFIXFLAG, LIBMATTI_JOPT_Strings_LINE_SEPARATOR);

    char *result = strdup(LIBMATTI_JL_StringBuilder_ToString(buffer));

    LIBMATTI_ML_ExtraDataTextRenderer_Free(textRenderer);
    LIBMATTI_JL_StringBuilder_Free(buffer);
    LIBMATTI_L4J_ThrowableProxy_Free(proxy);

    return result;
}
