#ifndef MATTICRAFT_MODLAUNCHER_MLCLASSLOADERCONTEXTSELECTOR_H
#define MATTICRAFT_MODLAUNCHER_MLCLASSLOADERCONTEXTSELECTOR_H

#include "libmatti/bsl/sjh/cl/ModuleClassLoader.h"

char *LIBMATTI_ML_MLClassLoaderContextSelector_ToContextMapKey(
    const LIBMATTI_CL_ModuleClassLoader *moduleClassLoader);
char *LIBMATTI_ML_MLClassLoaderContextSelector_ToContextMapKeyForClassLoader(
    const LIBMATTI_CL_ClassLoader *classLoader);

#endif //MATTICRAFT_MODLAUNCHER_MLCLASSLOADERCONTEXTSELECTOR_H
