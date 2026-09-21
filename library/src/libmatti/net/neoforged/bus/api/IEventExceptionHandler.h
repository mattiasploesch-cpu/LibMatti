// Port of net.neoforged.bus.api.IEventExceptionHandler.

#ifndef MATTICRAFT_BUS_API_IEVENTEXCEPTIONHANDLER_H
#define MATTICRAFT_BUS_API_IEVENTEXCEPTIONHANDLER_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/bus/api/EventListener.h"

#include <stddef.h>

typedef struct LIBMATTI_BUS_IEventBus LIBMATTI_BUS_IEventBus;

// Java: public interface IEventExceptionHandler
typedef struct LIBMATTI_BUS_IEventExceptionHandler LIBMATTI_BUS_IEventExceptionHandler;

struct LIBMATTI_BUS_IEventExceptionHandler
{
    void *self;

    // Java: void handleException(IEventBus bus, Event event, EventListener[] listeners, int index, Throwable throwable)
    void (*handleException)(void *self, LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event,
                            LIBMATTI_BUS_EventListener **listeners, size_t listenerCount, int index,
                            LIBMATTI_JL_Throwable *throwable);
};

// Java: void handleException(IEventBus bus, Event event, EventListener[] listeners, int index, Throwable throwable)
void LIBMATTI_BUS_IEventExceptionHandler_HandleException(const LIBMATTI_BUS_IEventExceptionHandler *handler,
                                                         LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event,
                                                         LIBMATTI_BUS_EventListener **listeners, size_t listenerCount,
                                                         int index, LIBMATTI_JL_Throwable *throwable);

#endif //MATTICRAFT_BUS_API_IEVENTEXCEPTIONHANDLER_H
