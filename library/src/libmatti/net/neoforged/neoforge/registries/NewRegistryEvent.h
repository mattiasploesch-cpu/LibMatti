// Port of net.neoforged.neoforge.registries.NewRegistryEvent.
// Java: public class NewRegistryEvent extends Event implements IModBusEvent - mods register
// their own registries through it. The port records the created registry names; the actual
// Registry objects are the game port's part (BuiltInRegistries lives in Minecraft).

#ifndef MATTICRAFT_NEOFORGE_REGISTRIES_NEWREGISTRYEVENT_H
#define MATTICRAFT_NEOFORGE_REGISTRIES_NEWREGISTRYEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Registries_NewRegistryEvent LIBMATTI_NEOFORGE_Registries_NewRegistryEvent;

// Java: public class NewRegistryEvent extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Registries_NewRegistryEvent
{
    LIBMATTI_BUS_Event base;
};

// Java: NewRegistryEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Class(void);

// Java: public NewRegistryEvent()
LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_New(void);
void LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Free(LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *event);

#endif //MATTICRAFT_NEOFORGE_REGISTRIES_NEWREGISTRYEVENT_H
