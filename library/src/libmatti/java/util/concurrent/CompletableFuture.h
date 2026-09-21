// Port of java.util.concurrent.CompletableFuture<T> for the loader's use:
// T is void*, completion is explicit (the port has no background threads on
// its own - the host completes futures from its own code).

#ifndef MATTICRAFT_JAVA_UTIL_CONCURRENT_COMPLETABLEFUTURE_H
#define MATTICRAFT_JAVA_UTIL_CONCURRENT_COMPLETABLEFUTURE_H

#include <stddef.h>

typedef struct LIBMATTI_JUC_CompletableFuture LIBMATTI_JUC_CompletableFuture;

// Java: public static <U> CompletableFuture<U> completedFuture(U value)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_CompletedFuture(void *value);
// Java: public CompletableFuture() - a new, not-completed future
LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_New(void);
void LIBMATTI_JUC_CompletableFuture_Free(LIBMATTI_JUC_CompletableFuture *future);

// Java: public boolean complete(T value) - 1 if this call completed it
int LIBMATTI_JUC_CompletableFuture_Complete(LIBMATTI_JUC_CompletableFuture *future, void *value);
// Java: public boolean completeExceptionally(Throwable ex)
int LIBMATTI_JUC_CompletableFuture_CompleteExceptionally(LIBMATTI_JUC_CompletableFuture *future, void *throwable);
// Java: public boolean isDone()
int LIBMATTI_JUC_CompletableFuture_IsDone(const LIBMATTI_JUC_CompletableFuture *future);
// Java: public boolean isCompletedExceptionally()
int LIBMATTI_JUC_CompletableFuture_IsCompletedExceptionally(const LIBMATTI_JUC_CompletableFuture *future);
// Java: public T join() - blocks in Java; in the port it aborts when not done
void *LIBMATTI_JUC_CompletableFuture_Join(const LIBMATTI_JUC_CompletableFuture *future);
// Java: public T get() - same as join for the port
void *LIBMATTI_JUC_CompletableFuture_Get(const LIBMATTI_JUC_CompletableFuture *future);
// Java: public Throwable exception() - the port's name for the exceptional result (NULL = none)
void *LIBMATTI_JUC_CompletableFuture_Exception(const LIBMATTI_JUC_CompletableFuture *future);

// Java: public CompletableFuture<T> thenApply(Function<T,U>) - returns a new future,
// completed immediately when this one is (the direct-executor path)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_ThenApply(
    LIBMATTI_JUC_CompletableFuture *future, void *(*function)(void *value, void *self), void *self);
// Java: public CompletableFuture<T> thenRun(Runnable)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_ThenRun(
    LIBMATTI_JUC_CompletableFuture *future, void (*action)(void *self), void *self);
// Java: public static CompletableFuture<Void> allOf(CompletableFuture<?>... cfs) -
// the returned future is done when all inputs are done (poll-based for the port)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_JUC_CompletableFuture_AllOf(
    LIBMATTI_JUC_CompletableFuture **futures, size_t count);

#endif //MATTICRAFT_JAVA_UTIL_CONCURRENT_COMPLETABLEFUTURE_H
