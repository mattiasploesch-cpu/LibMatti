// Internal vtable shared by the rendering backends. The game only includes
// render.h; the client_opengl/client_vulkan/client_sdl files implement this
// interface and expose their ops through the three *Ops() functions below.

#ifndef MATTICRAFT_MATTI_RENDER_RENDER_INTERNAL_H
#define MATTICRAFT_MATTI_RENDER_RENDER_INTERNAL_H

#include "libmatti/matti/render/render.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct LIBMATTI_RenderBackendOps
{
    // "opengl", "vulkan", "sdl3"
    const char *name;
    // Human readable API/driver description for logs.
    const char *description;

    // 1 when the backend was compiled in and its runtime libraries are
    // available. Init may still fail (no display, no device, ...).
    int (*is_available)(void);

    // Create the native window/context. renderer->backend is free for the
    // backend's own state. Return 1 on success, 0 on failure (set an error
    // through LIBMATTI_Render_SetLastError).
    int (*init)(LIBMATTI_Renderer *renderer);

    // Release everything init acquired (also called for partial inits).
    void (*shutdown)(LIBMATTI_Renderer *renderer);

    void (*poll_events)(LIBMATTI_Renderer *renderer);
    int (*should_close)(const LIBMATTI_Renderer *renderer);
    void (*get_window_size)(const LIBMATTI_Renderer *renderer, int *width, int *height);

    // Upload renderer->pixels (renderer->width x renderer->height) and
    // present it in the window.
    void (*present)(LIBMATTI_Renderer *renderer);

    double (*get_time)(const LIBMATTI_Renderer *renderer);
    void (*set_title)(LIBMATTI_Renderer *renderer, const char *title);
    void *(*get_native_window)(const LIBMATTI_Renderer *renderer);

    // Input in logical framebuffer pixels (0..renderer->width/height), so the
    // UI does not have to know whether the backend uses GLFW or SDL. Keys are
    // GLFW key codes, mouse buttons are the GLFW numbering (0 left, 1 right,
    // 2 middle); client_sdl.c translates SDL events to those values.
    void (*get_mouse_position)(const LIBMATTI_Renderer *renderer, double *x, double *y);
    int (*is_mouse_down)(const LIBMATTI_Renderer *renderer, int button);
    int (*is_key_down)(const LIBMATTI_Renderer *renderer, int key);
    void (*request_close)(LIBMATTI_Renderer *renderer);
} LIBMATTI_RenderBackendOps;

struct LIBMATTI_Renderer
{
    const LIBMATTI_RenderBackendOps *ops;
    void *backend;
    LIBMATTI_RenderConfig config;
    char *title;
    uint32_t *pixels;
    int width;
    int height;
};

// Backend registrations (one per client_*.c file).
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_OpenGLOps(void);
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_VulkanOps(void);
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_SDL3Ops(void);

// printf-style error reporting; read back through LIBMATTI_Render_GetLastError.
void LIBMATTI_Render_SetLastError(const char *format, ...);

// Common allocation helpers used by render.c.
LIBMATTI_Renderer *LIBMATTI_Render_Alloc(const LIBMATTI_RenderBackendOps *ops,
                                         const LIBMATTI_RenderConfig *config);
void LIBMATTI_Render_Free(LIBMATTI_Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MATTI_RENDER_RENDER_INTERNAL_H
