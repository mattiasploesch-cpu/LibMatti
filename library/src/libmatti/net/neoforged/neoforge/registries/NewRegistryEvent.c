// Port of net.neoforged.neoforge.registries.NewRegistryEvent.

#include "libmatti/net/neoforged/neoforge/registries/NewRegistryEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>

LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.registries.NewRegistryEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public NewRegistryEvent()
LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_New(void)
{
    LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *event =
        calloc(1, sizeof(LIBMATTI_NEOFORGE_Registries_NewRegistryEvent));
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Class());
    return event;
}

void LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Free(LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *event)
{
    free(event);
}
