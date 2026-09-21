// Port of net.neoforged.neoforge.network.event.RegisterPayloadHandlersEvent.

#include "libmatti/net/neoforged/neoforge/network/event/RegisterPayloadHandlersEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.network.event.RegisterPayloadHandlersEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public RegisterPayloadHandlersEvent()
LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *
LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_New(void)
{
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event =
        calloc(1, sizeof(LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent));
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Class());
    return event;
}

// Java: public PayloadRegistrar registrar(String version) + registrar.register(...)
void LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Register(
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event, const char *payloadId,
    const char *version, int optional)
{
    if (event->registrationCount == event->registrationCapacity)
    {
        event->registrationCapacity = event->registrationCapacity > 0 ? event->registrationCapacity * 2 : 8;
        event->payloadIds = realloc(event->payloadIds, sizeof(char *) * event->registrationCapacity);
        event->versions = realloc(event->versions, sizeof(char *) * event->registrationCapacity);
        event->optionals = realloc(event->optionals, sizeof(int) * event->registrationCapacity);
    }
    event->payloadIds[event->registrationCount] = strdup(payloadId);
    event->versions[event->registrationCount] = strdup(version);
    event->optionals[event->registrationCount] = optional;
    event->registrationCount++;
}

void LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Free(
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event)
{
    if (event == NULL) return;
    for (size_t i = 0; i < event->registrationCount; i++)
    {
        free(event->payloadIds[i]);
        free(event->versions[i]);
    }
    free(event->payloadIds);
    free(event->versions);
    free(event->optionals);
    free(event);
}
