#include "LogEvent.h"

#include "libmatti/org/apache/logging/log4j/core/impl/ThrowableProxy.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_L4J_LogEvent *LIBMATTI_L4J_LogEvent_New(const char *level, const char *loggerName, const char *message)
{
    LIBMATTI_L4J_LogEvent *event = calloc(1, sizeof(LIBMATTI_L4J_LogEvent));
    event->level = level;
    event->loggerName = loggerName != NULL ? strdup(loggerName) : NULL;
    event->message = message != NULL ? strdup(message) : NULL;
    return event;
}

void LIBMATTI_L4J_LogEvent_Free(LIBMATTI_L4J_LogEvent *event)
{
    if (event == NULL) return;
    free(event->loggerName);
    free(event->message);
    LIBMATTI_L4J_ThrowableProxy_Free(event->thrownProxy);
    free(event);
}

const char *LIBMATTI_L4J_LogEvent_GetLevel(const LIBMATTI_L4J_LogEvent *event)
{
    return event->level;
}

const char *LIBMATTI_L4J_LogEvent_GetLoggerName(const LIBMATTI_L4J_LogEvent *event)
{
    return event->loggerName;
}

const char *LIBMATTI_L4J_LogEvent_GetMessage(const LIBMATTI_L4J_LogEvent *event)
{
    return event->message;
}

long LIBMATTI_L4J_LogEvent_GetTimeMillis(const LIBMATTI_L4J_LogEvent *event)
{
    return event->timeMillis;
}

LIBMATTI_JL_Throwable *LIBMATTI_L4J_LogEvent_GetThrown(const LIBMATTI_L4J_LogEvent *event)
{
    return event->thrown;
}

LIBMATTI_L4J_ThrowableProxy *LIBMATTI_L4J_LogEvent_GetThrownProxy(LIBMATTI_L4J_LogEvent *event)
{
    if (event->thrownProxy == NULL && event->thrown != NULL)
        event->thrownProxy = LIBMATTI_L4J_ThrowableProxy_New(event->thrown);
    return event->thrownProxy;
}

void LIBMATTI_L4J_LogEvent_SetThrown(LIBMATTI_L4J_LogEvent *event, LIBMATTI_JL_Throwable *thrown)
{
    event->thrown = thrown;
}

void LIBMATTI_L4J_LogEvent_SetThrownProxy(LIBMATTI_L4J_LogEvent *event, LIBMATTI_L4J_ThrowableProxy *proxy)
{
    event->thrownProxy = proxy;
}
