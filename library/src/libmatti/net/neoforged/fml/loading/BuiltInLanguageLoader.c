// Port of net.neoforged.fml.loading.BuiltInLanguageLoader.

#include "libmatti/net/neoforged/fml/loading/BuiltInLanguageLoader.h"

#include "libmatti/net/neoforged/fml/FMLVersion.h"

// Java: JarVersionLookupHandler.getVersion(this.getClass()).orElse(Files.isDirectory(lpPath) ? FMLVersion.getVersion() : null)
const char *LIBMATTI_FML_BuiltInLanguageLoader_Version(void)
{
    return LIBMATTI_FML_FMLVersion_GetVersion();
}
