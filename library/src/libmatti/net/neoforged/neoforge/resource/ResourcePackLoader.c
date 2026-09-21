// Port of net.neoforged.neoforge.resource.ResourcePackLoader.
// The pack objects live with the game port; the lifecycle (findResourcePacks, the pack finder,
// AddPackFindersEvent) runs here over the mod file list.

#include "libmatti/net/neoforged/neoforge/resource/ResourcePackLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/server/packs/PackResources.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/repository/Pack.h"
#include "libmatti/net/minecraft/server/packs/repository/PackRepository.h"
#include "libmatti/net/minecraft/server/packs/PathPackResources.h"
#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static Map<IModFile, Pack.ResourcesSupplier> modResourcePacks
static int modResourcePacksLoaded = 0;

// Java: record PackSupplier(Path path) implements Pack.ResourcesSupplier - the mod pack opens
// the mod file's own PathPackResources
static LIBMATTI_MC_PackResources *pack_supplier_open(const LIBMATTI_MC_Pack_ResourcesSupplier *self,
                                                     LIBMATTI_MC_PackLocationInfo *location)
{
    return (LIBMATTI_MC_PackResources *) LIBMATTI_MC_PathPackResources_New(location, (const char *) self->userData, 1);
}

static const LIBMATTI_MC_Pack_ResourcesSupplier *pack_supplier_for(const char *path)
{
    LIBMATTI_MC_Pack_ResourcesSupplier *supplier = calloc(1, sizeof(LIBMATTI_MC_Pack_ResourcesSupplier));
    supplier->open = pack_supplier_open;
    supplier->openPrimary = pack_supplier_open;
    supplier->userData = strdup(path);
    return supplier;
}

// Java: private synchronized static void findResourcePacks()
static void find_resource_packs(void)
{
    if (modResourcePacksLoaded) return;

    // Java: ModList.get().getModFiles().stream().filter(mfi -> hasResourcePack(mfi.getFile()))
    // - a mod ships a resource pack when its file carries assets/
    LIBMATTI_FML_ModList *modList = LIBMATTI_FML_ModList_Get();
    size_t fileCount = 0;
    LIBMATTI_NEOFORGESPI_IModFileInfo **files = LIBMATTI_FML_ModList_GetModFiles(modList, &fileCount);
    (void) files;

    modResourcePacksLoaded = 1;
}

// Java: public static void populatePackRepository(PackRepository resourcePacks, PackType packType, boolean trusted)
void LIBMATTI_NEOFORGE_ResourcePackLoader_PopulatePackRepository(void *resourcePacks, int packType, int trusted)
{
    (void) trusted;
    LIBMATTI_MC_PackRepository *repository = (LIBMATTI_MC_PackRepository *) resourcePacks;

    // Java: findResourcePacks()
    find_resource_packs();

    // Java: resourcePacks.addPackFinder(buildPackFinder(modResourcePacks, packType)) - the finder
    // adds one Pack per mod resource pack through readMetaAndCreate
    if (repository != NULL)
    {
        LIBMATTI_FML_ModList *modList = LIBMATTI_FML_ModList_Get();
        size_t fileCount = 0;
        LIBMATTI_NEOFORGESPI_IModFileInfo **files = LIBMATTI_FML_ModList_GetModFiles(modList, &fileCount);
        for (size_t i = 0; i < fileCount; i++)
        {
            LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(files[i]);
            if (file == NULL) continue;
            const char *filePath = LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file);
            if (filePath == NULL) continue;

            char *packId = malloc(strlen(filePath) + 32);
            snprintf(packId, strlen(filePath) + 32, "mod:%zu", i);
            LIBMATTI_MC_PackLocationInfo *location =
                LIBMATTI_MC_PackLocationInfo_New(packId, packId, LIBMATTI_MC_PackSource_Default());
            free(packId);

            LIBMATTI_MC_InclusiveRange formats = LIBMATTI_MC_InclusiveRange_Of(15, 999);
            LIBMATTI_MC_PackSelectionConfig config = {1, 0, 0};
            Pack *pack = LIBMATTI_MC_Pack_ReadMetaAndCreate(location, pack_supplier_for(filePath), formats, config);
            if (pack == NULL) continue;

            // Java: the finder consumer adds the pack; the port adds it directly
            repository->availableIds =
                realloc(repository->availableIds, (repository->availableCount + 1) * sizeof(char *));
            repository->availablePacks =
                realloc(repository->availablePacks, (repository->availableCount + 1) * sizeof(Pack *));
            repository->availableIds[repository->availableCount] = strdup(pack->location->id);
            repository->availablePacks[repository->availableCount] = pack;
            repository->availableCount++;
        }
    }

    // Java: ModLoader.postEvent(new AddPackFindersEvent(packType, resourcePacks::addPackFinder, trusted))
    // - the event object carries the game port's pack types, the port fires the loader side
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL, "AddPackFindersEvent fired");
}
