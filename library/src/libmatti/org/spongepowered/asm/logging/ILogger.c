// Port of org.spongepowered.asm.logging.ILogger - the dispatch helpers.

#include "libmatti/org/spongepowered/asm/logging/ILogger.h"

void LIBMATTI_SP_ILogger_Catching(const LIBMATTI_SP_ILogger *logger, LIBMATTI_SP_Level level,
                                  LIBMATTI_JL_Throwable *t)
{
    if (logger != NULL && logger->catching != NULL) logger->catching(logger->self, level, t);
}

void LIBMATTI_SP_ILogger_CatchingDefault(const LIBMATTI_SP_ILogger *logger, LIBMATTI_JL_Throwable *t)
{
    LIBMATTI_SP_ILogger_Catching(logger, LIBMATTI_SP_Level_ERROR, t);
}

void LIBMATTI_SP_ILogger_Trace(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_TRACE, message);
}

void LIBMATTI_SP_ILogger_Debug(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_DEBUG, message);
}

void LIBMATTI_SP_ILogger_Info(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_INFO, message);
}

void LIBMATTI_SP_ILogger_Warn(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_WARN, message);
}

void LIBMATTI_SP_ILogger_Error(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_ERROR, message);
}

void LIBMATTI_SP_ILogger_Fatal(const LIBMATTI_SP_ILogger *logger, const char *message)
{
    LIBMATTI_SP_ILogger_Log(logger, LIBMATTI_SP_Level_FATAL, message);
}

void LIBMATTI_SP_ILogger_Log(const LIBMATTI_SP_ILogger *logger, LIBMATTI_SP_Level level, const char *message)
{
    if (logger != NULL && logger->logMessage != NULL) logger->logMessage(logger->self, level, NULL, message);
}

LIBMATTI_JL_Throwable *LIBMATTI_SP_ILogger_Throwing(const LIBMATTI_SP_ILogger *logger, LIBMATTI_JL_Throwable *t)
{
    if (logger == NULL || logger->throwing == NULL) return t;
    return logger->throwing(logger->self, t);
}
