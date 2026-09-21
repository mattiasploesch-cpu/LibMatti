// Port of net.neoforged.neoforge.registries.ModifyRegistriesEvent.

#ifndef MATTICRAFT_NEOFORGE_REGISTRIES_MODIFYREGISTRIESEVENT_H
#define MATTICRAFT_NEOFORGE_REGISTRIES_MODIFYREGISTRIESEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent;

// Java: public class ModifyRegistriesEvent extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent
{
    LIBMATTI_BUS_Event base;
};

// Java: ModifyRegistriesEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent_Class(void);

// Java: public ModifyRegistriesEvent()
LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent *LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent_New(void);
void LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent_Free(LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent *event);

#endif //MATTICRAFT_NEOFORGE_REGISTRIES_MODIFYREGISTRIESEVENT_H
