// Port of net.neoforged.neoforge.common.NeoForge.

#include "libmatti/net/neoforged/neoforge/common/NeoForge.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/bus/BusBuilderImpl.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>

// Java: the class checker lambda - IModBusEvents are rejected on the game bus
static void reject_imod_bus_events(void *self, LIBMATTI_JL_Class *eventType)
{
    (void) self;
    if (!LIBMATTI_JL_Class_IsAssignableFrom(LIBMATTI_FML_Event_IModBusEvent_Class(), eventType)) return;

    // Java: throw new IllegalArgumentException(
    //     "IModBusEvent events are not allowed on the common NeoForge bus! Use a mod bus instead.")
    LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                             "IModBusEvent events are not allowed on the common NeoForge bus! Use a mod bus instead.");
}

// Java: public static final IEventBus EVENT_BUS = BusBuilder.builder().startShutdown().classChecker(...).build()
LIBMATTI_BUS_IEventBus *LIBMATTI_NEOFORGE_NeoForge_EVENT_BUS(void)
{
    static LIBMATTI_BUS_IEventBus *eventBus = NULL;
    if (eventBus != NULL) return eventBus;

    // The bus lives for the process lifetime, like Java's static; the builder itself is dropped.
    LIBMATTI_BUS_BusBuilderImpl *builder = LIBMATTI_BUS_BusBuilderImpl_New();
    LIBMATTI_BUS_BusBuilder_StartShutdown(&builder->base);

    LIBMATTI_BUS_IEventClassChecker checker = {NULL, reject_imod_bus_events};
    LIBMATTI_BUS_BusBuilder_ClassChecker(&builder->base, checker);

    eventBus = LIBMATTI_BUS_BusBuilder_Build(&builder->base);
    LIBMATTI_BUS_BusBuilderImpl_Free(builder);
    return eventBus;
}
