#include "libmatti/java/lang/Thread.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: the VM keeps the Thread objects; the C port holds the threads it created
static LIBMATTI_JL_Thread **threads = NULL;
static size_t threadCount = 0;
static long nextThreadId = 1;

// Java: Thread.currentThread() resolves through the VM's per-thread state
static pthread_key_t currentThreadKey;
static pthread_once_t currentThreadKeyOnce = PTHREAD_ONCE_INIT;

static void create_current_thread_key(void)
{
    pthread_key_create(&currentThreadKey, NULL);
}

// Java: the VM creates the thread
LIBMATTI_JL_Thread *LIBMATTI_JL_Thread_New(const char *name)
{
    LIBMATTI_JL_Thread *thread = calloc(1, sizeof(LIBMATTI_JL_Thread));
    thread->name = name != NULL ? strdup(name) : NULL;
    thread->threadId = nextThreadId++;

    threads = realloc(threads, sizeof(*threads) * (threadCount + 1));
    threads[threadCount++] = thread;

    return thread;
}

void LIBMATTI_JL_Thread_Free(LIBMATTI_JL_Thread *thread)
{
    if (thread == NULL) return;

    for (size_t i = 0; i < threadCount; i++)
    {
        if (threads[i] != thread) continue;

        memmove(&threads[i], &threads[i + 1], sizeof(*threads) * (threadCount - i - 1));
        threadCount--;
        break;
    }

    free(thread->name);
    free(thread);
}

// Java: public static Thread currentThread()
// The VM's main thread is named "main"; the C port creates it on first use,
// because it depends on the initial context class loader.
LIBMATTI_JL_Thread *LIBMATTI_JL_Thread_CurrentThread(void)
{
    pthread_once(&currentThreadKeyOnce, create_current_thread_key);

    LIBMATTI_JL_Thread *thread = pthread_getspecific(currentThreadKey);
    if (thread != NULL) return thread;

    thread = LIBMATTI_JL_Thread_New("main");
    // Java: the initial context class loader of the main thread is the system class loader
    thread->contextClassLoader = LIBMATTI_JL_ClassLoader_GetSystemClassLoader();
    pthread_setspecific(currentThreadKey, thread);
    return thread;
}

// Java: the VM binds a Thread to the thread it runs on
void LIBMATTI_JL_Thread_SetCurrentThread(LIBMATTI_JL_Thread *thread)
{
    pthread_once(&currentThreadKeyOnce, create_current_thread_key);
    pthread_setspecific(currentThreadKey, thread);
}

// Java: public static Map<Thread, StackTraceElement[]> getAllStackTraces()
LIBMATTI_JL_Thread **LIBMATTI_JL_Thread_GetAllThreads(size_t *count)
{
    LIBMATTI_JL_Thread_CurrentThread();
    *count = threadCount;
    return threads;
}

// Java: public String getName()
const char *LIBMATTI_JL_Thread_GetName(const LIBMATTI_JL_Thread *thread)
{
    return thread->name;
}

// Java: public void setName(String name)
void LIBMATTI_JL_Thread_SetName(LIBMATTI_JL_Thread *thread, const char *name)
{
    free(thread->name);
    thread->name = strdup(name);
}

// Java: public long threadId()
long LIBMATTI_JL_Thread_ThreadId(const LIBMATTI_JL_Thread *thread)
{
    return thread->threadId;
}

// Java: public ClassLoader getContextClassLoader()
LIBMATTI_JL_ClassLoader *LIBMATTI_JL_Thread_GetContextClassLoader(const LIBMATTI_JL_Thread *thread)
{
    return thread->contextClassLoader;
}

// Java: public void setContextClassLoader(ClassLoader cl)
void LIBMATTI_JL_Thread_SetContextClassLoader(LIBMATTI_JL_Thread *thread, LIBMATTI_JL_ClassLoader *classLoader)
{
    thread->contextClassLoader = classLoader;
}

// Java: public ThreadGroup getThreadGroup()
void *LIBMATTI_JL_Thread_GetThreadGroup(const LIBMATTI_JL_Thread *thread)
{
    return thread->threadGroup;
}

// Java: the VM assigns the thread's group
void LIBMATTI_JL_Thread_SetThreadGroup(LIBMATTI_JL_Thread *thread, void *group)
{
    thread->threadGroup = group;
}

// Java: public void interrupt()
void LIBMATTI_JL_Thread_Interrupt(LIBMATTI_JL_Thread *thread)
{
    thread->interrupted = 1;
}

// Java: public boolean isInterrupted()
int LIBMATTI_JL_Thread_IsInterrupted(const LIBMATTI_JL_Thread *thread)
{
    return thread->interrupted;
}

// Java: public static void sleep(long millis)
void LIBMATTI_JL_Thread_Sleep(long millis)
{
    struct timespec duration;
    duration.tv_sec = millis / 1000;
    duration.tv_nsec = (millis % 1000) * 1000000L;
    nanosleep(&duration, NULL);
}
