// Port of net.neoforged.fml.common.Mod.

#include "libmatti/net/neoforged/fml/common/Mod.h"

// Java: dist() default { Dist.CLIENT, Dist.DEDICATED_SERVER }
static const LIBMATTI_DIST_Dist DEFAULT_DIST[2] = {LIBMATTI_DIST_CLIENT, LIBMATTI_DIST_DEDICATED_SERVER};

LIBMATTI_FML_Mod LIBMATTI_FML_Mod_Defaults(const char *value)
{
    LIBMATTI_FML_Mod mod = {value, DEFAULT_DIST, 2, NULL, 0};
    return mod;
}
