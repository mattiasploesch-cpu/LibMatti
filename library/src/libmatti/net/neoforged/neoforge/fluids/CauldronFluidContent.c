// Port of net.neoforged.neoforge.fluids.CauldronFluidContent.
// The vanilla registrations touch Blocks/Fluids (game port), so the port fires the event with
// the modded registration window and marks the vanilla cauldron names as registered.

#include "libmatti/net/neoforged/neoforge/fluids/CauldronFluidContent.h"

#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/fluids/RegisterCauldronFluidContentEvent.h"

#include <stdlib.h>

// Java: private static final Map<Block, CauldronFluidContent> CONTENT
static int contentInitialized = 0;

// Java: public static void init()
void LIBMATTI_NEOFORGE_CauldronFluidContent_Init(void)
{
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event =
        LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_New();

    // Java: the vanilla registrations before the event fires
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Register(event, "minecraft:cauldron");
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Register(event, "minecraft:water_cauldron");
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Register(event, "minecraft:lava_cauldron");

    // Java: Modded registrations - ModLoader.postEvent(registerEvent)
    LIBMATTI_FML_ModLoader_PostEvent(&event->base);
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Free(event);

    contentInitialized = 1;
    (void) contentInitialized;
}
