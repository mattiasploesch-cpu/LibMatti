// Port of net.neoforged.bus.ConsumerEventHandler.

#include "libmatti/net/neoforged/bus/ConsumerEventHandler.h"

#include <stdlib.h>
#include <string.h>

static void consumer_invoke(void *self, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_ConsumerEventHandler *handler = self;
    handler->consumer->accept(handler->consumer->self, event);
}

// Java: public String toString() { return consumer.toString(); }
// The erased C functional interface has no toString of its own, so the port names the wrapper.
static char *consumer_to_string(void *self)
{
    (void)self;
    return strdup("net.neoforged.bus.ConsumerEventHandler");
}

LIBMATTI_BUS_ConsumerEventHandler *LIBMATTI_BUS_ConsumerEventHandler_New(const LIBMATTI_JU_Consumer *consumer)
{
    LIBMATTI_BUS_ConsumerEventHandler *handler = calloc(1, sizeof(LIBMATTI_BUS_ConsumerEventHandler));
    handler->base.self = handler;
    handler->base.invoke = consumer_invoke;
    handler->base.toString = consumer_to_string;
    handler->consumer = consumer;
    return handler;
}

static void with_predicate_invoke(void *self, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_ConsumerEventHandler_WithPredicate *handler = self;
    if (handler->predicate->test(handler->predicate->self, event))
        handler->base.consumer->accept(handler->base.consumer->self, event);
}

static LIBMATTI_BUS_EventListener *with_predicate_get_without_check(void *self)
{
    LIBMATTI_BUS_ConsumerEventHandler_WithPredicate *handler = self;
    return &handler->withoutCheck;
}

// Java: ConsumerEventHandler does not override toString() for WithPredicate
static char *with_predicate_to_string(void *self)
{
    (void)self;
    return strdup("net.neoforged.bus.ConsumerEventHandler");
}

LIBMATTI_BUS_ConsumerEventHandler_WithPredicate *LIBMATTI_BUS_ConsumerEventHandler_WithPredicate_New(
    const LIBMATTI_JU_Consumer *consumer, const LIBMATTI_JU_Predicate *predicate)
{
    LIBMATTI_BUS_ConsumerEventHandler_WithPredicate *handler =
        calloc(1, sizeof(LIBMATTI_BUS_ConsumerEventHandler_WithPredicate));

    handler->base.base.self = handler;
    handler->base.base.invoke = with_predicate_invoke;
    handler->base.base.getWithoutCheck = with_predicate_get_without_check;
    handler->base.base.toString = with_predicate_to_string;
    handler->base.consumer = consumer;
    handler->predicate = predicate;

    // Java: this.withoutCheck = new ConsumerEventHandler(consumer);
    handler->withoutCheck.self = &handler->base;
    handler->withoutCheck.invoke = consumer_invoke;
    handler->withoutCheck.toString = consumer_to_string;

    return handler;
}
