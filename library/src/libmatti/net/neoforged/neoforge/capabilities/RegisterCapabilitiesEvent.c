// Port of net.neoforged.neoforge.capabilities.RegisterCapabilitiesEvent.

#include "libmatti/net/neoforged/neoforge/capabilities/RegisterCapabilitiesEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>
#include <string.h>

static void add_name(char ***names, size_t *count, size_t *capacity, const char *name)
{
    if (*count == *capacity)
    {
        *capacity = *capacity > 0 ? *capacity * 2 : 8;
        *names = realloc(*names, sizeof(char *) * *capacity);
    }
    (*names)[(*count)++] = strdup(name);
}

LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.capabilities.RegisterCapabilitiesEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public RegisterCapabilitiesEvent()
LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *
LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_New(void)
{
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event =
        calloc(1, sizeof(LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent));
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Class());
    return event;
}

void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Register(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event, const char *capability)
{
    add_name(&event->capabilities, &event->capabilityCount, &event->capabilityCapacity, capability);
}

void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_SetProxyable(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event, const char *capability)
{
    add_name(&event->proxyable, &event->proxyableCount, &event->proxyableCapacity, capability);
}

void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Free(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event)
{
    if (event == NULL) return;
    for (size_t i = 0; i < event->capabilityCount; i++)
        free(event->capabilities[i]);
    free(event->capabilities);
    for (size_t i = 0; i < event->proxyableCount; i++)
        free(event->proxyable[i]);
    free(event->proxyable);
    free(event);
}
