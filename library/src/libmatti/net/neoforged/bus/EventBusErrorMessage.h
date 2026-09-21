// Port of net.neoforged.bus.EventBusErrorMessage.
// Java implements org.apache.logging.log4j.message.Message / StringBuilderFormattable so log4j can
// format the message lazily; the C logger takes a string, so the port returns formatTo's output.

#ifndef MATTICRAFT_BUS_EVENTBUSERRORMESSAGE_H
#define MATTICRAFT_BUS_EVENTBUSERRORMESSAGE_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/bus/api/EventListener.h"

#include <stddef.h>

typedef struct LIBMATTI_BUS_EventBusErrorMessage LIBMATTI_BUS_EventBusErrorMessage;

// Java: public EventBusErrorMessage(Event event, int index, EventListener[] listeners, Throwable throwable)
// Java keeps the event commented out; the C port keeps the parameter for the signature.
LIBMATTI_BUS_EventBusErrorMessage *LIBMATTI_BUS_EventBusErrorMessage_New(LIBMATTI_BUS_Event *event, int index,
                                                                        LIBMATTI_BUS_EventListener **listeners,
                                                                        size_t listenerCount,
                                                                        LIBMATTI_JL_Throwable *throwable);

// Java: public void formatTo(StringBuilder buffer)
char *LIBMATTI_BUS_EventBusErrorMessage_FormatTo(const LIBMATTI_BUS_EventBusErrorMessage *message);

void LIBMATTI_BUS_EventBusErrorMessage_Free(LIBMATTI_BUS_EventBusErrorMessage *message);

#endif //MATTICRAFT_BUS_EVENTBUSERRORMESSAGE_H
