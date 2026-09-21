// Port of net.neoforged.fml.loading.LanguageProviderLoader.
// The FMLJavaModLanguageProvider and moddiscovery.ModFile are ported.

#ifndef MATTICRAFT_FML_LOADING_LANGUAGEPROVIDERLOADER_H
#define MATTICRAFT_FML_LOADING_LANGUAGEPROVIDERLOADER_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_LanguageProviderLoader LIBMATTI_FML_LanguageProviderLoader;

// Java: LanguageProviderLoader(ILaunchContext launchContext)
LIBMATTI_FML_LanguageProviderLoader *LIBMATTI_FML_LanguageProviderLoader_New(
    LIBMATTI_NEOFORGESPI_ILaunchContext *launchContext);

// Java: public void forEach(Consumer<IModLanguageLoader> consumer)
void LIBMATTI_FML_LanguageProviderLoader_ForEach(LIBMATTI_FML_LanguageProviderLoader *loader,
                                                 void (*consumer)(LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                                  void *userdata),
                                                 void *userdata);
// Java: public <T> Stream<T> applyForEach(Function<IModLanguageLoader, T> function)
void **LIBMATTI_FML_LanguageProviderLoader_ApplyForEach(LIBMATTI_FML_LanguageProviderLoader *loader,
                                                        void *(*function)(LIBMATTI_NEOFORGESPI_IModLanguageLoader *loader,
                                                                          void *userdata),
                                                        void *userdata, size_t *count);

// Java: public IModLanguageLoader findLanguage(ModFile mf, @Nullable String modLoader, @Nullable VersionRange modLoaderVersion)
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_LanguageProviderLoader_FindLanguage(
    LIBMATTI_FML_LanguageProviderLoader *loader, void *modFile, const char *modLoader,
    const LIBMATTI_NEOFORGESPI_VersionRange *modLoaderVersion);

void LIBMATTI_FML_LanguageProviderLoader_Free(LIBMATTI_FML_LanguageProviderLoader *loader);

#endif //MATTICRAFT_FML_LOADING_LANGUAGEPROVIDERLOADER_H
