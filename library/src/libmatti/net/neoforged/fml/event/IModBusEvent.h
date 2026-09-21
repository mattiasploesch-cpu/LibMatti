// Port of net.neoforged.fml.event.IModBusEvent.
// Java: public interface IModBusEvent {} - the marker interface used as the bus marker type
// (BusBuilder.markerType) and for the instanceof check in AutomaticEventSubscriber.
// The C port keys a marker interface by its Class, like ICancellableEvent.

#ifndef MATTICRAFT_FML_EVENT_IMODBUSEVENT_H
#define MATTICRAFT_FML_EVENT_IMODBUSEVENT_H

#include "libmatti/java/lang/Class.h"

// Java: IModBusEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_IModBusEvent_Class(void);

#endif //MATTICRAFT_FML_EVENT_IMODBUSEVENT_H
