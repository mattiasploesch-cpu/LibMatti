// Port of org.lwjgl.glfw.GLFW. The binding resolves the native libglfw at
// runtime with dlopen, exactly like the tinyfd binding does for tinyfiledialogs.
// A missing or unloaded library means "not available" instead of a hard link error.

#include "libmatti/org/lwjgl/glfw/GLFW.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

// Java: GLFW is a class of static natives; the port keeps one set of function pointers.
static void *glfw_lib = NULL;

#define GLFW_DLSYM(field, name)                                                                  \
    do                                                                                           \
    {                                                                                            \
        if (glfw_##field == NULL) glfw_##field = dlsym(glfw_lib, name);                          \
    } while (0)

static int (*glfw_init)(void) = NULL;
static void (*glfw_init_hint)(int, int) = NULL;
static int (*glfw_get_platform)(void) = NULL;
static int (*glfw_platform_supported)(int) = NULL;
static void (*glfw_default_window_hints)(void) = NULL;
static void (*glfw_window_hint)(int, int) = NULL;
static void (*glfw_window_hint_string)(int, const char *) = NULL;
static unsigned long (*glfw_create_window)(int, int, const char *, unsigned long, unsigned long) = NULL;
static void (*glfw_destroy_window)(unsigned long) = NULL;
static unsigned long (*glfw_get_primary_monitor)(void) = NULL;
static const struct GLFWvidmode *(*glfw_get_video_mode)(unsigned long) = NULL;
static void (*glfw_get_monitor_pos)(unsigned long, int *, int *) = NULL;
static void (*glfw_get_window_size)(unsigned long, int *, int *) = NULL;
static unsigned long (*glfw_set_window_size_callback)(unsigned long, void *) = NULL;
static void (*glfw_maximize_window)(unsigned long) = NULL;
static void (*glfw_set_window_pos)(unsigned long, int, int) = NULL;
static void (*glfw_get_window_pos)(unsigned long, int *, int *) = NULL;
static void (*glfw_set_window_title)(unsigned long, const char *) = NULL;
static void (*glfw_show_window)(unsigned long) = NULL;
static void (*glfw_poll_events)(void) = NULL;
static void (*glfw_make_context_current)(unsigned long) = NULL;
static unsigned long (*glfw_get_current_context)(void) = NULL;
static void (*glfw_swap_interval)(int) = NULL;
static void (*glfw_swap_buffers)(unsigned long) = NULL;
static int (*glfw_get_error)(const char **) = NULL;
static int (*glfw_window_should_close)(unsigned long) = NULL;
static int (*glfw_get_mouse_button)(unsigned long, int) = NULL;
static int (*glfw_get_key)(unsigned long, int) = NULL;
static void (*glfw_set_input_mode)(unsigned long, int, int) = NULL;
static void (*glfw_get_cursor_pos)(unsigned long, double *, double *) = NULL;
static void (*glfw_terminate)(void) = NULL;
static void (*glfw_set_window_should_close)(unsigned long, int) = NULL;
static void (*glfw_get_framebuffer_size)(unsigned long, int *, int *) = NULL;
static void (*glfw_focus_window)(unsigned long) = NULL;
static double (*glfw_get_time)(void) = NULL;
static void (*glfw_wait_events_timeout)(double) = NULL;
static void (*glfw_post_empty_event)(void) = NULL;

// Java: GLFWVidMode is a struct view over the native pointer (GLFWvidmode:
// width, height, redBits, greenBits, blueBits, refreshRate).
struct GLFWvidmode
{
    int width;
    int height;
    int redBits;
    int greenBits;
    int blueBits;
    int refreshRate;
};

// Java: the native libglfw. GLFW 3 ships as libglfw.so.3 on Linux.
static void ensure_library(void)
{
    if (glfw_lib != NULL) return;

    const char *names[] = {"libglfw.so.3", "libglfw.so", NULL};
    for (int i = 0; names[i] != NULL; i++)
    {
        glfw_lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
        if (glfw_lib != NULL) break;
    }
}

int LIBMATTI_GLFW_glfwInit(void)
{
    ensure_library();
    if (glfw_lib == NULL) return LIBMATTI_GLFW_FALSE;
    GLFW_DLSYM(init, "glfwInit");
    if (glfw_init == NULL) return LIBMATTI_GLFW_FALSE;
    return glfw_init();
}

void LIBMATTI_GLFW_glfwInitHint(int hint, int value)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(init_hint, "glfwInitHint");
    if (glfw_init_hint != NULL) glfw_init_hint(hint, value);
}

int LIBMATTI_GLFW_glfwGetPlatform(void)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_platform, "glfwGetPlatform");
    if (glfw_get_platform == NULL) return 0;
    return glfw_get_platform();
}

int LIBMATTI_GLFW_glfwPlatformSupported(int platform)
{
    ensure_library();
    if (glfw_lib == NULL) return LIBMATTI_GLFW_FALSE;
    GLFW_DLSYM(platform_supported, "glfwPlatformSupported");
    if (glfw_platform_supported == NULL) return LIBMATTI_GLFW_FALSE;
    return glfw_platform_supported(platform);
}

void LIBMATTI_GLFW_glfwDefaultWindowHints(void)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(default_window_hints, "glfwDefaultWindowHints");
    if (glfw_default_window_hints != NULL) glfw_default_window_hints();
}

void LIBMATTI_GLFW_glfwWindowHint(int hint, int value)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(window_hint, "glfwWindowHint");
    if (glfw_window_hint != NULL) glfw_window_hint(hint, value);
}

void LIBMATTI_GLFW_glfwWindowHintString(int hint, const char *value)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(window_hint_string, "glfwWindowHintString");
    if (glfw_window_hint_string != NULL) glfw_window_hint_string(hint, value);
}

long LIBMATTI_GLFW_glfwCreateWindow(int width, int height, const char *title, long monitor, long share)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(create_window, "glfwCreateWindow");
    if (glfw_create_window == NULL) return 0;
    return (long) glfw_create_window(width, height, title, (unsigned long) monitor, (unsigned long) share);
}

void LIBMATTI_GLFW_glfwDestroyWindow(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(destroy_window, "glfwDestroyWindow");
    if (glfw_destroy_window != NULL) glfw_destroy_window((unsigned long) window);
}

long LIBMATTI_GLFW_glfwGetPrimaryMonitor(void)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_primary_monitor, "glfwGetPrimaryMonitor");
    if (glfw_get_primary_monitor == NULL) return 0;
    return (long) glfw_get_primary_monitor();
}

int LIBMATTI_GLFW_glfwGetVideoMode(long monitor, int *width, int *height, int *redBits, int *greenBits,
                                   int *blueBits, int *refreshRate)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_video_mode, "glfwGetVideoMode");
    if (glfw_get_video_mode == NULL) return 0;
    const struct GLFWvidmode *mode = glfw_get_video_mode((unsigned long) monitor);
    if (mode == NULL) return 0;
    if (width != NULL) *width = mode->width;
    if (height != NULL) *height = mode->height;
    if (redBits != NULL) *redBits = mode->redBits;
    if (greenBits != NULL) *greenBits = mode->greenBits;
    if (blueBits != NULL) *blueBits = mode->blueBits;
    if (refreshRate != NULL) *refreshRate = mode->refreshRate;
    return 1;
}

void LIBMATTI_GLFW_glfwGetMonitorPos(long monitor, int *xpos, int *ypos)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(get_monitor_pos, "glfwGetMonitorPos");
    if (glfw_get_monitor_pos != NULL) glfw_get_monitor_pos((unsigned long) monitor, xpos, ypos);
}

void LIBMATTI_GLFW_glfwGetWindowSize(long window, int *width, int *height)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(get_window_size, "glfwGetWindowSize");
    if (glfw_get_window_size != NULL) glfw_get_window_size((unsigned long) window, width, height);
}

void LIBMATTI_GLFW_glfwSetWindowSizeCallback(long window, LIBMATTI_GLFW_WindowSizeCallback callback)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(set_window_size_callback, "glfwSetWindowSizeCallback");
    if (glfw_set_window_size_callback == NULL) return;
    // The native callback signature matches the port's callback signature.
    glfw_set_window_size_callback((unsigned long) window, (void *) callback);
}

void LIBMATTI_GLFW_glfwMaximizeWindow(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(maximize_window, "glfwMaximizeWindow");
    if (glfw_maximize_window != NULL) glfw_maximize_window((unsigned long) window);
}

void LIBMATTI_GLFW_glfwSetWindowPos(long window, int xpos, int ypos)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(set_window_pos, "glfwSetWindowPos");
    if (glfw_set_window_pos != NULL) glfw_set_window_pos((unsigned long) window, xpos, ypos);
}

void LIBMATTI_GLFW_glfwShowWindow(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(show_window, "glfwShowWindow");
    if (glfw_show_window != NULL) glfw_show_window((unsigned long) window);
}

void LIBMATTI_GLFW_glfwGetWindowPos(long window, int *xpos, int *ypos)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(get_window_pos, "glfwGetWindowPos");
    if (glfw_get_window_pos != NULL) glfw_get_window_pos((unsigned long) window, xpos, ypos);
}

void LIBMATTI_GLFW_glfwSetWindowTitle(long window, const char *title)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(set_window_title, "glfwSetWindowTitle");
    if (glfw_set_window_title != NULL) glfw_set_window_title((unsigned long) window, title);
}

void LIBMATTI_GLFW_glfwPollEvents(void)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(poll_events, "glfwPollEvents");
    if (glfw_poll_events != NULL) glfw_poll_events();
}

void LIBMATTI_GLFW_glfwMakeContextCurrent(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(make_context_current, "glfwMakeContextCurrent");
    if (glfw_make_context_current != NULL) glfw_make_context_current((unsigned long) window);
}

void LIBMATTI_GLFW_glfwSwapInterval(int interval)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(swap_interval, "glfwSwapInterval");
    if (glfw_swap_interval != NULL) glfw_swap_interval(interval);
}

void LIBMATTI_GLFW_glfwWaitEventsTimeout(double timeout)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(wait_events_timeout, "glfwWaitEventsTimeout");
    if (glfw_wait_events_timeout != NULL) glfw_wait_events_timeout(timeout);
}

void LIBMATTI_GLFW_glfwPostEmptyEvent(void)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(post_empty_event, "glfwPostEmptyEvent");
    if (glfw_post_empty_event != NULL) glfw_post_empty_event();
}

void LIBMATTI_GLFW_glfwSwapBuffers(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(swap_buffers, "glfwSwapBuffers");
    if (glfw_swap_buffers != NULL) glfw_swap_buffers((unsigned long) window);
}

int LIBMATTI_GLFW_glfwGetError(const char **description)
{
    ensure_library();
    if (glfw_lib == NULL) return LIBMATTI_GLFW_NO_ERROR;
    GLFW_DLSYM(get_error, "glfwGetError");
    if (glfw_get_error == NULL) return LIBMATTI_GLFW_NO_ERROR;
    return glfw_get_error(description);
}

int LIBMATTI_GLFW_glfwWindowShouldClose(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return LIBMATTI_GLFW_TRUE;
    GLFW_DLSYM(window_should_close, "glfwWindowShouldClose");
    if (glfw_window_should_close == NULL) return LIBMATTI_GLFW_TRUE;
    return glfw_window_should_close((unsigned long) window);
}

int LIBMATTI_GLFW_glfwGetKey(long window, int key)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_key, "glfwGetKey");
    if (glfw_get_key == NULL) return 0;
    return glfw_get_key((unsigned long) window, key);
}

int LIBMATTI_GLFW_glfwGetMouseButton(long window, int button)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_mouse_button, "glfwGetMouseButton");
    if (glfw_get_mouse_button == NULL) return 0;
    return glfw_get_mouse_button((unsigned long) window, button);
}

void LIBMATTI_GLFW_glfwSetInputMode(long window, int mode, int value)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(set_input_mode, "glfwSetInputMode");
    if (glfw_set_input_mode != NULL)
        glfw_set_input_mode((unsigned long) window, mode, value);
}

void LIBMATTI_GLFW_glfwGetCursorPos(long window, double *xpos, double *ypos)
{
    // A dead binding or window leaves the cursor at (0, 0) - the polling
    // caller treats that as "no movement".
    if (xpos != NULL) *xpos = 0.0;
    if (ypos != NULL) *ypos = 0.0;
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(get_cursor_pos, "glfwGetCursorPos");
    if (glfw_get_cursor_pos != NULL)
        glfw_get_cursor_pos((unsigned long) window, xpos, ypos);
}

void LIBMATTI_GLFW_glfwTerminate(void)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(terminate, "glfwTerminate");
    if (glfw_terminate != NULL) glfw_terminate();
}

void LIBMATTI_GLFW_glfwSetWindowShouldClose(long window, int value)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(set_window_should_close, "glfwSetWindowShouldClose");
    if (glfw_set_window_should_close != NULL) glfw_set_window_should_close((unsigned long) window, value);
}

void LIBMATTI_GLFW_glfwGetFramebufferSize(long window, int *width, int *height)
{
    ensure_library();
    if (glfw_lib == NULL)
    {
        if (width != NULL) *width = 0;
        if (height != NULL) *height = 0;
        return;
    }
    GLFW_DLSYM(get_framebuffer_size, "glfwGetFramebufferSize");
    if (glfw_get_framebuffer_size != NULL) glfw_get_framebuffer_size((unsigned long) window, width, height);
}

void LIBMATTI_GLFW_glfwFocusWindow(long window)
{
    ensure_library();
    if (glfw_lib == NULL) return;
    GLFW_DLSYM(focus_window, "glfwFocusWindow");
    if (glfw_focus_window != NULL) glfw_focus_window((unsigned long) window);
}

double LIBMATTI_GLFW_glfwGetTime(void)
{
    ensure_library();
    if (glfw_lib == NULL) return 0.0;
    GLFW_DLSYM(get_time, "glfwGetTime");
    if (glfw_get_time == NULL) return 0.0;
    return glfw_get_time();
}

long LIBMATTI_GLFW_glfwGetCurrentContext(void)
{
    ensure_library();
    if (glfw_lib == NULL) return 0;
    GLFW_DLSYM(get_current_context, "glfwGetCurrentContext");
    if (glfw_get_current_context == NULL) return 0;
    return (long) glfw_get_current_context();
}
