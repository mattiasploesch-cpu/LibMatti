// Port of net.neoforged.bus.GeneratedEventListener.
// Java: public abstract non-sealed class GeneratedEventListener extends EventListener {}
// The ASM-generated wrapper holds the captured instance in a field and the handler MethodHandle in the
// hidden class's class data; the C port stores those two things directly.

#ifndef MATTICRAFT_BUS_GENERATEDEVENTLISTENER_H
#define MATTICRAFT_BUS_GENERATEDEVENTLISTENER_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"

typedef struct LIBMATTI_BUS_GeneratedEventListener LIBMATTI_BUS_GeneratedEventListener;

struct LIBMATTI_BUS_GeneratedEventListener
{
    // Java: extends EventListener
    LIBMATTI_BUS_EventListener base;
    // Java: private final Object instance; (only present in the non-static wrapper)
    void *instance;
    // Java: the class-data MethodHandle - it is invoked with (instance, event)
    void (*callback)(void *instance, LIBMATTI_BUS_Event *event);
};

// Java: the generated constructor - NULL when the wrapper could not be created
LIBMATTI_BUS_GeneratedEventListener *LIBMATTI_BUS_GeneratedEventListener_New(
    void *instance, void (*callback)(void *instance, LIBMATTI_BUS_Event *event));

#endif //MATTICRAFT_BUS_GENERATEDEVENTLISTENER_H
