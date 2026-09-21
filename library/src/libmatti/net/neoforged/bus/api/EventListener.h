// Port of net.neoforged.bus.api.EventListener.
// Java: public abstract sealed class EventListener permits ConsumerEventHandler, GeneratedEventListener,
//       SubscribeEventListener { public abstract void invoke(Event event); }
// The C port replaces the class hierarchy with the function pointer the VM would dispatch to; the
// permitted wrappers keep their own fields next to this struct.

#ifndef MATTICRAFT_BUS_API_EVENTLISTENER_H
#define MATTICRAFT_BUS_API_EVENTLISTENER_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_BUS_EventListener LIBMATTI_BUS_EventListener;

struct LIBMATTI_BUS_EventListener
{
    void *self;

    // Java: public abstract void invoke(Event event)
    void (*invoke)(void *self, LIBMATTI_BUS_Event *event);
    // Java: instanceof IWrapperListener -> getWithoutCheck(); NULL when the listener is not a wrapper
    LIBMATTI_BUS_EventListener *(*getWithoutCheck)(void *self);
    // Java: public String toString()
    char *(*toString)(void *self);
};

// Java: public abstract void invoke(Event event)
void LIBMATTI_BUS_EventListener_Invoke(const LIBMATTI_BUS_EventListener *listener, LIBMATTI_BUS_Event *event);
// Java: public String toString()
char *LIBMATTI_BUS_EventListener_ToString(const LIBMATTI_BUS_EventListener *listener);

#endif //MATTICRAFT_BUS_API_EVENTLISTENER_H
