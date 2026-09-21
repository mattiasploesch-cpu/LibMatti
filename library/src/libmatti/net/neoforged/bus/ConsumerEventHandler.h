// Port of net.neoforged.bus.ConsumerEventHandler.

#ifndef MATTICRAFT_BUS_CONSUMEREVENTHANDLER_H
#define MATTICRAFT_BUS_CONSUMEREVENTHANDLER_H

#include "libmatti/java/util/function/Consumer.h"
#include "libmatti/java/util/function/Predicate.h"
#include "libmatti/net/neoforged/bus/api/EventListener.h"

// Java: public sealed class ConsumerEventHandler extends EventListener
typedef struct LIBMATTI_BUS_ConsumerEventHandler LIBMATTI_BUS_ConsumerEventHandler;

struct LIBMATTI_BUS_ConsumerEventHandler
{
    // Java: extends EventListener
    LIBMATTI_BUS_EventListener base;
    // Java: protected final Consumer<Event> consumer;
    const LIBMATTI_JU_Consumer *consumer;
};

// Java: public ConsumerEventHandler(Consumer<Event> consumer)
LIBMATTI_BUS_ConsumerEventHandler *LIBMATTI_BUS_ConsumerEventHandler_New(const LIBMATTI_JU_Consumer *consumer);

// Java: public static final class WithPredicate extends ConsumerEventHandler implements IWrapperListener
typedef struct LIBMATTI_BUS_ConsumerEventHandler_WithPredicate LIBMATTI_BUS_ConsumerEventHandler_WithPredicate;

struct LIBMATTI_BUS_ConsumerEventHandler_WithPredicate
{
    // Java: extends ConsumerEventHandler
    LIBMATTI_BUS_ConsumerEventHandler base;
    // Java: private final Predicate<Event> predicate;
    const LIBMATTI_JU_Predicate *predicate;
    // Java: private final EventListener withoutCheck;
    LIBMATTI_BUS_EventListener withoutCheck;
};

// Java: public WithPredicate(Consumer<Event> consumer, Predicate<Event> predicate)
LIBMATTI_BUS_ConsumerEventHandler_WithPredicate *LIBMATTI_BUS_ConsumerEventHandler_WithPredicate_New(
    const LIBMATTI_JU_Consumer *consumer, const LIBMATTI_JU_Predicate *predicate);

#endif //MATTICRAFT_BUS_CONSUMEREVENTHANDLER_H
