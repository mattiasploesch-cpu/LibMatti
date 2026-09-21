// Port of java.util.ServiceLoader.
// Java discovers providers through the module layer and META-INF/services. In
// pure C there is no module system, so the port keeps the ServiceLoader surface
// and lets the host register the providers (the same seam Stage 1 uses to hand
// the modules to ModLauncher).

#ifndef MATTICRAFT_SERVICELOADER_H
#define MATTICRAFT_SERVICELOADER_H

#include <stddef.h>

// Java: ServiceLoader.Provider<T>
typedef struct
{
    const char *serviceType;
    void *provider;
    const char *providerClassName;
    // Java: clazz.getModule().getLayer().configuration().findModule(...).reference().location()
    const char *location;
} LIBMATTI_JU_ServiceLoader_Provider;

// Java: public final class ServiceLoader<S> implements Iterable<S>
typedef struct LIBMATTI_JU_ServiceLoader
{
    const char *serviceType;
    LIBMATTI_JU_ServiceLoader_Provider *providers;
    size_t count;
} LIBMATTI_JU_ServiceLoader;

// Java: the META-INF/services registration, driven by the host instead of modules
void LIBMATTI_JU_ServiceLoader_Register(const char *serviceType, void *provider, const char *providerClassName,
                                        const char *location);

// Java: public static <S> ServiceLoader<S> load(ModuleLayer layer, Class<S> service)
LIBMATTI_JU_ServiceLoader *LIBMATTI_JU_ServiceLoader_Load(const void *layer, const char *serviceType);
// Java: public static <S> ServiceLoader<S> load(Class<S> service)
LIBMATTI_JU_ServiceLoader *LIBMATTI_JU_ServiceLoader_LoadFrom(const char *serviceType);

// Java: public Stream<Provider<S>> stream()
LIBMATTI_JU_ServiceLoader_Provider *LIBMATTI_JU_ServiceLoader_Stream(const LIBMATTI_JU_ServiceLoader *loader,
                                                                     size_t *count);

// Java: ServiceLoaderUtils.fileNameFor(Class<?>) - the location registered with the provider
const char *LIBMATTI_JU_ServiceLoader_LocationForClass(const char *providerClassName);

void LIBMATTI_JU_ServiceLoader_Free(LIBMATTI_JU_ServiceLoader *loader);

#endif //MATTICRAFT_SERVICELOADER_H
