// Port of net.neoforged.bus.EventBusErrorMessage.

#include "libmatti/net/neoforged/bus/EventBusErrorMessage.h"

#include "libmatti/java/io/PrintWriter.h"
#include "libmatti/java/io/StringWriter.h"
#include "libmatti/java/lang/StringBuilder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LIBMATTI_BUS_EventBusErrorMessage
{
    // Java: private final Event event; - commented out in Java
    int index;
    LIBMATTI_BUS_EventListener **listeners;
    size_t listenerCount;
    LIBMATTI_JL_Throwable *throwable;
};

LIBMATTI_BUS_EventBusErrorMessage *LIBMATTI_BUS_EventBusErrorMessage_New(LIBMATTI_BUS_Event *event, int index,
                                                                        LIBMATTI_BUS_EventListener **listeners,
                                                                        size_t listenerCount,
                                                                        LIBMATTI_JL_Throwable *throwable)
{
    (void)event;

    LIBMATTI_BUS_EventBusErrorMessage *message = calloc(1, sizeof(LIBMATTI_BUS_EventBusErrorMessage));
    message->index = index;
    message->listeners = listeners;
    message->listenerCount = listenerCount;
    message->throwable = throwable;
    return message;
}

// Java: public void formatTo(StringBuilder buffer)
// The caller owns the returned string.
char *LIBMATTI_BUS_EventBusErrorMessage_FormatTo(const LIBMATTI_BUS_EventBusErrorMessage *message)
{
    char number[32];
    const char *reason = LIBMATTI_JL_Throwable_GetMessage(message->throwable);

    LIBMATTI_JL_StringBuilder *buffer =
        LIBMATTI_JL_StringBuilder_NewFromString("Exception caught during firing event: ");
    LIBMATTI_JL_StringBuilder_Append(buffer, reason != NULL ? reason : "null");
    LIBMATTI_JL_StringBuilder_AppendChar(buffer, '\n');
    LIBMATTI_JL_StringBuilder_Append(buffer, "\tIndex: ");

    snprintf(number, sizeof(number), "%d", message->index);
    LIBMATTI_JL_StringBuilder_Append(buffer, number);
    LIBMATTI_JL_StringBuilder_AppendChar(buffer, '\n');
    LIBMATTI_JL_StringBuilder_Append(buffer, "\tListeners:\n");

    for (size_t i = 0; i < message->listenerCount; i++)
    {
        char *listener = LIBMATTI_BUS_EventListener_ToString(message->listeners[i]);
        LIBMATTI_JL_StringBuilder_Append(buffer, "\t\t");

        snprintf(number, sizeof(number), "%d", (int)i);
        LIBMATTI_JL_StringBuilder_Append(buffer, number);
        LIBMATTI_JL_StringBuilder_Append(buffer, ": ");
        LIBMATTI_JL_StringBuilder_Append(buffer, listener);
        LIBMATTI_JL_StringBuilder_AppendChar(buffer, '\n');
        free(listener);
    }

    // Java: final StringWriter sw = new StringWriter(); throwable.printStackTrace(new PrintWriter(sw));
    //       buffer.append(sw.getBuffer());
    LIBMATTI_JI_StringWriter *sw = LIBMATTI_JI_StringWriter_New();
    LIBMATTI_JI_PrintWriter *pw = LIBMATTI_JI_PrintWriter_New(&sw->base);
    LIBMATTI_JL_Throwable_PrintStackTraceTo(message->throwable, pw);
    LIBMATTI_JL_StringBuilder_Append(buffer, LIBMATTI_JI_StringWriter_ToString(sw));
    LIBMATTI_JI_PrintWriter_Free(pw);
    LIBMATTI_JI_StringWriter_Free(sw);

    char *formatted = strdup(LIBMATTI_JL_StringBuilder_ToString(buffer));
    LIBMATTI_JL_StringBuilder_Free(buffer);
    return formatted;
}

void LIBMATTI_BUS_EventBusErrorMessage_Free(LIBMATTI_BUS_EventBusErrorMessage *message)
{
    free(message);
}
