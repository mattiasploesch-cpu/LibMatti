//
// Replacement for org.apache.logging.log4j.LogManager / Logger.
// log4j is an external dependency; this layer keeps the used surface
// (LogManager.getLogger() + logger.info/debug/error/fatal with a marker and
// log4j parameterized messages "{}") and writes to stdout/stderr.
//

#ifndef MATTICRAFT_MODLAUNCHER_LOGMANAGER_H
#define MATTICRAFT_MODLAUNCHER_LOGMANAGER_H

#include "libmatti/cpw/modlauncher/LogMarkers.h"

typedef struct LIBMATTI_ML_Logger LIBMATTI_ML_Logger;

// Java: org.apache.logging.log4j.Level / org.slf4j.event.Level
typedef enum
{
    LIBMATTI_ML_LEVEL_TRACE,
    LIBMATTI_ML_LEVEL_DEBUG,
    LIBMATTI_ML_LEVEL_INFO,
    LIBMATTI_ML_LEVEL_WARN,
    LIBMATTI_ML_LEVEL_ERROR,
    LIBMATTI_ML_LEVEL_FATAL
} LIBMATTI_ML_Level;

// Java: logger.atLevel(Level)
void LIBMATTI_ML_Logger_Log(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                            LIBMATTI_ML_Level level, const char *message, ...);

// Java: static Logger getLogger()
LIBMATTI_ML_Logger *LIBMATTI_ML_LogManager_GetLogger(void);

// Java: logger.<level>(marker, message, params...) - "{}" are replaced by the
// arguments in order. All arguments are strings.
void LIBMATTI_ML_Logger_Trace(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                              const char *message, ...);
// Java: logger.isEnabled(Level.TRACE) (marker may be NULL) - the dumpClass gate
int LIBMATTI_ML_Logger_IsEnabledTrace(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker);
void LIBMATTI_ML_Logger_Debug(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                              const char *message, ...);
void LIBMATTI_ML_Logger_Info(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                             const char *message, ...);
void LIBMATTI_ML_Logger_Warn(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                             const char *message, ...);
void LIBMATTI_ML_Logger_Error(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                              const char *message, ...);
void LIBMATTI_ML_Logger_Fatal(const LIBMATTI_ML_Logger *logger, const LIBMATTI_ML_LogMarker *marker,
                              const char *message, ...);

#endif //MATTICRAFT_MODLAUNCHER_LOGMANAGER_H
