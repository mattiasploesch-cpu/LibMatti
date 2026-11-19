







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
