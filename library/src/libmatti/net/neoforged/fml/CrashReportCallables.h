// Port of net.neoforged.fml.CrashReportCallables.

#ifndef MATTICRAFT_FML_CRASHREPORTCALLABLES_H
#define MATTICRAFT_FML_CRASHREPORTCALLABLES_H

#include "libmatti/net/neoforged/fml/ICrashReportHeader.h"
#include "libmatti/net/neoforged/fml/ISystemReportExtender.h"

#include <stddef.h>

// Java: public static void registerCrashCallable(ISystemReportExtender callable)
void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallable(LIBMATTI_FML_ISystemReportExtender *callable);
// Java: public static void registerCrashCallable(String headerName, Supplier<String> reportGenerator)
void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplier(const char *headerName,
                                                                     char *(*reportGenerator)(void *userdata),
                                                                     void *userdata);
// Java: public static void registerCrashCallable(String headerName, Supplier<String> reportGenerator, BooleanSupplier active)
void LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplierActive(
    const char *headerName, char *(*reportGenerator)(void *userdata), void *userdata,
    int (*active)(void *userdata));
// Java: public static void registerHeader(ICrashReportHeader header)
void LIBMATTI_FML_CrashReportCallables_RegisterHeader(LIBMATTI_FML_ICrashReportHeader *header);

// Java: public static List<ISystemReportExtender> allCrashCallables()
LIBMATTI_FML_ISystemReportExtender **LIBMATTI_FML_CrashReportCallables_AllCrashCallables(size_t *count);
// Java: public static Stream<String> getHeaders(); caller frees the array and each element
char **LIBMATTI_FML_CrashReportCallables_GetHeaders(size_t *count);

#endif //MATTICRAFT_FML_CRASHREPORTCALLABLES_H
