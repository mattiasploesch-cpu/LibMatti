// Port of org.spongepowered.asm.util.ReEntranceLock.

#include "libmatti/org/spongepowered/asm/util/ReEntranceLock.h"

#include <stdlib.h>

LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_New(int maxDepth)
{
    LIBMATTI_SP_ReEntranceLock *lock = calloc(1, sizeof(*lock));
    lock->maxDepth = maxDepth;
    return lock;
}

void LIBMATTI_SP_ReEntranceLock_Free(LIBMATTI_SP_ReEntranceLock *lock)
{
    free(lock);
}

int LIBMATTI_SP_ReEntranceLock_GetMaxDepth(const LIBMATTI_SP_ReEntranceLock *lock)
{
    return lock->maxDepth;
}

int LIBMATTI_SP_ReEntranceLock_GetDepth(const LIBMATTI_SP_ReEntranceLock *lock)
{
    return lock->depth;
}

// Java: public ReEntranceLock push() { this.depth++; return this; }
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Push(LIBMATTI_SP_ReEntranceLock *lock)
{
    lock->depth++;
    return lock;
}

// Java: public ReEntranceLock pop()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Pop(LIBMATTI_SP_ReEntranceLock *lock)
{
    lock->depth--;
    return lock;
}

// Java: public boolean check() { return this.depth < this.maxDepth; }
int LIBMATTI_SP_ReEntranceLock_Check(const LIBMATTI_SP_ReEntranceLock *lock)
{
    return lock->depth < lock->maxDepth;
}

// Java: public boolean checkAndSet() { if (this.check()) { return this.set().isSet(); } return false; }
int LIBMATTI_SP_ReEntranceLock_CheckAndSet(LIBMATTI_SP_ReEntranceLock *lock)
{
    if (LIBMATTI_SP_ReEntranceLock_Check(lock))
        return LIBMATTI_SP_ReEntranceLock_IsSet(LIBMATTI_SP_ReEntranceLock_Set(lock));
    return 0;
}

// Java: public ReEntranceLock set()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Set(LIBMATTI_SP_ReEntranceLock *lock)
{
    lock->set = 1;
    return lock;
}

// Java: public boolean isSet()
int LIBMATTI_SP_ReEntranceLock_IsSet(const LIBMATTI_SP_ReEntranceLock *lock)
{
    return lock->set;
}

// Java: public ReEntranceLock clear()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Clear(LIBMATTI_SP_ReEntranceLock *lock)
{
    lock->set = 0;
    return lock;
}
