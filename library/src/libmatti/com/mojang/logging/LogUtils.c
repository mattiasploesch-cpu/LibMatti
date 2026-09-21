// Port of com.mojang.logging.LogUtils over the launcher's LogManager.

#include "libmatti/com/mojang/logging/LogUtils.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLogger(void)
{
    // Java: LogManager.getLogger() with the caller's class name; the port's
    // LogManager has one shared logger
    return LIBMATTI_ML_LogManager_GetLogger();
}

struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLoggerForClass(const char *className)
{
    (void) className;
    return LIBMATTI_ML_LogManager_GetLogger();
}

struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLoggerFor(void *clazz)
{
    (void) clazz;
    return LIBMATTI_ML_LogManager_GetLogger();
}

const char *LIBMATTI_ML_LogUtils_InDev(void)
{
    // Java: "in dev" when no version marker is set
    return "in dev";
}
