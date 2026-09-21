// Port of net.neoforged.fml.javafmlmod.AutomaticEventSubscriber.
// Java scans a class's declared methods for @SubscribeEvent through reflection; C has neither
// annotations nor reflection, so a mod registers its handler methods and the subscriber dispatches
// them exactly like Java does: IModBusEvent events go to the mod's bus, everything else to the game bus.

#ifndef MATTICRAFT_FML_JAVAFMLMOD_AUTOMATICEVENTSUBSCRIBER_H
#define MATTICRAFT_FML_JAVAFMLMOD_AUTOMATICEVENTSUBSCRIBER_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/bus/api/SubscribeEvent.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"

#include <stddef.h>

// Java: private static final Type AUTO_SUBSCRIBER = Type.getType(EventBusSubscriber.class);
#define LIBMATTI_FML_AutomaticEventSubscriber_EVENTBUSSUBSCRIBER "net.neoforged.fml.common.EventBusSubscriber"
// Java: private static final Type MOD_TYPE = Type.getType(Mod.class);
#define LIBMATTI_FML_AutomaticEventSubscriber_MOD "net.neoforged.fml.common.Mod"

// Java: the @SubscribeEvent method itself (Method + the annotation's values)
// The handler's self is the class name the method was registered for.
void LIBMATTI_FML_AutomaticEventSubscriber_RegisterMethod(
    const char *className, const char *methodName, int isStatic, LIBMATTI_JL_Class *eventType,
    LIBMATTI_BUS_SubscribeEvent subInfo, void (*callback)(void *self, LIBMATTI_BUS_Event *event));

// Java: public static void inject(ModContainer mod, ModFileScanData scanData, Module layer)
// Java passes the java.lang.Module the entrypoints were loaded from; the C port keeps its name.
void LIBMATTI_FML_AutomaticEventSubscriber_Inject(LIBMATTI_FML_ModContainer *mod,
                                                  LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                  const char *moduleName);

// Java: public static EnumSet<Dist> getSides(Object data)
// Java takes the annotation value (a List<EnumHolder>); the port takes the string the scan data
// carries and returns the number of sides written. NULL means the annotation had no value; when the
// result would not fit in 'sides' nothing is written.
size_t LIBMATTI_FML_AutomaticEventSubscriber_GetSides(const char *data, LIBMATTI_DIST_Dist *sides, size_t capacity);

#endif //MATTICRAFT_FML_JAVAFMLMOD_AUTOMATICEVENTSUBSCRIBER_H
