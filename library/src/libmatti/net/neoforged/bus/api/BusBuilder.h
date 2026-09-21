// Port of net.neoforged.bus.api.BusBuilder.
// The Java setters return the builder for chaining; the C port does the same.
// The handler and the checker are passed by value, which is the C form of Java's field assignment.

#ifndef MATTICRAFT_BUS_API_BUSBUILDER_H
#define MATTICRAFT_BUS_API_BUSBUILDER_H

#include "libmatti/java/lang/Class.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/bus/api/IEventClassChecker.h"
#include "libmatti/net/neoforged/bus/api/IEventExceptionHandler.h"

// Java: public interface BusBuilder
typedef struct LIBMATTI_BUS_BusBuilder LIBMATTI_BUS_BusBuilder;

struct LIBMATTI_BUS_BusBuilder
{
    void *self;

    // Java: BusBuilder setExceptionHandler(IEventExceptionHandler handler)
    LIBMATTI_BUS_BusBuilder *(*setExceptionHandler)(void *self, LIBMATTI_BUS_IEventExceptionHandler handler);
    // Java: BusBuilder startShutdown()
    LIBMATTI_BUS_BusBuilder *(*startShutdown)(void *self);
    // Java: BusBuilder checkTypesOnDispatch()
    LIBMATTI_BUS_BusBuilder *(*checkTypesOnDispatch)(void *self);
    // Java: default BusBuilder markerType(Class<?> markerInterface)
    LIBMATTI_BUS_BusBuilder *(*markerType)(void *self, LIBMATTI_JL_Class *markerInterface);
    // Java: BusBuilder classChecker(IEventClassChecker checker)
    LIBMATTI_BUS_BusBuilder *(*classChecker)(void *self, LIBMATTI_BUS_IEventClassChecker checker);
    // Java: BusBuilder allowPerPhasePost()
    LIBMATTI_BUS_BusBuilder *(*allowPerPhasePost)(void *self);
    // Java: IEventBus build()
    LIBMATTI_BUS_IEventBus *(*build)(void *self);
};

// Java: static BusBuilder builder() { return new BusBuilderImpl(); } - implemented by BusBuilderImpl
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_Builder(void);

// Java: BusBuilder setExceptionHandler(IEventExceptionHandler handler)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_SetExceptionHandler(LIBMATTI_BUS_BusBuilder *builder,
                                                                     LIBMATTI_BUS_IEventExceptionHandler handler);
// Java: BusBuilder startShutdown()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_StartShutdown(LIBMATTI_BUS_BusBuilder *builder);
// Java: BusBuilder checkTypesOnDispatch()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_CheckTypesOnDispatch(LIBMATTI_BUS_BusBuilder *builder);
// Java: default BusBuilder markerType(Class<?> markerInterface)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_MarkerType(LIBMATTI_BUS_BusBuilder *builder,
                                                           LIBMATTI_JL_Class *markerInterface);
// Java: BusBuilder classChecker(IEventClassChecker checker)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_ClassChecker(LIBMATTI_BUS_BusBuilder *builder,
                                                             LIBMATTI_BUS_IEventClassChecker checker);
// Java: BusBuilder allowPerPhasePost()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_AllowPerPhasePost(LIBMATTI_BUS_BusBuilder *builder);
// Java: IEventBus build()
LIBMATTI_BUS_IEventBus *LIBMATTI_BUS_BusBuilder_Build(const LIBMATTI_BUS_BusBuilder *builder);

#endif //MATTICRAFT_BUS_API_BUSBUILDER_H
