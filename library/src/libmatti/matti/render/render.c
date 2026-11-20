



#include "libmatti/matti/render/render_internal.h"

#include "libmatti/matti/Platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>



static char last_error[512];

void LIBMATTI_Render_SetLastError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(last_error, sizeof(last_error), format, args);
    va_end(args);
}

const char *LIBMATTI_Render_GetLastError(void)
{
    return last_error[0] != '\0' ? last_error : "no error";
}

LIBMATTI_RenderConfig LIBMATTI_Render_DefaultConfig(void)
{
    LIBMATTI_RenderConfig config;
    config.width = 854;
    config.height = 480;
    config.title = "Matticraft";
    config.fullscreen = 0;
    config.vsync = 1;
    config.layer = LIBMATTI_RENDER_LAYER_AUTO;
    config.api = LIBMATTI_RENDER_API_AUTO;
    return config;
}

LIBMATTI_RenderSelection LIBMATTI_Render_DefaultSelection(void)
{
    LIBMATTI_RenderSelection selection;
    selection.layer = LIBMATTI_RENDER_LAYER_AUTO;
    selection.api = LIBMATTI_RENDER_API_AUTO;
    return selection;
}

const char *LIBMATTI_Render_LayerName(LIBMATTI_RenderLayer layer)
{
    switch (layer)
    {
        case LIBMATTI_RENDER_LAYER_NATIVE: return "native";
        case LIBMATTI_RENDER_LAYER_SDL: return "sdl";
        default: return "auto";
    }
}

const char *LIBMATTI_Render_ApiName(LIBMATTI_RenderApi api)
{
    switch (api)
    {
        case LIBMATTI_RENDER_API_OPENGL: return "opengl";
        case LIBMATTI_RENDER_API_VULKAN: return "vulkan";
        case LIBMATTI_RENDER_API_DIRECTX: return "directx";
        case LIBMATTI_RENDER_API_METAL: return "metal";
        default: return "auto";
    }
}

const char *LIBMATTI_Render_SelectionString(LIBMATTI_RenderSelection selection, char *buffer, int buffer_size)
{
    if (buffer == NULL || buffer_size <= 0)
        return "";
    snprintf(buffer, (size_t) buffer_size, "%s:%s", LIBMATTI_Render_LayerName(selection.layer),
             LIBMATTI_Render_ApiName(selection.api));
    return buffer;
}

static int parse_layer_token(const char *token, LIBMATTI_RenderLayer *out)
{
    if (strcasecmp(token, "native") == 0 || strcasecmp(token, "pure") == 0)
        *out = LIBMATTI_RENDER_LAYER_NATIVE;
    else if (strcasecmp(token, "sdl") == 0 || strcasecmp(token, "sdl3") == 0)
        *out = LIBMATTI_RENDER_LAYER_SDL;
    else if (strcasecmp(token, "auto") == 0)
        *out = LIBMATTI_RENDER_LAYER_AUTO;
    else
        return 0;
    return 1;
}

static int parse_api_token(const char *token, LIBMATTI_RenderApi *out)
{
    if (strcasecmp(token, "opengl") == 0 || strcasecmp(token, "gl") == 0)
        *out = LIBMATTI_RENDER_API_OPENGL;
    else if (strcasecmp(token, "vulkan") == 0 || strcasecmp(token, "vk") == 0)
        *out = LIBMATTI_RENDER_API_VULKAN;
    else if (strcasecmp(token, "directx") == 0 || strcasecmp(token, "dx") == 0
             || strcasecmp(token, "d3d") == 0)
        *out = LIBMATTI_RENDER_API_DIRECTX;
    else if (strcasecmp(token, "metal") == 0)
        *out = LIBMATTI_RENDER_API_METAL;
    else if (strcasecmp(token, "auto") == 0)
        *out = LIBMATTI_RENDER_API_AUTO;
    else
        return 0;
    return 1;
}

int LIBMATTI_Render_ParseSelection(const char *text, LIBMATTI_RenderSelection *out)
{
    if (out == NULL)
        return 0;
    *out = LIBMATTI_Render_DefaultSelection();
    if (text == NULL || text[0] == '\0' || strcasecmp(text, "auto") == 0)
        return 1;

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s", text);
    char *save = NULL;
    char *first = strtok_r(buffer, ":", &save);
    if (first == NULL)
        return 0;

    LIBMATTI_RenderLayer layer;
    LIBMATTI_RenderApi api;
    if (parse_layer_token(first, &layer))
    {
        out->layer = layer;
        char *second = strtok_r(NULL, ":", &save);
        if (second != NULL && !parse_api_token(second, &out->api))
            return 0;
    }
    else if (parse_api_token(first, &api))
    {
        out->api = api;



        out->layer = (api == LIBMATTI_RENDER_API_DIRECTX || api == LIBMATTI_RENDER_API_METAL)
                         ? LIBMATTI_RENDER_LAYER_SDL
                         : LIBMATTI_RENDER_LAYER_NATIVE;
    }
    else
    {
        return 0;
    }

    return 1;
}

LIBMATTI_RenderBackendKind LIBMATTI_Render_BackendFromName(const char *name)
{
    if (name == NULL || name[0] == '\0' || strcasecmp(name, "auto") == 0)
        return LIBMATTI_RENDER_BACKEND_AUTO;
    if (strcasecmp(name, "android") == 0)
        return LIBMATTI_RENDER_BACKEND_ANDROID;
    if (strcasecmp(name, "opengl") == 0 || strcasecmp(name, "gl") == 0)
        return LIBMATTI_RENDER_BACKEND_OPENGL;
    if (strcasecmp(name, "vulkan") == 0 || strcasecmp(name, "vk") == 0)
        return LIBMATTI_RENDER_BACKEND_VULKAN;
    if (strcasecmp(name, "sdl3") == 0 || strcasecmp(name, "sdl") == 0)
        return LIBMATTI_RENDER_BACKEND_SDL3;
    return LIBMATTI_RENDER_BACKEND_UNKNOWN;
}

const char *LIBMATTI_Render_BackendName(LIBMATTI_RenderBackendKind backend)
{
    switch (backend)
    {
        case LIBMATTI_RENDER_BACKEND_AUTO: return "auto";
        case LIBMATTI_RENDER_BACKEND_ANDROID: return "android";
        case LIBMATTI_RENDER_BACKEND_OPENGL: return "opengl";
        case LIBMATTI_RENDER_BACKEND_VULKAN: return "vulkan";
        case LIBMATTI_RENDER_BACKEND_SDL3: return "sdl3";
        default: return "unknown";
    }
}

static const LIBMATTI_RenderBackendOps *ops_for(LIBMATTI_RenderBackendKind backend)
{
    switch (backend)
    {
        case LIBMATTI_RENDER_BACKEND_ANDROID: return LIBMATTI_Render_AndroidOps();
        case LIBMATTI_RENDER_BACKEND_OPENGL: return LIBMATTI_Render_OpenGLOps();
        case LIBMATTI_RENDER_BACKEND_VULKAN: return LIBMATTI_Render_VulkanOps();
        case LIBMATTI_RENDER_BACKEND_SDL3: return LIBMATTI_Render_SDL3Ops();
        default: return NULL;
    }
}

int LIBMATTI_Render_IsBackendAvailable(LIBMATTI_RenderBackendKind backend)
{
    const LIBMATTI_RenderBackendOps *ops = ops_for(backend);
    if (ops == NULL || ops->is_available == NULL)
        return 0;
    return ops->is_available();
}

const char *LIBMATTI_Render_AvailableBackends(void)
{
    static char available[96];
    available[0] = '\0';
    const LIBMATTI_RenderBackendKind order[] = {LIBMATTI_RENDER_BACKEND_ANDROID,
                                                 LIBMATTI_RENDER_BACKEND_OPENGL,
                                                 LIBMATTI_RENDER_BACKEND_VULKAN,
                                                 LIBMATTI_RENDER_BACKEND_SDL3};
    for (size_t i = 0; i < sizeof(order) / sizeof(order[0]); i++)
    {
        if (!LIBMATTI_Render_IsBackendAvailable(order[i]))
            continue;
        if (available[0] != '\0')
            strncat(available, " ", sizeof(available) - strlen(available) - 1);
        strncat(available, LIBMATTI_Render_BackendName(order[i]),
                sizeof(available) - strlen(available) - 1);
    }
    if (available[0] == '\0')
        snprintf(available, sizeof(available), "(none)");
    return available;
}

LIBMATTI_Renderer *LIBMATTI_Render_Alloc(const LIBMATTI_RenderBackendOps *ops,
                                         const LIBMATTI_RenderConfig *config)
{
    LIBMATTI_Renderer *renderer = calloc(1, sizeof(LIBMATTI_Renderer));
    if (renderer == NULL)
        return NULL;

    renderer->ops = ops;
    renderer->config = config != NULL ? *config : LIBMATTI_Render_DefaultConfig();
    if (renderer->config.width <= 0)
        renderer->config.width = 854;
    if (renderer->config.height <= 0)
        renderer->config.height = 480;
    if (renderer->config.title == NULL)
        renderer->config.title = "Matticraft";

    renderer->title = strdup(renderer->config.title);
    renderer->width = renderer->config.width;
    renderer->height = renderer->config.height;
    renderer->pixels = calloc((size_t) renderer->width * (size_t) renderer->height,
                              sizeof(uint32_t));
    if (renderer->title == NULL || renderer->pixels == NULL)
    {
        LIBMATTI_Render_Free(renderer);
        return NULL;
    }
    return renderer;
}

void LIBMATTI_Render_Free(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    free(renderer->pixels);
    free(renderer->title);
    free(renderer);
}

LIBMATTI_Renderer *LIBMATTI_Render_Init(LIBMATTI_RenderBackendKind backend,
                                        const LIBMATTI_RenderConfig *config)
{
    const LIBMATTI_RenderBackendOps *ops = NULL;
    if (backend == LIBMATTI_RENDER_BACKEND_AUTO)
    {
        const LIBMATTI_RenderBackendKind order[] = {LIBMATTI_RENDER_BACKEND_ANDROID,
                                                     LIBMATTI_RENDER_BACKEND_OPENGL,
                                                     LIBMATTI_RENDER_BACKEND_SDL3,
                                                     LIBMATTI_RENDER_BACKEND_VULKAN};
        for (size_t i = 0; i < sizeof(order) / sizeof(order[0]); i++)
        {
            if (LIBMATTI_Render_IsBackendAvailable(order[i]))
            {
                ops = ops_for(order[i]);
                backend = order[i];
                break;
            }
        }
        if (ops == NULL)
        {
            LIBMATTI_Render_SetLastError("no rendering backend is available (tried: %s)",
                                         LIBMATTI_Render_AvailableBackends());
            return NULL;
        }
    }
    else
    {
        ops = ops_for(backend);
    }

    if (ops == NULL)
    {
        LIBMATTI_Render_SetLastError("unknown rendering backend '%s'",
                                     LIBMATTI_Render_BackendName(backend));
        return NULL;
    }
    if (ops->is_available != NULL && !ops->is_available())
    {
        LIBMATTI_Render_SetLastError("rendering backend '%s' is not available on this host",
                                     ops->name);
        return NULL;
    }

    LIBMATTI_Renderer *renderer = LIBMATTI_Render_Alloc(ops, config);
    if (renderer == NULL)
    {
        LIBMATTI_Render_SetLastError("out of memory while creating the '%s' renderer", ops->name);
        return NULL;
    }

    if (ops->init == NULL || !ops->init(renderer))
    {
        if (last_error[0] == '\0')
            LIBMATTI_Render_SetLastError("rendering backend '%s' failed to initialize", ops->name);
        LIBMATTI_Render_Free(renderer);
        return NULL;
    }
    return renderer;
}

LIBMATTI_Renderer *LIBMATTI_Render_InitSelection(const LIBMATTI_RenderSelection *selection,
                                                 const LIBMATTI_RenderConfig *config)
{
    LIBMATTI_RenderSelection chosen = selection != NULL ? *selection : LIBMATTI_Render_DefaultSelection();
    LIBMATTI_RenderConfig render_config = config != NULL ? *config : LIBMATTI_Render_DefaultConfig();

    if (chosen.api == LIBMATTI_RENDER_API_DIRECTX || chosen.api == LIBMATTI_RENDER_API_METAL)
    {
        const char *api_name = LIBMATTI_Render_ApiName(chosen.api);
        if (!LIBMATTI_Platform_SupportsApi(api_name))
        {
            LIBMATTI_Render_SetLastError("%s is not available on %s", api_name,
                                         LIBMATTI_Platform_Name());
            return NULL;
        }
        if (chosen.layer == LIBMATTI_RENDER_LAYER_NATIVE)
        {
            LIBMATTI_Render_SetLastError(
                "%s is only available through the SDL layer; choose sdl:%s", api_name, api_name);
            return NULL;
        }
        chosen.layer = LIBMATTI_RENDER_LAYER_SDL;
    }
    if (chosen.api == LIBMATTI_RENDER_API_OPENGL && chosen.layer == LIBMATTI_RENDER_LAYER_AUTO)
        chosen.layer = LIBMATTI_RENDER_LAYER_NATIVE;
    if (chosen.api == LIBMATTI_RENDER_API_VULKAN && chosen.layer == LIBMATTI_RENDER_LAYER_AUTO)
        chosen.layer = LIBMATTI_RENDER_LAYER_NATIVE;
    if (chosen.layer == LIBMATTI_RENDER_LAYER_AUTO && chosen.api == LIBMATTI_RENDER_API_AUTO)
    {


        chosen.layer = LIBMATTI_RENDER_LAYER_NATIVE;
    }

    render_config.layer = chosen.layer;
    render_config.api = chosen.api;

    LIBMATTI_RenderBackendKind backend = LIBMATTI_RENDER_BACKEND_AUTO;
#if defined(__ANDROID__)
    /* Android has no GLFW/SDL window; the Activity owns the SurfaceView and
     * the EGL backend draws into it. */
    if (chosen.layer != LIBMATTI_RENDER_LAYER_SDL)
        backend = LIBMATTI_RENDER_BACKEND_ANDROID;
    else
        backend = LIBMATTI_RENDER_BACKEND_SDL3;
#else
    if (chosen.layer == LIBMATTI_RENDER_LAYER_SDL)
        backend = LIBMATTI_RENDER_BACKEND_SDL3;
    else if (chosen.api == LIBMATTI_RENDER_API_OPENGL)
        backend = LIBMATTI_RENDER_BACKEND_OPENGL;
    else if (chosen.api == LIBMATTI_RENDER_API_VULKAN)
        backend = LIBMATTI_RENDER_BACKEND_VULKAN;
#endif

    LIBMATTI_Renderer *renderer = LIBMATTI_Render_Init(backend, &render_config);
    if (renderer == NULL && backend != LIBMATTI_RENDER_BACKEND_AUTO)
    {
        char failure[512];
        snprintf(failure, sizeof(failure), "%s", LIBMATTI_Render_GetLastError());
        LIBMATTI_Render_SetLastError("%s (requested %s:%s)", failure, LIBMATTI_Render_LayerName(chosen.layer),
                                     LIBMATTI_Render_ApiName(chosen.api));
    }
    return renderer;
}

LIBMATTI_Renderer *LIBMATTI_Render_InitByName(const char *name,
                                              const LIBMATTI_RenderConfig *config)
{
    LIBMATTI_RenderSelection selection;
    if (!LIBMATTI_Render_ParseSelection(name, &selection))
    {
        LIBMATTI_Render_SetLastError("unknown rendering selection '%s' (try native:opengl, native:vulkan, "
                                     "sdl:vulkan, sdl:opengl, sdl:directx; available backends: %s)",
                                     name != NULL ? name : "(null)", LIBMATTI_Render_AvailableBackends());
        return NULL;
    }
    return LIBMATTI_Render_InitSelection(&selection, config);
}

LIBMATTI_RenderLayer LIBMATTI_Render_GetLayer(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL ? renderer->config.layer : LIBMATTI_RENDER_LAYER_AUTO;
}

LIBMATTI_RenderApi LIBMATTI_Render_GetApi(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL ? renderer->config.api : LIBMATTI_RENDER_API_AUTO;
}

LIBMATTI_Renderer *LIBMATTI_Render_InitOpenGL(const LIBMATTI_RenderConfig *config)
{
    return LIBMATTI_Render_Init(LIBMATTI_RENDER_BACKEND_OPENGL, config);
}

LIBMATTI_Renderer *LIBMATTI_Render_InitVulkan(const LIBMATTI_RenderConfig *config)
{
    return LIBMATTI_Render_Init(LIBMATTI_RENDER_BACKEND_VULKAN, config);
}

LIBMATTI_Renderer *LIBMATTI_Render_InitSDL3(const LIBMATTI_RenderConfig *config)
{
    return LIBMATTI_Render_Init(LIBMATTI_RENDER_BACKEND_SDL3, config);
}

void LIBMATTI_Render_Destroy(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    if (renderer->ops != NULL && renderer->ops->shutdown != NULL)
        renderer->ops->shutdown(renderer);
    LIBMATTI_Render_Free(renderer);
}

const char *LIBMATTI_Render_GetBackendName(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL && renderer->ops != NULL ? renderer->ops->name : "unknown";
}

const char *LIBMATTI_Render_GetBackendDescription(const LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->description == NULL)
        return "unknown";
    return renderer->ops->description;
}

int LIBMATTI_Render_GetWidth(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL ? renderer->width : 0;
}

int LIBMATTI_Render_GetHeight(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL ? renderer->height : 0;
}

void LIBMATTI_Render_GetSize(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    if (width != NULL)
        *width = renderer != NULL ? renderer->width : 0;
    if (height != NULL)
        *height = renderer != NULL ? renderer->height : 0;
}

void LIBMATTI_Render_GetWindowSize(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    if (renderer != NULL && renderer->ops != NULL && renderer->ops->get_window_size != NULL)
    {
        renderer->ops->get_window_size(renderer, width, height);
        return;
    }
    LIBMATTI_Render_GetSize(renderer, width, height);
}

int LIBMATTI_Render_Resize(LIBMATTI_Renderer *renderer, int width, int height)
{
    if (renderer == NULL || width <= 0 || height <= 0)
        return 0;
    if (width == renderer->width && height == renderer->height)
        return 1;

    uint32_t *pixels = calloc((size_t) width * (size_t) height, sizeof(uint32_t));
    if (pixels == NULL)
        return 0;

    int copyWidth = width < renderer->width ? width : renderer->width;
    int copyHeight = height < renderer->height ? height : renderer->height;
    for (int y = 0; y < copyHeight; y++)
    {
        memcpy(pixels + (size_t) y * (size_t) width,
               renderer->pixels + (size_t) y * (size_t) renderer->width,
               (size_t) copyWidth * sizeof(uint32_t));
    }

    free(renderer->pixels);
    renderer->pixels = pixels;
    renderer->width = width;
    renderer->height = height;
    return 1;
}

void LIBMATTI_Render_Clear(LIBMATTI_Renderer *renderer, uint32_t argb)
{
    if (renderer == NULL || renderer->pixels == NULL)
        return;
    size_t count = (size_t) renderer->width * (size_t) renderer->height;
    for (size_t i = 0; i < count; i++)
        renderer->pixels[i] = argb;
}

void LIBMATTI_Render_DrawPixel(LIBMATTI_Renderer *renderer, int x, int y, uint32_t argb)
{
    if (renderer == NULL || renderer->pixels == NULL)
        return;
    if (x < 0 || y < 0 || x >= renderer->width || y >= renderer->height)
        return;
    renderer->pixels[(size_t) y * (size_t) renderer->width + (size_t) x] = argb;
}

uint32_t LIBMATTI_Render_GetPixel(const LIBMATTI_Renderer *renderer, int x, int y)
{
    if (renderer == NULL || renderer->pixels == NULL)
        return 0;
    if (x < 0 || y < 0 || x >= renderer->width || y >= renderer->height)
        return 0;
    return renderer->pixels[(size_t) y * (size_t) renderer->width + (size_t) x];
}

void LIBMATTI_Render_FillRect(LIBMATTI_Renderer *renderer, int x, int y, int width, int height,
                              uint32_t argb)
{
    if (renderer == NULL || renderer->pixels == NULL || width <= 0 || height <= 0)
        return;

    long long x0 = x;
    long long y0 = y;
    long long x1 = (long long) x + width;
    long long y1 = (long long) y + height;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > renderer->width) x1 = renderer->width;
    if (y1 > renderer->height) y1 = renderer->height;
    if (x0 >= x1 || y0 >= y1)
        return;

    for (long long row = y0; row < y1; row++)
    {
        uint32_t *line = renderer->pixels + (size_t) row * (size_t) renderer->width;
        for (long long column = x0; column < x1; column++)
            line[column] = argb;
    }
}

void LIBMATTI_Render_DrawLine(LIBMATTI_Renderer *renderer, int x0, int y0, int x1, int y1,
                              uint32_t argb)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    for (;;)
    {
        LIBMATTI_Render_DrawPixel(renderer, x0, y0, argb);
        if (x0 == x1 && y0 == y1)
            break;
        int twice = 2 * error;
        if (twice >= dy)
        {
            error += dy;
            x0 += sx;
        }
        if (twice <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}

void LIBMATTI_Render_Present(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->present == NULL)
        return;
    renderer->ops->present(renderer);
}

void LIBMATTI_Render_PollEvents(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->poll_events == NULL)
        return;
    renderer->ops->poll_events(renderer);
}

int LIBMATTI_Render_ShouldClose(const LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->should_close == NULL)
        return 1;
    return renderer->ops->should_close(renderer);
}

double LIBMATTI_Render_GetTime(const LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->get_time == NULL)
        return 0.0;
    return renderer->ops->get_time(renderer);
}

void LIBMATTI_Render_SetTitle(LIBMATTI_Renderer *renderer, const char *title)
{
    if (renderer == NULL || title == NULL)
        return;
    char *copy = strdup(title);
    if (copy == NULL)
        return;
    free(renderer->title);
    renderer->title = copy;
    renderer->config.title = renderer->title;
    if (renderer->ops != NULL && renderer->ops->set_title != NULL)
        renderer->ops->set_title(renderer, renderer->title);
}

void LIBMATTI_Render_GetMousePosition(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    if (x != NULL)
        *x = 0.0;
    if (y != NULL)
        *y = 0.0;
    if (renderer == NULL || renderer->ops == NULL
        || renderer->ops->get_mouse_position == NULL)
        return;
    renderer->ops->get_mouse_position(renderer, x, y);
}

int LIBMATTI_Render_IsMouseDown(const LIBMATTI_Renderer *renderer, int button)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->is_mouse_down == NULL)
        return 0;
    return renderer->ops->is_mouse_down(renderer, button);
}

int LIBMATTI_Render_IsKeyDown(const LIBMATTI_Renderer *renderer, int key)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->is_key_down == NULL)
        return 0;
    return renderer->ops->is_key_down(renderer, key);
}

void LIBMATTI_Render_RequestClose(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->request_close == NULL)
        return;
    renderer->ops->request_close(renderer);
}

void *LIBMATTI_Render_GetNativeWindow(const LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL || renderer->ops == NULL || renderer->ops->get_native_window == NULL)
        return NULL;
    return renderer->ops->get_native_window(renderer);
}

const uint32_t *LIBMATTI_Render_GetPixels(const LIBMATTI_Renderer *renderer)
{
    return renderer != NULL ? renderer->pixels : NULL;
}

int LIBMATTI_Render_SavePPM(const LIBMATTI_Renderer *renderer, const char *path)
{
    if (renderer == NULL || path == NULL)
        return -1;
    FILE *out = fopen(path, "wb");
    if (out == NULL)
        return -1;
    fprintf(out, "P6\n%d %d\n255\n", renderer->width, renderer->height);
    for (int y = 0; y < renderer->height; y++)
    {
        for (int x = 0; x < renderer->width; x++)
        {
            uint32_t argb = renderer->pixels[(size_t) y * (size_t) renderer->width + (size_t) x];
            unsigned char rgb[3] = {(unsigned char) ((argb >> 16) & 0xFFu),
                                    (unsigned char) ((argb >> 8) & 0xFFu),
                                    (unsigned char) (argb & 0xFFu)};
            if (fwrite(rgb, 1, sizeof(rgb), out) != sizeof(rgb))
            {
                fclose(out);
                return -1;
            }
        }
    }
    fclose(out);
    return 0;
}
