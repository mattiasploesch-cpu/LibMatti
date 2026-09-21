// Port of net.neoforged.neoforge.capabilities.CapabilityHooks.
// registerVanillaProviders / registerFallbackVanillaProviders touch vanilla Blocks, Items and
// Entities - the game port's part. The init lifecycle (double-init guard, the mod event) runs
// exactly like Java.

#include "libmatti/net/neoforged/neoforge/capabilities/CapabilityHooks.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/capabilities/RegisterCapabilitiesEvent.h"

#include <stdlib.h>

// Java: private static boolean initialized
static int initialized = 0;
// Java: private static boolean initFinished
static int initFinished = 0;

// Java: public static void init()
void LIBMATTI_NEOFORGE_CapabilityHooks_Init(void)
{
    // Java: if (initialized) throw new IllegalArgumentException("CapabilityHooks.init() called twice")
    if (initialized)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "IllegalArgumentException: CapabilityHooks.init() called twice");
        return;
    }
    initialized = 1;

    // Java: var event = new RegisterCapabilitiesEvent(); ModLoader.postEventWrapContainerInModOrder(event)
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent *event =
        LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_New();
    LIBMATTI_FML_ModLoader_PostEventWrapContainerInModOrder(&event->base);
    LIBMATTI_NEOFORGE_Capabilities_RegisterCapabilitiesEvent_Free(event);

    initFinished = 1;
    (void) initFinished;
}
