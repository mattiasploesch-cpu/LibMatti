




#ifndef MATTICRAFT_SP_ASM_UTIL_REENTRANCELOCK_H
#define MATTICRAFT_SP_ASM_UTIL_REENTRANCELOCK_H

// Java: public class ReEntranceLock
typedef struct LIBMATTI_SP_ReEntranceLock LIBMATTI_SP_ReEntranceLock;

struct LIBMATTI_SP_ReEntranceLock
{
// Java: private final int maxDepth
    int maxDepth;
// Java: private final ThreadLocal<Integer> depth
    int depth;
// Java: private final ThreadLocal<Boolean> set
    int set;
};

// Java: public ReEntranceLock(int maxDepth)
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_New(int maxDepth);
void LIBMATTI_SP_ReEntranceLock_Free(LIBMATTI_SP_ReEntranceLock *lock);

// Java: public int getMaxDepth()
int LIBMATTI_SP_ReEntranceLock_GetMaxDepth(const LIBMATTI_SP_ReEntranceLock *lock);
// Java: public int getDepth()
int LIBMATTI_SP_ReEntranceLock_GetDepth(const LIBMATTI_SP_ReEntranceLock *lock);
// Java: public ReEntranceLock push()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Push(LIBMATTI_SP_ReEntranceLock *lock);
// Java: public ReEntranceLock pop()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Pop(LIBMATTI_SP_ReEntranceLock *lock);
// Java: public boolean check()
int LIBMATTI_SP_ReEntranceLock_Check(const LIBMATTI_SP_ReEntranceLock *lock);
// Java: public boolean checkAndSet()
int LIBMATTI_SP_ReEntranceLock_CheckAndSet(LIBMATTI_SP_ReEntranceLock *lock);
// Java: public ReEntranceLock set()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Set(LIBMATTI_SP_ReEntranceLock *lock);
// Java: public boolean isSet()
int LIBMATTI_SP_ReEntranceLock_IsSet(const LIBMATTI_SP_ReEntranceLock *lock);
// Java: public ReEntranceLock clear()
LIBMATTI_SP_ReEntranceLock *LIBMATTI_SP_ReEntranceLock_Clear(LIBMATTI_SP_ReEntranceLock *lock);

#endif
