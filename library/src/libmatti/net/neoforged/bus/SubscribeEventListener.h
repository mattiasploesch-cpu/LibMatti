// Port of net.neoforged.bus.SubscribeEventListener.
// Java creates the handler through EventListenerFactory (an ASM hidden class) and reads the
// @SubscribeEvent values from the Method; the C port takes the handler and the values directly.

#ifndef MATTICRAFT_BUS_SUBSCRIBEEVENTLISTENER_H
#define MATTICRAFT_BUS_SUBSCRIBEEVENTLISTENER_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"
#include "libmatti/net/neoforged/bus/api/EventPriority.h"
#include "libmatti/net/neoforged/bus/api/SubscribeEvent.h"

typedef struct LIBMATTI_BUS_SubscribeEventListener LIBMATTI_BUS_SubscribeEventListener;

struct LIBMATTI_BUS_SubscribeEventListener
{
    // Java: extends EventListener
    LIBMATTI_BUS_EventListener base;
    // Java: private final EventListener handler;
    LIBMATTI_BUS_EventListener *handler;
    // Java: private final SubscribeEvent subInfo;
    LIBMATTI_BUS_SubscribeEvent subInfo;
    // Java: private final String readable;
    char *readable;
};

// Java: public SubscribeEventListener(Object target, Method method)
// Java builds readable as "@SubscribeEvent: " + target + " " + method.getName() + getMethodDescriptor(method);
// the C port takes the already stringified descriptor.
LIBMATTI_BUS_SubscribeEventListener *LIBMATTI_BUS_SubscribeEventListener_New(const char *target,
                                                                             const char *methodName,
                                                                             const char *methodDescriptor,
                                                                             LIBMATTI_BUS_EventListener *handler,
                                                                             LIBMATTI_BUS_SubscribeEvent subInfo);

// Java: public EventPriority getPriority()
LIBMATTI_BUS_EventPriority LIBMATTI_BUS_SubscribeEventListener_GetPriority(
    const LIBMATTI_BUS_SubscribeEventListener *listener);

void LIBMATTI_BUS_SubscribeEventListener_Free(LIBMATTI_BUS_SubscribeEventListener *listener);

#endif //MATTICRAFT_BUS_SUBSCRIBEEVENTLISTENER_H
