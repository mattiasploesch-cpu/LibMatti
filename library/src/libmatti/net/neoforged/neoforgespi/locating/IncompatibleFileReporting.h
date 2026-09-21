// Port of net.neoforged.neoforgespi.locating.IncompatibleFileReporting.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_INCOMPATIBLEFILEREPORTING_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_INCOMPATIBLEFILEREPORTING_H

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

// Java: public enum IncompatibleFileReporting { ERROR, WARN_ALWAYS, WARN_ON_KNOWN_INCOMPATIBILITY, IGNORE }
typedef enum
{
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_ERROR,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ALWAYS,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_WARN_ON_KNOWN_INCOMPATIBILITY,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_IGNORE
} LIBMATTI_NEOFORGESPI_IncompatibleFileReporting;

// Java: public Level getLogLevel()
LIBMATTI_ML_Level LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetLogLevel(
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting);
// Java: public ModLoadingIssue.Severity getIssueSeverity()
LIBMATTI_FML_ModLoadingIssue_Severity LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetIssueSeverity(
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_INCOMPATIBLEFILEREPORTING_H
