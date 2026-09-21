// Port of net.neoforged.neoforge.registries.RegistryManager.
// The vanilla/modded Registry maps live in the game port (BuiltInRegistries is Minecraft);
// the manager keeps the event lifecycle the loader drives: postNewRegistryEvent, initDataMaps,
// and the pending-registry bookkeeping mods use to hand their registries over.

#ifndef MATTICRAFT_NEOFORGE_REGISTRIES_REGISTRYMANAGER_H
#define MATTICRAFT_NEOFORGE_REGISTRIES_REGISTRYMANAGER_H

#include "libmatti/net/neoforged/neoforge/registries/NewRegistryEvent.h"
#include "libmatti/net/neoforged/neoforge/registries/RegisterEvent.h"

// Java: public static void postNewRegistryEvent()
void LIBMATTI_NEOFORGE_RegistryManager_PostNewRegistryEvent(void);

// Java: public static void initDataMaps()
void LIBMATTI_NEOFORGE_RegistryManager_InitDataMaps(void);

// Java: static Set<ResourceLocation> getPendingModdedRegistries - a mod registers a created
// registry through NewRegistryEvent; the port records the names so the "never registered"
// check below has the same inputs
void LIBMATTI_NEOFORGE_RegistryManager_AddPendingModdedRegistry(const char *registryName);
// Java: pendingModdedRegistries.removeIf(BuiltInRegistries.REGISTRY::containsKey) - the port
// clears the pending set like the fill step of NewRegistryEvent does
void LIBMATTI_NEOFORGE_RegistryManager_ClearPendingModdedRegistries(void);

// Java: the vanilla registry keys snapshot (Set.copyOf(BuiltInRegistries.REGISTRY.keySet()))
// - names added between postNewRegistryEvent and the snapshot queries
void LIBMATTI_NEOFORGE_RegistryManager_AddVanillaRegistryKey(const char *registryName);
int LIBMATTI_NEOFORGE_RegistryManager_IsVanillaRegistryKey(const char *registryName);

#endif //MATTICRAFT_NEOFORGE_REGISTRIES_REGISTRYMANAGER_H
