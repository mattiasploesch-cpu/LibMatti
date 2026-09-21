// Port of net.neoforged.neoforge.logging.CrashReportExtender.

#ifndef MATTICRAFT_NEOFORGE_LOGGING_CRASHREPORTEXTENDER_H
#define MATTICRAFT_NEOFORGE_LOGGING_CRASHREPORTEXTENDER_H

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include <stddef.h>

// Java: public static File dumpModLoadingCrashReport(final Logger logger, final List<ModLoadingIssue> issues, final File topLevelDir)
// Writes crash-reports/crash-<timestamp>-fml.txt under topLevelDir; NULL = the report could not
// be written (Java logs "Failed to save crash report").
// Returns a new string (the file path), the caller frees it.
char *LIBMATTI_NEOFORGE_CrashReportExtender_DumpModLoadingCrashReport(const LIBMATTI_ML_Logger *logger,
                                                                     LIBMATTI_FML_ModLoadingIssue **issues,
                                                                     size_t issueCount, const char *topLevelDir);

#endif //MATTICRAFT_NEOFORGE_LOGGING_CRASHREPORTEXTENDER_H
