





#ifndef MATTICRAFT_MOJANG_LOGGING_LOGUTILS_H
#define MATTICRAFT_MOJANG_LOGGING_LOGUTILS_H

struct LIBMATTI_ML_Logger;

// Java: public static Logger getLogger() / getLogger(String className) /
// getLogger(Class<?> clazz)
struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLogger(void);
struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLoggerForClass(const char *className);
struct LIBMATTI_ML_Logger *LIBMATTI_ML_LogUtils_GetLoggerFor(void *clazz);

// Java: public static String inDev() - the "in dev" marker string
const char *LIBMATTI_ML_LogUtils_InDev(void);

#endif
