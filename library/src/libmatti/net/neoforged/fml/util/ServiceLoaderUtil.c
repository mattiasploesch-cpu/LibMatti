#include "libmatti/net/neoforged/fml/util/ServiceLoaderUtil.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LoggerFactory.getLogger(ServiceLoaderUtil.class);
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static String identifyService(Object o)
static char *identifyService(const LIBMATTI_FML_Service *service)
{
    // Java: var sourcePath = identifySourcePath(o); return o.getClass().getName() + " from " + sourcePath;
    // Java's additional services are instances and carry their own class name; the port marks them <built-in>.
    const char *className = service->serviceClassName != NULL ? service->serviceClassName : "<built-in>";
    char *sourcePath = LIBMATTI_FML_ServiceLoaderUtil_IdentifySourcePath(service->serviceClassName, NULL);
    size_t length = strlen(className) + 6 + strlen(sourcePath) + 1;
    char *result = malloc(length);
    snprintf(result, length, "%s from %s", className, sourcePath);
    free(sourcePath);
    return result;
}

// Java: public static <T> List<T> loadServices(ILaunchContext context, Class<T> serviceClass)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadServices(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks)
{
    return LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(context, serviceClass, NULL, 0, hooks);
}

// Java: public static <T> List<T> loadServices(ILaunchContext context, Class<T> serviceClass, Collection<T> additionalServices)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    LIBMATTI_FML_Service *additionalServices, size_t additionalCount,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks)
{
    LIBMATTI_JU_ServiceLoader *loader = LIBMATTI_NEOFORGESPI_ILaunchContext_LoadServices(context, serviceClass);
    size_t providerCount = 0;
    LIBMATTI_JU_ServiceLoader_Provider *providers =
        LIBMATTI_JU_ServiceLoader_Stream(loader, &providerCount);

    // Java: additionalServices.stream() then the ServiceLoader services, distinct()
    LIBMATTI_FML_ServiceList *result = calloc(1, sizeof(LIBMATTI_FML_ServiceList));
    result->services = calloc(additionalCount + providerCount, sizeof(LIBMATTI_FML_Service));

    for (size_t i = 0; i < additionalCount; i++)
    {
        result->services[result->count] = additionalServices[i];
        result->services[result->count].builtIn = 1;
        result->count++;
    }

    for (size_t i = 0; i < providerCount; i++)
    {
        LIBMATTI_JU_ServiceLoader_Provider *provider = &providers[i];

        // Java: filter.test(p.type())
        if (hooks->filter != NULL && !hooks->filter(provider->providerClassName))
        {
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE,
                                     "Filtering out service provider {} for service class {}",
                                     provider->providerClassName, serviceClass);
            continue;
        }

        // Java: p.get() - a ServiceConfigurationError is logged and the provider skipped
        void *service = hooks->instantiate(provider);
        if (service == NULL)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_CORE,
                                     "Failed to load implementation for {}", serviceClass);
            continue;
        }

        // Java: distinct()
        int duplicate = 0;
        for (size_t j = 0; j < result->count; j++)
        {
            if (result->services[j].service == service)
            {
                duplicate = 1;
                break;
            }
        }
        if (duplicate)
            continue;

        result->services[result->count].service = service;
        result->services[result->count].serviceClassName = provider->providerClassName;
        result->count++;
    }

    // Java: servicesStream.sorted(Comparator.comparingInt(getPriority).reversed())
    if (hooks->applyPriority)
    {
        for (size_t i = 1; i < result->count; i++)
        {
            LIBMATTI_FML_Service current = result->services[i];
            size_t j = i;
            while (j > 0 && hooks->getPriority(result->services[j - 1].service) <
                                    hooks->getPriority(current.service))
            {
                result->services[j] = result->services[j - 1];
                j--;
            }
            result->services[j] = current;
        }
    }

    // log4j formats the count; the port's logger only substitutes strings
    char implementationCount[32];
    snprintf(implementationCount, sizeof(implementationCount), "%zu", result->count);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Found {} implementations of {}:",
                             implementationCount, serviceClass);
    for (size_t i = 0; i < result->count; i++)
    {
        char *identified = identifyService(&result->services[i]);
        if (result->services[i].builtIn)
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "\t[built-in] {}", identified);
        else
            LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "\t{}", identified);
        free(identified);
    }

    LIBMATTI_JU_ServiceLoader_Free(loader);
    return result;
}

// Java: public static <T> List<T> loadEarlyServices(ILaunchContext context, Class<T> serviceClass, Collection<T> additionalServices)
LIBMATTI_FML_ServiceList *LIBMATTI_FML_ServiceLoaderUtil_LoadEarlyServices(
    LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass,
    LIBMATTI_FML_Service *additionalServices, size_t additionalCount,
    const LIBMATTI_FML_ServiceLoaderUtil_Hooks *hooks)
{
    LIBMATTI_FML_ServiceList *services = LIBMATTI_FML_ServiceLoaderUtil_LoadServicesWithAdditional(
        context, serviceClass, additionalServices, additionalCount, hooks);

    // Java: context.addLocated(Path.of(codeSource.getLocation().toURI())) for each service
    for (size_t i = 0; i < services->count; i++)
    {
        if (services->services[i].serviceClassName == NULL) continue;

        const char *location = LIBMATTI_JU_ServiceLoader_LocationForClass(services->services[i].serviceClassName);
        if (location != NULL)
            LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(context, location);
    }

    return services;
}

// Java: public static String identifySourcePath(Object object)
char *LIBMATTI_FML_ServiceLoaderUtil_IdentifySourcePath(const char *className, const char *codeSourceLocation)
{
    (void) className;
    if (codeSourceLocation == NULL)
    {
        // Java: codeLocation.toString() when the URI cannot be parsed
        return LIBMATTI_FML_PathPrettyPrinting_PrettyPrint("");
    }
    return LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(codeSourceLocation);
}

void LIBMATTI_FML_ServiceLoaderUtil_FreeServiceList(LIBMATTI_FML_ServiceList *list)
{
    if (list == NULL)
        return;
    free(list->services);
    free(list);
}
