#include "ThrowableProxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *dup_or_null(const char *value)
{
    return value != NULL ? strdup(value) : NULL;
}

LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_ThrowableProxy_New(const LIBMATTI_JL_Throwable *throwable)
{
    if (throwable == NULL) return NULL;

    LIBMATTI_L4J_ThrowableProxy *proxy = calloc(1, sizeof(LIBMATTI_L4J_ThrowableProxy));
    proxy->name = dup_or_null(LIBMATTI_JL_Throwable_GetName(throwable));
    proxy->message = dup_or_null(LIBMATTI_JL_Throwable_GetMessage(throwable));

    size_t count = 0;
    const LIBMATTI_JL_StackTraceElement *frames = LIBMATTI_JL_Throwable_GetStackTrace(throwable, &count);

    if (count > 0)
    {
        proxy->extendedStackTrace = calloc(count, sizeof(*proxy->extendedStackTrace));

        for (size_t i = 0; i < count; i++)
        {
            proxy->extendedStackTrace[i].element = frames[i];
            proxy->extendedStackTrace[i].exact = 1;
        }

        proxy->extendedStackTraceCount = count;
    }

    proxy->cause = LIBMATTI_L4J_ThrowableProxy_New(LIBMATTI_JL_Throwable_GetCause(throwable));

    size_t suppressedCount = 0;
    LIBMATTI_JL_Throwable **suppressed = LIBMATTI_JL_Throwable_GetSuppressed(throwable, &suppressedCount);

    if (suppressedCount > 0)
    {
        proxy->suppressed = calloc(suppressedCount, sizeof(*proxy->suppressed));

        for (size_t i = 0; i < suppressedCount; i++)
            proxy->suppressed[i] = LIBMATTI_L4J_ThrowableProxy_New(suppressed[i]);

        proxy->suppressedCount = suppressedCount;
    }

    return proxy;
}

void LIBMATTI_L4J_ThrowableProxy_Free(LIBMATTI_L4J_ThrowableProxy *proxy)
{
    if (proxy == NULL) return;

    free(proxy->name);
    free(proxy->message);

    for (size_t i = 0; i < proxy->extendedStackTraceCount; i++)
    {
        free(proxy->extendedStackTrace[i].classLoaderName);
        free(proxy->extendedStackTrace[i].moduleName);
        free(proxy->extendedStackTrace[i].version);
    }
    free(proxy->extendedStackTrace);

    for (size_t i = 0; i < proxy->suppressedCount; i++)
        LIBMATTI_L4J_ThrowableProxy_Free(proxy->suppressed[i]);
    free(proxy->suppressed);

    LIBMATTI_L4J_ThrowableProxy_Free(proxy->cause);
    free(proxy);
}

const char *LIBMATTI_L4J_ThrowableProxy_GetName(const LIBMATTI_L4J_ThrowableProxy *proxy)
{
    return proxy->name;
}

const char *LIBMATTI_L4J_ThrowableProxy_GetMessage(const LIBMATTI_L4J_ThrowableProxy *proxy)
{
    return proxy->message;
}

LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_ThrowableProxy_GetCause(const LIBMATTI_L4J_ThrowableProxy *proxy)
{
    return proxy->cause;
}

const LIBMATTI_L4J_ExtendedStackTraceElement *LIBMATTI_L4J_ThrowableProxy_GetExtendedStackTrace(
    const LIBMATTI_L4J_ThrowableProxy *proxy, size_t *count)
{
    *count = proxy->extendedStackTraceCount;
    return proxy->extendedStackTrace;
}

void LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(LIBMATTI_L4J_ThrowableProxy *proxy,
                                                            LIBMATTI_JL_StringBuilder *toAppendTo,
                                                            char **ignorePackages, size_t ignorePackageCount,
                                                            LIBMATTI_L4J_TextRenderer *textRenderer,
                                                            const char *suffix, const char *lineSeparator)
{
    (void)ignorePackages;
    (void)ignorePackageCount;

    const char *separator = lineSeparator != NULL ? lineSeparator : "\n";

    for (size_t i = 0; i < proxy->extendedStackTraceCount; i++)
    {
        LIBMATTI_L4J_ExtendedStackTraceElement *element = &proxy->extendedStackTrace[i];

        LIBMATTI_JL_StringBuilder_Append(toAppendTo, "\tat ");
        textRenderer->render(textRenderer, element->element.className, toAppendTo, "StackTraceElement.ClassName");
        LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, '.');
        textRenderer->render(textRenderer, element->element.methodName, toAppendTo, "StackTraceElement.MethodName");
        LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, '(');

        if (element->element.nativeMethod)
        {
            LIBMATTI_JL_StringBuilder_Append(toAppendTo, "Native Method");
        }
        else
        {
            LIBMATTI_JL_StringBuilder_Append(toAppendTo,
                                             element->element.fileName != NULL ? element->element.fileName
                                                                               : "Unknown Source");

            if (element->element.lineNumber >= 0)
            {
                char line[32];
                snprintf(line, sizeof(line), ":%d", element->element.lineNumber);
                LIBMATTI_JL_StringBuilder_Append(toAppendTo, line);
            }
        }

        LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, ')');

        if (element->moduleName != NULL)
        {
            LIBMATTI_JL_StringBuilder_Append(toAppendTo, " [");
            LIBMATTI_JL_StringBuilder_Append(toAppendTo, element->moduleName);

            if (element->version != NULL)
            {
                LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, '/');
                LIBMATTI_JL_StringBuilder_Append(toAppendTo, element->version);
            }

            if (!element->exact) LIBMATTI_JL_StringBuilder_Append(toAppendTo, ":?");
            LIBMATTI_JL_StringBuilder_AppendChar(toAppendTo, ']');
        }

        textRenderer->render(textRenderer, suffix != NULL ? suffix : "", toAppendTo, "Suffix");
        LIBMATTI_JL_StringBuilder_Append(toAppendTo, separator);
    }

    if (proxy->cause != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(toAppendTo, "Caused by: ");
        LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(proxy->cause, toAppendTo, ignorePackages,
                                                              ignorePackageCount, textRenderer, suffix, lineSeparator);
    }
}
