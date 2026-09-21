// Port of net.neoforged.neoforge.network.registration.NetworkRegistry.

#include "libmatti/net/neoforged/neoforge/network/registration/NetworkRegistry.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/network/event/RegisterPayloadHandlersEvent.h"

#include <stdlib.h>
#include <string.h>

// Java: protected static final Map<...> PAYLOAD_REGISTRATIONS / SERVERBOUND_HANDLERS /
// CLIENTBOUND_HANDLERS - the maps hold the handler objects; the port records the registrations
static char **payloadIds = NULL;
static size_t payloadCount = 0;
static size_t payloadCapacity = 0;

// Java: protected static boolean setup = false
static int setup = 0;

// Java: public static void setup()
void LIBMATTI_NEOFORGE_NetworkRegistry_Setup(void)
{
    // Java: if (setup) throw new IllegalStateException("The network registry can only be setup once.")
    if (setup)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "IllegalStateException: The network registry can only be setup once.");
        return;
    }

    // Java: ModLoader.postEvent(new RegisterPayloadHandlersEvent())
    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent *event =
        LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_New();
    LIBMATTI_FML_ModLoader_PostEvent(&event->base);

    // the registrations the mods handed in stay for the negotiation (game port side)
    for (size_t i = 0; i < event->registrationCount; i++)
    {
        if (payloadCount == payloadCapacity)
        {
            payloadCapacity = payloadCapacity > 0 ? payloadCapacity * 2 : 8;
            payloadIds = realloc(payloadIds, sizeof(char *) * payloadCapacity);
        }
        payloadIds[payloadCount++] = strdup(event->payloadIds[i]);
    }

    LIBMATTI_NEOFORGE_Network_RegisterPayloadHandlersEvent_Free(event);

    setup = 1;
}

// Java: protected static boolean setup
int LIBMATTI_NEOFORGE_NetworkRegistry_IsSetup(void)
{
    return setup;
}

// Java: public static <T, B> void register(...) - "Cannot register payload ... after
// registration phase." when setup already ran
int LIBMATTI_NEOFORGE_NetworkRegistry_Register(const char *payloadId, const char *version, int optional)
{
    (void) version;
    (void) optional;
    if (setup)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "UnsupportedOperationException: Cannot register payload {} after registration phase.",
                                 payloadId);
        return 0;
    }

    if (payloadCount == payloadCapacity)
    {
        payloadCapacity = payloadCapacity > 0 ? payloadCapacity * 2 : 8;
        payloadIds = realloc(payloadIds, sizeof(char *) * payloadCapacity);
    }
    payloadIds[payloadCount++] = strdup(payloadId);
    return 1;
}
