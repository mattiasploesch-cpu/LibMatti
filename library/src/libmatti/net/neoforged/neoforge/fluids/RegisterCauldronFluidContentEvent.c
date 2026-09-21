// Port of net.neoforged.neoforge.fluids.RegisterCauldronFluidContentEvent.

#include "libmatti/net/neoforged/neoforge/fluids/RegisterCauldronFluidContentEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.fluids.RegisterCauldronFluidContentEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public RegisterCauldronFluidContentEvent()
LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *
LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_New(void)
{
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event =
        calloc(1, sizeof(LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent));
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Class());
    return event;
}

// Java: public void register(Block block, Fluid fluid, int fluidLevel, IntProperty levelProperty)
void LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Register(
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event, const char *blockName)
{
    if (event->registrationCount == event->registrationCapacity)
    {
        event->registrationCapacity = event->registrationCapacity > 0 ? event->registrationCapacity * 2 : 8;
        event->blocks = realloc(event->blocks, sizeof(char *) * event->registrationCapacity);
    }
    event->blocks[event->registrationCount++] = strdup(blockName);
}

void LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Free(
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event)
{
    if (event == NULL) return;
    for (size_t i = 0; i < event->registrationCount; i++)
        free(event->blocks[i]);
    free(event->blocks);
    free(event);
}
