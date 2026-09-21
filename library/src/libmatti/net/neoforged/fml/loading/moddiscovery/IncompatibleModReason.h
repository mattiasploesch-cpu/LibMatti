// Port of net.neoforged.fml.loading.moddiscovery.IncompatibleModReason.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_INCOMPATIBLEMODREASON_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_INCOMPATIBLEMODREASON_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

// Java: public enum IncompatibleModReason
typedef enum
{
    LIBMATTI_FML_IncompatibleModReason_OLDFORGE,
    LIBMATTI_FML_IncompatibleModReason_MINECRAFT_FORGE,
    LIBMATTI_FML_IncompatibleModReason_FABRIC,
    LIBMATTI_FML_IncompatibleModReason_QUILT,
    LIBMATTI_FML_IncompatibleModReason_LITELOADER,
    LIBMATTI_FML_IncompatibleModReason_OPTIFINE,
    LIBMATTI_FML_IncompatibleModReason_BUKKIT
} LIBMATTI_FML_IncompatibleModReason;

// Java: public String getReason()
char *LIBMATTI_FML_IncompatibleModReason_GetReason(LIBMATTI_FML_IncompatibleModReason reason);

// Java: public static Optional<IncompatibleModReason> detect(JarContents jar) - 1 if present
int LIBMATTI_FML_IncompatibleModReason_Detect(const LIBMATTI_FML_JarContents *jar,
                                              LIBMATTI_FML_IncompatibleModReason *out);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_INCOMPATIBLEMODREASON_H
