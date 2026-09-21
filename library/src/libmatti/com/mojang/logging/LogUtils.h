// Port of com.mojang.logging.LogUtils (Mojang's tiny log4j wrapper).
// No public source repo exists (binary-only maven artifact), so this follows
// the decompiled API: getLogger() returns the Logger for the calling class.
// The port's single-Logger LogManager makes the class-name parameter a no-op.

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

#endif //MATTICRAFT_MOJANG_LOGGING_LOGUTILS_H
