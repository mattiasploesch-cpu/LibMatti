





#ifndef MATTICRAFT_NEOFORGE_REGISTRIES_GAMEDATA_H
#define MATTICRAFT_NEOFORGE_REGISTRIES_GAMEDATA_H

#include <stddef.h>

// Java: public static void unfreezeData()
void LIBMATTI_NEOFORGE_GameData_UnfreezeData(void);
// Java: public static void freezeData()
void LIBMATTI_NEOFORGE_GameData_FreezeData(void);
// Java: public static void postRegisterEvents()
void LIBMATTI_NEOFORGE_GameData_PostRegisterEvents(void);

// Java: public static Set<Identifier> getRegistrationOrder()
// Attributes first, then data components, then particle types, then the vanilla order, then the
// modded registries alphabetically (the vanilla ordering comments in Java).
// Out: array of new strings, the caller frees each entry and the array.
char **LIBMATTI_NEOFORGE_GameData_GetRegistrationOrder(size_t *count);



void LIBMATTI_NEOFORGE_GameData_AddRootRegistry(const char *registryName);

#endif
