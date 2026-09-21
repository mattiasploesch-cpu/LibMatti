// Port of net.neoforged.fml.ModList.
// Java's Stream/Function/BiConsumer results become callbacks with userdata. Java's
// completableFutureFromExceptionList/gather (the dispatch helpers) run through the
// java.util.concurrent.CompletableFuture port.

#ifndef MATTICRAFT_FML_MODLIST_H
#define MATTICRAFT_FML_MODLIST_H

#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stddef.h>

// Java: private final Map<String, ModFileInfo> fileById
typedef struct
{
    char *modId;
    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo;
} LIBMATTI_FML_ModList_ModFileById;

// Java: private Map<String, ModContainer> indexedMods
typedef struct
{
    char *modId;
    LIBMATTI_FML_ModContainer *container;
} LIBMATTI_FML_ModList_IndexedMod;

// Java: public class ModList
typedef struct LIBMATTI_FML_ModList LIBMATTI_FML_ModList;

struct LIBMATTI_FML_ModList
{
    // Java: private final List<IModFileInfo> modFiles
    LIBMATTI_NEOFORGESPI_IModFileInfo **modFiles;
    size_t modFileCount;
    // Java: private final List<IModInfo> sortedList
    LIBMATTI_NEOFORGESPI_IModInfo **sortedList;
    size_t sortedCount;
    LIBMATTI_FML_ModList_ModFileById *fileById;
    size_t fileByIdCount;
    // Java: private List<ModContainer> mods
    LIBMATTI_FML_ModContainer **mods;
    size_t modCount;
    LIBMATTI_FML_ModList_IndexedMod *indexedMods;
    size_t indexedModCount;
    // Java: private List<ModFileScanData> modFileScanData
    LIBMATTI_NEOFORGESPI_ModFileScanData **modFileScanData;
    size_t modFileScanDataCount;
    // Java: private List<ModContainer> sortedContainers
    LIBMATTI_FML_ModContainer **sortedContainers;
    size_t sortedContainerCount;
};

// Java: java.util.function.Consumer<IModFile> / Function<IModFile, T>
typedef void (*LIBMATTI_FML_ModList_ModFileConsumer)(LIBMATTI_NEOFORGESPI_IModFile *file, void *userdata);
// Java: java.util.function.Consumer<ModContainer> / Function<ModContainer, T>
typedef void (*LIBMATTI_FML_ModList_ModContainerConsumer)(LIBMATTI_FML_ModContainer *container, void *userdata);
// Java: java.util.function.BiConsumer<String, ModContainer>
typedef void (*LIBMATTI_FML_ModList_ModContainerBiConsumer)(const char *modId, LIBMATTI_FML_ModContainer *container,
                                                           void *userdata);

// Java: public static ModList of(List<ModFile> modFiles, List<ModInfo> sortedList)
LIBMATTI_FML_ModList *LIBMATTI_FML_ModList_Of(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
                                             LIBMATTI_FML_ModInfo **sortedList, size_t sortedCount);
// Java: public static ModList get()
LIBMATTI_FML_ModList *LIBMATTI_FML_ModList_Get(void);

// Java: private String fileToLine(IModFile mf)
char *LIBMATTI_FML_ModList_FileToLine(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: private String crashReport()
char *LIBMATTI_FML_ModList_CrashReport(LIBMATTI_FML_ModList *modList);

// Java: public List<IModFileInfo> getModFiles()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_FML_ModList_GetModFiles(const LIBMATTI_FML_ModList *modList,
                                                                    size_t *count);
// Java: public IModFileInfo getModFileById(String modid)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModList_GetModFileById(const LIBMATTI_FML_ModList *modList,
                                                                      const char *modid);

// Java: void setLoadedMods(List<ModContainer> modContainers)
void LIBMATTI_FML_ModList_SetLoadedMods(LIBMATTI_FML_ModList *modList, LIBMATTI_FML_ModContainer **containers,
                                       size_t count);

// Java: public Optional<? extends ModContainer> getModContainerById(String modId) - NULL when absent
LIBMATTI_FML_ModContainer *LIBMATTI_FML_ModList_GetModContainerById(const LIBMATTI_FML_ModList *modList,
                                                                   const char *modId);
// Java: public List<IModInfo> getMods()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_FML_ModList_GetMods(const LIBMATTI_FML_ModList *modList, size_t *count);
// Java: public boolean isLoaded(String modTarget)
int LIBMATTI_FML_ModList_IsLoaded(const LIBMATTI_FML_ModList *modList, const char *modTarget);
// Java: public int size()
int LIBMATTI_FML_ModList_Size(const LIBMATTI_FML_ModList *modList);
// Java: public List<ModFileScanData> getAllScanData()
LIBMATTI_NEOFORGESPI_ModFileScanData **LIBMATTI_FML_ModList_GetAllScanData(LIBMATTI_FML_ModList *modList,
                                                                          size_t *count);

// Java: public void forEachModFile(Consumer<IModFile> fileConsumer)
void LIBMATTI_FML_ModList_ForEachModFile(const LIBMATTI_FML_ModList *modList,
                                         LIBMATTI_FML_ModList_ModFileConsumer fileConsumer, void *userdata);
// Java: public <T> Stream<T> applyForEachModFile(Function<IModFile, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModFile(const LIBMATTI_FML_ModList *modList,
                                              LIBMATTI_FML_ModList_ModFileConsumer function, void *userdata);
// Java: public <T> Stream<T> applyForEachModFileAlphabetical(Function<IModFile, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModFileAlphabetical(const LIBMATTI_FML_ModList *modList,
                                                         LIBMATTI_FML_ModList_ModFileConsumer function,
                                                         void *userdata);

// Java: public void forEachModContainer(BiConsumer<String, ModContainer> modContainerConsumer)
void LIBMATTI_FML_ModList_ForEachModContainer(const LIBMATTI_FML_ModList *modList,
                                              LIBMATTI_FML_ModList_ModContainerBiConsumer modContainerConsumer,
                                              void *userdata);
// Java: public List<ModContainer> getSortedMods()
LIBMATTI_FML_ModContainer **LIBMATTI_FML_ModList_GetSortedMods(const LIBMATTI_FML_ModList *modList, size_t *count);
// Java: public void forEachModInOrder(Consumer<ModContainer> containerConsumer)
void LIBMATTI_FML_ModList_ForEachModInOrder(const LIBMATTI_FML_ModList *modList,
                                            LIBMATTI_FML_ModList_ModContainerConsumer containerConsumer,
                                            void *userdata);
// Java: public <T> Stream<T> applyForEachModContainer(Function<ModContainer, T> function)
void LIBMATTI_FML_ModList_ApplyForEachModContainer(const LIBMATTI_FML_ModList *modList,
                                                  LIBMATTI_FML_ModList_ModContainerConsumer function, void *userdata);

// Java: @ApiStatus.Internal public static void clear()
void LIBMATTI_FML_ModList_Clear(void);

#endif //MATTICRAFT_FML_MODLIST_H
