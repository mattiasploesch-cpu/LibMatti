//
// Replacement for org.apache.logging.log4j.LogManager / Logger.
//

#include "LogManager.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libmatti/java/nio/file/Files.h"

struct LIBMATTI_ML_Logger
{
    int level; // 0 = trace .. 5 = fatal
};

static LIBMATTI_ML_Logger INSTANCE = {0};

LIBMATTI_ML_Logger *LIBMATTI_ML_LogManager_GetLogger(void)
{
    return &INSTANCE;
}

// log4j: "{}" is replaced by the next parameter
static void log_message(int level, const char *levelName, const LIBMATTI_ML_LogMarker *marker,
                        const char *message, va_list args)
{
    LIBMATTI_JNF_Files_DeleteIfExists("./matticraft.log");
    LIBMATTI_JNF_Files_CreateFile("./matticraft.log");
    FILE *matticraftLogFile = fopen("./matticraft.log", "w");
    LIBMATTI_JNF_Files_DeleteIfExists("./matticraft_err.log");
    LIBMATTI_JNF_Files_CreateFile("./matticraft_err.log");
    FILE *matticraftErrLogFile = fopen("./matticraft_err.log", "w");
    FILE *out = level >= 4 ? stderr : stdout;
    FILE *outfile = level >= 4 ? matticraftLogFile : matticraftErrLogFile;

    if (marker != NULL) fprintf(out, "[%s] ", marker->name);
    if (marker != NULL) fprintf(outfile, "[%s] ", marker->name);
    fprintf(out, "%-5s: ", levelName);
    fprintf(outfile, "%-5s: ", levelName);

    for (const char *c = message; *c != '\0'; c++)
    {
        if (c[0] == '{' && c[1] == '}')
        {
            const char *value = va_arg(args, const char *);
            fputs(value != NULL ? value : "null", out);
            fputs(value != NULL ? value : "null", outfile);
            c++;
        }
        else
        {
            fputc(*c, out);
            fputc(*c, outfile);
        }
    }

    fputc('\n', out);
    fputc('\n', outfile);
    fflush(out);
    }

#define LOG_LEVEL_FUNCTION(name, level, levelName)                                          \
    void LIBMATTI_ML_Logger_##name(const LIBMATTI_ML_Logger *logger,                        \
                                   const LIBMATTI_ML_LogMarker *marker,                      \
                                   const char *message, ...)                                \
    {                                                                                       \
        (void)logger;                                                                       \
        va_list args;                                                                       \
        va_start(args, message);                                                            \
        log_message(level, levelName, marker, message, args);                               \
        va_end(args);                                                                       \
    }

LOG_LEVEL_FUNCTION(Trace, 0, "TRACE")
LOG_LEVEL_FUNCTION(Debug, 1, "DEBUG")
LOG_LEVEL_FUNCTION(Info, 2, "INFO")
LOG_LEVEL_FUNCTION(Warn, 3, "WARN")
LOG_LEVEL_FUNCTION(Error, 4, "ERROR")
LOG_LEVEL_FUNCTION(Fatal, 5, "FATAL")

static const char *LEVEL_NAMES[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

// Java: logger.isEnabled(Level) - the port writes everything to the console, so the
// level check is what gates a TRACE-only path (the CLASSDUMP marker is accepted like
// any other marker; only the level decides).
int LIBMATTI_ML_Logger_IsEnabledTrace(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker)
{
    (void) marker;
    return logger->level <= LIBMATTI_ML_LEVEL_TRACE;
}

// Java: logger.atLevel(Level)
void LIBMATTI_ML_Logger_Log(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                            LIBMATTI_ML_Level level, const char *message, ...)
{
    (void)logger;
    va_list args;
    va_start(args, message);
    log_message(level, LEVEL_NAMES[level], marker, message, args);
    va_end(args);
}
