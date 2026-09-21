// Port of net.neoforged.bus.EventBus.
// Java's ConcurrentHashMap<Object, List<EventListener>> becomes an identity-keyed entry list behind a
// mutex (the keys are Objects and Classes, which do not override equals); listenerLists stays a
// LockHelper<Class<?>, ListenerList>.

#ifndef MATTICRAFT_BUS_EVENTBUS_H
#define MATTICRAFT_BUS_EVENTBUS_H

#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/bus/api/IEventClassChecker.h"
#include "libmatti/net/neoforged/bus/api/IEventExceptionHandler.h"
#include "libmatti/net/neoforged/bus/api/SubscribeEvent.h"
#include "libmatti/net/neoforged/bus/BusBuilderImpl.h"
#include "libmatti/net/neoforged/bus/LockHelper.h"
#include "libmatti/java/lang/Throwable.h"

#include <stddef.h>

typedef struct LIBMATTI_BUS_EventBus_ListenerMap LIBMATTI_BUS_EventBus_ListenerMap;
typedef struct LIBMATTI_BUS_EventBus LIBMATTI_BUS_EventBus;

struct LIBMATTI_BUS_EventBus
{
    // Java: public class EventBus implements IEventExceptionHandler, IEventBus
    LIBMATTI_BUS_IEventExceptionHandler handlerBase;
    LIBMATTI_BUS_IEventBus busBase;
    // Java: private final LockHelper<Class<?>, ListenerList> listenerLists
    LIBMATTI_BUS_LockHelper *listenerLists;
    // Java: private final ConcurrentHashMap<Object, List<EventListener>> listeners
    LIBMATTI_BUS_EventBus_ListenerMap *listeners;
    // Java: private final IEventExceptionHandler exceptionHandler
    const LIBMATTI_BUS_IEventExceptionHandler *exceptionHandler;
    // Java: private volatile boolean shutdown = false;
    int shutdown;
    // Java: private final IEventClassChecker classChecker
    LIBMATTI_BUS_IEventClassChecker classChecker;
    int checkTypesOnDispatch;
    int allowPerPhasePost;
};

// Java: public EventBus(final BusBuilderImpl busBuilder)
LIBMATTI_BUS_EventBus *LIBMATTI_BUS_EventBus_New(LIBMATTI_BUS_BusBuilderImpl *busBuilder);
void LIBMATTI_BUS_EventBus_Free(LIBMATTI_BUS_EventBus *bus);

// Java: public void handleException(...) - the bus is its own default exception handler
void LIBMATTI_BUS_EventBus_HandleException(LIBMATTI_BUS_EventBus *bus, LIBMATTI_BUS_IEventBus *eventBus,
                                           LIBMATTI_BUS_Event *event, LIBMATTI_BUS_EventListener **listeners,
                                           size_t listenerCount, int index, LIBMATTI_JL_Throwable *throwable);

// Java: private void registerListener(Object target, Method method) / register(Class<?> eventType, Object target, Method method)
// java.lang.reflect.Method and the @SubscribeEvent annotation have no C counterpart, so the port takes
// the target's name, the callback and the annotation values directly. The bus is addressed through its
// IEventBus, which is the only handle the builder hands out.
void LIBMATTI_BUS_EventBus_RegisterListener(LIBMATTI_BUS_IEventBus *eventBus, LIBMATTI_JL_Class *eventType,
                                            void *target, const char *targetName, const char *methodName,
                                            const char *methodDescriptor,
                                            void (*callback)(void *target, LIBMATTI_BUS_Event *event),
                                            LIBMATTI_BUS_SubscribeEvent subInfo);

#endif //MATTICRAFT_BUS_EVENTBUS_H
