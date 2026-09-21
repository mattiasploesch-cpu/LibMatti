// Port of cpw.mods.modlauncher.util.ServiceLoaderUtils.

#ifndef MATTICRAFT_MODLAUNCHER_SERVICELOADERUTILS_H
#define MATTICRAFT_MODLAUNCHER_SERVICELOADERUTILS_H

#include "libmatti/java/util/ServiceLoader.h"

#include <stddef.h>

// Java: static <T> Stream<T> streamServiceLoader(Supplier<ServiceLoader<T>> slSupplier,
//                                                Consumer<ServiceConfigurationError> errorConsumer)
// The stream is returned as an array; errorConsumer receives a message when a
// provider fails to load (Java: ServiceConfigurationError).
void **LIBMATTI_MLU_ServiceLoaderUtils_StreamServiceLoader(
    const void *layer, const char *serviceType,
    void (*errorConsumer)(const char *message, void *userdata), void *userdata, size_t *count);

// Java: static String fileNameFor(Class<?> clazz); caller frees
char *LIBMATTI_MLU_ServiceLoaderUtils_FileNameFor(const char *providerClassName);

#endif //MATTICRAFT_MODLAUNCHER_SERVICELOADERUTILS_H
