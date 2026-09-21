// Port of net.neoforged.bus.api.IEventExceptionHandler.

#include "libmatti/net/neoforged/bus/api/IEventExceptionHandler.h"

// Java: void handleException(IEventBus bus, Event event, EventListener[] listeners, int index, Throwable throwable)
void LIBMATTI_BUS_IEventExceptionHandler_HandleException(const LIBMATTI_BUS_IEventExceptionHandler *handler,
                                                         LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event,
                                                         LIBMATTI_BUS_EventListener **listeners, size_t listenerCount,
                                                         int index, LIBMATTI_JL_Throwable *throwable)
{
    handler->handleException(handler->self, bus, event, listeners, listenerCount, index, throwable);
}
