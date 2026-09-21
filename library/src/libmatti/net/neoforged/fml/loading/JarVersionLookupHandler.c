// Port of net.neoforged.fml.loading.JarVersionLookupHandler.
//
// Java reads the version from the class's module descriptor (rawVersion) or, when the class was
// loaded non-modularly, from its package (the jar manifest's Implementation-Version). The C
// artifact carries its manifest in the ELF ".matti_manifest" section, so both paths read
// Implementation-Version from the running executable, as Launcher.c does for modlauncher.

#include "libmatti/net/neoforged/fml/loading/JarVersionLookupHandler.h"

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stdlib.h>
#include <string.h>

// Java: clazz.getModule().getDescriptor().rawVersion() / clazz.getPackage().getImplementationVersion()
int LIBMATTI_FML_JarVersionLookupHandler_GetVersion(const LIBMATTI_JL_Class *clazz, const char **out)
{
    (void) clazz;

#ifdef __linux__
    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContents_Of("/proc/self/exe");
    if (contents == NULL) return 0;

    LIBMATTI_JU_Manifest *manifest = LIBMATTI_JH_JarContents_GetManifest(contents);
    const char *version = manifest != NULL ? LIBMATTI_JU_Manifest_GetMainValue(manifest, "Implementation-Version") : NULL;

    LIBMATTI_JH_JarContents_Close(contents);
    if (version == NULL) return 0;

    *out = version;
    return 1;
#else
    return 0;
#endif
}
