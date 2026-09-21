// Port of net.neoforged.fml.loading.TracingPrintStream.

// Dl_info/dladdr need the GNU extensions
#define _GNU_SOURCE

#include "libmatti/net/neoforged/fml/loading/TracingPrintStream.h"

#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LIBMATTI_FML_TracingPrintStream
{
    // Java: private final Logger logger
    LIBMATTI_ML_Logger *logger;
};

// Java: private static final int BASE_DEPTH = 4
// Java's getStackTrace() includes its own frame, so the caller sits one frame deeper than in a C
// backtrace(), which starts at getPrefix() itself.
#define BASE_DEPTH 3

LIBMATTI_FML_TracingPrintStream *LIBMATTI_FML_TracingPrintStream_New(LIBMATTI_ML_Logger *logger)
{
    LIBMATTI_FML_TracingPrintStream *stream = calloc(1, sizeof(LIBMATTI_FML_TracingPrintStream));
    stream->logger = logger;
    return stream;
}

void LIBMATTI_FML_TracingPrintStream_Free(LIBMATTI_FML_TracingPrintStream *stream)
{
    free(stream);
}

// Java: private static String getPrefix() - "[className:methodName:lineNumber]: "
static const char *get_prefix(void)
{
    static __thread char prefix[256];

    void *frames[BASE_DEPTH + 4];
    int count = backtrace(frames, BASE_DEPTH + 4);
    int index = count - 1 < BASE_DEPTH ? count - 1 : BASE_DEPTH; // Java: elems[Math.min(BASE_DEPTH, elems.length - 1)]

    const char *symbol = "?";
    Dl_info info;
    if (index >= 0 && dladdr(frames[index], &info) != 0 && info.dli_sname != NULL) symbol = info.dli_sname;

    // Java: the Kotlin IoPackage and Throwable frames mask the origin
    if (strncmp(symbol, "kotlin.io.", 10) == 0)
    {
        index = count - 1 < BASE_DEPTH + 2 ? count - 1 : BASE_DEPTH + 2;
        if (index >= 0 && dladdr(frames[index], &info) != 0 && info.dli_sname != NULL) symbol = info.dli_sname;
    }
    else if (strncmp(symbol, "java.lang.Throwable", 19) == 0)
    {
        index = count - 1 < BASE_DEPTH + 4 ? count - 1 : BASE_DEPTH + 4;
        if (index >= 0 && dladdr(frames[index], &info) != 0 && info.dli_sname != NULL) symbol = info.dli_sname;
    }

    // Java: className:methodName:lineNumber - a C backtrace only names the function
    snprintf(prefix, sizeof(prefix), "[%s:0]: ", symbol);
    return prefix;
}

// Java: private void log(String s) { logger.info("{}{}", getPrefix(), s); }
static void log_string(LIBMATTI_FML_TracingPrintStream *stream, const char *value)
{
    LIBMATTI_ML_Logger_Info(stream->logger, NULL, "{}{}", get_prefix(), value);
}

// Java: @Override public void println(Object o)
void LIBMATTI_FML_TracingPrintStream_PrintlnObject(LIBMATTI_FML_TracingPrintStream *stream, const char *value)
{
    log_string(stream, value);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnString(LIBMATTI_FML_TracingPrintStream *stream, const char *value)
{
    log_string(stream, value);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnBoolean(LIBMATTI_FML_TracingPrintStream *stream, int value)
{
    log_string(stream, value ? "true" : "false");
}

void LIBMATTI_FML_TracingPrintStream_PrintlnChar(LIBMATTI_FML_TracingPrintStream *stream, char value)
{
    char text[2] = {value, '\0'};
    log_string(stream, text);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnInt(LIBMATTI_FML_TracingPrintStream *stream, int value)
{
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    log_string(stream, text);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnLong(LIBMATTI_FML_TracingPrintStream *stream, long long value)
{
    char text[32];
    snprintf(text, sizeof(text), "%lld", value);
    log_string(stream, text);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnFloat(LIBMATTI_FML_TracingPrintStream *stream, float value)
{
    char text[32];
    snprintf(text, sizeof(text), "%g", (double) value);
    log_string(stream, text);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnDouble(LIBMATTI_FML_TracingPrintStream *stream, double value)
{
    char text[32];
    snprintf(text, sizeof(text), "%g", value);
    log_string(stream, text);
}

void LIBMATTI_FML_TracingPrintStream_PrintlnChars(LIBMATTI_FML_TracingPrintStream *stream, const char *value)
{
    log_string(stream, value);
}
