// Port of net.neoforged.neoforgespi.language.IModLanguageLoader.
// The collaborators (ModContainer, ModFileScanData, ModLoadingException, IModFile,
// ModuleLayer, IIssueReporting) are all ported; ModContainer and ModFileScanData
// are passed as opaque pointers because the C interface keeps the Java types abstract.

#ifndef MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODLANGUAGELOADER_H
#define MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODLANGUAGELOADER_H

#include "libmatti/java/lang/ModuleLayer.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/neoforgespi/IIssueReporting.h"

#include <stddef.h>

// Java: public interface IModLanguageLoader
typedef struct LIBMATTI_NEOFORGESPI_IModLanguageLoader LIBMATTI_NEOFORGESPI_IModLanguageLoader;

struct LIBMATTI_NEOFORGESPI_IModLanguageLoader
{
    void *self;

    // Java: String name()
    const char *(*name)(void *self);
    // Java: String version()
    const char *(*version)(void *self);
    // Java: ModContainer loadMod(IModInfo info, ModFileScanData modFileScanResults, ModuleLayer layer)
    void *(*loadMod)(void *self, void *info, void *modFileScanResults, LIBMATTI_JL_ModuleLayer *layer);
    // Java: default void validate(IModFile file, Collection<ModContainer> loadedContainers, IIssueReporting reporter)
    void (*validate)(void *self, void *file, void **loadedContainers, size_t loadedContainerCount,
                     LIBMATTI_NEOFORGESPI_IIssueReporting *reporter);
};

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_IModLanguageLoader_Name(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader);
// Java: public String version()
const char *LIBMATTI_NEOFORGESPI_IModLanguageLoader_Version(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader);
// Java: public ModContainer loadMod(IModInfo info, ModFileScanData modFileScanResults, ModuleLayer layer)
void *LIBMATTI_NEOFORGESPI_IModLanguageLoader_LoadMod(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                     void *info, void *modFileScanResults,
                                                     LIBMATTI_JL_ModuleLayer *layer);
// Java: public default void validate(IModFile file, Collection<ModContainer> loadedContainers, IIssueReporting reporter)
void LIBMATTI_NEOFORGESPI_IModLanguageLoader_Validate(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                      void *file, void **loadedContainers, size_t loadedContainerCount,
                                                      LIBMATTI_NEOFORGESPI_IIssueReporting *reporter);

#endif //MATTICRAFT_NEOFORGESPI_LANGUAGE_IMODLANGUAGELOADER_H
