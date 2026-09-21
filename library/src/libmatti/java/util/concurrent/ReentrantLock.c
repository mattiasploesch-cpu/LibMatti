#include "libmatti/java/util/concurrent/ReentrantLock.h"

#include <stdlib.h>

LIBMATTI_JUC_ReentrantLock *LIBMATTI_JUC_ReentrantLock_New(void)
{
    LIBMATTI_JUC_ReentrantLock *lock = calloc(1, sizeof(LIBMATTI_JUC_ReentrantLock));
    pthread_mutex_init(&lock->mutex, NULL);
    return lock;
}

void LIBMATTI_JUC_ReentrantLock_Free(LIBMATTI_JUC_ReentrantLock *lock)
{
    if (lock == NULL)
        return;
    pthread_mutex_destroy(&lock->mutex);
    free(lock);
}

void LIBMATTI_JUC_ReentrantLock_Lock(LIBMATTI_JUC_ReentrantLock *lock)
{
    pthread_mutex_lock(&lock->mutex);
    lock->owner = pthread_self();
    lock->locked = 1;
    lock->holdCount++;
}

void LIBMATTI_JUC_ReentrantLock_Unlock(LIBMATTI_JUC_ReentrantLock *lock)
{
    if (--lock->holdCount == 0)
        lock->locked = 0;
    pthread_mutex_unlock(&lock->mutex);
}

int LIBMATTI_JUC_ReentrantLock_TryLock(LIBMATTI_JUC_ReentrantLock *lock)
{
    if (pthread_mutex_trylock(&lock->mutex) != 0)
        return 0;
    lock->owner = pthread_self();
    lock->locked = 1;
    lock->holdCount++;
    return 1;
}

int LIBMATTI_JUC_ReentrantLock_GetHoldCount(const LIBMATTI_JUC_ReentrantLock *lock)
{
    return lock->holdCount;
}

int LIBMATTI_JUC_ReentrantLock_IsHeldByCurrentThread(const LIBMATTI_JUC_ReentrantLock *lock)
{
    return lock->locked && pthread_equal(lock->owner, pthread_self());
}
