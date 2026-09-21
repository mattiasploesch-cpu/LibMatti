// Port of net.neoforged.fml.loading.LoadingModList.
// Java's IdentityHashMap folds into the C port's pointer arrays: the mod files are
// compared by pointer identity, which is the same semantics.

#ifndef MATTICRAFT_FML_LOADING_LOADINGMODLIST_H
#define MATTICRAFT_FML_LOADING_LOADINGMODLIST_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"

#include <stddef.h>

// Java: Map<ModInfo, List<ModInfo>> modDependencies
typedef struct
{
    LIBMATTI_FML_ModInfo **keys;
    LIBMATTI_FML_ModInfo ***values;
    size_t *valueCounts;
    size_t count;
} LIBMATTI_FML_ModDependencyMap;

// Java: public class LoadingModList
typedef struct LIBMATTI_FML_LoadingModList LIBMATTI_FML_LoadingModList;

struct LIBMATTI_FML_LoadingModList
{
    LIBMATTI_NEOFORGESPI_IModFileInfo **plugins;
    size_t pluginCount;
    LIBMATTI_NEOFORGESPI_IModFile **gameLibraries;
    size_t gameLibraryCount;
    LIBMATTI_FML_ModFileInfo **modFiles;
    size_t modFileCount;
    LIBMATTI_FML_ModInfo **sortedList;
    size_t sortedCount;
    LIBMATTI_FML_ModDependencyMap modDependencies;
    // Java: Map<String, ModFileInfo> fileById
    char **fileByIdKeys;
    LIBMATTI_FML_ModFileInfo **fileByIdValues;
    size_t fileByIdCount;
    // Java: @Nullable private volatile Map<String, IModFile> fileByPackage
    char **fileByPackageKeys;
    LIBMATTI_NEOFORGESPI_IModFile **fileByPackageValues;
    size_t fileByPackageCount;
    int fileByPackageBuilt;
    LIBMATTI_FML_ModLoadingIssue **modLoadingIssues;
    size_t modLoadingIssueCount;
    // Java: Set<IModFile> allModFiles
    LIBMATTI_NEOFORGESPI_IModFile **allModFiles;
    size_t allModFileCount;
};

// Java: public static LoadingModList of(List<ModFile> plugins, List<ModFile> gameLibraries, List<ModFile> modFiles, List<ModInfo> sortedList, List<ModLoadingIssue> issues, Map<ModInfo, List<ModInfo>> modDependencies)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_LoadingModList_Of(
    LIBMATTI_FML_ModFile **plugins, size_t pluginCount,
    LIBMATTI_FML_ModFile **gameLibraries, size_t gameLibraryCount,
    LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
    LIBMATTI_FML_ModInfo **sortedList, size_t sortedCount,
    LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount,
    const LIBMATTI_FML_ModDependencyMap *modDependencies);

void LIBMATTI_FML_LoadingModList_Free(LIBMATTI_FML_LoadingModList *list);

// Java: public boolean contains(IModFile modFile)
int LIBMATTI_FML_LoadingModList_Contains(const LIBMATTI_FML_LoadingModList *list,
                                         LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public List<IModFileInfo> getPlugins()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_FML_LoadingModList_GetPlugins(const LIBMATTI_FML_LoadingModList *list,
                                                                          size_t *count);
// Java: public List<IModFile> getGameLibraries()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetGameLibraries(const LIBMATTI_FML_LoadingModList *list,
                                                                            size_t *count);
// Java: public List<ModFileInfo> getModFiles()
LIBMATTI_FML_ModFileInfo **LIBMATTI_FML_LoadingModList_GetModFiles(const LIBMATTI_FML_LoadingModList *list,
                                                                  size_t *count);
// Java: public Set<IModFile> getAllModFiles()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetAllModFiles(const LIBMATTI_FML_LoadingModList *list,
                                                                          size_t *count);
// Java: public ModFileInfo getModFileById(String modid)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_LoadingModList_GetModFileById(const LIBMATTI_FML_LoadingModList *list,
                                                                    const char *modid);
// Java: public List<ModInfo> getMods()
LIBMATTI_FML_ModInfo **LIBMATTI_FML_LoadingModList_GetMods(const LIBMATTI_FML_LoadingModList *list, size_t *count);
// Java: public List<ModInfo> getDependencies(IModInfo mod)
LIBMATTI_FML_ModInfo **LIBMATTI_FML_LoadingModList_GetDependencies(const LIBMATTI_FML_LoadingModList *list,
                                                                  LIBMATTI_FML_ModInfo *mod, size_t *count);
// Java: public boolean hasErrors()
int LIBMATTI_FML_LoadingModList_HasErrors(const LIBMATTI_FML_LoadingModList *list);
// Java: public List<ModLoadingIssue> getModLoadingIssues()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_LoadingModList_GetModLoadingIssues(
    const LIBMATTI_FML_LoadingModList *list, size_t *count);
// Java: list.getModLoadingIssues().addAll(issues)
void LIBMATTI_FML_LoadingModList_AddIssues(LIBMATTI_FML_LoadingModList *list,
                                           LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount);
// Java: Map<String, IModFile> getPackageIndex()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetPackageIndex(LIBMATTI_FML_LoadingModList *list,
                                                                           const char ***packageNames, size_t *count);

#endif //MATTICRAFT_FML_LOADING_LOADINGMODLIST_H
