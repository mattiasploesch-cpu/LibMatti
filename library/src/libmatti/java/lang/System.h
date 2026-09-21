#ifndef MATTICRAFT_JAVA_LANG_SYSTEM_H
#define MATTICRAFT_JAVA_LANG_SYSTEM_H

#include "libmatti/java/io/PrintWriter.h"

// Java: public static final PrintStream out
LIBMATTI_JI_PrintWriter *LIBMATTI_JL_System_Out(void);
// Java: public static final PrintStream err
LIBMATTI_JI_PrintWriter *LIBMATTI_JL_System_Err(void);

// Java: public static String getProperty(String key) - the C port reads the environment
const char *LIBMATTI_JL_System_GetProperty(const char *key);
// Java: public static String setProperty(String key, String value) - the C port writes the environment
const char *LIBMATTI_JL_System_SetProperty(const char *key, const char *value);
// Java: public static String getenv(String name)
const char *LIBMATTI_JL_System_Getenv(const char *name);

// Java: public static void exit(int status)
void LIBMATTI_JL_System_Exit(int status);

// Java: public static long currentTimeMillis()
long long LIBMATTI_JL_System_CurrentTimeMillis(void);

#endif //MATTICRAFT_JAVA_LANG_SYSTEM_H
