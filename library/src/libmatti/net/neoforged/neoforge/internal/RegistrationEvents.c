// Port of net.neoforged.neoforge.internal.RegistrationEvents.

#include "libmatti/net/neoforged/neoforge/internal/RegistrationEvents.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/neoforge/attachment/ForcedChunkManager.h"
#include "libmatti/net/neoforged/neoforge/capabilities/CapabilityHooks.h"
#include "libmatti/net/neoforged/neoforge/common/world/poi/PoiTypeExtender.h"
#include "libmatti/net/neoforged/neoforge/fluids/CauldronFluidContent.h"
#include "libmatti/net/neoforged/neoforge/registries/RegistryManager.h"

#include <stdlib.h>

// Java: private static boolean canModifyComponents;
static int canModifyComponents = 0;

// Java: the event ModifyDefaultComponentsEvent extends Event; the port models it inline because the
// game port owns the real component system.
typedef struct
{
    LIBMATTI_BUS_Event base;
} ModifyDefaultComponentsEvent;

static LIBMATTI_JL_Class *modify_default_components_event_class(void)
{
    static LIBMATTI_JL_Class *clazz = NULL;
    if (clazz == NULL)
        clazz = LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.event.ModifyDefaultComponentsEvent", NULL,
                                                LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    return clazz;
}

// Java: public static void modifyComponents()
void LIBMATTI_NEOFORGE_RegistrationEvents_ModifyComponents(void)
{
    canModifyComponents = 1;
    ModifyDefaultComponentsEvent *event = calloc(1, sizeof(ModifyDefaultComponentsEvent));
    LIBMATTI_BUS_Event_Init(&event->base, modify_default_components_event_class());
    LIBMATTI_FML_ModLoader_PostEvent(&event->base);
    free(event);
    canModifyComponents = 0;
}

// Java: static void init()
void LIBMATTI_NEOFORGE_RegistrationEvents_Init(void *userdata)
{
    (void) userdata;
    // Java order, kept 1:1:
    //   CauldronFluidContent.init();       // must be before capability event
    //   CapabilityHooks.init();            // must be after cauldron event
    //   ForcedChunkManager.init();
    //   RegistryManager.initDataMaps();
    //   modifyComponents();
    //   PoiTypeExtender.extendPoiTypes();
    LIBMATTI_NEOFORGE_CauldronFluidContent_Init();
    LIBMATTI_NEOFORGE_CapabilityHooks_Init();
    LIBMATTI_NEOFORGE_ForcedChunkManager_Init();
    LIBMATTI_NEOFORGE_RegistryManager_InitDataMaps();
    LIBMATTI_NEOFORGE_RegistrationEvents_ModifyComponents();
    LIBMATTI_NEOFORGE_PoiTypeExtender_ExtendPoiTypes();
}

// Java: public static boolean canModifyComponents()
int LIBMATTI_NEOFORGE_RegistrationEvents_CanModifyComponents(void)
{
    return canModifyComponents;
}
