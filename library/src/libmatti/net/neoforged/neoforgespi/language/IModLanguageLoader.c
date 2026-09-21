#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_IModLanguageLoader_Name(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader)
{
    return loader->name(loader->self);
}

// Java: public String version()
const char *LIBMATTI_NEOFORGESPI_IModLanguageLoader_Version(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader)
{
    return loader->version(loader->self);
}

// Java: public ModContainer loadMod(IModInfo info, ModFileScanData modFileScanResults, ModuleLayer layer)
void *LIBMATTI_NEOFORGESPI_IModLanguageLoader_LoadMod(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                     void *info, void *modFileScanResults,
                                                     LIBMATTI_JL_ModuleLayer *layer)
{
    return loader->loadMod(loader->self, info, modFileScanResults, layer);
}

// Java: public default void validate(IModFile file, Collection<ModContainer> loadedContainers, IIssueReporting reporter)
void LIBMATTI_NEOFORGESPI_IModLanguageLoader_Validate(const LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                      void *file, void **loadedContainers,
                                                      size_t loadedContainerCount,
                                                      LIBMATTI_NEOFORGESPI_IIssueReporting *reporter)
{
    if (loader->validate != NULL)
        loader->validate(loader->self, file, loadedContainers, loadedContainerCount, reporter);
}
