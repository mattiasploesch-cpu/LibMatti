#ifndef MATTICRAFT_FML_NEOFORGELAUNCHHANDLER_H
#define MATTICRAFT_FML_NEOFORGELAUNCHHANDLER_H

#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"

// Java: the ILaunchHandlerService of the NeoForge target. Its ServiceRunner
// starts the NeoForge entrypoint net.neoforged.fml.startup.Client, which is the
// launch profile main class of NeoForge 1.21.11.
typedef struct
{
    LIBMATTI_MLA_ILaunchHandlerService base;
} LIBMATTI_FML_NeoForgeLaunchHandler;

// Java: the provider of a mod's META-INF/services/cpw.mods.modlauncher.api.ILaunchHandlerService
// entry; the host registers it (java/util/ServiceLoader.h).
LIBMATTI_FML_NeoForgeLaunchHandler *LIBMATTI_FML_NeoForgeLaunchHandler_New(void);
void LIBMATTI_FML_NeoForgeLaunchHandler_Free(LIBMATTI_FML_NeoForgeLaunchHandler *handler);

#endif //MATTICRAFT_FML_NEOFORGELAUNCHHANDLER_H
