#include "libmatti/net/neoforged/fml/loading/UniqueModListBuilder.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: Map<String, List<ModFile>> - the collecting(groupingBy(...)) result
typedef struct
{
    char **keys;
    LIBMATTI_FML_ModFile ***values;
    size_t *valueCounts;
    size_t count;
} ModFileGroups;

static int groupIndex(const ModFileGroups *groups, const char *key)
{
    for (size_t i = 0; i < groups->count; i++)
        if (strcmp(groups->keys[i], key) == 0)
            return (int) i;
    return -1;
}

static void groupAdd(ModFileGroups *groups, const char *key, LIBMATTI_FML_ModFile *modFile)
{
    int index = groupIndex(groups, key);
    if (index < 0)
    {
        index = (int) groups->count++;
        groups->keys = realloc(groups->keys, sizeof(char *) * groups->count);
        groups->values = realloc(groups->values, sizeof(LIBMATTI_FML_ModFile **) * groups->count);
        groups->valueCounts = realloc(groups->valueCounts, sizeof(size_t) * groups->count);
        groups->keys[index] = strdup(key);
        groups->values[index] = NULL;
        groups->valueCounts[index] = 0;
    }

    groups->values[index] = realloc(groups->values[index],
                                    sizeof(LIBMATTI_FML_ModFile *) * (groups->valueCounts[index] + 1));
    groups->values[index][groups->valueCounts[index]++] = modFile;
}

static void groupsFree(ModFileGroups *groups)
{
    for (size_t i = 0; i < groups->count; i++)
    {
        free(groups->keys[i]);
        free(groups->values[i]);
    }
    free(groups->keys);
    free(groups->values);
    free(groups->valueCounts);
}

// Java: private static String getModId(ModFile modFile) { return modFile.getId(); }
static const char *getModId(LIBMATTI_FML_ModFile *modFile)
{
    return LIBMATTI_NEOFORGESPI_IModFile_GetId(LIBMATTI_FML_ModFile_AsModFile(modFile));
}

// Java: private ArtifactVersion getVersion(ModFile mf)
static const LIBMATTI_NEOFORGESPI_ArtifactVersion *getVersion(LIBMATTI_FML_ModFile *modFile)
{
    if (modFile->modFileInfo == NULL)
        return LIBMATTI_FML_ModFile_GetJarVersion(modFile);

    size_t count = 0;
    LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(modFile->modFileInfo, &count);
    if (count == 0)
        return LIBMATTI_FML_ModFile_GetJarVersion(modFile);

    return LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(modInfos[0]);
}

struct LIBMATTI_FML_UniqueModListBuilder
{
    LIBMATTI_FML_ModFile **modFiles;
    size_t modFileCount;
};

// Java: public UniqueModListBuilder(List<ModFile> modFiles)
LIBMATTI_FML_UniqueModListBuilder *LIBMATTI_FML_UniqueModListBuilder_New(LIBMATTI_FML_ModFile **modFiles,
                                                                         size_t modFileCount)
{
    LIBMATTI_FML_UniqueModListBuilder *builder = calloc(1, sizeof(LIBMATTI_FML_UniqueModListBuilder));
    builder->modFiles = modFiles;
    builder->modFileCount = modFileCount;
    return builder;
}

// Java: private ModFile selectNewestModInfo(Map.Entry<String, List<ModFile>> fullList)
static LIBMATTI_FML_ModFile *selectNewestModInfo(ModFileGroups *groups, size_t index)
{
    // Java: if (modInfoList.size() > 1) { modInfoList.sort(Comparator.comparing(this::getVersion).reversed()); ... }
    for (size_t i = 0; i + 1 < groups->valueCounts[index]; i++)
        for (size_t j = i + 1; j < groups->valueCounts[index]; j++)
        {
            int order = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_Compare(getVersion(groups->values[index][i]),
                                                                         getVersion(groups->values[index][j]));
            if (order < 0) // Java: reversed()
            {
                LIBMATTI_FML_ModFile *swap = groups->values[index][i];
                groups->values[index][i] = groups->values[index][j];
                groups->values[index][j] = swap;
            }
        }

    if (groups->valueCounts[index] > 1)
    {
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Found multiple mod files for modid {}, selecting the most recent",
                                groups->keys[index]);
    }

    return groups->values[index][0];
}

// Java: public UniqueModListData buildUniqueList()
LIBMATTI_FML_UniqueModListData *LIBMATTI_FML_UniqueModListBuilder_BuildUniqueList(
    LIBMATTI_FML_UniqueModListBuilder *builder, LIBMATTI_FML_ModLoadingIssue ***issues, size_t *issueCount)
{
    *issues = NULL;
    *issueCount = 0;

    // Java: collect(Collections.groupingBy(...)) split by whether the file has mod file info
    ModFileGroups modGroups = {0};
    ModFileGroups libGroups = {0};
    for (size_t i = 0; i < builder->modFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = builder->modFiles[i];
        if (modFile->modFileInfo != NULL)
            groupAdd(&modGroups, getModId(modFile), modFile);
        else
            groupAdd(&libGroups, getModId(modFile), modFile);
    }

    // Java: uniqueModList = modFilesByFirstId.entrySet().stream().map(this::selectNewestModInfo).toList();
    LIBMATTI_FML_ModFile **uniqueModList = malloc(sizeof(LIBMATTI_FML_ModFile *) * (modGroups.count > 0 ? modGroups.count : 1));
    size_t uniqueModCount = 0;
    // Java: uniqueLibListWithVersion = libFilesWithVersionByModuleName.entrySet().stream().map(this::selectNewestModInfo).toList();
    LIBMATTI_FML_ModFile **uniqueLibList = malloc(sizeof(LIBMATTI_FML_ModFile *) * (libGroups.count > 0 ? libGroups.count : 1));
    size_t uniqueLibCount = 0;

    // Java: Map<String, List<IModInfo>> modIds = uniqueModList.stream().map(ModFile::getModInfos).flatMap(...).collect(groupingBy(IModInfo::getModId));
    char **modIdKeys = NULL;
    LIBMATTI_NEOFORGESPI_IModInfo ***modIdValues = NULL;
    size_t *modIdValueCounts = NULL;
    size_t modIdCount = 0;

    for (size_t i = 0; i < modGroups.count; i++)
    {
        LIBMATTI_FML_ModFile *selected = selectNewestModInfo(&modGroups, i);
        uniqueModList[uniqueModCount++] = selected;

        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(selected->modFileInfo,
                                                                                    &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(modInfos[j]);
            size_t index = modIdCount;
            for (size_t k = 0; k < modIdCount; k++)
                if (strcmp(modIdKeys[k], modId) == 0)
                    index = k;
            if (index == modIdCount)
            {
                modIdCount++;
                modIdKeys = realloc(modIdKeys, sizeof(char *) * modIdCount);
                modIdValues = realloc(modIdValues, sizeof(LIBMATTI_NEOFORGESPI_IModInfo **) * modIdCount);
                modIdValueCounts = realloc(modIdValueCounts, sizeof(size_t) * modIdCount);
                modIdKeys[index] = strdup(modId);
                modIdValues[index] = NULL;
                modIdValueCounts[index] = 0;
            }
            modIdValues[index] = realloc(modIdValues[index],
                                         sizeof(LIBMATTI_NEOFORGESPI_IModInfo *) * (modIdValueCounts[index] + 1));
            modIdValues[index][modIdValueCounts[index]++] = modInfos[j];
        }
    }

    for (size_t i = 0; i < libGroups.count; i++)
        uniqueLibList[uniqueLibCount++] = selectNewestModInfo(&libGroups, i);

    // Java: List<ModLoadingIssue> dupedModErrors = modIds.values().stream().filter(modInfos -> modInfos.size() > 1).map(...)
    for (size_t i = 0; i < modIdCount; i++)
    {
        if (modIdValueCounts[i] <= 1) continue;

        LIBMATTI_FML_ModFile **files = malloc(sizeof(LIBMATTI_FML_ModFile *) * modIdValueCounts[i]);
        for (size_t j = 0; j < modIdValueCounts[i]; j++)
            files[j] = LIBMATTI_FML_ModFileInfo_GetFile(
                LIBMATTI_FML_ModInfo_GetOwningFile(LIBMATTI_FML_ModInfo_AsConcrete(modIdValues[i][j])));

        // Java: mods.stream().map(modInfo -> modInfo.getOwningFile().getFile().getFileName()).collect(joining(", "))
        char *fileNames = strdup("");
        size_t namesLength = 0;
        for (size_t j = 0; j < modIdValueCounts[i]; j++)
        {
            const char *fileName = LIBMATTI_NEOFORGESPI_IModFile_GetFileName(LIBMATTI_FML_ModFile_AsModFile(files[j]));
            size_t fileNameLength = strlen(fileName);
            fileNames = realloc(fileNames, namesLength + fileNameLength + 3);
            if (j > 0) fileNames[namesLength++] = ',', fileNames[namesLength++] = ' ';
            memcpy(fileNames + namesLength, fileName, fileNameLength + 1);
            namesLength += fileNameLength;
        }

        const char *args[] = {modIdKeys[i], fileNames};
        *issues = realloc(*issues, sizeof(LIBMATTI_FML_ModLoadingIssue *) * (*issueCount + 1));
        (*issues)[(*issueCount)++] = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.duplicate_mod", args, 2);
        free(fileNames);
        free(files);
    }

    // Java: List<ModLoadingIssue> dupedLibErrors = versionedLibIds.values().stream().filter(modFiles -> modFiles.size() > 1).map(...)
    for (size_t i = 0; i < libGroups.count; i++)
    {
        if (libGroups.valueCounts[i] <= 1) continue;

        char *fileNames = strdup("");
        size_t namesLength = 0;
        for (size_t j = 0; j < libGroups.valueCounts[i]; j++)
        {
            const char *fileName = LIBMATTI_NEOFORGESPI_IModFile_GetFileName(
                LIBMATTI_FML_ModFile_AsModFile(libGroups.values[i][j]));
            size_t fileNameLength = strlen(fileName);
            fileNames = realloc(fileNames, namesLength + fileNameLength + 3);
            if (j > 0) fileNames[namesLength++] = ',', fileNames[namesLength++] = ' ';
            memcpy(fileNames + namesLength, fileName, fileNameLength + 1);
            namesLength += fileNameLength;
        }

        const char *args[] = {libGroups.keys[i], fileNames};
        *issues = realloc(*issues, sizeof(LIBMATTI_FML_ModLoadingIssue *) * (*issueCount + 1));
        (*issues)[(*issueCount)++] = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.duplicate_mod", args, 2);
        free(fileNames);
    }

    if (*issueCount > 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Found duplicate mods or libraries");
        free(uniqueModList);
        free(uniqueLibList);
        for (size_t i = 0; i < modIdCount; i++)
        {
            free(modIdKeys[i]);
            free(modIdValues[i]);
        }
        free(modIdKeys);
        free(modIdValues);
        free(modIdValueCounts);
        groupsFree(&modGroups);
        groupsFree(&libGroups);
        return NULL;
    }

    // Java: Map<String, List<ModFile>> uniqueModFilesByFirstId = uniqueModList.stream().collect(groupingBy(UniqueModListBuilder::getModId));
    LIBMATTI_FML_UniqueModListData *data = calloc(1, sizeof(LIBMATTI_FML_UniqueModListData));

    // Java: List<ModFile> loadedList = new ArrayList<>(); loadedList.addAll(uniqueModList); loadedList.addAll(uniqueLibListWithVersion);
    data->modFileCount = uniqueModCount + uniqueLibCount;
    data->modFiles = malloc(sizeof(LIBMATTI_FML_ModFile *) * (data->modFileCount > 0 ? data->modFileCount : 1));
    memcpy(data->modFiles, uniqueModList, sizeof(LIBMATTI_FML_ModFile *) * uniqueModCount);
    memcpy(data->modFiles + uniqueModCount, uniqueLibList, sizeof(LIBMATTI_FML_ModFile *) * uniqueLibCount);

    // Java: Set<ModFile> discardedModFiles = Collections.newSetFromMap(new IdentityHashMap<>()); discardedModFiles.addAll(this.modFiles); loadedList.forEach(discardedModFiles::remove);
    data->discardedFiles = malloc(sizeof(LIBMATTI_FML_ModFile *) * (builder->modFileCount > 0 ? builder->modFileCount : 1));
    for (size_t i = 0; i < builder->modFileCount; i++)
    {
        int loaded = 0;
        for (size_t j = 0; j < data->modFileCount && !loaded; j++)
            if (data->modFiles[j] == builder->modFiles[i])
                loaded = 1;
        if (!loaded) data->discardedFiles[data->discardedFileCount++] = builder->modFiles[i];
    }

    // Java: group the unique mod files by their first mod id
    ModFileGroups byFirstId = {0};
    for (size_t i = 0; i < uniqueModCount; i++)
        groupAdd(&byFirstId, getModId(uniqueModList[i]), uniqueModList[i]);

    free(uniqueModList);
    free(uniqueLibList);

    data->modFilesByFirstIdCount = byFirstId.count;
    data->modFilesByFirstIdKeys = byFirstId.keys;
    data->modFilesByFirstIdValues = byFirstId.values;
    data->modFilesByFirstIdValueCounts = byFirstId.valueCounts;

    for (size_t i = 0; i < modIdCount; i++)
    {
        free(modIdKeys[i]);
        free(modIdValues[i]);
    }
    free(modIdKeys);
    free(modIdValues);
    free(modIdValueCounts);
    groupsFree(&modGroups);
    groupsFree(&libGroups);
    return data;
}

void LIBMATTI_FML_UniqueModListData_Free(LIBMATTI_FML_UniqueModListData *data)
{
    if (data == NULL) return;
    for (size_t i = 0; i < data->modFilesByFirstIdCount; i++)
    {
        free(data->modFilesByFirstIdKeys[i]);
        free(data->modFilesByFirstIdValues[i]);
    }
    free(data->modFilesByFirstIdKeys);
    free(data->modFilesByFirstIdValues);
    free(data->modFilesByFirstIdValueCounts);
    free(data->modFiles);
    free(data->discardedFiles);
    free(data);
}
