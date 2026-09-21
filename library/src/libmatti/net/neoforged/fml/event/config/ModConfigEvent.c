// Port of net.neoforged.fml.event.config.ModConfigEvent.

#include "libmatti/net/neoforged/fml/event/config/ModConfigEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>

// Java: ModConfigEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.config.ModConfigEvent", NULL,
                                           LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public static class Loading extends ModConfigEvent
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.config.ModConfigEvent$Loading", NULL,
                                           LIBMATTI_FML_Event_Config_ModConfigEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public static class Reloading extends ModConfigEvent
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.config.ModConfigEvent$Reloading", NULL,
                                           LIBMATTI_FML_Event_Config_ModConfigEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public static class Unloading extends ModConfigEvent
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.config.ModConfigEvent$Unloading", NULL,
                                           LIBMATTI_FML_Event_Config_ModConfigEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: ModConfigEvent(ModConfig config) + the subclass constructor running super(config)
static LIBMATTI_FML_Event_Config_ModConfigEvent *new_event(LIBMATTI_FML_ModConfig *config, LIBMATTI_JL_Class *clazz)
{
    LIBMATTI_FML_Event_Config_ModConfigEvent *event = calloc(1, sizeof(*event));
    LIBMATTI_BUS_Event_Init(&event->base, clazz);
    event->config = config;
    return event;
}

LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_New(
    LIBMATTI_FML_ModConfig *config)
{
    return new_event(config, LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_Class());
}

LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_New(
    LIBMATTI_FML_ModConfig *config)
{
    return new_event(config, LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_Class());
}

LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_New(
    LIBMATTI_FML_ModConfig *config)
{
    return new_event(config, LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_Class());
}

void LIBMATTI_FML_Event_Config_ModConfigEvent_Free(LIBMATTI_FML_Event_Config_ModConfigEvent *event)
{
    free(event);
}

LIBMATTI_FML_ModConfig *LIBMATTI_FML_Event_Config_ModConfigEvent_GetConfig(
    const LIBMATTI_FML_Event_Config_ModConfigEvent *event)
{
    return event->config;
}
