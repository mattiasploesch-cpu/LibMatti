// Port of java.lang.Thread.

#ifndef MATTICRAFT_JAVA_LANG_THREAD_H
#define MATTICRAFT_JAVA_LANG_THREAD_H

#include "libmatti/java/lang/ClassLoader.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_Thread LIBMATTI_JL_Thread;

struct LIBMATTI_JL_Thread
{
    // Java: String getName()
    char *name;
    // Java: long threadId()
    long threadId;
    // Java: ClassLoader getContextClassLoader()
    LIBMATTI_JL_ClassLoader *contextClassLoader;
    // Java: ThreadGroup getThreadGroup()
    void *threadGroup;
    int interrupted;
};

// Java: public static Thread currentThread()
LIBMATTI_JL_Thread *LIBMATTI_JL_Thread_CurrentThread(void);
// Java: the VM binds a Thread to the thread it runs on
void LIBMATTI_JL_Thread_SetCurrentThread(LIBMATTI_JL_Thread *thread);
// Java: public static Map<Thread, StackTraceElement[]> getAllStackTraces() - the C port's registry of known threads
LIBMATTI_JL_Thread **LIBMATTI_JL_Thread_GetAllThreads(size_t *count);
// Java: the VM creates the thread
LIBMATTI_JL_Thread *LIBMATTI_JL_Thread_New(const char *name);
void LIBMATTI_JL_Thread_Free(LIBMATTI_JL_Thread *thread);

// Java: public String getName()
const char *LIBMATTI_JL_Thread_GetName(const LIBMATTI_JL_Thread *thread);
// Java: public void setName(String name)
void LIBMATTI_JL_Thread_SetName(LIBMATTI_JL_Thread *thread, const char *name);
// Java: public long threadId()
long LIBMATTI_JL_Thread_ThreadId(const LIBMATTI_JL_Thread *thread);
// Java: public ClassLoader getContextClassLoader()
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_Thread_GetContextClassLoader(const LIBMATTI_JL_Thread *thread);
// Java: public ThreadGroup getThreadGroup()
void *LIBMATTI_JL_Thread_GetThreadGroup(const LIBMATTI_JL_Thread *thread);
// Java: the VM assigns the thread's group
void LIBMATTI_JL_Thread_SetThreadGroup(LIBMATTI_JL_Thread *thread, void *group);
// Java: public void setContextClassLoader(ClassLoader cl)
void LIBMATTI_JL_Thread_SetContextClassLoader(LIBMATTI_JL_Thread *thread, LIBMATTI_JL_ClassLoader *classLoader);
// Java: public void interrupt()
void LIBMATTI_JL_Thread_Interrupt(LIBMATTI_JL_Thread *thread);
// Java: public boolean isInterrupted()
int LIBMATTI_JL_Thread_IsInterrupted(const LIBMATTI_JL_Thread *thread);
// Java: public static void sleep(long millis)
void LIBMATTI_JL_Thread_Sleep(long millis);

#endif //MATTICRAFT_JAVA_LANG_THREAD_H
