#pragma once

#if defined(_WIN32)

#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef interface
#undef interface
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN 1
#define _SC_PHYS_PAGES 2
#define _SC_PAGESIZE 3
#endif

static inline char *matti_strndup(const char *text, size_t length)
{
    size_t used = 0;
    while (used < length && text[used] != '\0')
        used++;
    char *copy = (char *) malloc(used + 1);
    if (copy != NULL)
    {
        memcpy(copy, text, used);
        copy[used] = '\0';
    }
    return copy;
}
#define strndup matti_strndup

static inline int matti_setenv(const char *name, const char *value, int overwrite)
{
    (void) overwrite;
    return _putenv_s(name, value != NULL ? value : "");
}
#define setenv matti_setenv

static inline int matti_unsetenv(const char *name)
{
    return _putenv_s(name, "");
}
#define unsetenv matti_unsetenv

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#endif

static inline int matti_nanosleep(const struct timespec *request, struct timespec *remaining)
{
    (void) remaining;
    DWORD milliseconds = (DWORD) (request->tv_sec * 1000 + request->tv_nsec / 1000000);
    Sleep(milliseconds);
    return 0;
}
#define nanosleep matti_nanosleep

static inline int matti_usleep(unsigned int microseconds)
{
    Sleep((DWORD) (microseconds / 1000));
    return 0;
}
#define usleep matti_usleep

static inline int matti_clock_gettime(int clock_id, struct timespec *value)
{
    if (clock_id == CLOCK_MONOTONIC)
    {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        value->tv_sec = (time_t) (counter.QuadPart / frequency.QuadPart);
        value->tv_nsec = (long) ((counter.QuadPart % frequency.QuadPart) * 1000000000LL /
                                 frequency.QuadPart);
    }
    else
    {
        value->tv_sec = time(NULL);
        value->tv_nsec = 0;
    }
    return 0;
}
#define clock_gettime matti_clock_gettime

static inline long matti_sysconf(int name)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    if (name == _SC_NPROCESSORS_ONLN)
        return (long) info.dwNumberOfProcessors;
    if (name == _SC_PAGESIZE)
        return (long) info.dwPageSize;
    if (name == _SC_PHYS_PAGES)
    {
        MEMORYSTATUSEX memory;
        memory.dwLength = sizeof(memory);
        if (GlobalMemoryStatusEx(&memory))
            return (long) (memory.ullTotalPhys / info.dwPageSize);
    }
    return -1;
}
#define sysconf matti_sysconf

static inline struct tm *matti_gmtime_r(const time_t *timep, struct tm *result)
{
    return gmtime_s(result, timep) == 0 ? result : NULL;
}
#define gmtime_r matti_gmtime_r

static inline struct tm *matti_localtime_r(const time_t *timep, struct tm *result)
{
    return localtime_s(result, timep) == 0 ? result : NULL;
}
#define localtime_r matti_localtime_r

static inline char *matti_realpath(const char *path, char *resolved)
{
    (void) resolved;
    return _fullpath(NULL, path, 0);
}
#define realpath matti_realpath

static inline int matti_readlink(const char *path, char *buffer, size_t size)
{
    (void) path;
    (void) buffer;
    (void) size;
    return -1;
}
#define readlink matti_readlink

static inline int matti_mkdir(const char *path, int mode)
{
    (void) mode;
    return _mkdir(path);
}
#define mkdir matti_mkdir

static inline int matti_mkstemp(char *pattern)
{
    if (_mktemp_s(pattern, strlen(pattern) + 1) != 0)
        return -1;
    return _open(pattern, _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
}
#define mkstemp matti_mkstemp

static inline int matti_mkstemps(char *pattern, int suffix_length)
{
    (void) pattern;
    (void) suffix_length;
    return -1;
}
#define mkstemps matti_mkstemps

static inline char *matti_mkdtemp(char *pattern)
{
    if (_mktemp_s(pattern, strlen(pattern) + 1) != 0)
        return NULL;
    if (_mkdir(pattern) != 0)
        return NULL;
    return pattern;
}
#define mkdtemp matti_mkdtemp

static inline int matti_fork(void)
{
    return -1;
}
#define fork matti_fork

static inline int matti_kill(int pid, int signal)
{
    (void) pid;
    (void) signal;
    return -1;
}
#define kill matti_kill

#endif
