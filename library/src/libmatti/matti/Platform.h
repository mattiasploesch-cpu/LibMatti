







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


const char *LIBMATTI_Platform_Arch(void);


int LIBMATTI_Platform_Bits(void);
int LIBMATTI_Platform_IsLittleEndian(void);

int LIBMATTI_Platform_IsWindows(void);
int LIBMATTI_Platform_IsMacOS(void);
int LIBMATTI_Platform_IsLinux(void);
int LIBMATTI_Platform_IsAndroid(void);
int LIBMATTI_Platform_IsDesktop(void);



int LIBMATTI_Platform_SupportsApi(const char *api);



const char *LIBMATTI_Platform_DefaultRenderer(void);

#ifdef __cplusplus
}
#endif

#endif
