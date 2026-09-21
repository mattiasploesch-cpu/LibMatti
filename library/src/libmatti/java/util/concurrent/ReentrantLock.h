// Port of java.util.concurrent.locks.ReentrantLock (the fair=false case the
// loader uses) built on pthread_mutex.

#ifndef MATTICRAFT_JAVA_UTIL_CONCURRENT_REENTRANTLOCK_H
#define MATTICRAFT_JAVA_UTIL_CONCURRENT_REENTRANTLOCK_H

#include <pthread.h>

typedef struct LIBMATTI_JUC_ReentrantLock
{
    pthread_mutex_t mutex;
    pthread_t owner;   // Java: the exclusive owner thread
    int locked;        // 0/1, guards reading owner
    int holdCount;     // Java: the reentrancy count
} LIBMATTI_JUC_ReentrantLock;

// Java: public ReentrantLock()
LIBMATTI_JUC_ReentrantLock *LIBMATTI_JUC_ReentrantLock_New(void);
void LIBMATTI_JUC_ReentrantLock_Free(LIBMATTI_JUC_ReentrantLock *lock);

// Java: public void lock()
void LIBMATTI_JUC_ReentrantLock_Lock(LIBMATTI_JUC_ReentrantLock *lock);
// Java: public void unlock()
void LIBMATTI_JUC_ReentrantLock_Unlock(LIBMATTI_JUC_ReentrantLock *lock);
// Java: public boolean tryLock()
int LIBMATTI_JUC_ReentrantLock_TryLock(LIBMATTI_JUC_ReentrantLock *lock);
// Java: public int getHoldCount()
int LIBMATTI_JUC_ReentrantLock_GetHoldCount(const LIBMATTI_JUC_ReentrantLock *lock);
// Java: public boolean isHeldByCurrentThread()
int LIBMATTI_JUC_ReentrantLock_IsHeldByCurrentThread(const LIBMATTI_JUC_ReentrantLock *lock);

#endif //MATTICRAFT_JAVA_UTIL_CONCURRENT_REENTRANTLOCK_H
