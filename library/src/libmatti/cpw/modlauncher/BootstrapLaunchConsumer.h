// Port of cpw.mods.modlauncher.BootstrapLaunchConsumer.

#ifndef MATTICRAFT_MODLAUNCHER_BOOTSTRAPLAUNCHCONSUMER_H
#define MATTICRAFT_MODLAUNCHER_BOOTSTRAPLAUNCHCONSUMER_H

#include "libmatti/bsl/BootstrapLauncher.h"
#include "libmatti/bsl/sjh/cl/ModuleClassLoader.h"

// Java: public class BootstrapLaunchConsumer implements Consumer<String[]>
//       public void accept(final String[] strings) { Launcher.main(strings); }
// C: the consumer the bootstrap launcher calls with the module class loader it
// built (the C equivalent of the module layer) and the program arguments.
void LIBMATTI_ML_BootstrapLaunchConsumer_Accept(const LIBMATTI_CL_ModuleClassLoader *classLoader, int argc,
                                                char *argv[]);

// Java: the status an uncaught exception out of Launcher.run() would terminate
// the JVM with; the host reads it back after the bootstrap launcher returns.
int LIBMATTI_ML_BootstrapLaunchConsumer_GetLauncherStatus(void);

// Java: module-info provides java.util.function.Consumer with BootstrapLaunchConsumer
//       (BootstrapLauncher finds it through ServiceLoader). C has no service files,
//       so registering the consumer with the bootstrap launcher seam is this step.
void LIBMATTI_ML_BootstrapLaunchConsumer_Register(void);

#endif //MATTICRAFT_MODLAUNCHER_BOOTSTRAPLAUNCHCONSUMER_H
