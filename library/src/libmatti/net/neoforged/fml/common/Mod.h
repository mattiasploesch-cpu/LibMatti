// Port of net.neoforged.fml.common.Mod.
// Java reads the annotation off a class through reflection (@Retention(RUNTIME) @Target(TYPE));
// C has neither annotations nor reflection, so the port passes the annotation's values as a value.
// Java defaults: dist = { Dist.CLIENT, Dist.DEDICATED_SERVER }, depends = {}.

#ifndef MATTICRAFT_FML_COMMON_MOD_H
#define MATTICRAFT_FML_COMMON_MOD_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"

#include <stddef.h>

// Java: public @interface Mod
typedef struct
{
    // Java: String value();
    const char *value;
    // Java: Dist[] dist() default { Dist.CLIENT, Dist.DEDICATED_SERVER };
    const LIBMATTI_DIST_Dist *dist;
    size_t distCount;
    // Java: String[] depends() default {};
    const char **depends;
    size_t dependsCount;
} LIBMATTI_FML_Mod;

// Java: the annotation defaults for dist and depends
LIBMATTI_FML_Mod LIBMATTI_FML_Mod_Defaults(const char *value);

#endif //MATTICRAFT_FML_COMMON_MOD_H
