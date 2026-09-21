// Port of net.neoforged.bus.EventListenerFactory.
// Java generates a wrapper class with ASM and binds the callback through the hidden class's class
// data (MethodHandles.lookup().defineHiddenClassWithClassData), so that invoke() calls the method
// handle without reflection. C has neither hidden classes nor method handles over struct members, so
// the port creates the equivalent wrapper directly: the listener keeps the target and the callback.

#ifndef MATTICRAFT_BUS_EVENTLISTENERFACTORY_H
#define MATTICRAFT_BUS_EVENTLISTENERFACTORY_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"

// Java: public static EventListener create(Method callback, Object target)
// Java's per-Method factory cache (LockHelper<Method, MethodHandle>) caches the generated wrapper
// class; the C wrapper holds no generated state, so there is nothing to cache.
LIBMATTI_BUS_EventListener *LIBMATTI_BUS_EventListenerFactory_Create(
    void *target, void (*callback)(void *target, LIBMATTI_BUS_Event *event));

#endif //MATTICRAFT_BUS_EVENTLISTENERFACTORY_H
