// Port of net.neoforged.fml.event.IModBusEvent.

#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

// Java: IModBusEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_IModBusEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.IModBusEvent", NULL, NULL,
                                           LIBMATTI_JL_Modifier_INTERFACE | LIBMATTI_JL_Modifier_ABSTRACT);
}
