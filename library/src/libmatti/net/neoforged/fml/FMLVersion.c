#include "libmatti/net/neoforged/fml/FMLVersion.h"

#include "libmatti/net/neoforged/fml/FMLVersionProperties.h"

// Java: public static String getVersion()
const char *LIBMATTI_FML_FMLVersion_GetVersion(void)
{
    // Java: return FMLVersionProperties.VERSION;
    return LIBMATTI_FML_FMLVersionProperties_VERSION;
}
