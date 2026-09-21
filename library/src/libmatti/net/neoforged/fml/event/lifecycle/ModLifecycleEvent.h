// Port of net.neoforged.fml.event.lifecycle.ModLifecycleEvent.
// Java's Stream<IMCMessage> becomes a consumer callback (see InterModComms).

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_MODLIFECYCLEEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_MODLIFECYCLEEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/fml/InterModComms.h"

typedef struct LIBMATTI_FML_ModContainer LIBMATTI_FML_ModContainer;

// Java: public abstract class ModLifecycleEvent extends Event implements IModBusEvent
typedef struct LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent;

struct LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent
{
    // Java: extends Event implements IModBusEvent
    LIBMATTI_BUS_Event base;
    // Java: private final ModContainer container;
    LIBMATTI_FML_ModContainer *container;
};

// Java: ModLifecycleEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Class(void);

// Java: public ModLifecycleEvent(ModContainer container)
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Init(LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
                                                        LIBMATTI_FML_ModContainer *container);

// Java: public ModContainer getContainer()
LIBMATTI_FML_ModContainer *LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetContainer(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event);

// Java: public Stream<InterModComms.IMCMessage> getIMCStream()
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetIMCStream(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *userdata);
// Java: public Stream<InterModComms.IMCMessage> getIMCStream(Predicate<String> methodFilter)
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetIMCStreamFiltered(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
    int (*methodFilter)(const char *method, void *userdata), void *filterUserdata,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *userdata);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_MODLIFECYCLEEVENT_H
