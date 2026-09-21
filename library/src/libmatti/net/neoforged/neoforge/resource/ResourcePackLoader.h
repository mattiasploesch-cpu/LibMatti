// Port of net.neoforged.neoforge.resource.ResourcePackLoader.
// The PackRepository/Pack objects belong to the game port; the port keeps the lifecycle:
// discovering the mods that ship assets/, firing AddPackFindersEvent, and reporting the
// mod resource packs the game port will mount.

#ifndef MATTICRAFT_NEOFORGE_RESOURCE_RESOURCEPACKLOADER_H
#define MATTICRAFT_NEOFORGE_RESOURCE_RESOURCEPACKLOADER_H

// Java: public static void populatePackRepository(PackRepository resourcePacks, PackType packType, boolean trusted)
// 'resourcePacks' is the game port's PackRepository (opaque).
void LIBMATTI_NEOFORGE_ResourcePackLoader_PopulatePackRepository(void *resourcePacks, int packType, int trusted);

#endif //MATTICRAFT_NEOFORGE_RESOURCE_RESOURCEPACKLOADER_H
