// Port of net.neoforged.neoforge.fluids.RegisterCauldronFluidContentEvent.

#ifndef MATTICRAFT_NEOFORGE_FLUIDS_REGISTERCAULDRONFLUIDCONTENTEVENT_H
#define MATTICRAFT_NEOFORGE_FLUIDS_REGISTERCAULDRONFLUIDCONTENTEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent;

// Java: public class RegisterCauldronFluidContentEvent extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent
{
    LIBMATTI_BUS_Event base;
    // Java: the registered (block, fluid) mappings - the port keeps the block names
    char **blocks;
    size_t registrationCount;
    size_t registrationCapacity;
};

// Java: RegisterCauldronFluidContentEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Class(void);

// Java: public RegisterCauldronFluidContentEvent()
LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *
LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_New(void);
void LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Free(
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event);

// Java: public void register(Block block, Fluid fluid, int fluidLevel, ...)
void LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent_Register(
    LIBMATTI_NEOFORGE_Fluids_RegisterCauldronFluidContentEvent *event, const char *blockName);

#endif //MATTICRAFT_NEOFORGE_FLUIDS_REGISTERCAULDRONFLUIDCONTENTEVENT_H
