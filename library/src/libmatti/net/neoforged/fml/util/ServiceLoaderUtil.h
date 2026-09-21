// Port of net.neoforged.fml.util.ServiceLoaderUtil.
// Java uses generics and reflection (Class<T>, Predicate<Class<? extends T>>,
// protection domains). The C port keeps the same flow and takes those reflective
// pieces as hooks the caller supplies.

#ifndef MATTICRAFT_FML_UTIL_SERVICELOADERUTIL_H
#define MATTICRAFT_FML_UTIL_SERVICELOADERUTIL_H

#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"

#include <stddef.h>

// Java: List<T> - the loaded services together with service.getClass().getName()
typedef struct
{
    void *service;
    const char *serviceClassName;
    // Java: additionalServices.contains(service)
    int builtIn;
} LIBMATTI_FML_Service;

typedef struct
{
    LIBMATTI_FML_Service *services;
    size_t count;
} LIBMATTI_FML_ServiceList;

// The reflective pieces of the Java implementation.
typedef struct
{
    // Java: Predicate<Class<? extends T>> filter
    int (*filter)(const char *providerClassName);
    // Java: IOrderedProvider.class.isAssignableFrom(serviceClass)
    int applyPriority;
    // Java: ((IOrderedProvider) service).getPriority()
    int (*getPriority)(void *service);
    // Java: p.get()
    void *(*instantiate)(const LIBMATTI_JU_ServiceLoader_Provider *provider);
} LIBMATTI_FML_ServiceLoaderUtil_Hooks;

// Java: public static <T> List<T> loadServices(ILaunchContext context, Class<T> serviceClass)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadServices(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks);

// Java: public static <T> List<T> loadServices(ILaunchContext context, Class<T> serviceClass, Collection<T> additionalServices)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    LIBMATTI_FML_Service *additionalServices, size_t additionalCount,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks);

// Java: public static <T> List<T> loadEarlyServices(ILaunchContext context, Class<T> serviceClass, Collection<T> additionalServices)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadEarlyServices(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    LIBMATTI_FML_Service *additionalServices, size_t additionalCount,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks);

// Java: public static String identifySourcePath(Object object)
// codeSourceLocation stands in for object.getClass().getProtectionDomain().getCodeSource().getLocation()
char *LIBMATTI_FML_ServiceLoaderUtil_IdentifySourcePath(const char *className, const char *codeSourceLocation);

void LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(LIBMATTI_FML_ServiceList *list);

#endif //MATTICRAFT_FML_UTIL_SERVICELOADERUTIL_H
