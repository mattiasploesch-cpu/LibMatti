// Port of cpw.mods.modlauncher.util.ServiceLoaderUtils.

#include "libmatti/cpw/modlauncher/util/ServiceLoaderUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void **LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
    const void *layer, const char *serviceType,
    void (*errorConsumer)(const char *message, void *userdata), void *userdata, size_t *count)
{
    LIBMATTI_JU_ServiceLoader *loader = LIBMATTI_JU_ServiceLoader_Load(layer, serviceType);

    size_t providerCount = 0;
    LIBMATTI_JU_ServiceLoader_Provider *providers = LIBMATTI_JU_ServiceLoader_Stream(loader, &providerCount);

    void **result = NULL;
    size_t resultCount = 0;

    for (size_t i = 0; i < providerCount; i++)
    {
        if (providers[i].provider == NULL)
        {
            // Java: catch (ServiceConfigurationError sce) { errorConsumer.accept(sce); return null; }
            if (errorConsumer != NULL)
            {
                char message[256];
                snprintf(message, sizeof(message), "ServiceConfigurationError loading %s",
                         providers[i].providerClassName != NULL ? providers[i].providerClassName : "null");
                errorConsumer(message, userdata);
            }
            continue;
        }

        result = realloc(result, sizeof(*result) * (resultCount + 1));
        result[resultCount++] = providers[i].provider;
    }

    LIBMATTI_JU_ServiceLoader_Free(loader);
    *count = resultCount;
    return result;
}

char *LIBMATTI_MLU_ServiceLoaderUtils_FileNameFor(const char *providerClassName)
{
    // Java: clazz.getModule().getLayer().configuration().findModule(...)
    //           .flatMap(rm -> rm.reference().location()).map(...).orElse("MISSING FILE")
    const char *location = LIBMATTI_JU_ServiceLoader_LocationForClass(providerClassName);
    if (location == NULL) return strdup("MISSING FILE");

    const char *fileName = strrchr(location, '/');
    return strdup(fileName != NULL ? fileName + 1 : location);
}
