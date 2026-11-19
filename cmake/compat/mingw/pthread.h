#pragma once

#include <windows.h>
#include <process.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

typedef HANDLE pthread_t;
typedef SRWLOCK pthread_mutex_t;
typedef INIT_ONCE pthread_once_t;
typedef DWORD pthread_key_t;

#define PTHREAD_MUTEX_INITIALIZER {{0}}
#define PTHREAD_ONCE_INIT INIT_ONCE_STATIC_INIT

static inline int pthread_mutex_init(pthread_mutex_t *mutex, const void *attributes)
{
    (void) attributes;
    InitializeSRWLock(mutex);
    return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    (void) mutex;
    return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    AcquireSRWLockExclusive(mutex);
    return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    return TryAcquireSRWLockExclusive(mutex) ? 0 : EBUSY;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    ReleaseSRWLockExclusive(mutex);
    return 0;
}

typedef struct MattiThreadStart
{
    void *(*function)(void *);
    void *argument;
} MattiThreadStart;

static inline unsigned __stdcall matti_thread_entry(void *argument)
{
    MattiThreadStart *start = (MattiThreadStart *) argument;
    start->function(start->argument);
    free(start);
    return 0;
}

static inline int pthread_create(pthread_t *thread, const void *attributes, void *(*function)(void *),
                                 void *argument)
{
    (void) attributes;
    MattiThreadStart *start = (MattiThreadStart *) malloc(sizeof(*start));
    if (start == NULL)
        return ENOMEM;
    start->function = function;
    start->argument = argument;
    uintptr_t handle = _beginthreadex(NULL, 0, matti_thread_entry, start, 0, NULL);
    if (handle == 0)
    {
        free(start);
        return EAGAIN;
    }
    *thread = (HANDLE) handle;
    return 0;
}

static inline int pthread_join(pthread_t thread, void **result)
{
    (void) result;
    if (thread == NULL)
        return EINVAL;
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline pthread_t pthread_self(void)
{
    return GetCurrentThread();
}

static inline int pthread_equal(pthread_t left, pthread_t right)
{
    return left == right;
}

static inline int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    (void) destructor;
    *key = TlsAlloc();
    return *key == TLS_OUT_OF_INDEXES ? EAGAIN : 0;
}

static inline int pthread_setspecific(pthread_key_t key, const void *value)
{
    return TlsSetValue(key, (void *) value) ? 0 : EINVAL;
}

static inline void *pthread_getspecific(pthread_key_t key)
{
    return TlsGetValue(key);
}

static inline BOOL WINAPI matti_once_callback(PINIT_ONCE once, PVOID argument, PVOID *context)
{
    (void) once;
    (void) context;
    ((void (*)(void)) argument)();
    return TRUE;
}

static inline int pthread_once(pthread_once_t *once, void (*function)(void))
{
    return InitOnceExecuteOnce(once, matti_once_callback, (void *) function, NULL) ? 0 : EAGAIN;
}
