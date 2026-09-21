// Port of net.neoforged.fml.loading.mixin.FMLMixinLogger.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinLogger.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Throwable.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    // Java: private final Logger logger
    LIBMATTI_ML_Logger *logger;
    // Java: the logger name is the ILogger id
    char *name;
} FMLMixinLogger;

// Java: LEVELS maps the Mixin LEVEL ordinals to log4j2 levels 1:1
static LIBMATTI_ML_Level map_level(LIBMATTI_SP_Level level)
{
    switch (level)
    {
    case LIBMATTI_SP_Level_FATAL: return LIBMATTI_ML_LEVEL_FATAL;
    case LIBMATTI_SP_Level_ERROR: return LIBMATTI_ML_LEVEL_ERROR;
    case LIBMATTI_SP_Level_WARN: return LIBMATTI_ML_LEVEL_WARN;
    case LIBMATTI_SP_Level_INFO: return LIBMATTI_ML_LEVEL_INFO;
    case LIBMATTI_SP_Level_DEBUG: return LIBMATTI_ML_LEVEL_DEBUG;
    case LIBMATTI_SP_Level_TRACE: return LIBMATTI_ML_LEVEL_TRACE;
    }
    return LIBMATTI_ML_LEVEL_INFO;
}

static const char *get_id(void *self)
{
    return ((FMLMixinLogger *) self)->name;
}

static const char *get_type(void *self)
{
    (void) self;
    // Java: return "Log4j2 (via FML)";
    return "Log4j2 (via FML)";
}

// Java: void catching(Level level, Throwable t) { logger.catching(LEVELS[level.ordinal()], t); }
static void catching(void *self, LIBMATTI_SP_Level level, LIBMATTI_JL_Throwable *t)
{
    (void) self;
    (void) level;
    if (t != NULL) LIBMATTI_JL_Throwable_PrintStackTrace(t);
}

// Java: the message methods route through log(level, message, params)
static void log_message(void *self, LIBMATTI_SP_Level level, LIBMATTI_JL_Throwable *t, const char *message)
{
    FMLMixinLogger *logger = self;
    (void) t;

    LIBMATTI_ML_Logger_Log(logger->logger, NULL, map_level(level), "{}", message != NULL ? message : "");
}

static LIBMATTI_JL_Throwable *throwing(void *self, LIBMATTI_JL_Throwable *t)
{
    (void) self;
    if (t != NULL) LIBMATTI_JL_Throwable_PrintStackTrace(t);
    return t;
}

// Java: public FMLMixinLogger(String name)
LIBMATTI_SP_ILogger *LIBMATTI_FML_FMLMixinLogger_Instance(const char *name)
{
    // Java: the loggers map caches by name (FMLMixinService.loggers)
    static struct
    {
        char *name;
        LIBMATTI_SP_ILogger *logger;
    } *loggers = NULL;
    static size_t loggerCount = 0;

    for (size_t i = 0; i < loggerCount; i++)
        if (strcmp(loggers[i].name, name) == 0) return loggers[i].logger;

    FMLMixinLogger *impl = calloc(1, sizeof(*impl));
    impl->logger = LIBMATTI_ML_LogManager_GetLogger();
    impl->name = strdup(name);

    LIBMATTI_SP_ILogger *logger = calloc(1, sizeof(*logger));
    logger->self = impl;
    logger->getId = get_id;
    logger->getType = get_type;
    logger->catching = catching;
    logger->logMessage = log_message;
    logger->throwing = throwing;

    loggers = realloc(loggers, sizeof(*loggers) * (loggerCount + 1));
    loggers[loggerCount].name = impl->name;
    loggers[loggerCount].logger = logger;
    loggerCount++;

    return logger;
}
