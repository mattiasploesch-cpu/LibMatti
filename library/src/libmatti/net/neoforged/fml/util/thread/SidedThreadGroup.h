// Port of net.neoforged.fml.util.thread.SidedThreadGroup.
// Java extends ThreadGroup and implements ThreadFactory; C has neither, so the port keeps the
// group identity in a registry (which is what EffectiveSide's `instanceof` tests) and models
// newThread(Runnable) as starting a thread that runs the runnable.

#ifndef MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUP_H
#define MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUP_H

#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/LogicalSide.h"

typedef struct LIBMATTI_FML_SidedThreadGroup LIBMATTI_FML_SidedThreadGroup;

struct LIBMATTI_FML_SidedThreadGroup
{
    // Java: private final LogicalSide side
    LIBMATTI_FML_LogicalSide side;
};

// Java: SidedThreadGroup(LogicalSide side) { super(side.name()); }
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroup_New(LIBMATTI_FML_LogicalSide side);
void LIBMATTI_FML_SidedThreadGroup_Free(LIBMATTI_FML_SidedThreadGroup *group);

// Java: public LogicalSide getSide()
LIBMATTI_FML_LogicalSide LIBMATTI_FML_SidedThreadGroup_GetSide(const LIBMATTI_FML_SidedThreadGroup *group);

// Java: the group this thread belongs to, or NULL (Java: Thread.currentThread().getThreadGroup())
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroup_GetThreadGroup(const LIBMATTI_JL_Thread *thread);

// Java: @Override public Thread newThread(Runnable runnable) - NULL if the thread cannot be created
LIBMATTI_JL_Thread *LIBMATTI_FML_SidedThreadGroup_NewThread(LIBMATTI_FML_SidedThreadGroup *group,
                                                            void (*runnable)(void *), void *userdata);

#endif //MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUP_H
