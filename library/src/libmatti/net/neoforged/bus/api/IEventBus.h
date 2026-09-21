// Port of net.neoforged.bus.api.IEventBus.
// Java's <T extends Event> generics erase to Event*; the listener overloads that infer the event type
// from a lambda through TypeResolver cannot be expressed in C, so the port keeps the overloads taking
// the event Class explicitly plus the helpers that apply the Java defaults.

#ifndef MATTICRAFT_BUS_API_IEVENTBUS_H
#define MATTICRAFT_BUS_API_IEVENTBUS_H

#include "libmatti/java/lang/Class.h"
#include "libmatti/java/util/function/Consumer.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/bus/api/EventPriority.h"

// Java: public interface IEventBus
typedef struct LIBMATTI_BUS_IEventBus LIBMATTI_BUS_IEventBus;

struct LIBMATTI_BUS_IEventBus
{
    void *self;

    // Java: void register(Object target)
    // The C port logs that listener scanning needs annotations and reflection (TODO); the native
    // registration path is LIBMATTI_BUS_EventBus_AddSubscribeListener.
    void (*registerTarget)(void *self, void *target);
    // Java: <T extends Event> void addListener(EventPriority priority, boolean receiveCanceled, Class<T> eventType, Consumer<T> consumer)
    void (*addListener)(void *self, LIBMATTI_BUS_EventPriority priority, int receiveCanceled,
                        LIBMATTI_JL_Class *eventType, const LIBMATTI_JU_Consumer *consumer);
    // Java: void unregister(Object object)
    void (*unregister)(void *self, void *object);
    // Java: <T extends Event> T post(T event)
    LIBMATTI_BUS_Event *(*post)(void *self, LIBMATTI_BUS_Event *event);
    // Java: <T extends Event> T post(EventPriority phase, T event)
    LIBMATTI_BUS_Event *(*postPhase)(void *self, LIBMATTI_BUS_EventPriority phase, LIBMATTI_BUS_Event *event);
    // Java: void start()
    void (*start)(void *self);
};

// Java: void register(Object target)
void LIBMATTI_BUS_IEventBus_Register(const LIBMATTI_BUS_IEventBus *bus, void *target);
// Java: <T extends Event> void addListener(EventPriority priority, boolean receiveCanceled, Class<T> eventType, Consumer<T> consumer)
void LIBMATTI_BUS_IEventBus_AddListener(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_EventPriority priority,
                                        int receiveCanceled, LIBMATTI_JL_Class *eventType,
                                        const LIBMATTI_JU_Consumer *consumer);
// Java: <T extends Event> void addListener(Class<T> eventType, Consumer<T> consumer)
void LIBMATTI_BUS_IEventBus_AddListenerDefault(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_JL_Class *eventType,
                                               const LIBMATTI_JU_Consumer *consumer);
// Java: <T extends Event> void addListener(EventPriority priority, Class<T> eventType, Consumer<T> consumer)
void LIBMATTI_BUS_IEventBus_AddListenerWithPriority(const LIBMATTI_BUS_IEventBus *bus,
                                                    LIBMATTI_BUS_EventPriority priority, LIBMATTI_JL_Class *eventType,
                                                    const LIBMATTI_JU_Consumer *consumer);
// Java: <T extends Event> void addListener(boolean receiveCanceled, Class<T> eventType, Consumer<T> consumer)
void LIBMATTI_BUS_IEventBus_AddListenerReceiveCanceled(const LIBMATTI_BUS_IEventBus *bus, int receiveCanceled,
                                                       LIBMATTI_JL_Class *eventType,
                                                       const LIBMATTI_JU_Consumer *consumer);
// Java: void unregister(Object object)
void LIBMATTI_BUS_IEventBus_Unregister(const LIBMATTI_BUS_IEventBus *bus, void *object);
// Java: <T extends Event> T post(T event)
LIBMATTI_BUS_Event *LIBMATTI_BUS_IEventBus_Post(const LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event);
// Java: <T extends Event> T post(EventPriority phase, T event)
LIBMATTI_BUS_Event *LIBMATTI_BUS_IEventBus_PostPhase(const LIBMATTI_BUS_IEventBus *bus,
                                                     LIBMATTI_BUS_EventPriority phase, LIBMATTI_BUS_Event *event);
// Java: void start()
void LIBMATTI_BUS_IEventBus_Start(const LIBMATTI_BUS_IEventBus *bus);

#endif //MATTICRAFT_BUS_API_IEVENTBUS_H
