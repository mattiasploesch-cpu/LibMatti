// Port of org.spongepowered.asm.logging.ILogger (sponge-mixin 0.17.3+mixin.0.8.7).
// Java's interface becomes the virtual-dispatch struct the C port uses for its interfaces.
// The Throwable-taking overloads collapse into the NULL-throwable form (C has no overloads).

#ifndef MATTICRAFT_SP_ASM_LOGGING_ILOGGER_H
#define MATTICRAFT_SP_ASM_LOGGING_ILOGGER_H

#include "libmatti/org/spongepowered/asm/logging/Level.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_Throwable LIBMATTI_JL_Throwable;

// Java: public interface ILogger
typedef struct LIBMATTI_SP_ILogger LIBMATTI_SP_ILogger;

struct LIBMATTI_SP_ILogger
{
    void *self;

    // Java: String getId()
    const char *(*getId)(void *self);
    // Java: String getType()
    const char *(*getType)(void *self);
    // Java: void catching(Level level, Throwable t) / catching(Throwable t)
    void (*catching)(void *self, LIBMATTI_SP_Level level, LIBMATTI_JL_Throwable *t);
    // Java: void <level>(String message, Object... params) - the port's logger substitutes
    // {} with strings, so the params are passed pre-formatted by the caller
    void (*logMessage)(void *self, LIBMATTI_SP_Level level, LIBMATTI_JL_Throwable *t, const char *message);
    // Java: <T extends Throwable> T throwing(T t)
    LIBMATTI_JL_Throwable *(*throwing)(void *self, LIBMATTI_JL_Throwable *t);
};

// Java: void catching(Level level, Throwable t)
void LIBMATTI_SP_ILogger_Catching(const LIBMATTI_SP_ILogger *logger, LIBMATTI_SP_Level level,
                                  LIBMATTI_JL_Throwable *t);
// Java: void catching(Throwable t)
void LIBMATTI_SP_ILogger_CatchingDefault(const LIBMATTI_SP_ILogger *logger, LIBMATTI_JL_Throwable *t);
// Java: the logging methods at their level (message only; params are pre-formatted)
void LIBMATTI_SP_ILogger_Trace(const LIBMATTI_SP_ILogger *logger, const char *message);
void LIBMATTI_SP_ILogger_Debug(const LIBMATTI_SP_ILogger *logger, const char *message);
void LIBMATTI_SP_ILogger_Info(const LIBMATTI_SP_ILogger *logger, const char *message);
void LIBMATTI_SP_ILogger_Warn(const LIBMATTI_SP_ILogger *logger, const char *message);
void LIBMATTI_SP_ILogger_Error(const LIBMATTI_SP_ILogger *logger, const char *message);
void LIBMATTI_SP_ILogger_Fatal(const LIBMATTI_SP_ILogger *logger, const char *message);
// Java: void log(Level level, String message, Object... params)
void LIBMATTI_SP_ILogger_Log(const LIBMATTI_SP_ILogger *logger, LIBMATTI_SP_Level level, const char *message);
// Java: <T extends Throwable> T throwing(T t)
LIBMATTI_JL_Throwable *LIBMATTI_SP_ILogger_Throwing(const LIBMATTI_SP_ILogger *logger, LIBMATTI_JL_Throwable *t);

#endif //MATTICRAFT_SP_ASM_LOGGING_ILOGGER_H
