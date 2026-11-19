#pragma once

#include <windows.h>

#define RTLD_LAZY 0
#define RTLD_NOW 0
#define RTLD_LOCAL 0
#define RTLD_GLOBAL 0
#define RTLD_NOLOAD 0
#define RTLD_DEFAULT NULL
#define RTLD_DI_LINKMAP 2

typedef struct Dl_info
{
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
} Dl_info;

static inline void *dlopen(const char *path, int flags)
{
    (void) flags;
    return (void *) LoadLibraryA(path);
}

static inline void *dlsym(void *handle, const char *name)
{
    HMODULE module = handle != NULL ? (HMODULE) handle : GetModuleHandleA(NULL);
    return (void *) GetProcAddress(module, name);
}

static inline int dlclose(void *handle)
{
    return FreeLibrary((HMODULE) handle) ? 0 : -1;
}

static inline const char *dlerror(void)
{
    static char message[512];
    DWORD error = GetLastError();
    if (error == 0)
        return NULL;
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, message, sizeof(message), NULL);
    return message;
}

static inline int dlinfo(void *handle, int request, void *result)
{
    (void) handle;
    (void) request;
    (void) result;
    return -1;
}

static inline int dladdr(const void *address, Dl_info *info)
{
    HMODULE module = NULL;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR) address, &module))
        return 0;
    info->dli_fbase = (void *) module;
    info->dli_fname = NULL;
    info->dli_sname = NULL;
    info->dli_saddr = NULL;
    return 1;
}
