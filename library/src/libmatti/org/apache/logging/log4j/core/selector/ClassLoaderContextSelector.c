#include "ClassLoaderContextSelector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *LIBMATTI_L4J_ClassLoaderContextSelector_ToContextMapKey(const LIBMATTI_CL_ClassLoader *loader)
{
    char key[64];
    snprintf(key, sizeof(key), "ClassLoader@%p", (const void *)loader);
    return strdup(key);
}
