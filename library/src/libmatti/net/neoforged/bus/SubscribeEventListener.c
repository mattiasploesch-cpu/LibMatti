// Port of net.neoforged.bus.SubscribeEventListener.

#include "libmatti/net/neoforged/bus/SubscribeEventListener.h"

#include "libmatti/net/neoforged/bus/api/ICancellableEvent.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void subscribe_invoke(void *self, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_SubscribeEventListener *listener = self;
    if (listener->handler == NULL) return;

    // Java: if (subInfo.receiveCanceled() || !((ICancellableEvent) event).isCanceled())
    // The cast is safe because the check is removed when the event is not cancellable:
    // ListenerList unwraps this wrapper to the handler in that case.
    if (listener->subInfo.receiveCanceled || !LIBMATTI_BUS_ICancellableEvent_IsCanceled(event))
        LIBMATTI_BUS_EventListener_Invoke(listener->handler, event);
}

static LIBMATTI_BUS_EventListener *subscribe_get_without_check(void *self)
{
    return ((LIBMATTI_BUS_SubscribeEventListener *)self)->handler;
}

static char *subscribe_to_string(void *self)
{
    return strdup(((LIBMATTI_BUS_SubscribeEventListener *)self)->readable);
}

LIBMATTI_BUS_SubscribeEventListener *LIBMATTI_BUS_SubscribeEventListener_New(const char *target,
                                                                             const char *methodName,
                                                                             const char *methodDescriptor,
                                                                             LIBMATTI_BUS_EventListener *handler,
                                                                             LIBMATTI_BUS_SubscribeEvent subInfo)
{
    const char *descriptor = methodDescriptor != NULL ? methodDescriptor : "";

    LIBMATTI_BUS_SubscribeEventListener *listener = calloc(1, sizeof(LIBMATTI_BUS_SubscribeEventListener));
    listener->base.self = listener;
    listener->base.invoke = subscribe_invoke;
    listener->base.getWithoutCheck = subscribe_get_without_check;
    listener->base.toString = subscribe_to_string;
    listener->handler = handler;
    listener->subInfo = subInfo;

    size_t size = strlen(target) + strlen(methodName) + strlen(descriptor) + strlen("@SubscribeEvent: \0") + 4;
    listener->readable = malloc(size);
    snprintf(listener->readable, size, "@SubscribeEvent: %s %s%s", target, methodName, descriptor);

    return listener;
}

// Java: public EventPriority getPriority()
LIBMATTI_BUS_EventPriority LIBMATTI_BUS_SubscribeEventListener_GetPriority(
    const LIBMATTI_BUS_SubscribeEventListener *listener)
{
    return listener->subInfo.priority;
}

void LIBMATTI_BUS_SubscribeEventListener_Free(LIBMATTI_BUS_SubscribeEventListener *listener)
{
    free(listener->readable);
    free(listener);
}
