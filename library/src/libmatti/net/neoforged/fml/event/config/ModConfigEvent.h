// Port of net.neoforged.fml.event.config.ModConfigEvent.
// Java: public class ModConfigEvent extends Event implements IModBusEvent
//       with the subclasses Loading, Reloading and Unloading.

#ifndef MATTICRAFT_FML_EVENT_CONFIG_MODCONFIGEVENT_H
#define MATTICRAFT_FML_EVENT_CONFIG_MODCONFIGEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_FML_ModConfig LIBMATTI_FML_ModConfig;

// Java: public class ModConfigEvent extends Event implements IModBusEvent
typedef struct LIBMATTI_FML_Event_Config_ModConfigEvent LIBMATTI_FML_Event_Config_ModConfigEvent;

struct LIBMATTI_FML_Event_Config_ModConfigEvent
{
    // Java: extends Event implements IModBusEvent
    LIBMATTI_BUS_Event base;
    // Java: private final ModConfig config
    LIBMATTI_FML_ModConfig *config;
};

// Java: ModConfigEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Class(void);
// Java: ModConfigEvent.Loading.class / Reloading.class / Unloading.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_Class(void);
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_Class(void);
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_Class(void);

// Java: public ModConfigEvent(ModConfig config) / public Loading(ModConfig config) / ...
LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Loading_New(
    LIBMATTI_FML_ModConfig *config);
LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Reloading_New(
    LIBMATTI_FML_ModConfig *config);
LIBMATTI_FML_Event_Config_ModConfigEvent *LIBMATTI_FML_Event_Config_ModConfigEvent_Unloading_New(
    LIBMATTI_FML_ModConfig *config);
void LIBMATTI_FML_Event_Config_ModConfigEvent_Free(LIBMATTI_FML_Event_Config_ModConfigEvent *event);

// Java: public ModConfig getConfig()
LIBMATTI_FML_ModConfig *LIBMATTI_FML_Event_Config_ModConfigEvent_GetConfig(
    const LIBMATTI_FML_Event_Config_ModConfigEvent *event);

#endif //MATTICRAFT_FML_EVENT_CONFIG_MODCONFIGEVENT_H
