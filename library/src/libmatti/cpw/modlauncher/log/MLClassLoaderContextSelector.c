#include "libmatti/cpw/modlauncher/log/MLClassLoaderContextSelector.h"

#include "libmatti/org/apache/logging/log4j/core/selector/ClassLoaderContextSelector.h"

#include <stdlib.h>
#include <string.h>

char *LIBMATTI_ML_MLClassLoaderContextSelector_ToContextMapKey(
    const LIBMATTI_CL_ModuleClassLoader *moduleClassLoader)
{
    (void)moduleClassLoader;
    return strdup("MCL");
}

char *LIBMATTI_ML_MLClassLoaderContextSelector_ToContextMapKeyForClassLoader(
    const LIBMATTI_CL_ClassLoader *classLoader)
{
    return LIBMATTI_L4J_ClassLoaderContextSelector_ToContextMapKey(classLoader);
}
