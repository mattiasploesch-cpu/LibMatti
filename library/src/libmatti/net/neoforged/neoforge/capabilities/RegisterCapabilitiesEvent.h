// Port of net.neoforged.neoforge.capabilities.RegisterCapabilitiesEvent.
// The capability maps (Block/Entity/Item -> provider) live with the game port; the port keeps
// the event shell and the registrations the mods hand in (the capability string keys).

#ifndef MATTICRAFT_NEOFORGE_CAPABILITIES_REGISTERCAPABILITIESEVENT_H
#define MATTICRAFT_NEOFORGE_CAPABILITIES_REGISTERCAPABILITIESEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent;

// Java: public class RegisterCapabilitiesEvent extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent
{
    LIBMATTI_BUS_Event base;
    // Java: the registered (capability, target) pairs - the port keeps the names
    char **capabilities;
    size_t capabilityCount;
    size_t capabilityCapacity;
    // Java: setProxyable(...) - the proxyable capability names
    char **proxyable;
    size_t proxyableCount;
    size_t proxyableCapacity;
};

// Java: RegisterCapabilitiesEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Class(void);

// Java: public RegisterCapabilitiesEvent()
LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *
LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_New(void);
void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Free(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event);

// Java: public <T, C> void register(...) - the provider object stays with the game port
void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Register(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event, const char *capability);
// Java: public <T> void setProxyable(...)
void LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_SetProxyable(
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event, const char *capability);

#endif //MATTICRAFT_NEOFORGE_CAPABILITIES_REGISTERCAPABILITIESEVENT_H
