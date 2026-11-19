#pragma once

#include <windows.h>
#include <stddef.h>

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define MAP_PRIVATE 0x1
#define MAP_ANONYMOUS 0x2
#define MAP_FAILED ((void *) -1)

static inline void *mmap(void *address, size_t length, int protection, int flags, int descriptor,
                         long offset)
{
    (void) address;
    (void) protection;
    (void) flags;
    (void) descriptor;
    (void) offset;
    return VirtualAlloc(NULL, length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

static inline int munmap(void *address, size_t length)
{
    (void) length;
    return VirtualFree(address, 0, MEM_RELEASE) ? 0 : -1;
}

static inline int mprotect(void *address, size_t length, int protection)
{
    (void) protection;
    DWORD previous = 0;
    return VirtualProtect(address, length, PAGE_EXECUTE_READWRITE, &previous) ? 0 : -1;
}
