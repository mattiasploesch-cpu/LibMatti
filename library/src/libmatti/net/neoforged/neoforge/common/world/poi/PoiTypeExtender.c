// Port of net.neoforged.neoforge.common.world.poi.PoiTypeExtender.
// Java fires ExtendPoiTypesEvent and the callback maps BlockStates to PoiType holders; the
// BlockState/PoiType maps are the game port's part, so the port fires the event shell (mods
// receive it through the mod bus) without the vanilla map access.

#include "libmatti/net/neoforged/neoforge/common/world/poi/PoiTypeExtender.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>

// Java: ExtendPoiTypesEvent extends Event implements IModBusEvent
typedef struct
{
    LIBMATTI_BUS_Event base;
} ExtendPoiTypesEvent;

static LIBMATTI_JL_Class *extend_poi_types_event_class(void)
{
    static LIBMATTI_JL_Class *clazz = NULL;
    if (clazz == NULL)
    {
        clazz = LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.event.level.poi.ExtendPoiTypesEvent", NULL,
                                                LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
        LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    }
    return clazz;
}

// Java: public static void extendPoiTypes()
void LIBMATTI_NEOFORGE_PoiTypeExtender_ExtendPoiTypes(void)
{
    // Java: ModLoader.postEvent(new ExtendPoiTypesEvent(PoiTypeExtender::register))
    ExtendPoiTypesEvent *event = calloc(1, sizeof(ExtendPoiTypesEvent));
    LIBMATTI_BUS_Event_Init(&event->base, extend_poi_types_event_class());
    LIBMATTI_FML_ModLoader_PostEvent(&event->base);
    free(event);
}
