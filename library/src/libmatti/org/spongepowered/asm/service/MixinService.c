// Port of org.spongepowered.asm.service.MixinService.
// "Provides access to the service layer which connects the mixin transformer to a particular host
// environment." The ServiceLoader lookups of Java resolve through the mixin.bootstrapService /
// mixin.service system properties, exactly like the Java code's property bypass - in the C port
// the property values name the registered service structs (fml).

#include "libmatti/org/spongepowered/asm/service/MixinService.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinLogger.h"
#include "libmatti/org/spongepowered/asm/logging/ILogger.h"

#include <stdlib.h>
#include <string.h>

// Java: private static <T> T getDefaultLogger() { return new LoggerAdapterConsole("mixin")...; }
// The launcher's LogManager is the port's console adapter; declared here because the
// singleton needs it before the service exists.
LIBMATTI_SP_ILogger *LIBMATTI_SP_MixinService_DefaultLogger(void);

// Java: static class LogBuffer
typedef struct
{
    char *message;
} LogEntry;

typedef struct
{
    LogEntry *entries;
    size_t count;
    int flushed;
    LIBMATTI_SP_ILogger *logger;
} LogBuffer;

static void log_buffer_debug(LogBuffer *buffer, const char *message)
{
    if (buffer->flushed)
    {
        LIBMATTI_SP_ILogger_Debug(buffer->logger, message);
        return;
    }
    buffer->entries = realloc(buffer->entries, sizeof(LogEntry) * (buffer->count + 1));
    buffer->entries[buffer->count++].message = strdup(message);
}

// Java: synchronized void flush(ILogger logger)
static void log_buffer_flush(LogBuffer *buffer, LIBMATTI_SP_ILogger *logger)
{
    for (size_t i = 0; i < buffer->count; i++)
    {
        LIBMATTI_SP_ILogger_Debug(logger, buffer->entries[i].message);
        free(buffer->entries[i].message);
    }
    free(buffer->entries);
    buffer->entries = NULL;
    buffer->count = 0;
    buffer->flushed = 1;
    buffer->logger = logger;
}

// Java: private static LogBuffer logBuffer = new LogBuffer();
static LogBuffer logBuffer = {NULL, 0, 0, NULL};

// Java: private static MixinService instance;
static int booted = 0;
static LIBMATTI_SP_IMixinService *service = NULL;
static LIBMATTI_SP_IGlobalPropertyService *propertyService = NULL;

// The service implementations the port ships (Java: ServiceLoader over META-INF/services).
LIBMATTI_SP_IMixinService *LIBMATTI_FML_FMLMixinService_Instance(void);
LIBMATTI_SP_IMixinServiceBootstrap *LIBMATTI_FML_FMLMixinServiceBootstrap_Instance(void);
LIBMATTI_SP_IGlobalPropertyService *LIBMATTI_FML_FMLMixinGlobalProperties_Instance(void);

// Java: private void runBootServices() - "bypass service loader if the mixin.bootstrapService
// system property yields the desired IMixinServiceBootstrap implementation directly"
static void run_boot_services(void)
{
    const char *serviceCls = LIBMATTI_JL_System_GetProperty("mixin.bootstrapService");
    if (serviceCls != NULL)
    {
        // Java: bootService.bootstrap(); bootedServices.add(bootService.getServiceClassName());
        LIBMATTI_SP_IMixinServiceBootstrap *bootService = LIBMATTI_FML_FMLMixinServiceBootstrap_Instance();
        bootService->bootstrap(bootService->self);
        return;
    }

    // Java: ServiceLoader.load(IMixinServiceBootstrap.class) - no services without the property
}

// Java: private static MixinService getInstance() -> the constructor runs runBootServices()
static void ensure_instance(void)
{
    if (booted) return;
    booted = 1;
    run_boot_services();
}

// Java: public static void boot()
void LIBMATTI_SP_MixinService_Boot(void)
{
    ensure_instance();
}

// Java: private IMixinService initService()
static LIBMATTI_SP_IMixinService *init_service(void)
{
    const char *serviceCls = LIBMATTI_JL_System_GetProperty("mixin.service");
    if (serviceCls != NULL)
    {
        // Java: the property names the service class; the port's registered service is FML's.
        // Java: if (!service.isValid()) throw new RuntimeException("invalid service ...")
        LIBMATTI_SP_IMixinService *fmlService = LIBMATTI_FML_FMLMixinService_Instance();
        if (!fmlService->isValid(fmlService->self))
        {
            log_buffer_debug(&logBuffer, "invalid service configured via system property");
            return NULL;
        }
        return fmlService;
    }

    // Java: ServiceLoader.load(IMixinService.class) -> ServiceNotAvailableError
    log_buffer_debug(&logBuffer, "No mixin host service is available. Services: ");
    return NULL;
}

// Java: public static IMixinService getService()
LIBMATTI_SP_IMixinService *LIBMATTI_SP_MixinService_GetService(void)
{
    ensure_instance();

    if (service == NULL)
    {
        service = init_service();
        // Java: ILogger serviceLogger = this.service.getLogger("mixin"); logBuffer.flush(serviceLogger);
        if (service != NULL) log_buffer_flush(&logBuffer, service->getLogger(service->self, "mixin"));
        else log_buffer_flush(&logBuffer, LIBMATTI_SP_MixinService_DefaultLogger());
    }
    return service;
}

// Java: private IGlobalPropertyService initPropertyService()
static LIBMATTI_SP_IGlobalPropertyService *init_property_service(void)
{
    // Java: ServiceLoader.load(IGlobalPropertyService.class) - the port registers FML's impl
    return LIBMATTI_FML_FMLMixinGlobalProperties_Instance();
}

// Java: public static IGlobalPropertyService getGlobalPropertyService()
LIBMATTI_SP_IGlobalPropertyService *LIBMATTI_SP_MixinService_GetGlobalPropertyService(void)
{
    ensure_instance();

    if (propertyService == NULL) propertyService = init_property_service();
    return propertyService;
}

// Java: private static <T> T getDefaultLogger() { return new LoggerAdapterConsole("mixin")...; }
// The launcher's LogManager is the port's console adapter. The ILogger interface dispatches
// through a struct, so the default logger wraps the LogManager logger.
LIBMATTI_SP_ILogger *LIBMATTI_SP_MixinService_DefaultLogger(void)
{
    return LIBMATTI_FML_FMLMixinLogger_Instance("mixin");
}
