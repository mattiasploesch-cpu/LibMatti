// Port of net.neoforged.bus.api.BusBuilder.

#include "libmatti/net/neoforged/bus/api/BusBuilder.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: markerType's lambda: eventType -> { if (!markerInterface.isAssignableFrom(eventType)) throw ... }
// The captured markerInterface is the checker's self.
static void marker_type_check(void *self, LIBMATTI_JL_Class *eventType)
{
    LIBMATTI_JL_Class *markerInterface = self;
    if (LIBMATTI_JL_Class_IsAssignableFrom(markerInterface, eventType)) return;

    // Java: throw new IllegalArgumentException("This bus only accepts subclasses of " + markerInterface + ", which " + eventType + " is not.")
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "This bus only accepts subclasses of {}, which {} is not.",
                             LIBMATTI_JL_Class_GetName(markerInterface), LIBMATTI_JL_Class_GetName(eventType));
}

// Java: BusBuilder setExceptionHandler(IEventExceptionHandler handler)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_SetExceptionHandler(LIBMATTI_BUS_BusBuilder *builder,
                                                                     LIBMATTI_BUS_IEventExceptionHandler handler)
{
    return builder->setExceptionHandler(builder->self, handler);
}

// Java: BusBuilder startShutdown()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_StartShutdown(LIBMATTI_BUS_BusBuilder *builder)
{
    return builder->startShutdown(builder->self);
}

// Java: BusBuilder checkTypesOnDispatch()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_CheckTypesOnDispatch(LIBMATTI_BUS_BusBuilder *builder)
{
    return builder->checkTypesOnDispatch(builder->self);
}

// Java: default BusBuilder markerType(Class<?> markerInterface)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_MarkerType(LIBMATTI_BUS_BusBuilder *builder,
                                                           LIBMATTI_JL_Class *markerInterface)
{
    // Java: if (!markerInterface.isInterface()) throw new IllegalArgumentException("Cannot specify a class marker type")
    if (!LIBMATTI_JL_Class_IsInterface(markerInterface))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cannot specify a class marker type");
        return builder;
    }

    LIBMATTI_BUS_IEventClassChecker checker = {markerInterface, marker_type_check};
    return builder->classChecker(builder->self, checker);
}

// Java: BusBuilder classChecker(IEventClassChecker checker)
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_ClassChecker(LIBMATTI_BUS_BusBuilder *builder,
                                                             LIBMATTI_BUS_IEventClassChecker checker)
{
    return builder->classChecker(builder->self, checker);
}

// Java: BusBuilder allowPerPhasePost()
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_AllowPerPhasePost(LIBMATTI_BUS_BusBuilder *builder)
{
    return builder->allowPerPhasePost(builder->self);
}

// Java: IEventBus build()
LIBMATTI_BUS_IEventBus *LIBMATTI_BUS_BusBuilder_Build(const LIBMATTI_BUS_BusBuilder *builder)
{
    return builder->build(builder->self);
}
