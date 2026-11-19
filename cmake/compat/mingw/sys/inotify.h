#pragma once

#include <stdint.h>

struct inotify_event
{
    int wd;
    uint32_t mask;
    uint32_t cookie;
    uint32_t len;
    char name[1];
};

#define IN_MODIFY 0x00000002
#define IN_ATTRIB 0x00000004
#define IN_CREATE 0x00000100
#define IN_DELETE 0x00000200
#define IN_NONBLOCK 0x00000800
#define IN_CLOEXEC 0x00080000

static inline int inotify_init1(int flags)
{
    (void) flags;
    return -1;
}

static inline int inotify_add_watch(int descriptor, const char *path, uint32_t mask)
{
    (void) descriptor;
    (void) path;
    (void) mask;
    return -1;
}

static inline int inotify_rm_watch(int descriptor, int watch)
{
    (void) descriptor;
    (void) watch;
    return -1;
}
