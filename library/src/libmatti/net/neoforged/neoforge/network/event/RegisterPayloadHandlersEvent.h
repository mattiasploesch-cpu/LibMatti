// Port of net.neoforged.neoforge.network.event.RegisterPayloadHandlersEvent.

#ifndef MATTICRAFT_NEOFORGE_NETWORK_EVENT_REGISTERPAYLOADHANDLENSEVENT_H
#define MATTICRAFT_NEOFORGE_NETWORK_EVENT_REGISTERPAYLOADHANDLENSEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent;

// Java: public class RegisterPayloadHandlersEvent extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent
{
    LIBMATTI_BUS_Event base;
    // Java: registrar(String version) hands out PayloadRegistrations; the port records the
    // (payload id, version, optional) tuples the registrar collected
    char **payloadIds;
    char **versions;
    int *optionals;
    size_t registrationCount;
    size_t registrationCapacity;
};

// Java: RegisterPayloadHandlersEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Class(void);

// Java: public RegisterPayloadHandlersEvent()
LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *
LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_New(void);
void LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Free(
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event);

// Java: public PayloadRegistrar registrar(String version) - the registrations the registrar makes
void LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Register(
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event, const char *payloadId,
    const char *version, int optional);

#endif //MATTICRAFT_NEOFORGE_NETWORK_EVENT_REGISTERPAYLOADHANDLENSEVENT_H
