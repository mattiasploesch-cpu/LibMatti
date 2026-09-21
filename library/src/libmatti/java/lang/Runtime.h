// Port of java.lang.Runtime (the surface the loader uses).

#ifndef MATTICRAFT_JAVA_LANG_RUNTIME_H
#define MATTICRAFT_JAVA_LANG_RUNTIME_H

// Java: public static Runtime getRuntime()
typedef struct LIBMATTI_JL_Runtime
{
    int unused;
} LIBMATTI_JL_Runtime;

LIBMATTI_JL_Runtime *LIBMATTI_JL_Runtime_GetRuntime(void);

// Java: public int availableProcessors()
int LIBMATTI_JL_Runtime_AvailableProcessors(void);

// Java: public long maxMemory() / totalMemory() / freeMemory()
long LIBMATTI_JL_Runtime_MaxMemory(void);
long LIBMATTI_JL_Runtime_TotalMemory(void);
long LIBMATTI_JL_Runtime_FreeMemory(void);

// Java: public static int version() - the JVM's feature release
int LIBMATTI_JL_Runtime_Version(void);

#endif //MATTICRAFT_JAVA_LANG_RUNTIME_H
