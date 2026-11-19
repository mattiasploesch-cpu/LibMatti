#pragma once

#include <sys/types.h>

#define WIFEXITED(status) (1)
#define WEXITSTATUS(status) (status)
#define WIFSIGNALED(status) (0)

static inline pid_t waitpid(pid_t pid, int *status, int options)
{
    (void) pid;
    (void) status;
    (void) options;
    return -1;
}
