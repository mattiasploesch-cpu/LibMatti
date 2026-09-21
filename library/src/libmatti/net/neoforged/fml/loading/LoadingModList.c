#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: public static LoadingModList of(List<ModFile> plugins, List<ModFile> gameLibraries, List<ModFile> modFiles, List<ModInfo> sortedList, List<ModLoadingIssue> issues, Map<ModInfo, List<ModInfo>> modDependencies)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_LoadingModList_Of(
    LIBMATTI_FML_ModFile **plugins, size_t pluginCount,
    LIBMATTI_FML_ModFile **gameLibraries, size_t gameLibraryCount,
    LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
    LIBMATTI_FML_ModInfo **sortedList, size_t sortedCount,
    LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount,
    const LIBMATTI_FML_ModDependencyMap *modDependencies)
{
    LIBMATTI_FML_LoadingModList *list = calloc(1, sizeof(LIBMATTI_FML_LoadingModList));

    // Java: this.plugins = plugins.stream().map(ModFile::getModFileInfo).collect(toList());
    list->plugins = calloc(pluginCount > 0 ? pluginCount : 1, sizeof(LIBMATTI_NEOFORGESPI_IModFileInfo *));
    for (size_t i = 0; i < pluginCount; i++)
        list->plugins[list->pluginCount++] = plugins[i]->modFileInfo;

    // Java: this.gameLibraries = List.copyOf(gameLibraries);
    list->gameLibraries = calloc(gameLibraryCount > 0 ? gameLibraryCount : 1, sizeof(LIBMATTI_NEOFORGESPI_IModFile *));
    for (size_t i = 0; i < gameLibraryCount; i++)
        list->gameLibraries[list->gameLibraryCount++] = LIBMATTI_FML_ModFile_AsModFile(gameLibraries[i]);

    // Java: this.modFiles = modFiles.stream().map(ModFile::getModFileInfo).map(ModFileInfo.class::cast).collect(toList());
    // Java: modFiles.stream().map(ModFile::getModFileInfo).map(ModFileInfo.class::cast)
    list->modFiles = calloc(modFileCount > 0 ? modFileCount : 1, sizeof(LIBMATTI_FML_ModFileInfo *));
    for (size_t i = 0; i < modFileCount; i++)
        list->modFiles[list->modFileCount++] = (LIBMATTI_FML_ModFileInfo *) modFiles[i]->modFileInfo;

    // Java: this.sortedList = new ArrayList<>(sortedList);
    list->sortedList = calloc(sortedCount > 0 ? sortedCount : 1, sizeof(LIBMATTI_FML_ModInfo *));
    memcpy(list->sortedList, sortedList, sizeof(LIBMATTI_FML_ModInfo *) * sortedCount);
    list->sortedCount = sortedCount;

    // Java: this.modDependencies = modDependencies;
    if (modDependencies != NULL)
        list->modDependencies = *modDependencies;

    // Java: this.fileById = this.modFiles.stream().map(ModFileInfo::getMods).flatMap(Collection::stream).map(ModInfo.class::cast).collect(toMap(ModInfo::getModId, ModInfo::getOwningFile));
    for (size_t i = 0; i < list->modFileCount; i++)
    {
        size_t modCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **mods = LIBMATTI_FML_ModFileInfo_GetMods(list->modFiles[i], &modCount);
        for (size_t j = 0; j < modCount; j++)
        {
            list->fileByIdKeys = realloc(list->fileByIdKeys, sizeof(char *) * (list->fileByIdCount + 1));
            list->fileByIdValues = realloc(list->fileByIdValues, sizeof(LIBMATTI_FML_ModFileInfo *) * (list->fileByIdCount + 1));
            list->fileByIdKeys[list->fileByIdCount] = strdup(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mods[j]));
            list->fileByIdValues[list->fileByIdCount] = LIBMATTI_FML_ModInfo_GetOwningFile(
                LIBMATTI_FML_ModInfo_AsConcrete(mods[j]));
            list->fileByIdCount++;
        }
    }

    // Java: this.modLoadingIssues = new ArrayList<>(); list.modLoadingIssues.addAll(issues);
    list->modLoadingIssues = calloc(issueCount > 0 ? issueCount : 1, sizeof(LIBMATTI_FML_ModLoadingIssue *));
    memcpy(list->modLoadingIssues, issues, sizeof(LIBMATTI_FML_ModLoadingIssue *) * issueCount);
    list->modLoadingIssueCount = issueCount;

    // Java: this.allModFiles.addAll(this.gameLibraries); addAll(modFiles); addAll(plugins);
    size_t allCount = list->gameLibraryCount + list->modFileCount + pluginCount;
    list->allModFiles = calloc(allCount > 0 ? allCount : 1, sizeof(LIBMATTI_NEOFORGESPI_IModFile *));
    for (size_t i = 0; i < list->gameLibraryCount; i++)
        list->allModFiles[list->allModFileCount++] = list->gameLibraries[i];
    for (size_t i = 0; i < list->modFileCount; i++)
        list->allModFiles[list->allModFileCount++] = LIBMATTI_FML_ModFile_AsModFile(
            LIBMATTI_FML_ModFileInfo_GetFile(list->modFiles[i]));
    for (size_t i = 0; i < list->pluginCount; i++)
        list->allModFiles[list->allModFileCount++] = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(list->plugins[i]);

    return list;
}

void LIBMATTI_FML_LoadingModList_Free(LIBMATTI_FML_LoadingModList *list)
{
    if (list == NULL) return;
    for (size_t i = 0; i < list->fileByIdCount; i++) free(list->fileByIdKeys[i]);
    free(list->fileByIdKeys);
    free(list->fileByIdValues);
    for (size_t i = 0; i < list->fileByPackageCount; i++) free(list->fileByPackageKeys[i]);
    free(list->fileByPackageKeys);
    free(list->fileByPackageValues);
    for (size_t i = 0; i < list->modDependencies.count; i++)
        free(list->modDependencies.values[i]);
    free(list->modDependencies.keys);
    free(list->modDependencies.values);
    free(list->modDependencies.valueCounts);
    free(list->plugins);
    free(list->gameLibraries);
    free(list->modFiles);
    free(list->sortedList);
    free(list->modLoadingIssues);
    free(list->allModFiles);
    free(list);
}

// Java: public boolean contains(IModFile modFile)
int LIBMATTI_FML_LoadingModList_Contains(const LIBMATTI_FML_LoadingModList *list,
                                         LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    for (size_t i = 0; i < list->allModFileCount; i++)
        if (list->allModFiles[i] == modFile)
            return 1;
    return 0;
}

// Java: public List<IModFileInfo> getPlugins()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_FML_LoadingModList_GetPlugins(const LIBMATTI_FML_LoadingModList *list,
                                                                          size_t *count)
{
    *count = list->pluginCount;
    return list->plugins;
}

// Java: public List<IModFile> getGameLibraries()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetGameLibraries(const LIBMATTI_FML_LoadingModList *list,
                                                                            size_t *count)
{
    *count = list->gameLibraryCount;
    return list->gameLibraries;
}

// Java: public List<ModFileInfo> getModFiles()
LIBMATTI_FML_ModFileInfo **LIBMATTI_FML_LoadingModList_GetModFiles(const LIBMATTI_FML_LoadingModList *list,
                                                                  size_t *count)
{
    *count = list->modFileCount;
    return list->modFiles;
}

// Java: public Set<IModFile> getAllModFiles()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetAllModFiles(const LIBMATTI_FML_LoadingModList *list,
                                                                          size_t *count)
{
    *count = list->allModFileCount;
    return list->allModFiles;
}

// Java: public ModFileInfo getModFileById(String modid)
LIBMATTI_FML_ModFileInfo *LIBMATTI_FML_LoadingModList_GetModFileById(const LIBMATTI_FML_LoadingModList *list,
                                                                    const char *modid)
{
    for (size_t i = 0; i < list->fileByIdCount; i++)
        if (strcmp(list->fileByIdKeys[i], modid) == 0)
            return list->fileByIdValues[i];
    return NULL;
}

// Java: public List<ModInfo> getMods()
LIBMATTI_FML_ModInfo **LIBMATTI_FML_LoadingModList_GetMods(const LIBMATTI_FML_LoadingModList *list, size_t *count)
{
    *count = list->sortedCount;
    return list->sortedList;
}

// Java: public List<ModInfo> getDependencies(IModInfo mod)
LIBMATTI_FML_ModInfo **LIBMATTI_FML_LoadingModList_GetDependencies(const LIBMATTI_FML_LoadingModList *list,
                                                                  LIBMATTI_FML_ModInfo *mod, size_t *count)
{
    for (size_t i = 0; i < list->modDependencies.count; i++)
    {
        if (list->modDependencies.keys[i] != mod) continue;
        *count = list->modDependencies.valueCounts[i];
        return list->modDependencies.values[i];
    }
    *count = 0;
    return NULL;
}

// Java: public boolean hasErrors()
int LIBMATTI_FML_LoadingModList_HasErrors(const LIBMATTI_FML_LoadingModList *list)
{
    // Java: return !modLoadingIssues.isEmpty() && modLoadingIssues.stream().anyMatch(issue -> issue.severity() == ERROR);
    if (list->modLoadingIssueCount == 0)
        return 0;

    for (size_t i = 0; i < list->modLoadingIssueCount; i++)
        if (list->modLoadingIssues[i]->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR)
            return 1;
    return 0;
}

// Java: public List<ModLoadingIssue> getModLoadingIssues()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_LoadingModList_GetModLoadingIssues(
    const LIBMATTI_FML_LoadingModList *list, size_t *count)
{
    *count = list->modLoadingIssueCount;
    return list->modLoadingIssues;
}

// Java: list.getModLoadingIssues().addAll(issues)
void LIBMATTI_FML_LoadingModList_AddIssues(LIBMATTI_FML_LoadingModList *list,
                                           LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount)
{
    list->modLoadingIssues = realloc(list->modLoadingIssues,
                                     sizeof(LIBMATTI_FML_ModLoadingIssue *) *
                                     (list->modLoadingIssueCount + issueCount));
    memcpy(list->modLoadingIssues + list->modLoadingIssueCount, issues,
           sizeof(LIBMATTI_FML_ModLoadingIssue *) * issueCount);
    list->modLoadingIssueCount += issueCount;
}

// Java: private Map<String, IModFile> buildPackageIndex()
static void buildPackageIndex(LIBMATTI_FML_LoadingModList *list)
{
    for (size_t i = 0; i < list->allModFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFile_AsConcrete(list->allModFiles[i]);
        LIBMATTI_JL_ModuleDescriptor *descriptor = LIBMATTI_FML_ModFile_GetModuleDescriptor(modFile);
        if (descriptor == NULL) continue;

        size_t packageCount = 0;
        char **packages = LIBMATTI_JL_ModuleDescriptor_Packages(descriptor, &packageCount);
        for (size_t j = 0; j < packageCount; j++)
        {
            size_t index = list->fileByPackageCount;
            for (size_t k = 0; k < list->fileByPackageCount; k++)
                if (strcmp(list->fileByPackageKeys[k], packages[j]) == 0)
                    index = k;
            if (index == list->fileByPackageCount)
            {
                list->fileByPackageCount++;
                list->fileByPackageKeys = realloc(list->fileByPackageKeys,
                                                  sizeof(char *) * list->fileByPackageCount);
                list->fileByPackageValues = realloc(list->fileByPackageValues,
                                                    sizeof(LIBMATTI_NEOFORGESPI_IModFile *) * list->fileByPackageCount);
                list->fileByPackageKeys[index] = strdup(packages[j]);
                list->fileByPackageValues[index] = NULL;
            }
            list->fileByPackageValues[index] = list->allModFiles[i];
        }
    }

    char entryCount[32];
    snprintf(entryCount, sizeof(entryCount), "%zu", list->fileByPackageCount);
    LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Built package index ({} entries)", entryCount);
}

// Java: Map<String, IModFile> getPackageIndex()
LIBMATTI_NEOFORGESPI_IModFile **LIBMATTI_FML_LoadingModList_GetPackageIndex(LIBMATTI_FML_LoadingModList *list,
                                                                           const char ***packageNames, size_t *count)
{
    if (!list->fileByPackageBuilt)
    {
        buildPackageIndex(list);
        list->fileByPackageBuilt = 1;
    }

    *packageNames = (const char **) list->fileByPackageKeys;
    *count = list->fileByPackageCount;
    return list->fileByPackageValues;
}

// Java: @Deprecated public static LoadingModList get() { return FMLLoader.getCurrent().getLoadingModList(); }
void *LIBMATTI_FML_LoadingModList_Get(void)
{
    return LIBMATTI_FML_FMLLoader_GetLoadingModList(LIBMATTI_FML_FMLLoader_GetCurrent());
}
