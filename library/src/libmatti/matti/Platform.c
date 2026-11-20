#include "libmatti/matti/Platform.h"

#include <string.h>
#include <strings.h>

#if defined(__ANDROID__)
#define MATTI_PLATFORM_ANDROID 1
#elif defined(_WIN32)
#define MATTI_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define MATTI_PLATFORM_MACOS 1
#elif defined(__FreeBSD__)
#define MATTI_PLATFORM_FREEBSD 1
#elif defined(__linux__)
#define MATTI_PLATFORM_LINUX 1
#endif

LIBMATTI_PlatformKind LIBMATTI_Platform_Get(void)
{
#if defined(MATTI_PLATFORM_ANDROID)
    return LIBMATTI_PLATFORM_ANDROID;
#elif defined(MATTI_PLATFORM_WINDOWS)
    return LIBMATTI_PLATFORM_WINDOWS;
#elif defined(MATTI_PLATFORM_MACOS)
    return LIBMATTI_PLATFORM_MACOS;
#elif defined(MATTI_PLATFORM_FREEBSD)
    return LIBMATTI_PLATFORM_FREEBSD;
#elif defined(MATTI_PLATFORM_LINUX)
    return LIBMATTI_PLATFORM_LINUX;
#else
    return LIBMATTI_PLATFORM_UNKNOWN;
#endif
}

const char *LIBMATTI_Platform_Name(void)
{
    switch (LIBMATTI_Platform_Get())
    {
        case LIBMATTI_PLATFORM_LINUX: return "linux";
        case LIBMATTI_PLATFORM_WINDOWS: return "windows";
        case LIBMATTI_PLATFORM_MACOS: return "macos";
        case LIBMATTI_PLATFORM_ANDROID: return "android";
        case LIBMATTI_PLATFORM_FREEBSD: return "freebsd";
        default: return "unknown";
    }
}

const char *LIBMATTI_Platform_Arch(void)
{
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#elif defined(__riscv) && __riscv_xlen == 64
    return "riscv64";
#else
    return "unknown";
#endif
}

int LIBMATTI_Platform_Bits(void)
{



    return (int) (sizeof(void *) * 8);
}

int LIBMATTI_Platform_IsLittleEndian(void)
{
    const unsigned int probe = 1u;
    return *(const unsigned char *) &probe == 1u;
}

int LIBMATTI_Platform_IsWindows(void) { return LIBMATTI_Platform_Get() == LIBMATTI_PLATFORM_WINDOWS; }
int LIBMATTI_Platform_IsMacOS(void) { return LIBMATTI_Platform_Get() == LIBMATTI_PLATFORM_MACOS; }
int LIBMATTI_Platform_IsLinux(void)
{
    LIBMATTI_PlatformKind kind = LIBMATTI_Platform_Get();
    return kind == LIBMATTI_PLATFORM_LINUX || kind == LIBMATTI_PLATFORM_FREEBSD;
}
int LIBMATTI_Platform_IsAndroid(void) { return LIBMATTI_Platform_Get() == LIBMATTI_PLATFORM_ANDROID; }
int LIBMATTI_Platform_IsDesktop(void) { return !LIBMATTI_Platform_IsAndroid(); }

int LIBMATTI_Platform_SupportsApi(const char *api)
{
    if (api == NULL)
        return 0;
    if (strcasecmp(api, "auto") == 0)
        return 1;
    if (strcasecmp(api, "opengl") == 0 || strcasecmp(api, "gl") == 0)
        return 1;
    if (strcasecmp(api, "vulkan") == 0 || strcasecmp(api, "vk") == 0)
        return LIBMATTI_Platform_IsWindows() || LIBMATTI_Platform_IsLinux() || LIBMATTI_Platform_IsAndroid();
    if (strcasecmp(api, "directx") == 0 || strcasecmp(api, "dx") == 0)
        return LIBMATTI_Platform_IsWindows();
    if (strcasecmp(api, "metal") == 0)
        return LIBMATTI_Platform_IsMacOS();
    return 0;
}

const char *LIBMATTI_Platform_DefaultRenderer(void)
{
    if (LIBMATTI_Platform_IsAndroid())
        return "native:opengl";
    if (LIBMATTI_Platform_IsWindows())
        return "sdl:directx";
    if (LIBMATTI_Platform_IsMacOS())
        return "sdl:opengl";



    return LIBMATTI_Platform_Bits() >= 64 ? "native:vulkan" : "native:opengl";
}
