






#ifndef MATTICRAFT_BUS_EVENTLISTENERFACTORY_H
#define MATTICRAFT_BUS_EVENTLISTENERFACTORY_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"

// Java: public static EventListener create(Method callback, Object target)
// Java's per-Method factory cache (LockHelper<Method, MethodHandle>) caches the generated wrapper
// class; the C wrapper holds no generated state, so there is nothing to cache.
LIBMATTI_BUS_EventListener *LIBMATTI_BUS_EventListenerFactory_Create(
    void *target, void (*callback)(void *target, LIBMATTI_BUS_Event *event));

#endif
