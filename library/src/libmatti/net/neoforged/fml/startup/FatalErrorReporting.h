#ifndef MATTICRAFT_FML_FATALERRORREPORTING_H
#define MATTICRAFT_FML_FATALERRORREPORTING_H

#include "libmatti/java/lang/Throwable.h"

// Java: public final class FatalErrorReporting
// Java: public static void reportFatalError(Throwable t)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalError(LIBMATTI_JL_Throwable *t);

// Java: public static void reportFatalError(Throwable t, @Nullable Path gameDir, @Nullable Path logFile, @Nullable Path crashReport)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorWithPaths(LIBMATTI_JL_Throwable *t, const char *gameDir,
                                                               const char *logFile, const char *crashReport);

// Java: public static void reportFatalErrorOnConsole(Throwable t)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorOnConsole(LIBMATTI_JL_Throwable *t);

// Java: public static void reportFatalError(String message)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorMessage(const char *message);

#endif //MATTICRAFT_FML_FATALERRORREPORTING_H
