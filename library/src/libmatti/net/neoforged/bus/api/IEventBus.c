// Port of net.neoforged.bus.api.IEventBus.

#include "libmatti/net/neoforged/bus/api/IEventBus.h"

// Java: void register(Object target)
void LIBMATTI_BUS_IEventBus_Register(const LIBMATTI_BUS_IEventBus *bus, void *target)
{
    bus->registerTarget(bus->self, target);
}

// Java: <T extends Event> void addListener(EventPriority priority, boolean receiveCanceled, Class<T> eventType, Consumer<T> consumer)
void LIBMATTI_BUS_IEventBus_AddListener(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_EventPriority priority,
                                        int receiveCanceled, LIBMATTI_JL_Class *eventType,
                                        const LIBMATTI_JU_Consumer *consumer)
{
    bus->addListener(bus->self, priority, receiveCanceled, eventType, consumer);
}

// Java: addListener(EventPriority.NORMAL, false, eventType, consumer)
void LIBMATTI_BUS_IEventBus_AddListenerDefault(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_JL_Class *eventType,
                                               const LIBMATTI_JU_Consumer *consumer)
{
    LIBMATTI_BUS_IEventBus_AddListener(bus, LIBMATTI_BUS_EventPriority_NORMAL, 0, eventType, consumer);
}

// Java: addListener(priority, false, eventType, consumer)
void LIBMATTI_BUS_IEventBus_AddListenerWithPriority(const LIBMATTI_BUS_IEventBus *bus,
                                                    LIBMATTI_BUS_EventPriority priority, LIBMATTI_JL_Class *eventType,
                                                    const LIBMATTI_JU_Consumer *consumer)
{
    LIBMATTI_BUS_IEventBus_AddListener(bus, priority, 0, eventType, consumer);
}

// Java: addListener(EventPriority.NORMAL, receiveCanceled, eventType, consumer)
void LIBMATTI_BUS_IEventBus_AddListenerReceiveCanceled(const LIBMATTI_BUS_IEventBus *bus, int receiveCanceled,
                                                       LIBMATTI_JL_Class *eventType,
                                                       const LIBMATTI_JU_Consumer *consumer)
{
    LIBMATTI_BUS_IEventBus_AddListener(bus, LIBMATTI_BUS_EventPriority_NORMAL, receiveCanceled, eventType, consumer);
}

// Java: void unregister(Object object)
void LIBMATTI_BUS_IEventBus_Unregister(const LIBMATTI_BUS_IEventBus *bus, void *object)
{
    bus->unregister(bus->self, object);
}

// Java: <T extends Event> T post(T event)
LIBMATTI_BUS_Event *LIBMATTI_BUS_IEventBus_Post(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event)
{
    return bus->post(bus->self, event);
}

// Java: <T extends Event> T post(EventPriority phase, T event)
LIBMATTI_BUS_Event *LIBMATTI_BUS_IEventBus_PostPhase(const LIBMATTI_BUS_IEventBus *bus,
                                                     LIBMATTI_BUS_EventPriority phase, LIBMATTI_BUS_Event *event)
{
    return bus->postPhase(bus->self, phase, event);
}

// Java: void start()
void LIBMATTI_BUS_IEventBus_Start(const LIBMATTI_BUS_IEventBus *bus)
{
    bus->start(bus->self);
}
