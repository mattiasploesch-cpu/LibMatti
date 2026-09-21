// Port of net.neoforged.neoforge.registries.GameData.
// The registry freeze/snapshot machinery and the per-registry RegisterEvent dispatch run exactly
// like Java; the MappedRegistry objects and their entries are the game port's part, so the port
// tracks the registry names and the registration order instead of Registry objects.

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

// the registries the game port has declared (BuiltInRegistries.REGISTRY keys)
void LIBMATTI_NEOFORGE_GameData_AddRootRegistry(const char *registryName);

#endif //MATTICRAFT_NEOFORGE_REGISTRIES_GAMEDATA_H
