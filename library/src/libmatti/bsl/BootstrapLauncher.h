//
// Created by administrator on 09.09.26.
//

#ifndef MATTICRAFT_BOOTSTRAPLAUNCHER_H
#define MATTICRAFT_BOOTSTRAPLAUNCHER_H

#include "libmatti/bsl/sjh/cl/ModuleClassLoader.h"
#include "libmatti/defines.h"

int LIBMATTI_BSL_RUN(int isolation, int argc, char *argv[], int debug);
int LIBMATTI_BSL_CLASSPATH_LoadLegacyClasspath(char **classpath[]);
int LIBMATTI_BSL_MODULES_FindLoadedModules(LIBMATTI_BSL_BoostrapLauncher_Module ***modules);

// Java: ServiceLoader.load(layer.layer(), Consumer.class) -> BootstrapLaunchConsumer
// The mod-launcher consumer is external; the C equivalent is a host-registered
// launch function that receives the built module class loader (the C equivalent
// of the module layer) and the program arguments.
typedef void (*LIBMATTI_BSL_LaunchConsumer)(const LIBMATTI_CL_ModuleClassLoader *classLoader, int argc, char *argv[]);

// Register the launch consumer invoked at the end of LIBMATTI_BSL_RUN.
// (Java: the consumer is discovered via ServiceLoader instead.)
void LIBMATTI_BSL_SetLaunchConsumer(LIBMATTI_BSL_LaunchConsumer consumer);

// Seam for the jar-metadata reading (the ported cpw jarhandling in libmatti/bsl/sjh/jarhandling).
int LIBMATTI_BSL_JAR_GetModuleName(const char *path, char **name);

// mergeModules handling (Java: private static getMergeFilenameMap).
// Builds the filename -> module number map; *map is an empty map when the
// mergeModules property is not set. Returns 0 on success, 1 on failure.
int LIBMATTI_BSL_FS_GetMergeFilenameMap(LIBMATTI_BSL_FilenameMap **map);


#endif //MATTICRAFT_BOOTSTRAPLAUNCHER_H