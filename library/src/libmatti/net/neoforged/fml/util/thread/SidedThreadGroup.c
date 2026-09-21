// Port of net.neoforged.fml.util.thread.SidedThreadGroup.

#include "libmatti/net/neoforged/fml/util/thread/SidedThreadGroup.h"

#include <pthread.h>
#include <stdlib.h>

// Java: `group instanceof SidedThreadGroup` - the port keeps the known groups in a registry
static LIBMATTI_FML_SidedThreadGroup **groups = NULL;
static size_t groupCount = 0;

// Java: SidedThreadGroup(LogicalSide side)
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroup_New(LIBMATTI_FML_LogicalSide side)
{
    LIBMATTI_FML_SidedThreadGroup *group = calloc(1, sizeof(LIBMATTI_FML_SidedThreadGroup));
    group->side = side;

    groups = realloc(groups, sizeof(*groups) * (groupCount + 1));
    groups[groupCount++] = group;
    return group;
}

void LIBMATTI_FML_SidedThreadGroup_Free(LIBMATTI_FML_SidedThreadGroup *group)
{
    if (group == NULL) return;

    for (size_t i = 0; i < groupCount; i++)
    {
        if (groups[i] != group) continue;
        groups[i] = groups[groupCount - 1];
        groupCount--;
        break;
    }
    free(group);
}

// Java: public LogicalSide getSide()
LIBMATTI_FML_LogicalSide LIBMATTI_FML_SidedThreadGroup_GetSide(const LIBMATTI_FML_SidedThreadGroup *group)
{
    return group->side;
}

// Java: `group instanceof SidedThreadGroup` on the current thread's group
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroup_GetThreadGroup(const LIBMATTI_JL_Thread *thread)
{
    void *threadGroup = LIBMATTI_JL_Thread_GetThreadGroup(thread);
    for (size_t i = 0; i < groupCount; i++)
        if (groups[i] == threadGroup) return groups[i];
    return NULL;
}

// Java: @Override public Thread newThread(Runnable runnable)
typedef struct
{
    // Java: new Thread(this, runnable)
    LIBMATTI_JL_Thread *thread;
    void (*runnable)(void *);
    void *userdata;
} ThreadStart;

static void *thread_start(void *arg)
{
    ThreadStart *start = arg;
    LIBMATTI_JL_Thread_SetCurrentThread(start->thread);
    start->runnable(start->userdata);
    free(start);
    return NULL;
}

LIBMATTI_JL_Thread *LIBMATTI_FML_SidedThreadGroup_NewThread(LIBMATTI_FML_SidedThreadGroup *group,
                                                            void (*runnable)(void *), void *userdata)
{
    const char *name = group->side == LIBMATTI_FML_LogicalSide_CLIENT ? "Client" : "Server";
    LIBMATTI_JL_Thread *thread = LIBMATTI_JL_Thread_New(name);
    LIBMATTI_JL_Thread_SetThreadGroup(thread, group);

    ThreadStart *start = malloc(sizeof(ThreadStart));
    start->thread = thread;
    start->runnable = runnable;
    start->userdata = userdata;

    pthread_t handle;
    if (pthread_create(&handle, NULL, thread_start, start) != 0)
    {
        free(start);
        return NULL;
    }
    return thread;
}
