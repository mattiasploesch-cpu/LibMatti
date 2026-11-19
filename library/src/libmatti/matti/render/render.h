// Common rendering interface for the Matticraft client.
//
// The game/client only talks to the functions in this header. A backend
// (OpenGL, Vulkan or SDL3) implements the small vtable in
// matti/render/render_internal.h: window creation, input, event polling and
// presenting the CPU framebuffer. Everything else - DrawPixel, FillRect,
// Clear, the pixel format - is shared and therefore works identically on
// every backend.
//
// Pixel format: one uint32_t per pixel, 0xAARRGGBB (ARGB, alpha in the high
// byte), rows top-to-bottom. The helper macros below build such colors.

#ifndef MATTICRAFT_MATTI_RENDER_RENDER_H
#define MATTICRAFT_MATTI_RENDER_RENDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// The backend that renders/presents the frame. AUTO picks the first
// available backend (OpenGL -> SDL3 -> Vulkan). MATTI_RENDERER can override
// the choice; there is deliberately no command-line renderer switch.
typedef enum LIBMATTI_RenderBackendKind
{
    LIBMATTI_RENDER_BACKEND_UNKNOWN = -1,
    LIBMATTI_RENDER_BACKEND_AUTO = 0,
    LIBMATTI_RENDER_BACKEND_OPENGL,
    LIBMATTI_RENDER_BACKEND_VULKAN,
    LIBMATTI_RENDER_BACKEND_SDL3
} LIBMATTI_RenderBackendKind;

// How the window and the graphics API are reached. NATIVE is the direct
// GLFW/OpenGL/Vulkan path (one window per API); SDL puts SDL's window and
// renderer underneath and picks the API through an SDL driver (Vulkan,
// OpenGL, Direct3D, Metal, ...). The player chooses the layer first and then
// the API inside it.
typedef enum LIBMATTI_RenderLayer
{
    LIBMATTI_RENDER_LAYER_AUTO = 0,
    LIBMATTI_RENDER_LAYER_NATIVE,
    LIBMATTI_RENDER_LAYER_SDL
} LIBMATTI_RenderLayer;

typedef enum LIBMATTI_RenderApi
{
    LIBMATTI_RENDER_API_AUTO = 0,
    LIBMATTI_RENDER_API_OPENGL,
    LIBMATTI_RENDER_API_VULKAN,
    LIBMATTI_RENDER_API_DIRECTX,
    LIBMATTI_RENDER_API_METAL
} LIBMATTI_RenderApi;

// "native:opengl", "native:vulkan", "sdl:vulkan", "sdl:opengl",
// "sdl:directx" or a single name ("opengl", "vulkan", "directx", "sdl3").
typedef struct LIBMATTI_RenderSelection
{
    LIBMATTI_RenderLayer layer;
    LIBMATTI_RenderApi api;
} LIBMATTI_RenderSelection;

typedef struct LIBMATTI_RenderConfig
{
    int width;
    int height;
    const char *title;
    int fullscreen;
    int vsync;
    // Filled from the selection; the SDL backend uses api to pick its driver.
    LIBMATTI_RenderLayer layer;
    LIBMATTI_RenderApi api;
} LIBMATTI_RenderConfig;

// Opaque renderer handle. Create with one of the Init functions, destroy with
// LIBMATTI_Render_Destroy.
typedef struct LIBMATTI_Renderer LIBMATTI_Renderer;

// 0xAARRGGBB colors.
#define LIBMATTI_RENDER_RGBA(r, g, b, a)                                                         \
    ((uint32_t) (((uint32_t) (a) & 0xFFu) << 24) | ((uint32_t) (r) & 0xFFu) << 16               \
     | ((uint32_t) (g) & 0xFFu) << 8 | ((uint32_t) (b) & 0xFFu))
#define LIBMATTI_RENDER_RGB(r, g, b) LIBMATTI_RENDER_RGBA((r), (g), (b), 255u)

// Sensible defaults: 854x480, resizable, vsync on, title "Matticraft".
LIBMATTI_RenderConfig LIBMATTI_Render_DefaultConfig(void);

// Backend name <-> enum. Names are case-insensitive; "gl" and "sdl" are
// accepted as aliases. Unknown names give LIBMATTI_RENDER_BACKEND_UNKNOWN.
LIBMATTI_RenderBackendKind LIBMATTI_Render_BackendFromName(const char *name);
const char *LIBMATTI_Render_BackendName(LIBMATTI_RenderBackendKind backend);

// Layer/API selection. ParseSelection accepts the strings above; the init
// picks the backend, checks the host (DirectX is Windows-only, on every other
// platform the SC layer reports a clear error) and lets the SDL backend pick
// the matching SDL render driver.
LIBMATTI_RenderSelection LIBMATTI_Render_DefaultSelection(void);
int LIBMATTI_Render_ParseSelection(const char *text, LIBMATTI_RenderSelection *out);
const char *LIBMATTI_Render_SelectionString(LIBMATTI_RenderSelection selection, char *buffer, int buffer_size);
const char *LIBMATTI_Render_LayerName(LIBMATTI_RenderLayer layer);
const char *LIBMATTI_Render_ApiName(LIBMATTI_RenderApi api);
LIBMATTI_Renderer *LIBMATTI_Render_InitSelection(const LIBMATTI_RenderSelection *selection,
                                                 const LIBMATTI_RenderConfig *config);
LIBMATTI_RenderLayer LIBMATTI_Render_GetLayer(const LIBMATTI_Renderer *renderer);
LIBMATTI_RenderApi LIBMATTI_Render_GetApi(const LIBMATTI_Renderer *renderer);
int LIBMATTI_Render_IsBackendAvailable(LIBMATTI_RenderBackendKind backend);
// "opengl vulkan sdl3" - the backends compiled in and usable on this host.
const char *LIBMATTI_Render_AvailableBackends(void);

// The three explicit init entry points. The public names are what a caller
// uses when it wants a specific backend; LIBMATTI_Render_InitByName accepts
// the same names plus "auto".
LIBMATTI_Renderer *LIBMATTI_Render_InitOpenGL(const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitVulkan(const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitSDL3(const LIBMATTI_RenderConfig *config);

LIBMATTI_Renderer *LIBMATTI_Render_Init(LIBMATTI_RenderBackendKind backend,
                                        const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitByName(const char *name,
                                              const LIBMATTI_RenderConfig *config);
void LIBMATTI_Render_Destroy(LIBMATTI_Renderer *renderer);

// The backend that was selected for this renderer ("opengl", "vulkan", "sdl3").
const char *LIBMATTI_Render_GetBackendName(const LIBMATTI_Renderer *renderer);
const char *LIBMATTI_Render_GetBackendDescription(const LIBMATTI_Renderer *renderer);
const char *LIBMATTI_Render_GetLastError(void);

// Logical framebuffer size (the size DrawPixel operates on). The window can
// have a different size; Present scales the framebuffer to the window.
int LIBMATTI_Render_GetWidth(const LIBMATTI_Renderer *renderer);
int LIBMATTI_Render_GetHeight(const LIBMATTI_Renderer *renderer);
void LIBMATTI_Render_GetSize(const LIBMATTI_Renderer *renderer, int *width, int *height);

// Current window size in pixels (after a resize).
void LIBMATTI_Render_GetWindowSize(const LIBMATTI_Renderer *renderer, int *width, int *height);

// Resize the logical framebuffer. The old content is preserved in the
// overlapping area; the rest is cleared to transparent black. Returns 0 on
// allocation failure (the old buffer stays usable).
int LIBMATTI_Render_Resize(LIBMATTI_Renderer *renderer, int width, int height);

// Drawing. All coordinates are in logical framebuffer pixels, origin top-left.
// Out-of-bounds pixels are ignored.
void LIBMATTI_Render_Clear(LIBMATTI_Renderer *renderer, uint32_t argb);
void LIBMATTI_Render_DrawPixel(LIBMATTI_Renderer *renderer, int x, int y, uint32_t argb);
uint32_t LIBMATTI_Render_GetPixel(const LIBMATTI_Renderer *renderer, int x, int y);
void LIBMATTI_Render_FillRect(LIBMATTI_Renderer *renderer, int x, int y, int width, int height,
                              uint32_t argb);
void LIBMATTI_Render_DrawLine(LIBMATTI_Renderer *renderer, int x0, int y0, int x1, int y1,
                              uint32_t argb);

// Upload the framebuffer to the window and swap buffers.
void LIBMATTI_Render_Present(LIBMATTI_Renderer *renderer);

// Pump the window system's event queue and update the should-close state.
void LIBMATTI_Render_PollEvents(LIBMATTI_Renderer *renderer);
int LIBMATTI_Render_ShouldClose(const LIBMATTI_Renderer *renderer);

double LIBMATTI_Render_GetTime(const LIBMATTI_Renderer *renderer);
void LIBMATTI_Render_SetTitle(LIBMATTI_Renderer *renderer, const char *title);

// Input. Coordinates are logical framebuffer pixels (the same space as
// DrawPixel), already mapped through the window scale and letterboxing.
// Keys use the GLFW key code numbering (org.lwjgl.glfw.GLFW), mouse buttons
// use 0 = left, 1 = right, 2 = middle. All functions are safe when the
// renderer has no input support.
void LIBMATTI_Render_GetMousePosition(const LIBMATTI_Renderer *renderer, double *x, double *y);
int LIBMATTI_Render_IsMouseDown(const LIBMATTI_Renderer *renderer, int button);
int LIBMATTI_Render_IsKeyDown(const LIBMATTI_Renderer *renderer, int key);

// Ask the window to close (the normal close button and the UI's Quit action
// share this path).
void LIBMATTI_Render_RequestClose(LIBMATTI_Renderer *renderer);

// The native window handle (GLFWwindow* / SDL_Window*). Backend specific and
// meant for integrations that need the real window.
void *LIBMATTI_Render_GetNativeWindow(const LIBMATTI_Renderer *renderer);

// Raw framebuffer access (0xAARRGGBB, top-to-bottom).
const uint32_t *LIBMATTI_Render_GetPixels(const LIBMATTI_Renderer *renderer);

// Debug helper: write the current framebuffer as binary PPM (P6). Returns 0
// on success.
int LIBMATTI_Render_SavePPM(const LIBMATTI_Renderer *renderer, const char *path);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MATTI_RENDER_RENDER_H
