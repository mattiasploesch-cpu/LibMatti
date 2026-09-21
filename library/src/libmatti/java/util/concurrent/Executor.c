#include "libmatti/java/util/concurrent/Executor.h"

#include <stdlib.h>

typedef struct
{
    LIBMATTI_JUC_Runnable command;
    void *self;
} QueuedCommand;

static void direct_execute(LIBMATTI_JUC_Executor *executor, LIBMATTI_JUC_Runnable command, void *self)
{
    (void) executor;
    command(self);
}

static void queued_execute(LIBMATTI_JUC_Executor *executor, LIBMATTI_JUC_Runnable command, void *self)
{
    if (executor->count == executor->capacity)
    {
        executor->capacity = executor->capacity == 0 ? 16 : executor->capacity * 2;
        executor->commands = realloc(executor->commands, sizeof(QueuedCommand) * executor->capacity);
    }
    QueuedCommand *commands = (QueuedCommand *) executor->commands;
    commands[executor->count].command = command;
    commands[executor->count].self = self;
    executor->count++;
}

static LIBMATTI_JUC_Executor DIRECT_EXECUTOR = {direct_execute, 0, NULL, 0, 0};

LIBMATTI_JUC_Executor *LIBMATTI_JUC_Executor_DirectExecutor(void)
{
    return &DIRECT_EXECUTOR;
}

LIBMATTI_JUC_Executor *LIBMATTI_JUC_Executor_QueuedExecutor(void)
{
    LIBMATTI_JUC_Executor *executor = calloc(1, sizeof(LIBMATTI_JUC_Executor));
    executor->queued = 1;
    executor->execute = queued_execute;
    return executor;
}

size_t LIBMATTI_JUC_Executor_DrainQueued(LIBMATTI_JUC_Executor *executor)
{
    QueuedCommand *commands = (QueuedCommand *) executor->commands;
    size_t run = 0;
    for (size_t i = 0; i < executor->count; i++)
    {
        commands[i].command(commands[i].self);
        run++;
    }
    executor->count = 0;
    return run;
}

int LIBMATTI_JUC_Executor_IsQueuedEmpty(const LIBMATTI_JUC_Executor *executor)
{
    return executor->count == 0;
}
