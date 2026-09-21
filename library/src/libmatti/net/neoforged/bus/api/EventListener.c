// Port of net.neoforged.bus.api.EventListener.

#include "libmatti/net/neoforged/bus/api/EventListener.h"

#include <string.h>

// Java: public abstract void invoke(Event event)
void LIBMATTI_BUS_EventListener_Invoke(const LIBMATTI_BUS_EventListener *listener, LIBMATTI_BUS_Event *event)
{
    listener->invoke(listener->self, event);
}

// Java: public String toString()
char *LIBMATTI_BUS_EventListener_ToString(const LIBMATTI_BUS_EventListener *listener)
{
    if (listener->toString == NULL) return strdup("null");
    return listener->toString(listener->self);
}
