// Port of net.neoforged.neoforge.internal.RegistrationEvents.
// The init task CommonModLoader.load() runs between the setup events.

#ifndef MATTICRAFT_NEOFORGE_INTERNAL_REGISTRATIONEVENTS_H
#define MATTICRAFT_NEOFORGE_INTERNAL_REGISTRATIONEVENTS_H

// Java: static void init() - passed to ModLoader.runInitTask as a Runnable
void LIBMATTI_NEOFORGE_RegistrationEvents_Init(void *userdata);

// Java: public static void modifyComponents()
void LIBMATTI_NEOFORGE_RegistrationEvents_ModifyComponents(void);

// Java: public static boolean canModifyComponents()
int LIBMATTI_NEOFORGE_RegistrationEvents_CanModifyComponents(void);

#endif //MATTICRAFT_NEOFORGE_INTERNAL_REGISTRATIONEVENTS_H
