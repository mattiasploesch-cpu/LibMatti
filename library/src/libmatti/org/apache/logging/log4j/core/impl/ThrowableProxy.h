#ifndef MATTICRAFT_L4J_THROWABLEPROXY_H
#define MATTICRAFT_L4J_THROWABLEPROXY_H

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/org/apache/logging/log4j/core/pattern/TextRenderer.h"

#include <stddef.h>

typedef struct
{
    LIBMATTI_JL_StackTraceElement element;
    char *classLoaderName;
    char *moduleName;
    char *version;
    int exact;
} LIBMATTI_L4J_ExtendedStackTraceElement;

typedef struct LIBMATTI_L4J_ThrowableProxy LIBMATTI_L4J_ThrowableProxy;

struct LIBMATTI_L4J_ThrowableProxy
{
    char *name;
    char *message;
    LIBMATTI_L4J_ExtendedStackTraceElement *extendedStackTrace;
    size_t extendedStackTraceCount;
    LIBMATTI_L4J_ThrowableProxy *cause;
    LIBMATTI_L4J_ThrowableProxy **suppressed;
    size_t suppressedCount;
};

LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_ThrowableProxy_New(const LIBMATTI_JL_Throwable *throwable);
void LIBMATTI_L4J_ThrowableProxy_Free(LIBMATTI_L4J_ThrowableProxy *proxy);

const char *LIBMATTI_L4J_ThrowableProxy_GetName(const LIBMATTI_L4J_ThrowableProxy *proxy);
const char *LIBMATTI_L4J_ThrowableProxy_GetMessage(const LIBMATTI_L4J_ThrowableProxy *proxy);
LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_ThrowableProxy_GetCause(const LIBMATTI_L4J_ThrowableProxy *proxy);
const LIBMATTI_L4J_ExtendedStackTraceElement *LIBMATTI_L4J_ThrowableProxy_GetExtendedStackTrace(
    const LIBMATTI_L4J_ThrowableProxy *proxy, size_t *count);

void LIBMATTI_L4J_ThrowableProxy_FormatExtendedStackTraceTo(LIBMATTI_L4J_ThrowableProxy *proxy,
                                                            LIBMATTI_JL_StringBuilder *toAppendTo,
                                                            char **ignorePackages, size_t ignorePackageCount,
                                                            LIBMATTI_L4J_TextRenderer *textRenderer,
                                                            const char *suffix, const char *lineSeparator);

#endif //MATTICRAFT_L4J_THROWABLEPROXY_H
