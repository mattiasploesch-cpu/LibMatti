// Port of net.neoforged.fml.mclanguageprovider.MinecraftModLanguageProvider.

#include "libmatti/net/neoforged/fml/mclanguageprovider/MinecraftModLanguageProvider.h"

#include "libmatti/net/neoforged/fml/loading/BuiltInLanguageLoader.h"
#include "libmatti/net/neoforged/fml/mclanguageprovider/MinecraftModContainer.h"

static LIBMATTI_NEOFORGESPI_IModLanguageLoader languageLoader;

// Java: public String name() { return "minecraft"; }
static const char *provider_name(void *self)
{
    (void)self;
    return LIBMATTI_FML_MinecraftModLanguageProvider_NAME;
}

// Java: public String version() (BuiltInLanguageLoader)
static const char *provider_version(void *self)
{
    (void)self;
    return LIBMATTI_FML_BuiltInLanguageLoader_Version();
}

// Java: public ModContainer loadMod(IModInfo info, ModFileScanData modFileScanResults, ModuleLayer layer)
static void *provider_load_mod(void *self, void *info, void *modFileScanResults, LIBMATTI_JL_ModuleLayer *layer)
{
    (void)self;
    (void)modFileScanResults;
    (void)layer;

    // Java: return new MinecraftModContainer(info);
    return LIBMATTI_FML_MinecraftModContainer_New(info);
}

// Java: the instance used as an IModLanguageLoader
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_MinecraftModLanguageProvider_AsLanguageLoader(void)
{
    languageLoader.self = NULL;
    languageLoader.name = provider_name;
    languageLoader.version = provider_version;
    languageLoader.loadMod = provider_load_mod;
    // Java: the default validate() does nothing
    languageLoader.validate = NULL;
    return &languageLoader;
}
