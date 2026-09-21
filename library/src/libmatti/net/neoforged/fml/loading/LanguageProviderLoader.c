#include "libmatti/net/neoforged/fml/loading/LanguageProviderLoader.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/VersionSupportMatrix.h"
#include "libmatti/net/neoforged/fml/javafmlmod/FMLJavaModLanguageProvider.h"
#include "libmatti/net/neoforged/fml/mclanguageprovider/MinecraftModLanguageProvider.h"
#include "libmatti/net/neoforged/fml/util/ServiceLoaderUtil.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogUtils.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private record ModLanguageWrapper(IModLanguageLoader modLanguageProvider, ArtifactVersion version) {}
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModLanguageLoader *modLanguageProvider;
    LIBMATTI_NEOFORGESPI_ArtifactVersion *version;
    char *name;
} ModLanguageWrapper;

struct LIBMATTI_FML_LanguageProviderLoader
{
    LIBMATTI_NEOFORGESPI_IModLanguageLoader **languageProviders;
    size_t languageProviderCount;
    ModLanguageWrapper *languageProviderMap;
    size_t languageProviderMapCount;
};

// Java: p.get()
static void *instantiateLanguageProvider(const LIBMATTI_JU_ServiceLoader_Provider *provider)
{
    return provider->provider;
}

// Java: the loaders shipped inside the FML module (loader/src/main/resources/META-INF/services/
// net.neoforged.neoforgespi.language.IModLanguageLoader). The C library has no service file to scan,
// so they are handed to ServiceLoaderUtil as the additional services, which is what Java's
// ServiceLoader also finds them as.
static void builtInLanguageProviders(LIBMATTI_FML_Service *services)
{
    services[0].service = LIBMATTI_FML_FMLJavaModLanguageProvider_AsLanguageLoader();
    services[0].serviceClassName = "net.neoforged.fml.javafmlmod.FMLJavaModLanguageProvider";
    services[0].builtIn = 0;
    services[1].service = LIBMATTI_FML_MinecraftModLanguageProvider_AsLanguageLoader();
    services[1].serviceClassName = "net.neoforged.fml.mclanguageprovider.MinecraftModLanguageProvider";
    services[1].builtIn = 0;
}

// Java: ServiceLoaderUtil.loadServices(launchContext, IModLanguageLoader.class)
static void loadServices(LIBMATTI_FML_LanguageProviderLoader *loader, LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext)
{
    LIBMATTI_FML_ServiceLoaderUtil_Hooks hooks = {
        .filter = NULL,
        .applyPriority = 0,
        .getPriority = NULL,
        .instantiate = instantiateLanguageProvider,
    };
    LIBMATTI_FML_Service builtIns[2];
    builtInLanguageProviders(builtIns);
    LIBMATTI_FML_ServiceList *services = LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(
        launchContext, "net.neoforged.neoforgespi.language.IModLanguageLoader", builtIns, 2, &hooks);

    loader->languageProviders = calloc(services->count > 0 ? services->count : 1, sizeof(void *));
    loader->languageProviderCount = services->count;
    for (size_t i = 0; i < services->count; i++)
        loader->languageProviders[i] = services->services[i].service;

    LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(services);
}

// Java: LanguageProviderLoader(ILaunchContext launchContext)
LIBMATTI_FML_LanguageProviderLoader *LIBMATTI_FML_LanguageProviderLoader_New(
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext)
{
    LIBMATTI_FML_LanguageProviderLoader *loader = calloc(1, sizeof(LIBMATTI_FML_LanguageProviderLoader));
    loadServices(loader, launchContext);
    LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress("Loading language providers");

    for (size_t i = 0; i < loader->languageProviderCount; i++)
    {
        LIBMATTI_NEOFORGESPI_IModLanguageLoader *lp = loader->languageProviders[i];
        const char *name = LIBMATTI_NEOFORGESPI_IModLanguageLoader_Name(lp);
        const char *version = LIBMATTI_NEOFORGESPI_IModLanguageLoader_Version(lp);

        // Java: if (version == null || version.isBlank()) { error; throw new RuntimeException(...); }
        if (version == NULL || version[0] == '\0')
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Found unversioned language provider {}",
                                     name);
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "Failed to find implementation version for language provider {}", name);
        }
        else
        {
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Found language provider {}, version {}",
                                     name, version);
            char message[256];
            snprintf(message, sizeof(message), "Loaded language provider %s %s", name, version);
            LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress(message);
        }

        loader->languageProviderMap = realloc(loader->languageProviderMap,
                                              sizeof(ModLanguageWrapper) * (loader->languageProviderMapCount + 1));
        ModLanguageWrapper *wrapper = &loader->languageProviderMap[loader->languageProviderMapCount];
        wrapper->modLanguageProvider = lp;
        wrapper->version = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(version != NULL ? version : "");
        wrapper->name = strdup(name != NULL ? name : "");
        loader->languageProviderMapCount++;
    }

    return loader;
}

// Java: public void forEach(Consumer<IModLanguageLoader> consumer)
void LIBMATTI_FML_LanguageProviderLoader_ForEach(LIBMATTI_FML_LanguageProviderLoader *loader,
                                                 void (*consumer)(LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                                  void *userdata),
                                                 void *userdata)
{
    for (size_t i = 0; i < loader->languageProviderCount; i++)
        consumer(loader->languageProviders[i], userdata);
}

// Java: public <T> Stream<T> applyForEach(Function<IModLanguageLoader, T> function)
void **LIBMATTI_FML_LanguageProviderLoader_ApplyForEach(LIBMATTI_FML_LanguageProviderLoader *loader,
                                                        void *(*function)(LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                                          void *userdata),
                                                        void *userdata, size_t *count)
{
    *count = loader->languageProviderCount;
    void **results = calloc(loader->languageProviderCount > 0 ? loader->languageProviderCount : 1, sizeof(void *));
    for (size_t i = 0; i < loader->languageProviderCount; i++)
        results[i] = function(loader->languageProviders[i], userdata);
    return results;
}

// Java: the Range.containsVersion(mlw.version()) lambda of findLanguage
typedef struct
{
    LIBMATTI_NEOFORGESPI_ArtifactVersion *version;
} ContainsVersionUserdata;

static int standardLookupContainsVersion(const char *lookupId, const LIBMATTI_NEOFORGESPI_VersionRange *range,
                                         void *userdata)
{
    (void) lookupId;
    ContainsVersionUserdata *data = userdata;
    return LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(range, data->version);
}

// Java: public IModLanguageLoader findLanguage(ModFile mf, @Nullable String modLoader, @Nullable VersionRange modLoaderVersion)
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_LanguageProviderLoader_FindLanguage(
    LIBMATTI_FML_LanguageProviderLoader *loader, void *modFile, const char *modLoader,
    const LIBMATTI_NEOFORGESPI_VersionRange *modLoaderVersion)
{
    // Java: Remove this in 1.21.6 or beyond
    if (modLoader != NULL && strcmp(modLoader, "lowcodefml") == 0)
    {
        LIBMATTI_ML_Logger_Warn(LOGGER(), NULL,
                                "Mod {} is using the deprecated 'lowcodefml' language loader. This warning can be fixed by simply removing the modLoader and loaderVersion settings from neoforge.mods.toml.",
                                LIBMATTI_NEOFORGESPI_IModFile_GetFileName(
                                    (LIBMATTI_NEOFORGESPI_IModFile *) modFile));
        modLoader = NULL;
        modLoaderVersion = NULL;
    }

    // Java: We default to the java language loader now, since it also supports use-cases without any code
    if (modLoader == NULL)
    {
        modLoader = LIBMATTI_FML_FMLJavaModLanguageProvider_NAME;
    }

    ModLanguageWrapper *mlw = NULL;
    for (size_t i = 0; i < loader->languageProviderMapCount; i++)
    {
        if (strcmp(loader->languageProviderMap[i].name, modLoader) == 0)
        {
            mlw = &loader->languageProviderMap[i];
            break;
        }
    }

    if (mlw == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING, "Missing language loader {} wanted by {}",
                                 modLoader,
                                 LIBMATTI_NEOFORGESPI_IModFile_GetFileName(
                                     (LIBMATTI_NEOFORGESPI_IModFile *) modFile));
        // Java: throw new ModLoadingException(ModLoadingIssue.error("fml.modloadingissue.language.missing", modLoader).withAffectedModFile(mf));
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.language.missing", (const char *[]){modLoader}, 1);
        LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(issue, modFile, NULL);
        LIBMATTI_FML_ModLoadingIssue_Free(issue);
        return NULL;
    }

    LIBMATTI_FML_VersionSupportMatrix *versionSupportMatrix =
        LIBMATTI_FML_FMLLoader_GetVersionSupportMatrix(LIBMATTI_FML_FMLLoader_GetCurrent());
    if (modLoaderVersion != NULL)
    {
        ContainsVersionUserdata data = {mlw->version};
        if (!LIBMATTI_FML_VersionSupportMatrix_TestVersionSupportMatrix(versionSupportMatrix, modLoaderVersion,
                                                                        modLoader, "languageloader",
                                                                        standardLookupContainsVersion, &data))
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_LOADING,
                                     "Language loader {} version {} is incompatible with {} required by {}", modLoader,
                                     mlw->version->version, modLoaderVersion->spec,
                                     LIBMATTI_NEOFORGESPI_IModFile_GetFileName(
                                         (LIBMATTI_NEOFORGESPI_IModFile *) modFile));
            LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                "fml.modloadingissue.language.missingversion",
                (const char *[]){modLoader, modLoaderVersion->spec, mlw->version->version}, 3);
            LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(issue, modFile, NULL);
            LIBMATTI_FML_ModLoadingIssue_Free(issue);
            return NULL;
        }
    }

    return mlw->modLanguageProvider;
}

void LIBMATTI_FML_LanguageProviderLoader_Free(LIBMATTI_FML_LanguageProviderLoader *loader)
{
    if (loader == NULL)
        return;

    // Java's wrappers are collected with the loader; the port frees what the map owns.
    for (size_t i = 0; i < loader->languageProviderMapCount; i++)
    {
        free(loader->languageProviderMap[i].name);
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(loader->languageProviderMap[i].version);
    }

    free(loader->languageProviders);
    free(loader->languageProviderMap);
    free(loader);
}
