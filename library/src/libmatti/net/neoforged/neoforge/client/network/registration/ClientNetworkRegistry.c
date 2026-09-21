// Port of net.neoforged.neoforge.client.network.registration.ClientNetworkRegistry.

#include "libmatti/net/neoforged/neoforge/client/network/registration/ClientNetworkRegistry.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/network/registration/NetworkRegistry.h"

// Java: private static boolean setupClient = false
static int setupClient = 0;

// Java: public static void setup()
void LIBMATTI_NEOFORGE_ClientNetworkRegistry_Setup(void)
{
    // Java: if (!NetworkRegistry.setup) throw new IllegalStateException(...)
    if (!LIBMATTI_NEOFORGE_NetworkRegistry_IsSetup())
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "IllegalStateException: ClientNetworkRegistry cannot be set up before main NetworkRegistry");
        return;
    }
    // Java: if (setupClient) throw new IllegalStateException("The client network registry can only be set up once.")
    if (setupClient)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "IllegalStateException: The client network registry can only be set up once.");
        return;
    }

    // Java: ModLoader.postEvent(new RegisterClientPayloadHandlersEvent()) - the handler maps
    // are the game port's part, the client event fires through the same mod bus
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Client payload handlers registered");

    setupClient = 1;
}

// Java: private static boolean setupClient
int LIBMATTI_NEOFORGE_ClientNetworkRegistry_IsSetup(void)
{
    return setupClient;
}
