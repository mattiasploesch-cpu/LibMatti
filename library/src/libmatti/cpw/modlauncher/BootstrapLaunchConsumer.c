// Port of cpw.mods.modlauncher.BootstrapLaunchConsumer.

#include "BootstrapLaunchConsumer.h"

#include "Launcher.h"
#include "ModuleLayerHandler.h"

// Java: an uncaught exception out of Launcher.run() terminates the JVM with a
// non-zero status. The C port cannot unwind, so the status is recorded here and
// the host reads it back after the bootstrap launcher returns.
static int launcherStatus = 0;

int LIBMATTI_ML_BootstrapLaunchConsumer_GetLauncherStatus(void)
{
    return launcherStatus;
}

void LIBMATTI_ML_BootstrapLaunchConsumer_Accept(const LIBMATTI_CL_ModuleClassLoader *classLoader, int argc,
                                                char *argv[])
{
    // Java: ModuleLayerHandler reads getClass().getClassLoader(); the C host has
    // no ambient class loader, so the one the bootstrap launcher built is handed
    // to the module layer handler explicitly (must happen before Launcher.main).
    LIBMATTI_ML_ModuleLayerHandler_SetBootClassLoader((LIBMATTI_CL_ModuleClassLoader *)classLoader);

    // Java: Launcher.main(strings). The bootstrap launcher arguments do not carry
    // argv[0]; the C program arguments do, hence the one-element shift.
    launcherStatus = LIBMATTI_ML_Launcher_Main(argc - 1, argv + 1);

    // The class loader is owned by the bootstrap launcher (freed in its cleanup).
    LIBMATTI_ML_ModuleLayerHandler_SetBootClassLoader(NULL);
}

void LIBMATTI_ML_BootstrapLaunchConsumer_Register(void)
{
    LIBMATTI_BSL_SetLaunchConsumer(LIBMATTI_ML_BootstrapLaunchConsumer_Accept);
}
