#ifndef MATTICRAFT_L4J_LOGEVENT_H
#define MATTICRAFT_L4J_LOGEVENT_H

#include "libmatti/java/lang/Throwable.h"

typedef struct LIBMATTI_L4J_ThrowableProxy LIBMATTI_L4J_ThrowableProxy;

typedef struct
{
    const char *level;
    char *loggerName;
    char *message;
    long timeMillis;
    LIBMATTI_JL_Throwable *thrown;
    LIBMATTI_L4J_ThrowableProxy *thrownProxy;
} LIBMATTI_L4J_LogEvent;

LIBMATTI_L4J_LogEvent *LIBMATTI_L4J_LogEvent_New(const char *level, const char *loggerName, const char *message);
void LIBMATTI_L4J_LogEvent_Free(LIBMATTI_L4J_LogEvent *event);

const char *LIBMATTI_L4J_LogEvent_GetLevel(const LIBMATTI_L4J_LogEvent *event);
const char *LIBMATTI_L4J_LogEvent_GetLoggerName(const LIBMATTI_L4J_LogEvent *event);
const char *LIBMATTI_L4J_LogEvent_GetMessage(const LIBMATTI_L4J_LogEvent *event);
long LIBMATTI_L4J_LogEvent_GetTimeMillis(const LIBMATTI_L4J_LogEvent *event);
LIBMATTI_JL_Throwable *LIBMATTI_L4J_LogEvent_GetThrown(const LIBMATTI_L4J_LogEvent *event);
LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_LogEvent_GetThrownProxy(LIBMATTI_L4J_LogEvent *event);
void LIBMATTI_L4J_LogEvent_SetThrown(LIBMATTI_L4J_LogEvent *event, LIBMATTI_JL_Throwable *thrown);
void LIBMATTI_L4J_LogEvent_SetThrownProxy(LIBMATTI_L4J_LogEvent *event, LIBMATTI_L4J_ThrowableProxy *proxy);

#endif //MATTICRAFT_L4J_LOGEVENT_H
