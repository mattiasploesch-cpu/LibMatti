// Port of net.neoforged.fml.loading.moddiscovery.ModDiscoverer.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODDISCOVERER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODDISCOVERER_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDependencyLocator.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileReader.h"

#include <stddef.h>

// Java: public record Result(List<ModFile> modFiles, List<ModLoadingIssue> discoveryIssues)
typedef struct
{
    LIBMATTI_FML_ModFile **modFiles;
    size_t modFileCount;
    LIBMATTI_FML_ModLoadingIssue **discoveryIssues;
    size_t discoveryIssueCount;
} LIBMATTI_FML_ModDiscoverer_Result;

// Java: public class ModDiscoverer
typedef struct LIBMATTI_FML_ModDiscoverer LIBMATTI_FML_ModDiscoverer;

struct LIBMATTI_FML_ModDiscoverer
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator **modFileLocators;
    size_t modFileLocatorCount;
    LIBMATTI_NEOFORGESPI_IDependencyLocator **dependencyLocators;
    size_t dependencyLocatorCount;
    LIBMATTI_NEOFORGESPI_IModFileReader **modFileReaders;
    size_t modFileReaderCount;
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext;
};

// Java: public ModDiscoverer(ILaunchContext launchContext)
LIBMATTI_FML_ModDiscoverer *LIBMATTI_FML_ModDiscoverer_New(LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext);

// Java: public ModDiscoverer(ILaunchContext launchContext, Collection<IModFileCandidateLocator> additionalModFileLocators)
LIBMATTI_FML_ModDiscoverer *LIBMATTI_FML_ModDiscoverer_NewWithLocators(
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext,
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator **additionalModFileLocators, size_t additionalCount);

// Java: public Result discoverMods(List<ModFile> additionalDependencySources)
LIBMATTI_FML_ModDiscoverer_Result *LIBMATTI_FML_ModDiscoverer_DiscoverMods(
    LIBMATTI_FML_ModDiscoverer *discoverer, LIBMATTI_FML_ModFile **additionalDependencySources, size_t count);

// Java has no destructor; the C port releases what the constructor allocated.
void LIBMATTI_FML_ModDiscoverer_Free(LIBMATTI_FML_ModDiscoverer *discoverer);
// Java has no destructor; the port releases the Result wrapper (its lists are handed to the caller).
void LIBMATTI_FML_ModDiscoverer_Result_Free(LIBMATTI_FML_ModDiscoverer_Result *result);

// Java: private String logReport(Collection<List<ModFile>> modFiles)
char *LIBMATTI_FML_ModDiscoverer_LogReport(LIBMATTI_FML_ModFile **modFiles, size_t modFileCount);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODDISCOVERER_H
