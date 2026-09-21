// Port of net.neoforged.fml.event.lifecycle.ModLifecycleEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/ModLifecycleEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

// Java: ModLifecycleEvent.class - the class implements IModBusEvent
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.ModLifecycleEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(),
                                        LIBMATTI_JL_Modifier_PUBLIC | LIBMATTI_JL_Modifier_ABSTRACT);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public ModLifecycleEvent(ModContainer container)
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Init(LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
                                                        LIBMATTI_FML_ModContainer *container)
{
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Class());
    event->container = container;
}

// Java: public ModContainer getContainer()
LIBMATTI_FML_ModContainer *LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetContainer(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event)
{
    return event->container;
}

// Java: public Stream<InterModComms.IMCMessage> getIMCStream()
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetIMCStream(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *userdata)
{
    // Java: return InterModComms.getMessages(this.container.getModId());
    LIBMATTI_FML_InterModComms_GetMessagesAll(LIBMATTI_FML_ModContainer_GetModId(event->container), consumer,
                                              userdata);
}

// Java: public Stream<InterModComms.IMCMessage> getIMCStream(Predicate<String> methodFilter)
void LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetIMCStreamFiltered(
    const LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent *event,
    int (*methodFilter)(const char *method, void *userdata), void *filterUserdata,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *userdata)
{
    // Java: return InterModComms.getMessages(this.container.getModId(), methodFilter);
    LIBMATTI_FML_InterModComms_GetMessages(LIBMATTI_FML_ModContainer_GetModId(event->container), methodFilter,
                                           filterUserdata, consumer, userdata);
}
