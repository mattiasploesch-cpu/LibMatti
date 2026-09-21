// Port of net.neoforged.fml.CrashReportCallables.

#include "libmatti/net/neoforged/fml/CrashReportCallables.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final List<ISystemReportExtender> crashCallables = Collections.synchronizedList(new ArrayList<>())
static LIBMATTI_FML_ISystemReportExtender **crashCallables = NULL;
static size_t crashCallableCount = 0;
// Java: private static final List<ICrashReportHeader> HEADERS = Collections.synchronizedList(new ArrayList<>())
static LIBMATTI_FML_ICrashReportHeader **headers = NULL;
static size_t headerCount = 0;

// Java: private static final Logger LOGGER = LogUtils.getLogger()
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: the anonymous ISystemReportExtender used by the (headerName, reportGenerator[, active]) overloads
typedef struct
{
    LIBMATTI_FML_ISystemReportExtender extender;
    const char *headerName;
    char *(*reportGenerator)(void *userdata);
    void *reportGeneratorUserdata;
    int (*active)(void *userdata);
    void *activeUserdata;
} HeaderSupplier;

static char *header_supplier_get_label(LIBMATTI_FML_ISystemReportExtender *self)
{
    return strdup(((HeaderSupplier *) self)->headerName);
}

static char *header_supplier_get(LIBMATTI_FML_ISystemReportExtender *self)
{
    HeaderSupplier *supplier = (HeaderSupplier *) self;
    return supplier->reportGenerator(supplier->reportGeneratorUserdata);
}

static int header_supplier_is_active(LIBMATTI_FML_ISystemReportExtender *self)
{
    // Java: try { return active.getAsBoolean(); } catch (Throwable t) { LOGGER.warn(...); return false; }
    HeaderSupplier *supplier = (HeaderSupplier *) self;
    if (supplier->active == NULL) return 1;
    return supplier->active(supplier->activeUserdata);
}

static LIBMATTI_FML_ISystemReportExtender *new_header_supplier(const char *headerName,
                                                               char *(*reportGenerator)(void *userdata),
                                                               void *reportGeneratorUserdata,
                                                               int (*active)(void *userdata), void *activeUserdata)
{
    HeaderSupplier *supplier = calloc(1, sizeof(HeaderSupplier));
    supplier->extender.getLabel = header_supplier_get_label;
    supplier->extender.isActive = header_supplier_is_active;
    supplier->extender.get = header_supplier_get;
    supplier->headerName = headerName;
    supplier->reportGenerator = reportGenerator;
    supplier->reportGeneratorUserdata = reportGeneratorUserdata;
    supplier->active = active;
    supplier->activeUserdata = activeUserdata;
    return &supplier->extender;
}

void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallable(LIBMATTI_FML_ISystemReportExtender *callable)
{
    crashCallables = realloc(crashCallables, sizeof(*crashCallables) * (crashCallableCount + 1));
    crashCallables[crashCallableCount++] = callable;
}

void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplier(const char *headerName,
                                                                     char *(*reportGenerator)(void *userdata),
                                                                     void *userdata)
{
    LIBMATTI_FML_CrashReportCallables_RegisterCrashCallable(
        new_header_supplier(headerName, reportGenerator, userdata, NULL, NULL));
}

void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplierActive(
    const char *headerName, char *(*reportGenerator)(void *userdata), void *userdata,
    int (*active)(void *userdata))
{
    LIBMATTI_FML_CrashReportCallables_RegisterCrashCallable(
        new_header_supplier(headerName, reportGenerator, userdata, active, userdata));
}

void LIBMATTI_FML_CrashReportCallables_RegisterHeader(LIBMATTI_FML_ICrashReportHeader *header)
{
    headers = realloc(headers, sizeof(*headers) * (headerCount + 1));
    headers[headerCount++] = header;
}

LIBMATTI_FML_ISystemReportExtender **LIBMATTI_FML_CrashReportCallables_AllCrashCallables(size_t *count)
{
    // Java: List.copyOf(crashCallables)
    LIBMATTI_FML_ISystemReportExtender **copy = malloc(sizeof(*copy) * (crashCallableCount > 0 ? crashCallableCount : 1));
    memcpy(copy, crashCallables, sizeof(*copy) * crashCallableCount);
    *count = crashCallableCount;
    return copy;
}

char **LIBMATTI_FML_CrashReportCallables_GetHeaders(size_t *count)
{
    // Java: HEADERS.stream().map(ICrashReportHeader::getHeader).filter(Objects::nonNull)
    char **result = malloc(sizeof(*result) * (headerCount > 0 ? headerCount : 1));
    size_t resultCount = 0;
    for (size_t i = 0; i < headerCount; i++)
    {
        char *header = LIBMATTI_FML_ICrashReportHeader_GetHeader(headers[i]);
        if (header == NULL) continue;
        result[resultCount++] = header;
    }
    *count = resultCount;
    return result;
}
