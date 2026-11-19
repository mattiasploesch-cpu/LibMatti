// Runtime view of the platform the one library was compiled for.
//
// The library is built once per target and detects its platform itself: the
// compile-time macros select the window/audio/storage paths and the runtime
// checks below decide which rendering APIs can work here. There is no separate
// "windows build" source tree - only this file and the platform backends.

#ifndef MATTICRAFT_MATTI_PLATFORM_H
#define MATTICRAFT_MATTI_PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum LIBMATTI_PlatformKind
{
    LIBMATTI_PLATFORM_UNKNOWN = 0,
    LIBMATTI_PLATFORM_LINUX,
    LIBMATTI_PLATFORM_WINDOWS,
    LIBMATTI_PLATFORM_MACOS,
    LIBMATTI_PLATFORM_ANDROID,
    LIBMATTI_PLATFORM_FREEBSD
} LIBMATTI_PlatformKind;

LIBMATTI_PlatformKind LIBMATTI_Platform_Get(void);
const char *LIBMATTI_Platform_Name(void);
// "x86_64", "x86", "arm64", "arm", "riscv64", "unknown".
const char *LIBMATTI_Platform_Arch(void);
// Pointer width: 32 or 64. Detected at runtime, no build flag needed.
int LIBMATTI_Platform_Bits(void);
int LIBMATTI_Platform_IsLittleEndian(void);

int LIBMATTI_Platform_IsWindows(void);
int LIBMATTI_Platform_IsMacOS(void);
int LIBMATTI_Platform_IsLinux(void);
int LIBMATTI_Platform_IsAndroid(void);
int LIBMATTI_Platform_IsDesktop(void);

// "opengl", "vulkan", "directx", "metal"; 1 when the API is usable here.
int LIBMATTI_Platform_SupportsApi(const char *api);
// The preferred selection on this platform ("native:vulkan", "sdl:vulkan",
// "sdl:directx", ...).
const char *LIBMATTI_Platform_DefaultRenderer(void);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MATTI_PLATFORM_H
