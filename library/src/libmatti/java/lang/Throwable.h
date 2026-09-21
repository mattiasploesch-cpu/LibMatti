#ifndef MATTICRAFT_JAVA_LANG_THROWABLE_H
#define MATTICRAFT_JAVA_LANG_THROWABLE_H

#include "libmatti/java/io/PrintWriter.h"

#include <stddef.h>

typedef struct
{
    char *className;
    char *methodName;
    char *fileName;
    int lineNumber;
    int nativeMethod;
} LIBMATTI_JL_StackTraceElement;

typedef struct LIBMATTI_JL_Throwable LIBMATTI_JL_Throwable;

struct LIBMATTI_JL_Throwable
{
    char *className;
    char *message;
    LIBMATTI_JL_StackTraceElement *stackTrace;
    size_t stackTraceCount;
    LIBMATTI_JL_Throwable *cause;
    LIBMATTI_JL_Throwable **suppressed;
    size_t suppressedCount;
};

LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_New(const char *message);
LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_NewNamed(const char *className, const char *message);
void LIBMATTI_JL_Throwable_Free(LIBMATTI_JL_Throwable *throwable);

const char *LIBMATTI_JL_Throwable_GetName(const LIBMATTI_JL_Throwable *throwable);
const char *LIBMATTI_JL_Throwable_GetMessage(const LIBMATTI_JL_Throwable *throwable);
LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_GetCause(const LIBMATTI_JL_Throwable *throwable);
void LIBMATTI_JL_Throwable_SetCause(LIBMATTI_JL_Throwable *throwable, LIBMATTI_JL_Throwable *cause);
LIBMATTI_JL_Throwable **LIBMATTI_JL_Throwable_GetSuppressed(const LIBMATTI_JL_Throwable *throwable, size_t *count);
void LIBMATTI_JL_Throwable_AddSuppressed(LIBMATTI_JL_Throwable *throwable, LIBMATTI_JL_Throwable *suppressed);
const LIBMATTI_JL_StackTraceElement *LIBMATTI_JL_Throwable_GetStackTrace(const LIBMATTI_JL_Throwable *throwable,
                                                                          size_t *count);
void LIBMATTI_JL_Throwable_SetStackTrace(LIBMATTI_JL_Throwable *throwable,
                                         const LIBMATTI_JL_StackTraceElement *stackTrace, size_t count);

// Java: public void printStackTrace()
void LIBMATTI_JL_Throwable_PrintStackTrace(const LIBMATTI_JL_Throwable *throwable);
// Java: public void printStackTrace(PrintWriter s)
void LIBMATTI_JL_Throwable_PrintStackTraceTo(const LIBMATTI_JL_Throwable *throwable,
                                             LIBMATTI_JI_PrintWriter *writer);

#endif //MATTICRAFT_JAVA_LANG_THROWABLE_H
