// SDL3 renderer backend: SDL window + SDL_Renderer + streaming ARGB texture.
// SDL owns the platform window and the graphics API (Vulkan/OpenGL/Metal/...),
// while the client keeps drawing into the common CPU framebuffer.
//
// Unlike the first version, this is a complete integration: SDL events update
// the input state (mouse position/buttons and GLFW-numbered keys), window
// resizes and high-DPI changes are reflected by the logical presentation, and
// the streaming texture is recreated when the framebuffer size changes.

#include "libmatti/matti/render/render_internal.h"

#include "libmatti/matti/Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(MATTI_HAVE_SDL3)

#include <SDL3/SDL.h>

typedef struct LIBMATTI_SDL3Backend
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int texture_width;
    int texture_height;

    float mouse_x;
    float mouse_y;
    int mouse_buttons[3];

    int closed;
    int sdl_ready;
    int initialized;
} LIBMATTI_SDL3Backend;

static int sdl3_is_available(void)
{
    return 1;
}

static void sdl3_backend_free(LIBMATTI_SDL3Backend *backend)
{
    if (backend == NULL)
        return;
    // Destroy whatever a failed or successful init created. Partial inits are
    // common here (window ok, renderer fails, ...), so every field is checked
    // independently of the "fully initialized" flag.
    if (backend->texture != NULL)
        SDL_DestroyTexture(backend->texture);
    if (backend->renderer != NULL)
        SDL_DestroyRenderer(backend->renderer);
    if (backend->window != NULL)
        SDL_DestroyWindow(backend->window);
    if (backend->sdl_ready)
        SDL_Quit();
    free(backend);
}

static int sdl3_init(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_SDL3Backend *backend = calloc(1, sizeof(LIBMATTI_SDL3Backend));
    if (backend == NULL)
    {
        LIBMATTI_Render_SetLastError("out of memory while creating the SDL3 backend");
        return 0;
    }
    renderer->backend = backend;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        LIBMATTI_Render_SetLastError("SDL3 init failed: %s", SDL_GetError());
        sdl3_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }
    backend->sdl_ready = 1;

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    int window_width = renderer->width;
    int window_height = renderer->height;
    if (renderer->config.fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
        // SDL3's default fullscreen mode is borderless desktop fullscreen. Set
        // it explicitly so no exclusive mode (and no host resolution change)
        // is selected. The UI is rendered at that native size below.
        const SDL_DisplayMode *desktop = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
        if (desktop != NULL && desktop->w > 0 && desktop->h > 0)
        {
            window_width = desktop->w;
            window_height = desktop->h;
        }
    }

    backend->window = SDL_CreateWindow(renderer->title, window_width, window_height, flags);
    if (backend->window == NULL)
    {
        LIBMATTI_Render_SetLastError("SDL3 window creation failed: %s", SDL_GetError());
        sdl3_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    if (renderer->config.fullscreen)
    {
        SDL_SetWindowFullscreenMode(backend->window, NULL);
        if (!LIBMATTI_Render_Resize(renderer, window_width, window_height))
        {
            LIBMATTI_Render_SetLastError("could not allocate the fullscreen framebuffer");
            sdl3_backend_free(backend);
            renderer->backend = NULL;
            return 0;
        }
    }

    // The player picked an API below the SDL layer (Vulkan/OpenGL/DirectX).
    // SDL render drivers are named after the API; the first matching driver
    // wins (Direct3D 12 -> 11 -> 9 on Windows).
    const char *driver = NULL;
    switch (renderer->config.api)
    {
        case LIBMATTI_RENDER_API_VULKAN: driver = "vulkan"; break;
        case LIBMATTI_RENDER_API_OPENGL: driver = "opengl"; break;
        case LIBMATTI_RENDER_API_METAL: driver = "metal"; break;
        case LIBMATTI_RENDER_API_DIRECTX:
        {
            if (!LIBMATTI_Platform_SupportsApi("directx"))
            {
                LIBMATTI_Render_SetLastError("DirectX requires Windows (this host is %s)",
                                             LIBMATTI_Platform_Name());
                sdl3_backend_free(backend);
                renderer->backend = NULL;
                return 0;
            }
            int drivers = SDL_GetNumRenderDrivers();
            for (int i = 0; i < drivers; i++)
            {
                const char *name = SDL_GetRenderDriver(i);
                if (name == NULL || strncmp(name, "direct3d", 8) != 0)
                    continue;
                // Prefer 12 over 11 over 9 as SDL lists them.
                if (driver == NULL || strcmp(name, "direct3d12") == 0)
                    driver = name;
            }
            if (driver == NULL)
            {
                LIBMATTI_Render_SetLastError("no Direct3D SDL render driver on this Windows host");
                sdl3_backend_free(backend);
                renderer->backend = NULL;
                return 0;
            }
            break;
        }
        default: driver = NULL; break;
    }

    backend->renderer = SDL_CreateRenderer(backend->window, driver);
    if (backend->renderer == NULL)
    {
        LIBMATTI_Render_SetLastError("SDL3 renderer creation failed (driver '%s'): %s",
                                     driver != NULL ? driver : "auto", SDL_GetError());
        sdl3_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }
    fprintf(stderr, "[render] sdl layer driver=%s api=%s\n", SDL_GetRendererName(backend->renderer),
            LIBMATTI_Render_ApiName(renderer->config.api));

    SDL_SetRenderVSync(backend->renderer, renderer->config.vsync ? 1 : 0);
    SDL_SetRenderLogicalPresentation(backend->renderer, renderer->width, renderer->height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    backend->texture = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, renderer->width,
                                         renderer->height);
    if (backend->texture == NULL)
    {
        LIBMATTI_Render_SetLastError("SDL3 texture creation failed: %s", SDL_GetError());
        sdl3_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }
    SDL_SetTextureScaleMode(backend->texture, SDL_SCALEMODE_NEAREST);
    backend->texture_width = renderer->width;
    backend->texture_height = renderer->height;

    backend->initialized = 1;
    return 1;
}

static void sdl3_shutdown(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    sdl3_backend_free((LIBMATTI_SDL3Backend *) renderer->backend);
    renderer->backend = NULL;
}

static int mouse_button_index(Uint8 button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT: return 0;
        case SDL_BUTTON_RIGHT: return 1;
        case SDL_BUTTON_MIDDLE: return 2;
        default: return -1;
    }
}

// The UI uses GLFW key codes (the same values as org.lwjgl.glfw.GLFW). SDL
// reports scancodes, so translate instead of leaking a second key numbering
// into the bridge.
static SDL_Scancode glfw_to_scancode(int key)
{
    if (key >= 'A' && key <= 'Z')
        return (SDL_Scancode) (SDL_SCANCODE_A + (key - 'A'));
    if (key >= 'a' && key <= 'z')
        return (SDL_Scancode) (SDL_SCANCODE_A + (key - 'a'));
    if (key >= '0' && key <= '9')
    {
        static const SDL_Scancode digits[10] = {
            SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
            SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9,
        };
        return digits[key - '0'];
    }

    switch (key)
    {
        case 32: return SDL_SCANCODE_SPACE;
        case 39: return SDL_SCANCODE_APOSTROPHE;
        case 44: return SDL_SCANCODE_COMMA;
        case 45: return SDL_SCANCODE_MINUS;
        case 46: return SDL_SCANCODE_PERIOD;
        case 47: return SDL_SCANCODE_SLASH;
        case 59: return SDL_SCANCODE_SEMICOLON;
        case 61: return SDL_SCANCODE_EQUALS;
        case 91: return SDL_SCANCODE_LEFTBRACKET;
        case 92: return SDL_SCANCODE_BACKSLASH;
        case 93: return SDL_SCANCODE_RIGHTBRACKET;
        case 96: return SDL_SCANCODE_GRAVE;
        case 256: return SDL_SCANCODE_ESCAPE;
        case 257: return SDL_SCANCODE_RETURN;
        case 258: return SDL_SCANCODE_TAB;
        case 259: return SDL_SCANCODE_BACKSPACE;
        case 260: return SDL_SCANCODE_INSERT;
        case 261: return SDL_SCANCODE_DELETE;
        case 262: return SDL_SCANCODE_RIGHT;
        case 263: return SDL_SCANCODE_LEFT;
        case 264: return SDL_SCANCODE_DOWN;
        case 265: return SDL_SCANCODE_UP;
        case 266: return SDL_SCANCODE_PAGEUP;
        case 267: return SDL_SCANCODE_PAGEDOWN;
        case 268: return SDL_SCANCODE_HOME;
        case 269: return SDL_SCANCODE_END;
        case 280: return SDL_SCANCODE_CAPSLOCK;
        case 281: return SDL_SCANCODE_SCROLLLOCK;
        case 282: return SDL_SCANCODE_NUMLOCKCLEAR;
        case 283: return SDL_SCANCODE_PRINTSCREEN;
        case 284: return SDL_SCANCODE_PAUSE;
        case 340: return SDL_SCANCODE_LSHIFT;
        case 341: return SDL_SCANCODE_LCTRL;
        case 342: return SDL_SCANCODE_LALT;
        case 343: return SDL_SCANCODE_LGUI;
        case 344: return SDL_SCANCODE_RSHIFT;
        case 345: return SDL_SCANCODE_RCTRL;
        case 346: return SDL_SCANCODE_RALT;
        case 347: return SDL_SCANCODE_RGUI;
        case 348: return SDL_SCANCODE_APPLICATION;
        case 330: return SDL_SCANCODE_KP_PERIOD;
        case 331: return SDL_SCANCODE_KP_DIVIDE;
        case 332: return SDL_SCANCODE_KP_MULTIPLY;
        case 333: return SDL_SCANCODE_KP_MINUS;
        case 334: return SDL_SCANCODE_KP_PLUS;
        case 335: return SDL_SCANCODE_KP_ENTER;
        case 336: return SDL_SCANCODE_KP_EQUALS;
        default: break;
    }

    if (key >= 290 && key <= 314)
        return (SDL_Scancode) (SDL_SCANCODE_F1 + (key - 290));
    if (key >= 320 && key <= 329)
        return (SDL_Scancode) (SDL_SCANCODE_KP_0 + (key - 320));
    return SDL_SCANCODE_UNKNOWN;
}

static void sdl3_poll_events(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (backend == NULL)
        return;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                backend->closed = 1;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (backend->window != NULL
                    && event.window.windowID == SDL_GetWindowID(backend->window))
                    backend->closed = 1;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                backend->mouse_x = event.motion.x;
                backend->mouse_y = event.motion.y;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                int index = mouse_button_index(event.button.button);
                if (index >= 0)
                    backend->mouse_buttons[index] =
                        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0;
                backend->mouse_x = event.button.x;
                backend->mouse_y = event.button.y;
                break;
            }
            default:
                break;
        }
    }
}

static int sdl3_should_close(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_SDL3Backend *backend = renderer->backend;
    return backend == NULL || backend->closed;
}

static void sdl3_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    const LIBMATTI_SDL3Backend *backend = renderer->backend;
    int w = renderer->width;
    int h = renderer->height;
    if (backend != NULL && backend->window != NULL)
    {
        if (!SDL_GetWindowSizeInPixels(backend->window, &w, &h) || w <= 0 || h <= 0)
        {
            w = renderer->width;
            h = renderer->height;
        }
    }
    if (width != NULL) *width = w;
    if (height != NULL) *height = h;
}

static int sdl3_ensure_texture(LIBMATTI_Renderer *renderer, LIBMATTI_SDL3Backend *backend)
{
    if (backend->texture != NULL && backend->texture_width == renderer->width
        && backend->texture_height == renderer->height)
        return 1;

    if (backend->texture != NULL)
        SDL_DestroyTexture(backend->texture);
    backend->texture = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, renderer->width,
                                         renderer->height);
    if (backend->texture == NULL)
    {
        LIBMATTI_Render_SetLastError("SDL3 texture recreation failed: %s", SDL_GetError());
        return 0;
    }
    SDL_SetTextureScaleMode(backend->texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(backend->renderer, renderer->width, renderer->height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    backend->texture_width = renderer->width;
    backend->texture_height = renderer->height;
    return 1;
}

static void sdl3_present(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (backend == NULL || !backend->initialized)
        return;
    if (!sdl3_ensure_texture(renderer, backend))
        return;

    if (!SDL_UpdateTexture(backend->texture, NULL, renderer->pixels, renderer->width * 4))
    {
        LIBMATTI_Render_SetLastError("SDL3 texture upload failed: %s", SDL_GetError());
        return;
    }

    SDL_SetRenderDrawColor(backend->renderer, 0, 0, 0, 255);
    SDL_RenderClear(backend->renderer);
    SDL_RenderTexture(backend->renderer, backend->texture, NULL, NULL);
    SDL_RenderPresent(backend->renderer);
}

static void sdl3_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    const LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
    if (backend == NULL || backend->renderer == NULL)
        return;

    float logical_x = 0.0f;
    float logical_y = 0.0f;
    if (SDL_RenderCoordinatesFromWindow(backend->renderer, backend->mouse_x, backend->mouse_y,
                                        &logical_x, &logical_y))
    {
        if (x != NULL) *x = (double) logical_x;
        if (y != NULL) *y = (double) logical_y;
        return;
    }

    int window_width = 0, window_height = 0;
    SDL_GetWindowSize(backend->window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        return;
    if (x != NULL) *x = (double) backend->mouse_x * renderer->width / window_width;
    if (y != NULL) *y = (double) backend->mouse_y * renderer->height / window_height;
}

static int sdl3_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    const LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (backend == NULL || button < 0 || button > 2)
        return 0;
    return backend->mouse_buttons[button];
}

static int sdl3_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    (void) renderer;
    SDL_Scancode scancode = glfw_to_scancode(key);
    if (scancode == SDL_SCANCODE_UNKNOWN)
        return 0;
    const bool *state = SDL_GetKeyboardState(NULL);
    return state != NULL && state[scancode];
}

static void sdl3_request_close(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (backend != NULL)
        backend->closed = 1;
}

static double sdl3_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return (double) SDL_GetTicksNS() / 1000000000.0;
}

static void sdl3_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    LIBMATTI_SDL3Backend *backend = renderer->backend;
    if (backend != NULL && backend->window != NULL)
        SDL_SetWindowTitle(backend->window, title);
}

static void *sdl3_get_native_window(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_SDL3Backend *backend = renderer->backend;
    return backend != NULL ? (void *) backend->window : NULL;
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_SDL3Ops(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "sdl3",
        "SDL3 renderer",
        sdl3_is_available,
        sdl3_init,
        sdl3_shutdown,
        sdl3_poll_events,
        sdl3_should_close,
        sdl3_get_window_size,
        sdl3_present,
        sdl3_get_time,
        sdl3_set_title,
        sdl3_get_native_window,
        sdl3_get_mouse_position,
        sdl3_is_mouse_down,
        sdl3_is_key_down,
        sdl3_request_close,
    };
    return &ops;
}

#else // MATTI_HAVE_SDL3

static int sdl3_is_available(void)
{
    return 0;
}

static int sdl3_init(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    LIBMATTI_Render_SetLastError("SDL3 support was not compiled in (install SDL3 development files)");
    return 0;
}

static void sdl3_shutdown(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static void sdl3_poll_events(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static int sdl3_should_close(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return 1;
}

static void sdl3_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    if (width != NULL) *width = renderer->width;
    if (height != NULL) *height = renderer->height;
}

static void sdl3_present(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static void sdl3_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    (void) renderer;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
}

static int sdl3_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    (void) renderer;
    (void) button;
    return 0;
}

static int sdl3_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    (void) renderer;
    (void) key;
    return 0;
}

static void sdl3_request_close(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static double sdl3_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return 0.0;
}

static void sdl3_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    (void) renderer;
    (void) title;
}

static void *sdl3_get_native_window(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return NULL;
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_SDL3Ops(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "sdl3",
        "SDL3 renderer (not compiled in)",
        sdl3_is_available,
        sdl3_init,
        sdl3_shutdown,
        sdl3_poll_events,
        sdl3_should_close,
        sdl3_get_window_size,
        sdl3_present,
        sdl3_get_time,
        sdl3_set_title,
        sdl3_get_native_window,
        sdl3_get_mouse_position,
        sdl3_is_mouse_down,
        sdl3_is_key_down,
        sdl3_request_close,
    };
    return &ops;
}

#endif // MATTI_HAVE_SDL3
