// Port of net.neoforged.bus.GeneratedEventListener.

#include "libmatti/net/neoforged/bus/GeneratedEventListener.h"

#include <stdlib.h>
#include <string.h>

static void generated_invoke(void *self, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_GeneratedEventListener *listener = self;
    listener->callback(listener->instance, event);
}

static char *generated_to_string(void *self)
{
    // Java: GeneratedEventListener does not override toString()
    (void)self;
    return strdup("net.neoforged.bus.GeneratedEventListener");
}

LIBMATTI_BUS_GeneratedEventListener *LIBMATTI_BUS_GeneratedEventListener_New(
    void *instance, void (*callback)(void *instance, LIBMATTI_BUS_Event *event))
{
    LIBMATTI_BUS_GeneratedEventListener *listener = calloc(1, sizeof(LIBMATTI_BUS_GeneratedEventListener));
    listener->base.self = listener;
    listener->base.invoke = generated_invoke;
    listener->base.toString = generated_to_string;
    listener->instance = instance;
    listener->callback = callback;
    return listener;
}
