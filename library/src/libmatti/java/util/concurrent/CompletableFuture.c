#include "libmatti/java/util/concurrent/CompletableFuture.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    void *(*function)(void *value, void *self);
    void (*action)(void *self);
    void *self;
    LIBMATTI_JUC_CompletableFuture *target;
} Dependent;

struct LIBMATTI_JUC_CompletableFuture
{
    int done;
    int exceptional;
    void *result;     // value on normal completion, the Throwable on exceptional
    Dependent *dependents;
    size_t dependentCount;
    size_t dependentCapacity;
};

static void notify_dependents(LIBMATTI_JUC_CompletableFuture *future)
{
    for (size_t i = 0; i < future->dependentCount; i++)
    {
        Dependent *dependent = &future->dependents[i];
        if (dependent->target != NULL)
        {
            // thenApply/thenRun: the dependent future fails with the source
            if (future->exceptional)
            {
                LIBMATTI_JUC_CompletableFuture_CompleteExceptionally(dependent->target, future->result);
                continue;
            }
            if (dependent->function != NULL)
                LIBMATTI_JUC_CompletableFuture_Complete(dependent->target, dependent->function(future->result, dependent->self));
            else if (dependent->action != NULL)
            {
                dependent->action(dependent->self);
                LIBMATTI_JUC_CompletableFuture_Complete(dependent->target, NULL);
            }
        }
        else if (dependent->action != NULL)
        {
            dependent->action(dependent->self);
        }
    }
    future->dependentCount = 0;
}

LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JUC_CompletableFuture));
}

LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_CompletedFuture(void *value)
{
    LIBMATTI_JUC_CompletableFuture *future = LIBMATTI_JUC_CompletableFuture_New();
    future->done = 1;
    future->result = value;
    return future;
}

void LIBMATTI_JUC_CompletableFuture_Free(LIBMATTI_JUC_CompletableFuture *future)
{
    if (future == NULL)
        return;
    free(future->dependents);
    free(future);
}

int LIBMATTI_JUC_CompletableFuture_Complete(LIBMATTI_JUC_CompletableFuture *future, void *value)
{
    if (future->done)
        return 0;
    future->done = 1;
    future->result = value;
    notify_dependents(future);
    return 1;
}

int LIBMATTI_JUC_CompletableFuture_CompleteExceptionally(LIBMATTI_JUC_CompletableFuture *future, void *throwable)
{
    if (future->done)
        return 0;
    future->done = 1;
    future->exceptional = 1;
    future->result = throwable;
    notify_dependents(future);
    return 1;
}

int LIBMATTI_JUC_CompletableFuture_IsDone(const LIBMATTI_JUC_CompletableFuture *future)
{
    return future->done;
}

int LIBMATTI_JUC_CompletableFuture_IsCompletedExceptionally(const LIBMATTI_JUC_CompletableFuture *future)
{
    return future->done && future->exceptional;
}

void *LIBMATTI_JUC_CompletableFuture_Join(const LIBMATTI_JUC_CompletableFuture *future)
{
    if (!future->done)
    {
        // Java would block here; the port has no scheduler to unblock it
        fprintf(stderr, "java.util.concurrent.CompletionException: future not completed (blocking unsupported)\n");
        abort();
    }
    if (future->exceptional)
    {
        // Java: CompletionException wrapping the cause
        fprintf(stderr, "java.util.concurrent.CompletionException: future completed exceptionally\n");
        abort();
    }
    return future->result;
}

void *LIBMATTI_JUC_CompletableFuture_Get(const LIBMATTI_JUC_CompletableFuture *future)
{
    return LIBMATTI_JUC_CompletableFuture_Join(future);
}

void *LIBMATTI_JUC_CompletableFuture_Exception(const LIBMATTI_JUC_CompletableFuture *future)
{
    return future->done && future->exceptional ? future->result : NULL;
}

static void add_dependent(LIBMATTI_JUC_CompletableFuture *future, Dependent dependent)
{
    if (future->done)
    {
        // Java: the dependent stage completes immediately through the direct executor
        if (dependent.target != NULL)
        {
            if (future->exceptional)
            {
                LIBMATTI_JUC_CompletableFuture_CompleteExceptionally(dependent.target, future->result);
                return;
            }
            if (dependent.function != NULL)
            {
                LIBMATTI_JUC_CompletableFuture_Complete(dependent.target, dependent.function(future->result, dependent.self));
                return;
            }
        }
        if (dependent.action != NULL)
        {
            dependent.action(dependent.self);
            if (dependent.target != NULL)
                LIBMATTI_JUC_CompletableFuture_Complete(dependent.target, NULL);
        }
        return;
    }

    if (future->dependentCount == future->dependentCapacity)
    {
        future->dependentCapacity = future->dependentCapacity == 0 ? 4 : future->dependentCapacity * 2;
        future->dependents = realloc(future->dependents, sizeof(Dependent) * future->dependentCapacity);
    }
    future->dependents[future->dependentCount++] = dependent;
}

LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_ThenApply(
    LIBMATTI_JUC_CompletableFuture *future, void *(*function)(void *value, void *self), void *self)
{
    LIBMATTI_JUC_CompletableFuture *target = LIBMATTI_JUC_CompletableFuture_New();
    Dependent dependent = {function, NULL, self, target};
    add_dependent(future, dependent);
    return target;
}

LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_ThenRun(
    LIBMATTI_JUC_CompletableFuture *future, void (*action)(void *self), void *self)
{
    LIBMATTI_JUC_CompletableFuture *target = LIBMATTI_JUC_CompletableFuture_New();
    Dependent dependent = {NULL, action, self, target};
    add_dependent(future, dependent);
    return target;
}

// Java: allOf completes when every input completed; the port polls via a
// dependent on each input that counts down a shared counter.
typedef struct
{
    LIBMATTI_JUC_CompletableFuture *target;
    size_t remaining;
} AllOfState;

static void all_of_countdown(AllOfState *state)
{
    if (--state->remaining == 0)
        LIBMATTI_JUC_CompletableFuture_Complete(state->target, NULL);
}

LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_AllOf(
    LIBMATTI_JUC_CompletableFuture **futures, size_t count)
{
    LIBMATTI_JUC_CompletableFuture *target = LIBMATTI_JUC_CompletableFuture_New();
    AllOfState *state = malloc(sizeof(AllOfState));
    state->target = target;
    state->remaining = count;

    // Java: an empty allOf completes immediately
    if (count == 0)
    {
        free(state);
        LIBMATTI_JUC_CompletableFuture_Complete(target, NULL);
        return target;
    }

    for (size_t i = 0; i < count; i++)
    {
        Dependent dependent = {NULL, (void (*)(void *)) all_of_countdown, state, NULL};
        add_dependent(futures[i], dependent);
    }
    return target;
}
