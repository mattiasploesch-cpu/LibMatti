#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

// Java: the constructor table
static const LIBMATTI_ML_Level LOG_LEVELS[] = {
    LIBMATTI_ML_LEVEL_ERROR,

    LIBMATTI_ML_LEVEL_WARN,

    LIBMATTI_ML_LEVEL_WARN,

    LIBMATTI_ML_LEVEL_WARN,

};

static const LIBMATTI_FML_ModLoadingIssue_Severity ISSUE_SEVERITIES[] = {
    LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR,

    LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING,

    LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING,

    LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING,

};

// Java: public Level getLogLevel()
LIBMATTI_ML_Level LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetLogLevel(
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting)
{
    return LOG_LEVELS[reporting];
}

// Java: public ModLoadingIssue.Severity getIssueSeverity()
LIBMATTI_FML_ModLoadingIssue_Severity LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_GetIssueSeverity(
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting reporting)
{
    return ISSUE_SEVERITIES[reporting];
}
