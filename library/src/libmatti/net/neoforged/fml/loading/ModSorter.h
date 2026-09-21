// Port of net.neoforged.fml.loading.ModSorter.
// Dependency overrides resolve through FMLConfig.getDependencyOverrides().

#ifndef MATTICRAFT_FML_LOADING_MODSORTER_H
#define MATTICRAFT_FML_LOADING_MODSORTER_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

#include <stddef.h>

// Java: public record DependencyResolutionResult(Collection<IModInfo.ModVersion> incompatibilities,
//         Collection<IModInfo.ModVersion> discouraged, Collection<IModInfo.ModVersion> versionResolution,
//         Map<String, ArtifactVersion> modVersions)
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **incompatibilities;
    size_t incompatibilityCount;
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **discouraged;
    size_t discouragedCount;
    LIBMATTI_NEOFORGESPI_IModInfo_ModVersion **versionResolution;
    size_t versionResolutionCount;
    // Java: Map<String, ArtifactVersion> modVersions
    char **modVersionKeys;
    LIBMATTI_NEOFORGESPI_ArtifactVersion **modVersionValues;
    size_t modVersionCount;
} LIBMATTI_FML_DependencyResolutionResult;

// Java: public static LoadingModList sort(List<ModFile> modFiles, List<ModLoadingIssue> issues)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_ModSorter_Sort(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount,
                                                         LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount);

// Java: public static LoadingModList sort(List<ModFile> plugins, List<ModFile> gameLibraries, List<ModFile> mods, List<ModLoadingIssue> issues)
LIBMATTI_FML_LoadingModList *LIBMATTI_FML_ModSorter_SortByType(
    LIBMATTI_FML_ModFile **plugins, size_t pluginCount, LIBMATTI_FML_ModFile **gameLibraries,
    size_t gameLibraryCount, LIBMATTI_FML_ModFile **mods, size_t modCount, LIBMATTI_FML_ModLoadingIssue **issues,
    size_t issueCount);

// Java: public List<ModLoadingIssue> buildWarningMessages()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_DependencyResolutionResult_BuildWarningMessages(
    const LIBMATTI_FML_DependencyResolutionResult *result, size_t *count);
// Java: public List<ModLoadingIssue> buildErrorMessages()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_DependencyResolutionResult_BuildErrorMessages(
    const LIBMATTI_FML_DependencyResolutionResult *result, size_t *count);

#endif //MATTICRAFT_FML_LOADING_MODSORTER_H
