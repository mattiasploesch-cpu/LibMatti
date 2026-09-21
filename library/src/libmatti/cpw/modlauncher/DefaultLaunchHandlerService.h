// Port of cpw.mods.modlauncher.DefaultLaunchHandlerService.

#ifndef MATTICRAFT_MODLAUNCHER_DEFAULTLAUNCHHANDLERSERVICE_H
#define MATTICRAFT_MODLAUNCHER_DEFAULTLAUNCHHANDLERSERVICE_H

#include "libmatti/cpw/modlauncher/api/ILaunchHandlerService.h"

// Java: public class DefaultLaunchHandlerService implements ILaunchHandlerService
typedef struct
{
    // Java: implements ILaunchHandlerService
    LIBMATTI_MLA_ILaunchHandlerService base;
} LIBMATTI_ML_DefaultLaunchHandlerService;

// Java: public static final String LAUNCH_PROPERTY = "minecraft.client.jar"
#define LIBMATTI_ML_DefaultLaunchHandlerService_LAUNCH_PROPERTY "minecraft.client.jar"

// Java: new DefaultLaunchHandlerService()
LIBMATTI_ML_DefaultLaunchHandlerService *LIBMATTI_ML_DefaultLaunchHandlerService_New(void);
void LIBMATTI_ML_DefaultLaunchHandlerService_Free(LIBMATTI_ML_DefaultLaunchHandlerService *service);

#endif //MATTICRAFT_MODLAUNCHER_DEFAULTLAUNCHHANDLERSERVICE_H
