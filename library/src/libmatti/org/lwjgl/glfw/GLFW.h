// Port of org.lwjgl.glfw.GLFW (the constants and functions DisplayWindow uses).
// LWJGL binds the native libglfw; the port dlopen()s it exactly like the tinyfd
// binding. Every symbol here has the name and signature of the GLFW C API.

#ifndef MATTICRAFT_LWJGL_GLFW_GLFW_H
#define MATTICRAFT_LWJGL_GLFW_GLFW_H

#include <stddef.h>

// Java: public static final int ... (values from org.lwjgl.glfw.GLFW)
#define LIBMATTI_GLFW_FALSE 0
#define LIBMATTI_GLFW_TRUE 1
#define LIBMATTI_GLFW_NO_ERROR 0
#define LIBMATTI_GLFW_CLIENT_API 0x00022001
#define LIBMATTI_GLFW_CONTEXT_CREATION_API 0x00022004
#define LIBMATTI_GLFW_CONTEXT_VERSION_MAJOR 0x00022002
#define LIBMATTI_GLFW_CONTEXT_VERSION_MINOR 0x00022003
#define LIBMATTI_GLFW_OPENGL_API 0x00030001
#define LIBMATTI_GLFW_NATIVE_CONTEXT_API 0x00036001
#define LIBMATTI_GLFW_OPENGL_PROFILE 0x00022008
#define LIBMATTI_GLFW_OPENGL_CORE_PROFILE 0x00032001
#define LIBMATTI_GLFW_OPENGL_FORWARD_COMPAT 0x00022006
#define LIBMATTI_GLFW_OPENGL_DEBUG_CONTEXT 0x00022007
#define LIBMATTI_GLFW_SOFT_FULLSCREEN 0x00020010
#define LIBMATTI_GLFW_VISIBLE 0x00020004
#define LIBMATTI_GLFW_RESIZABLE 0x00020003
#define LIBMATTI_GLFW_PLATFORM 0x00050003
#define LIBMATTI_GLFW_PLATFORM_COCOA 0x00060002
#define LIBMATTI_GLFW_PLATFORM_WAYLAND 0x00060003
#define LIBMATTI_GLFW_PLATFORM_X11 0x00060004
#define LIBMATTI_GLFW_X11_CLASS_NAME 0x00024001
#define LIBMATTI_GLFW_X11_INSTANCE_NAME 0x00024002
#define LIBMATTI_GLFW_CURSOR 0x00033001
#define LIBMATTI_GLFW_CURSOR_NORMAL 0x00034001
#define LIBMATTI_GLFW_CURSOR_HIDDEN 0x00034002
#define LIBMATTI_GLFW_CURSOR_DISABLED 0x00034003
#define LIBMATTI_GLFW_KEY_SPACE 32
#define LIBMATTI_GLFW_KEY_A 65
#define LIBMATTI_GLFW_KEY_S 83
#define LIBMATTI_GLFW_KEY_D 68
#define LIBMATTI_GLFW_KEY_W 87
#define LIBMATTI_GLFW_KEY_ESCAPE 256
// the GLFW key codes the vanilla KeyMappings bind (Java: the Options defaults)
#define LIBMATTI_GLFW_KEY_LEFT_SHIFT 340
#define LIBMATTI_GLFW_KEY_LEFT_CONTROL 341
#define LIBMATTI_GLFW_KEY_E 69
#define LIBMATTI_GLFW_KEY_Q 81
#define LIBMATTI_GLFW_KEY_T 84
#define LIBMATTI_GLFW_KEY_1 49
#define LIBMATTI_GLFW_KEY_2 50
#define LIBMATTI_GLFW_KEY_3 51
#define LIBMATTI_GLFW_KEY_4 52
#define LIBMATTI_GLFW_KEY_5 53
#define LIBMATTI_GLFW_KEY_6 54
#define LIBMATTI_GLFW_KEY_7 55
#define LIBMATTI_GLFW_KEY_8 56
#define LIBMATTI_GLFW_KEY_9 57
// the GLFW mouse-button codes (Java: InputConstants.Type.MOUSE)
#define LIBMATTI_GLFW_MOUSE_BUTTON_1 0
#define LIBMATTI_GLFW_MOUSE_BUTTON_2 1
#define LIBMATTI_GLFW_MOUSE_BUTTON_3 2
#define LIBMATTI_GLFW_PRESS 1
#define LIBMATTI_GLFW_RELEASE 0

// Java: public static boolean glfwInit()
int LIBMATTI_GLFW_glfwInit(void);
// Java: public static void glfwInitHint(int hint, int value)
void LIBMATTI_GLFW_glfwInitHint(int hint, int value);
// Java: public static int glfwGetPlatform()
int LIBMATTI_GLFW_glfwGetPlatform(void);
// Java: public static boolean glfwPlatformSupported(int platform)
int LIBMATTI_GLFW_glfwPlatformSupported(int platform);
// Java: public static void glfwDefaultWindowHints()
void LIBMATTI_GLFW_glfwDefaultWindowHints(void);
// Java: public static void glfwWindowHint(int hint, int value)
void LIBMATTI_GLFW_glfwWindowHint(int hint, int value);
// Java: public static void glfwWindowHintString(int hint, String value)
void LIBMATTI_GLFW_glfwWindowHintString(int hint, const char *value);
// Java: public static long glfwCreateWindow(int width, int height, String title, long monitor, long share)
long LIBMATTI_GLFW_glfwCreateWindow(int width, int height, const char *title, long monitor, long share);
// Java: public static void glfwDestroyWindow(long window)
void LIBMATTI_GLFW_glfwDestroyWindow(long window);
// Java: public static long glfwGetPrimaryMonitor()
long LIBMATTI_GLFW_glfwGetPrimaryMonitor(void);
// Java: public static GLFWVidMode glfwGetVideoMode(long monitor) - fills w/h, 0 = null
int LIBMATTI_GLFW_glfwGetVideoMode(long monitor, int *width, int *height, int *redBits, int *greenBits, int *blueBits, int *refreshRate);
// Java: public static void glfwGetMonitorPos(long monitor, int[] xpos, int[] ypos)
void LIBMATTI_GLFW_glfwGetMonitorPos(long monitor, int *xpos, int *ypos);
// Java: public static void glfwGetWindowSize(long window, int[] width, int[] height)
void LIBMATTI_GLFW_glfwGetWindowSize(long window, int *width, int *height);
// Java: public static void glfwSetWindowSizeCallback(long window, GLFWWindowSizeCallbackI cb) - NULL cb unsets
typedef void (*LIBMATTI_GLFW_WindowSizeCallback)(long window, int width, int height);
void LIBMATTI_GLFW_glfwSetWindowSizeCallback(long window, LIBMATTI_GLFW_WindowSizeCallback callback);
// Java: public static void glfwMaximizeWindow(long window)
void LIBMATTI_GLFW_glfwMaximizeWindow(long window);
// Java: public static void glfwSetWindowPos(long window, int xpos, int ypos)
void LIBMATTI_GLFW_glfwSetWindowPos(long window, int xpos, int ypos);
// Java: public static void glfwGetWindowPos(long window, int[] xpos, int[] ypos)
void LIBMATTI_GLFW_glfwGetWindowPos(long window, int *xpos, int *ypos);
// Java: public static void glfwSetWindowTitle(long window, String title)
void LIBMATTI_GLFW_glfwSetWindowTitle(long window, const char *title);
// Java: public static void glfwShowWindow(long window)
void LIBMATTI_GLFW_glfwShowWindow(long window);
// Java: public static void glfwPollEvents()
void LIBMATTI_GLFW_glfwPollEvents(void);
// Java: public static void glfwMakeContextCurrent(long window)
void LIBMATTI_GLFW_glfwMakeContextCurrent(long window);

long LIBMATTI_GLFW_glfwGetCurrentContext(void);
// Java: public static void glfwSwapInterval(int interval)
void LIBMATTI_GLFW_glfwSwapInterval(int interval);
// Java: public static void glfwWaitEventsTimeout(double timeout)
void LIBMATTI_GLFW_glfwWaitEventsTimeout(double timeout);
// Java: public static void glfwPostEmptyEvent(void)
void LIBMATTI_GLFW_glfwPostEmptyEvent(void);
// Java: public static void glfwSwapBuffers(long window)
void LIBMATTI_GLFW_glfwSwapBuffers(long window);
// Java: public static int glfwGetError(String[] description) - description may be NULL
int LIBMATTI_GLFW_glfwGetError(const char **description);
// Java: public static int glfwWindowShouldClose(long window)
int LIBMATTI_GLFW_glfwWindowShouldClose(long window);
// Java: public static int glfwGetKey(long window, int key)
int LIBMATTI_GLFW_glfwGetKey(long window, int key);
// Java: public static void glfwSetInputMode(long window, int mode, int value)
void LIBMATTI_GLFW_glfwSetInputMode(long window, int mode, int value);
// Java: public static void glfwGetCursorPos(long window, double[] xpos, double[] ypos)
void LIBMATTI_GLFW_glfwGetCursorPos(long window, double *xpos, double *ypos);

void LIBMATTI_GLFW_glfwTerminate(void);

void LIBMATTI_GLFW_glfwSetWindowShouldClose(long window, int value);

void LIBMATTI_GLFW_glfwGetFramebufferSize(long window, int *width, int *height);

void LIBMATTI_GLFW_glfwFocusWindow(long window);

double LIBMATTI_GLFW_glfwGetTime(void);

#endif //MATTICRAFT_LWJGL_GLFW_GLFW_H
