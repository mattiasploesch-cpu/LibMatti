//
// Created by administrator on 08.09.26.
//
#include "main.h"

#include <stdlib.h>

#include "bsl/BootstrapLauncher.h"
#include "cpw/modlauncher/BootstrapLaunchConsumer.h"

int start_from_bsl(int argc, char *argv[], char *envp[])
{
    // Java: BootstrapLauncher.run(...) ends with
    //   ServiceLoader.load(layer.layer(), Consumer.class).stream().findFirst().get().accept(args)
    // which resolves cpw.mods.modlauncher.BootstrapLaunchConsumer -> Launcher.main(args).
    // C has no service files, so the consumer is registered with the bootstrap launcher.
    LIBMATTI_ML_BootstrapLaunchConsumer_Register();

    int debug = 0;
    if (getenv("bsl.debug") != NULL)
    {
        debug = 1;
    }
    LIBMATTI_BSL_RUN(1, argc, argv, debug);

    // Java: the launcher runs inside accept(...); an uncaught exception there
    // terminates the JVM, so its status becomes the process exit status.
    return LIBMATTI_ML_BootstrapLaunchConsumer_GetLauncherStatus();
}
