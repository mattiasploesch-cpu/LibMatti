// Port of net.neoforged.bus.EventListenerFactory.

#include "libmatti/net/neoforged/bus/EventListenerFactory.h"

#include "libmatti/net/neoforged/bus/GeneratedEventListener.h"

// Java: public static EventListener create(Method callback, Object target)
LIBMATTI_BUS_EventListener *LIBMATTI_BUS_EventListenerFactory_Create(
    void *target, void (*callback)(void *target, LIBMATTI_BUS_Event *event))
{
    // Java: factory.invoke() for a static callback, factory.invoke(target) otherwise - C passes the
    // target either way and the callback ignores it when it is static.
    LIBMATTI_BUS_GeneratedEventListener *listener = LIBMATTI_BUS_GeneratedEventListener_New(target, callback);
    return &listener->base;
}
