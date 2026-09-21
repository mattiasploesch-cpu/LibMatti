#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/fml/loading/ModSorter.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/util/jar/Manifest.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/UniqueModListBuilder.h"
#include "libmatti/net/neoforged/fml/loading/VersionSupportMatrix.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModInfo.h"
#include "libmatti/net/neoforged/fml/loading/toposort/TopologicalSort.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: log4j formats the {} arguments; the port's logger takes strings only
static const char *formatNumber(long long value)
{
    static char buffers[8][32];
    static int slot = 0;
    char *buffer = buffers[slot];
    slot = (slot + 1) % 8;
    snprintf(buffer, sizeof(buffers[0]), "%lld", value);
    return buffer;
}

// Java: the ModSorter instance fields
typedef struct
{
    LIBMATTI_FML_UniqueModListBuilder *uniqueModListBuilder;
    LIBMATTI_FML_ModFile **modFiles;
    size_t modFileCount;
    LIBMATTI_FML_ModInfo **sortedList;
    size_t sortedCount;
    // Java: Map<ModInfo, List<ModInfo>> modDependencies
    LIBMATTI_FML_ModDependencyMap modDependencies;
    // Java: Map<String, IModInfo> modIdNameLookup
    char **lookupKeys;
    LIBMATTI_NEOFORGESPI_IModInfo **lookupValues;
    size_t lookupCount;
    LIBMATTI_FML_ModFile **systemMods;
    size_t systemModCount;
} ModSorter;

// Java: private IModInfo lookup(String modId) - modIdNameLookup.get(modId)
static LIBMATTI_NEOFORGESPI_IModInfo *lookup(ModSorter *sorter, const char *modId)
{
    for (size_t i = 0; i < sorter->lookupCount; i++)
        if (strcmp(sorter->lookupKeys[i], modId) == 0)
            return sorter->lookupValues[i];
    return NULL;
}

// Java: Collection.add - the port keeps a plain list, deduplicating like a Set where Java used one
static void versionSetAdd(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion ***set, size_t *count,
                          LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *value)
{
    for (size_t i = 0; i < *count; i++)
        if ((*set)[i] == value)
            return;
    *set = realloc(*set, sizeof(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *) * (*count + 1));
    (*set)[(*count)++] = value;
}

static LIBMATTI_FML_ModLoadingIssue **concat(LIBMATTI_FML_ModLoadingIssue **a, size_t aCount,
                                             LIBMATTI_FML_ModLoadingIssue **b, size_t bCount, size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **result = malloc(sizeof(LIBMATTI_FML_ModLoadingIssue *) * (aCount + bCount));
    memcpy(result, a, sizeof(LIBMATTI_FML_ModLoadingIssue *) * aCount);
    memcpy(result + aCount, b, sizeof(LIBMATTI_FML_ModLoadingIssue *) * bCount);
    *count = aCount + bCount;
    return result;
}

// Java: private void buildUniqueList()
static int buildUniqueList(ModSorter *sorter)
{
    LIBMATTI_FML_ModLoadingIssue **issues = NULL;
    size_t issueCount = 0;
    LIBMATTI_FML_UniqueModListData *data =
        LIBMATTI_FML_UniqueModListBuilder_BuildUniqueList(sorter->uniqueModListBuilder, &issues, &issueCount);
    if (data == NULL)
        return 0;

    // Java: uniqueModListData.discardedFiles().forEach(ModFile::close);
    for (size_t i = 0; i < data->discardedFileCount; i++)
        LIBMATTI_FML_ModFile_Close(data->discardedFiles[i]);

    sorter->modFiles = data->modFiles;
    sorter->modFileCount = data->modFileCount;

    // Java: detectSystemMods(uniqueModListData.modFilesByFirstId());
    const char *systemModNames[16];
    size_t systemModNameCount = 0;
    systemModNames[systemModNameCount++] = "minecraft";
    systemModNames[systemModNameCount++] = "neoforge";

    // Java: modFiles.stream().map(mf -> mf.getContents().getManifest().getMainAttributes().getValue("FML-System-Mods")).filter(Objects::nonNull).findFirst()
    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(
            LIBMATTI_FML_ModFile_GetContents(sorter->modFiles[i]));
        const char *value = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, "FML-System-Mods")
                                             : NULL;
        if (value == NULL) continue;

        char *copy = strdup(value);
        for (char *token = strtok(copy, ","); token != NULL && systemModNameCount < 16;
             token = strtok(NULL, ","))
            systemModNames[systemModNameCount++] = strdup(token);
        free(copy);
        break;
    }

    sorter->systemMods = NULL;
    for (size_t i = 0; i < systemModNameCount; i++)
    {
        for (size_t j = 0; j < data->modFilesByFirstIdCount; j++)
        {
            if (strcmp(data->modFilesByFirstIdKeys[j], systemModNames[i]) != 0) continue;
            if (data->modFilesByFirstIdValueCounts[j] == 0) continue;
            sorter->systemMods = realloc(sorter->systemMods,
                                         sizeof(LIBMATTI_FML_ModFile *) * (sorter->systemModCount + 1));
            sorter->systemMods[sorter->systemModCount++] = data->modFilesByFirstIdValues[j][0];
            break;
        }
    }

    // Java: modIdNameLookup = uniqueModListData.modFiles().stream().flatMap(mf -> mf.getModInfos().stream()).collect(toMap(IModInfo::getModId, mi -> mi));
    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
            sorter->modFiles[i]->modFileInfo, &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(modInfos[j]);
            size_t index = sorter->lookupCount;
            for (size_t k = 0; k < sorter->lookupCount; k++)
                if (strcmp(sorter->lookupKeys[k], modId) == 0)
                    index = k;
            if (index == sorter->lookupCount)
            {
                sorter->lookupCount++;
                sorter->lookupKeys = realloc(sorter->lookupKeys, sizeof(char *) * sorter->lookupCount);
                sorter->lookupValues = realloc(sorter->lookupValues,
                                               sizeof(LIBMATTI_NEOFORGESPI_IModInfo *) * sorter->lookupCount);
                sorter->lookupKeys[index] = strdup(modId);
            }
            sorter->lookupValues[index] = modInfos[j];
        }
    }

    // the mod file array is handed to the sorter; release the rest of the builder data
    data->modFiles = NULL;
    LIBMATTI_FML_UniqueModListData_Free(data);

    return 1;
}

// Java: private void addDependency(MutableGraph<ModInfo> topoGraph, IModInfo.ModVersion dep)
static void addDependency(ModSorter *sorter, LIBMATTI_FML_Graph *graph, LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *dep)
{
    LIBMATTI_FML_ModInfo *self = LIBMATTI_FML_ModInfo_AsConcrete(
        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(dep));
    LIBMATTI_NEOFORGESPI_IModInfo *targetModInfo = lookup(sorter, LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dep));
    // Java: if (!(targetModInfo instanceof ModInfo target)) return;
    if (targetModInfo == NULL || targetModInfo->self == NULL) return;
    LIBMATTI_FML_ModInfo *target = LIBMATTI_FML_ModInfo_AsConcrete(targetModInfo);
    if (self == target) return;

    switch (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOrdering(dep))
    {
        case LIBMATTI_NEOFORGESPI_IModInfo_Ordering_BEFORE:
            LIBMATTI_FML_Graph_PutEdge(graph, self, target);
            break;
        case LIBMATTI_NEOFORGESPI_IModInfo_Ordering_AFTER:
            LIBMATTI_FML_Graph_PutEdge(graph, target, self);
            break;
        default:
            break;
    }
}

// Java: Comparator.comparing(infos::get)
typedef struct
{
    LIBMATTI_FML_ModInfo **keys;
    int *counters;
    size_t count;
} ModInfoOrder;

static int modInfoComparator(void *a, void *b, void *userdata)
{
    ModInfoOrder *order = userdata;
    int left = 0;
    int right = 0;
    for (size_t i = 0; i < order->count; i++)
    {
        if (order->keys[i] == a) left = order->counters[i];
        if (order->keys[i] == b) right = order->counters[i];
    }
    return left - right;
}

static const char *modInfoName(void *node)
{
    return LIBMATTI_FML_ModInfo_GetModId(node);
}

// Java: private void sort(List<ModLoadingIssue> issues)
static int sortMods(ModSorter *sorter, LIBMATTI_FML_ModLoadingIssue ***issues, size_t *issueCount)
{
    *issues = NULL;
    *issueCount = 0;

    // Java: infos = modFiles.stream().flatMap(mf -> mf.getModInfos().stream()).map(ModInfo.class::cast).collect(toMap(identity(), e -> counter.incrementAndGet()));
    ModInfoOrder order = {0};
    LIBMATTI_FML_Graph *graph = LIBMATTI_FML_Graph_New(1, 0);

    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
            sorter->modFiles[i]->modFileInfo, &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            LIBMATTI_FML_ModInfo *modInfo = LIBMATTI_FML_ModInfo_AsConcrete(modInfos[j]);
            order.keys = realloc(order.keys, sizeof(LIBMATTI_FML_ModInfo *) * (order.count + 1));
            order.counters = realloc(order.counters, sizeof(int) * (order.count + 1));
            order.keys[order.count] = modInfo;
            order.counters[order.count] = (int) order.count + 1;
            order.count++;
            LIBMATTI_FML_Graph_AddNode(graph, modInfo);
        }
    }

    // Java: modFiles.stream().map(ModFile::getModInfos).mapMulti(Iterable::forEach).map(IModInfo::getDependencies).mapMulti(Iterable::forEach).forEach(dep -> addDependency(graph, dep));
    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
            sorter->modFiles[i]->modFileInfo, &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            size_t dependencyCount = 0;
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **dependencies =
                LIBMATTI_NEOFORGESPI_IModInfo_GetDependencies(modInfos[j], &dependencyCount);
            for (size_t k = 0; k < dependencyCount; k++)
                addDependency(sorter, graph, dependencies[k]);
        }
    }

    // Java: FMLConfig.getDependencyOverrides().forEach((id, overrides) -> {
    //           var target = (ModInfo) modIdNameLookup.get(id);
    //           if (target == null) issues.add(warning("fml.modloadingissue.depoverride.unknown_target", id));
    //           else for (var override : overrides) {
    //               var dep = (ModInfo) modIdNameLookup.get(override.modId());
    //               if (dep == null) issues.add(warning("fml.modloadingissue.depoverride.unknown_dependency", override.modId(), id));
    //               else if (!override.remove()) graph.putEdge(dep, target); } });
    for (size_t i = 0; i < sorter->lookupCount; i++)
    {
        size_t overrideCount = 0;
        LIBMATTI_FML_FMLConfig_DependencyOverride **overrides =
            LIBMATTI_FML_FMLConfig_GetOverrides(sorter->lookupKeys[i], &overrideCount);
        if (overrideCount == 0) continue;

        LIBMATTI_NEOFORGESPI_IModInfo *target = sorter->lookupValues[i];
        for (size_t o = 0; o < overrideCount; o++)
        {
            LIBMATTI_NEOFORGESPI_IModInfo *dep = lookup(sorter, overrides[o]->modId);
            if (dep == NULL)
            {
                const char *unknownModId = overrides[o]->modId;
                LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Warning(
                    "fml.modloadingissue.depoverride.unknown_dependency", &unknownModId, 1);
                LIBMATTI_FML_ModLoader_AddLoadingIssue(issue);
                LIBMATTI_FML_ModLoadingIssue_Free(issue);
            }
            else if (!overrides[o]->remove)
            {
                // Add ordering dependency overrides (random order -> target AFTER dependency)
                LIBMATTI_FML_Graph_PutEdge(graph, dep, target);
            }
        }
    }

    // Java: sorted = TopologicalSort.topologicalSort(graph, Comparator.comparing(infos::get));
    size_t sortedCount = 0;
    LIBMATTI_FML_CyclePresentException *cycles = NULL;
    void **sorted = LIBMATTI_FML_TopologicalSort_TopologicalSort(graph, modInfoComparator, &order, &sortedCount,
                                                                 &cycles);

    if (cycles != NULL)
    {
        char *message = LIBMATTI_FML_CyclePresentException_GetMessage(cycles);
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Mod Sorting failed. Detected Cycles: {}",
                                 message);
        free(message);

        // Java: dataList = cycles.stream().map(cycle -> cycle.stream().map(IModInfo::getModId).sorted().collect(joining(", "))).map(list -> ModLoadingIssue.error("fml.modloadingissue.cycle", list).withCause(e)).toList();
        size_t *cycleSizes = NULL;
        size_t cycleCount = 0;
        void ***cycleNodes = LIBMATTI_FML_CyclePresentException_GetCycles(cycles, &cycleSizes, &cycleCount);
        *issues = calloc(cycleCount > 0 ? cycleCount : 1, sizeof(LIBMATTI_FML_ModLoadingIssue *));
        for (size_t i = 0; i < cycleCount; i++)
        {
            char *list = strdup("");
            size_t listLength = 0;
            for (size_t j = 0; j < cycleSizes[i]; j++)
            {
                const char *modId = LIBMATTI_FML_ModInfo_GetModId(cycleNodes[i][j]);
                size_t modIdLength = strlen(modId);
                list = realloc(list, listLength + modIdLength + 3);
                if (j > 0) list[listLength++] = ',', list[listLength++] = ' ';
                memcpy(list + listLength, modId, modIdLength + 1);
                listLength += modIdLength;
            }
            const char *args[] = {list};
            (*issues)[(*issueCount)++] = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.cycle", args, 1);
            free(list);
        }

        free(sorted);
        free(order.keys);
        free(order.counters);
        LIBMATTI_FML_CyclePresentException_Free(cycles);
        LIBMATTI_FML_Graph_Free(graph);
        return 0;
    }

    free(order.keys);
    free(order.counters);

    sorter->sortedList = malloc(sizeof(LIBMATTI_FML_ModInfo *) * (sortedCount > 0 ? sortedCount : 1));
    for (size_t i = 0; i < sortedCount; i++) sorter->sortedList[i] = sorted[i];
    sorter->sortedCount = sortedCount;

    // Java: this.modDependencies = sorted.stream().collect(toMap(mi -> mi, mi -> List.copyOf(graph.predecessors(mi))));
    sorter->modDependencies.keys = malloc(sizeof(LIBMATTI_FML_ModInfo *) * (sortedCount > 0 ? sortedCount : 1));
    sorter->modDependencies.values = malloc(sizeof(LIBMATTI_FML_ModInfo **) * (sortedCount > 0 ? sortedCount : 1));
    sorter->modDependencies.valueCounts = malloc(sizeof(size_t) * (sortedCount > 0 ? sortedCount : 1));
    for (size_t i = 0; i < sortedCount; i++)
    {
        size_t predecessorCount = 0;
        void **predecessors = LIBMATTI_FML_Graph_Predecessors(graph, sorted[i], &predecessorCount);
        sorter->modDependencies.keys[sorter->modDependencies.count] = sorted[i];
        sorter->modDependencies.values[sorter->modDependencies.count] = malloc(
            sizeof(LIBMATTI_FML_ModInfo *) * (predecessorCount > 0 ? predecessorCount : 1));
        for (size_t j = 0; j < predecessorCount; j++)
            sorter->modDependencies.values[sorter->modDependencies.count][j] = predecessors[j];
        sorter->modDependencies.valueCounts[sorter->modDependencies.count] = predecessorCount;
        sorter->modDependencies.count++;
        free(predecessors);
    }

    // Java: this.modFiles = sorted.stream().map(mi -> mi.getOwningFile().getFile()).distinct().toList();
    LIBMATTI_FML_ModFile **files = malloc(sizeof(LIBMATTI_FML_ModFile *) * (sortedCount > 0 ? sortedCount : 1));
    size_t fileCount = 0;
    for (size_t i = 0; i < sortedCount; i++)
    {
        LIBMATTI_FML_ModFile *file = LIBMATTI_FML_ModFileInfo_GetFile(
            LIBMATTI_FML_ModInfo_GetOwningFile(sorter->sortedList[i]));
        int seen = 0;
        for (size_t j = 0; j < fileCount && !seen; j++)
            if (files[j] == file)
                seen = 1;
        if (!seen) files[fileCount++] = file;
    }
    free(sorted);
    free(sorter->modFiles);
    sorter->modFiles = files;
    sorter->modFileCount = fileCount;
    LIBMATTI_FML_Graph_Free(graph);
    return 1;
}

// Java: private static String formatDependencyError(IModInfo.ModVersion dependency, Map<String, ArtifactVersion> modVersions)
static char *formatDependencyError(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *dependency,
                                   const LIBMATTI_FML_DependencyResolutionResult *result)
{
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *installed = NULL;
    for (size_t i = 0; i < result->modVersionCount; i++)
        if (strcmp(result->modVersionKeys[i], LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency)) == 0)
            installed = result->modVersionValues[i];

    size_t length = strlen("\tMod ID: '', Requested by: '', Expected range: '', Actual version: '[MISSING]'") +
                    strlen(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency)) +
                    strlen(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(dependency))) +
                    strlen(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(dependency)->spec) +
                    (installed != NULL ? strlen(installed->version) : 0) + 32;
    char *message = malloc(length);
    snprintf(message, length, "\tMod ID: '%s', Requested by: '%s', Expected range: '%s', Actual version: '%s'",
             LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency),
             LIBMATTI_NEOFORGESPI_IModInfo_GetModId(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(dependency)),
             LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(dependency)->spec,
             installed != NULL ? installed->version : "[MISSING]");
    return message;
}

// Java: private static String formatIncompatibleDependencyError(IModInfo.ModVersion dependency, String type, Map<String, ArtifactVersion> modVersions)
static char *formatIncompatibleDependencyError(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *dependency, const char *type,
                                               const LIBMATTI_FML_DependencyResolutionResult *result)
{
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *version = NULL;
    for (size_t i = 0; i < result->modVersionCount; i++)
        if (strcmp(result->modVersionKeys[i], LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency)) == 0)
            version = result->modVersionValues[i];

    size_t length = 128 + strlen(type) + strlen(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency)) +
                    strlen(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(dependency)->spec) +
                    strlen(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(dependency))) +
                    (version != NULL ? strlen(version->version) : 0);
    char *message = malloc(length);
    snprintf(message, length, "\tMod '%s' %s '%s', versions: '%s'; Version found: '%s'",
             LIBMATTI_NEOFORGESPI_IModInfo_GetModId(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(dependency)),
             type, LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependency),
             LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(dependency)->spec,
             version != NULL ? version->version : "null");
    return message;
}

// Java: private boolean modVersionNotContained(IModInfo.ModVersion mv, Map<String, ArtifactVersion> modVersions)
typedef struct
{
    const LIBMATTI_FML_DependencyResolutionResult *result;
} ModVersionLookup;

static int standardLookup(const char *modId, const LIBMATTI_NEOFORGESPI_VersionRange *declaredRange, void *userdata)
{
    ModVersionLookup *lookupContext = userdata;
    for (size_t i = 0; i < lookupContext->result->modVersionCount; i++)
    {
        if (strcmp(lookupContext->result->modVersionKeys[i], modId) != 0) continue;
        const LIBMATTI_NEOFORGESPI_ArtifactVersion *version = lookupContext->result->modVersionValues[i];
        // Java: range.containsVersion(version) || version.toString().equals("0.0NONE")
        return LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(declaredRange, version) ||
               strcmp(version->version, "0.0NONE") == 0;
    }
    return 0;
}

static int modVersionNotContained(LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *mv,
                                  const LIBMATTI_FML_DependencyResolutionResult *result)
{
    // Java: FMLLoader.getCurrent().getVersionSupportMatrix()
    LIBMATTI_FML_VersionSupportMatrix *matrix = (LIBMATTI_FML_VersionSupportMatrix *)
        LIBMATTI_FML_FMLLoader_GetVersionSupportMatrix(LIBMATTI_FML_FMLLoader_GetCurrent());

    ModVersionLookup lookupContext = {result};
    return !LIBMATTI_FML_VersionSupportMatrix_TestVersionSupportMatrix(
        matrix, LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(mv),
        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv), "mod", standardLookup, &lookupContext);
}

// Java: private DependencyResolutionResult verifyDependencyVersions()
static LIBMATTI_FML_DependencyResolutionResult verifyDependencyVersions(ModSorter *sorter)
{
    LIBMATTI_FML_DependencyResolutionResult result = {0};

    // Java: var modVersions = modFiles.stream().map(ModFile::getModInfos).mapMulti(Iterable::forEach).collect(toMap(IModInfo::getModId, IModInfo::getVersion));
    result.modVersionKeys = malloc(sizeof(char *) * (sorter->modFileCount > 0 ? sorter->modFileCount : 1));
    result.modVersionValues = malloc(sizeof(LIBMATTI_NEOFORGESPI_ArtifactVersion *) *
                                     (sorter->modFileCount > 0 ? sorter->modFileCount : 1));
    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
            sorter->modFiles[i]->modFileInfo, &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            result.modVersionKeys[result.modVersionCount] = (char *) LIBMATTI_NEOFORGESPI_IModInfo_GetModId(modInfos[j]);
            result.modVersionValues[result.modVersionCount] =
                (LIBMATTI_NEOFORGESPI_ArtifactVersion *) LIBMATTI_NEOFORGESPI_IModInfo_GetVersion(modInfos[j]);
            result.modVersionCount++;
        }
    }

    // Java: var modRequirements = modVersionDependencies.values().stream().<IModInfo.ModVersion>mapMulti(Iterable::forEach).filter(mv -> mv.getSide().isCorrectSide()).collect(toSet());
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **requirements = NULL;
    size_t requirementCount = 0;
    for (size_t i = 0; i < sorter->modFileCount; i++)
    {
        size_t modInfoCount = 0;
        LIBMATTI_NEOFORGESPI_IModInfo **modInfos = LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(
            sorter->modFiles[i]->modFileInfo, &modInfoCount);
        for (size_t j = 0; j < modInfoCount; j++)
        {
            // Java: consider overrides and invalidate dependencies that are removed
            const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(modInfos[j]);
            size_t overrideCount = 0;
            LIBMATTI_FML_FMLConfig_DependencyOverride **overrides =
                LIBMATTI_FML_FMLConfig_GetOverrides(modId, &overrideCount);
            size_t dependencyCount = 0;
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **dependencies =
                LIBMATTI_NEOFORGESPI_IModInfo_GetDependencies(modInfos[j], &dependencyCount);
            for (size_t k = 0; k < dependencyCount; k++)
            {
                if (!LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsCorrectSide(
                        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetSide(dependencies[k])))
                    continue;
                if (overrideCount > 0)
                {
                    // Java: ids = overrides.filter(DependencyOverride::remove).map(::modId).toSet();
                    //       filter(v -> !ids.contains(v.getModId()))
                    const char *dependencyModId =
                        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(dependencies[k]);
                    int removed = 0;
                    for (size_t o = 0; o < overrideCount && !removed; o++)
                        removed = overrides[o]->remove && strcmp(overrides[o]->modId, dependencyModId) == 0;
                    if (removed) continue;
                }
                versionSetAdd(&requirements, &requirementCount, dependencies[k]);
            }
        }
    }

    size_t mandatoryRequired = 0;
    for (size_t i = 0; i < requirementCount; i++)
        if (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(requirements[i]) ==
            LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED)
            mandatoryRequired++;
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                             "Found {} mod requirements ({} mandatory, {} optional)",
                             formatNumber((long long) requirementCount), formatNumber((long long) mandatoryRequired),
                             formatNumber((long long) (requirementCount - mandatoryRequired)));

    // Java: var missingVersions = modRequirements.stream().filter(mv -> (mv.getType() == REQUIRED || (modVersions.containsKey(mv.getModId()) && mv.getType() == OPTIONAL)) && this.modVersionNotContained(mv, modVersions)).collect(toSet());
    for (size_t i = 0; i < requirementCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo_DependencyType type =
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(requirements[i]);
        int present = 0;
        for (size_t j = 0; j < result.modVersionCount; j++)
            if (strcmp(result.modVersionKeys[j],
                       LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(requirements[i])) == 0)
                present = 1;

        if ((type == LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED ||
             (present && type == LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_OPTIONAL)) &&
            modVersionNotContained(requirements[i], &result))
            versionSetAdd(&result.versionResolution, &result.versionResolutionCount, requirements[i]);
    }

    size_t mandatoryMissing = 0;
    for (size_t i = 0; i < result.versionResolutionCount; i++)
        if (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(result.versionResolution[i]) ==
            LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED)
            mandatoryMissing++;
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                             "Found {} mod requirements missing ({} mandatory, {} optional)",
                             formatNumber((long long) result.versionResolutionCount),
                             formatNumber((long long) mandatoryMissing),
                             formatNumber((long long) (result.versionResolutionCount - mandatoryMissing)));

    // Java: var incompatibleVersions / discouragedVersions
    for (size_t i = 0; i < requirementCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo_DependencyType type =
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(requirements[i]);
        if (type != LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_INCOMPATIBLE &&
            type != LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_DISCOURAGED)
            continue;

        int present = 0;
        for (size_t j = 0; j < result.modVersionCount; j++)
            if (strcmp(result.modVersionKeys[j],
                       LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(requirements[i])) == 0)
                present = 1;
        if (!present || modVersionNotContained(requirements[i], &result))
            continue;

        if (type == LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_INCOMPATIBLE)
            versionSetAdd(&result.incompatibilities, &result.incompatibilityCount, requirements[i]);
        else
            versionSetAdd(&result.discouraged, &result.discouragedCount, requirements[i]);
    }

    if (result.discouragedCount > 0)
    {
        for (size_t i = 0; i < result.discouragedCount; i++)
        {
            char *message = formatIncompatibleDependencyError(result.discouraged[i], "discourages", &result);
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Conflicts between mods:{}", message);
            free(message);
        }
    }
    if (mandatoryMissing > 0)
    {
        for (size_t i = 0; i < result.versionResolutionCount; i++)
        {
            if (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(result.versionResolution[i]) !=
                LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED)
                continue;
            char *message = formatDependencyError(result.versionResolution[i], &result);
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                     "Missing or unsupported mandatory dependencies:{}", message);
            free(message);
        }
    }
    if (result.versionResolutionCount - mandatoryMissing > 0)
    {
        for (size_t i = 0; i < result.versionResolutionCount; i++)
        {
            if (LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(result.versionResolution[i]) !=
                LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_OPTIONAL)
                continue;
            char *message = formatDependencyError(result.versionResolution[i], &result);
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                     "Unsupported installed optional dependencies:{}", message);
            free(message);
        }
    }
    if (result.incompatibilityCount > 0)
    {
        for (size_t i = 0; i < result.incompatibilityCount; i++)
        {
            char *message = formatIncompatibleDependencyError(result.incompatibilities[i], "is incompatible with",
                                                              &result);
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Incompatibilities between mods:{}",
                                     message);
            free(message);
        }
    }

    free(requirements);
    return result;
}

// Java: public List<ModLoadingIssue> buildWarningMessages()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_DependencyResolutionResult_BuildWarningMessages(
    const LIBMATTI_FML_DependencyResolutionResult *result, size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **issues = malloc(sizeof(LIBMATTI_FML_ModLoadingIssue *) *
                                                   (result->discouragedCount + 1));
    *count = 0;
    for (size_t i = 0; i < result->discouragedCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *mv = result->discouraged[i];
        const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv);
        const char *ownerModId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv));
        const char *installed = "null";
        for (size_t j = 0; j < result->modVersionCount; j++)
            if (strcmp(result->modVersionKeys[j], modId) == 0)
                installed = result->modVersionValues[j]->version;
        const char *reason = NULL;
        if (!LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetReason(mv, &reason))
            reason = "fml.modloading.discouragedmod.noreason";

        const char *args[] = {modId, ownerModId,
                              LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(mv)->spec, installed, reason};
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Warning(
            "fml.modloadingissue.discouragedmod", args, 5);
        issues[(*count)++] = LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(
            issue, LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv),
            LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv),
            LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(
                LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(
                    LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(
                        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv)))));
    }

    // Java: Stream.of(ModLoadingIssue.warning("fml.modloadingissue.discouragedmod.proceed"))
    issues[(*count)++] = LIBMATTI_FML_ModLoadingIssue_Warning("fml.modloadingissue.discouragedmod.proceed", NULL, 0);
    return issues;
}

// Java: public List<ModLoadingIssue> buildErrorMessages()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_DependencyResolutionResult_BuildErrorMessages(
    const LIBMATTI_FML_DependencyResolutionResult *result, size_t *count)
{
    LIBMATTI_FML_ModLoadingIssue **issues = malloc(
        sizeof(LIBMATTI_FML_ModLoadingIssue *) * (result->versionResolutionCount + result->incompatibilityCount));
    *count = 0;

    for (size_t i = 0; i < result->versionResolutionCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *mv = result->versionResolution[i];
        int required = LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetType(mv) ==
                       LIBMATTI_NEOFORGESPI_IModInfo_DependencyType_REQUIRED;
        const char *installed = "null";
        for (size_t j = 0; j < result->modVersionCount; j++)
            if (strcmp(result->modVersionKeys[j], LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv)) == 0)
                installed = result->modVersionValues[j]->version;

        const char *args[] = {LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv),
                              LIBMATTI_NEOFORGESPI_IModInfo_GetModId(
                                  LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv)),
                              LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(mv)->spec, installed};
        issues[(*count)++] = LIBMATTI_FML_ModLoadingIssue_Error(
            required ? "fml.modloadingissue.missingdependency" : "fml.modloadingissue.missingdependency.optional",
            args, 4);
    }

    for (size_t i = 0; i < result->incompatibilityCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModInfo_ModVersion *mv = result->incompatibilities[i];
        const char *installed = "null";
        for (size_t j = 0; j < result->modVersionCount; j++)
            if (strcmp(result->modVersionKeys[j], LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv)) == 0)
                installed = result->modVersionValues[j]->version;

        const char *args[] = {LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetModId(mv),
                              LIBMATTI_NEOFORGESPI_IModInfo_GetModId(
                                  LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetOwner(mv)),
                              LIBMATTI_NEOFORGESPI_IModInfo_ModVersion_GetVersionRange(mv)->spec, installed};
        issues[(*count)++] = LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.incompatiblemod", args, 4);
    }

    return issues;
}

// Java: public static LoadingModList sort(List<ModFile> modFiles, List<ModLoadingIssue> issues)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_ModSorter_Sort(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
                                                         LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount)
{
    // Java: split the mod files into plugin, game library and game content
    LIBMATTI_FML_ModFile **pluginContent = malloc(sizeof(LIBMATTI_FML_ModFile *) * (modFileCount > 0 ? modFileCount : 1));
    size_t pluginCount = 0;
    LIBMATTI_FML_ModFile **gameLibraryContent = malloc(sizeof(LIBMATTI_FML_ModFile *) * (modFileCount > 0 ? modFileCount : 1));
    size_t gameLibraryCount = 0;
    LIBMATTI_FML_ModFile **gameContent = malloc(sizeof(LIBMATTI_FML_ModFile *) * (modFileCount > 0 ? modFileCount : 1));
    size_t gameCount = 0;

    for (size_t i = 0; i < modFileCount; i++)
    {
        if (LIBMATTI_NEOFORGESPI_IModFile_GetType(LIBMATTI_FML_ModFile_AsModFile(modFiles[i])) ==
            LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY)
            pluginContent[pluginCount++] = modFiles[i];
        else if (LIBMATTI_NEOFORGESPI_IModFile_GetType(LIBMATTI_FML_ModFile_AsModFile(modFiles[i])) ==
                 LIBMATTI_NEOFORGESPI_IModFile_Type_GAMELIBRARY)
            gameLibraryContent[gameLibraryCount++] = modFiles[i];
        else
            gameContent[gameCount++] = modFiles[i];
    }

    LIBMATTI_FML_LoadingModList *list = LIBMATTI_FML_ModSorter_SortByType(
        pluginContent, pluginCount, gameLibraryContent, gameLibraryCount, gameContent, gameCount, issues, issueCount);

    free(pluginContent);
    free(gameLibraryContent);
    free(gameContent);
    return list;
}

// Java has no destructor; the C port releases what sort() allocated
static void freeSorter(ModSorter *sorter)
{
    free(sorter->systemMods);
    free(sorter->modFiles);
    free(sorter->sortedList);
    for (size_t i = 0; i < sorter->lookupCount; i++)
        free(sorter->lookupKeys[i]);
    free(sorter->lookupKeys);
    free(sorter->lookupValues);
    free(sorter->uniqueModListBuilder);
}

// Java: the dependency map is handed to the LoadingModList, so only the local copy is released here
static void freeModDependencies(LIBMATTI_FML_ModDependencyMap *dependencies)
{
    for (size_t i = 0; i < dependencies->count; i++)
        free(dependencies->values[i]);
    free(dependencies->keys);
    free(dependencies->values);
    free(dependencies->valueCounts);
}

static void freeResolutionResult(LIBMATTI_FML_DependencyResolutionResult *result)
{
    free(result->modVersionKeys);
    free(result->modVersionValues);
    free(result->versionResolution);
    free(result->incompatibilities);
    free(result->discouraged);
}

// Java: public static LoadingModList sort(List<ModFile> plugins, List<ModFile> gameLibraries, List<ModFile> mods, List<ModLoadingIssue> issues)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_ModSorter_SortByType(
    LIBMATTI_FML_ModFile **plugins, size_t pluginCount, LIBMATTI_FML_ModFile **gameLibraries,
    size_t gameLibraryCount, LIBMATTI_FML_ModFile **mods, size_t modCount, LIBMATTI_FML_ModLoadingIssue **issues,
    size_t issueCount)
{
    ModSorter sorter = {0};
    sorter.uniqueModListBuilder = LIBMATTI_FML_UniqueModListBuilder_New(mods, modCount);

    if (!buildUniqueList(&sorter))
    {
        // Java: catch (ModLoadingException e) { return LoadingModList.of(plugins, gameLibraries, ms.systemMods, ..., concat(issues, e.getIssues()), Map.of()); }
        size_t systemCount = 0;
        LIBMATTI_FML_ModInfo **systemModInfos = malloc(sizeof(LIBMATTI_FML_ModInfo *) * (sorter.systemModCount > 0 ? sorter.systemModCount : 1));
        for (size_t i = 0; i < sorter.systemModCount; i++)
            systemModInfos[systemCount++] = LIBMATTI_FML_ModInfo_AsConcrete(
                LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(sorter.systemMods[i]->modFileInfo, &(size_t){0})[0]);
        LIBMATTI_FML_LoadingModList *list = LIBMATTI_FML_LoadingModList_Of(
            plugins, pluginCount, gameLibraries, gameLibraryCount, sorter.systemMods, sorter.systemModCount,
            systemModInfos, systemCount, issues, issueCount, NULL);
        free(systemModInfos);
        freeSorter(&sorter);
        return list;
    }

    LIBMATTI_FML_DependencyResolutionResult resolutionResult = verifyDependencyVersions(&sorter);
    LIBMATTI_FML_LoadingModList *list;

    // Java: if (!resolutionResult.versionResolution.isEmpty() || !resolutionResult.incompatibilities.isEmpty())
    if (resolutionResult.versionResolutionCount > 0 || resolutionResult.incompatibilityCount > 0)
    {
        size_t errorCount = 0;
        LIBMATTI_FML_ModLoadingIssue **errors = LIBMATTI_FML_DependencyResolutionResult_BuildErrorMessages(
            &resolutionResult, &errorCount);
        size_t combinedCount = 0;
        LIBMATTI_FML_ModLoadingIssue **combined = concat(issues, issueCount, errors, errorCount, &combinedCount);

        size_t systemCount = 0;
        LIBMATTI_FML_ModInfo **systemModInfos = malloc(sizeof(LIBMATTI_FML_ModInfo *) * (sorter.systemModCount > 0 ? sorter.systemModCount : 1));
        for (size_t i = 0; i < sorter.systemModCount; i++)
        {
            size_t ignored = 0;
            systemModInfos[systemCount++] = LIBMATTI_FML_ModInfo_AsConcrete(
                LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(sorter.systemMods[i]->modFileInfo, &ignored)[0]);
        }

        list = LIBMATTI_FML_LoadingModList_Of(plugins, pluginCount, gameLibraries, gameLibraryCount, sorter.systemMods,
                                              sorter.systemModCount, systemModInfos, systemCount, combined,
                                              combinedCount, NULL);
        free(errors);
        free(combined);
        free(systemModInfos);
    }
    else
    {
        LIBMATTI_FML_ModLoadingIssue **sortIssues = NULL;
        size_t sortIssueCount = 0;
        if (sortMods(&sorter, &sortIssues, &sortIssueCount))
        {
            list = LIBMATTI_FML_LoadingModList_Of(plugins, pluginCount, gameLibraries, gameLibraryCount,
                                                  sorter.modFiles, sorter.modFileCount, sorter.sortedList,
                                                  sorter.sortedCount, issues, issueCount, &sorter.modDependencies);
            free(sortIssues);
        }
        else
        {
            size_t combinedCount = 0;
            LIBMATTI_FML_ModLoadingIssue **combined = concat(issues, issueCount, sortIssues, sortIssueCount,
                                                             &combinedCount);
            size_t systemCount = 0;
            LIBMATTI_FML_ModInfo **systemModInfos = malloc(sizeof(LIBMATTI_FML_ModInfo *) * (sorter.systemModCount > 0 ? sorter.systemModCount : 1));
            for (size_t i = 0; i < sorter.systemModCount; i++)
            {
                size_t ignored = 0;
                systemModInfos[systemCount++] = LIBMATTI_FML_ModInfo_AsConcrete(
                    LIBMATTI_NEOFORGESPI_IModFileInfo_GetMods(sorter.systemMods[i]->modFileInfo, &ignored)[0]);
            }

            list = LIBMATTI_FML_LoadingModList_Of(plugins, pluginCount, gameLibraries, gameLibraryCount,
                                                  sorter.systemMods, sorter.systemModCount, systemModInfos,
                                                  systemCount, combined, combinedCount, NULL);
            free(combined);
            free(sortIssues);
            free(systemModInfos);
            freeModDependencies(&sorter.modDependencies);
        }
    }

    // Java: if (!resolutionResult.discouraged.isEmpty()) list.getModLoadingIssues().addAll(resolutionResult.buildWarningMessages());
    if (resolutionResult.discouragedCount > 0)
    {
        size_t warningCount = 0;
        LIBMATTI_FML_ModLoadingIssue **warnings =
            LIBMATTI_FML_DependencyResolutionResult_BuildWarningMessages(&resolutionResult, &warningCount);
        LIBMATTI_FML_LoadingModList_AddIssues(list, warnings, warningCount);
        free(warnings);
    }

    // Java: close any mod-files discarded due to dependency constraint issues
    for (size_t i = 0; i < sorter.modFileCount; i++)
    {
        if (LIBMATTI_FML_LoadingModList_Contains(list,
                                                 LIBMATTI_FML_ModFile_AsModFile(sorter.modFiles[i])))
            continue;
        LIBMATTI_FML_ModFile_Close(sorter.modFiles[i]);
    }

    freeResolutionResult(&resolutionResult);
    freeSorter(&sorter);
    return list;
}
