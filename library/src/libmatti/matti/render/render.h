












#ifndef MATTICRAFT_MATTI_RENDER_RENDER_H
#define MATTICRAFT_MATTI_RENDER_RENDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif





typedef enum LIBMATTI_RenderBackendKind
{
    LIBMATTI_RENDER_BACKEND_UNKNOWN = -1,
    LIBMATTI_RENDER_BACKEND_AUTO = 0,
    LIBMATTI_RENDER_BACKEND_ANDROID,
    LIBMATTI_RENDER_BACKEND_OPENGL,
    LIBMATTI_RENDER_BACKEND_VULKAN,
    LIBMATTI_RENDER_BACKEND_SDL3
} LIBMATTI_RenderBackendKind;







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


    LIBMATTI_RenderLayer layer;
    LIBMATTI_RenderApi api;
} LIBMATTI_RenderConfig;




typedef struct LIBMATTI_Renderer LIBMATTI_Renderer;



#define LIBMATTI_RENDER_RGBA(r, g, b, a)                                                         \
    ((uint32_t) (((uint32_t) (a) & 0xFFu) << 24) | ((uint32_t) (r) & 0xFFu) << 16               \
     | ((uint32_t) (g) & 0xFFu) << 8 | ((uint32_t) (b) & 0xFFu))
#define LIBMATTI_RENDER_RGB(r, g, b) LIBMATTI_RENDER_RGBA((r), (g), (b), 255u)



LIBMATTI_RenderConfig LIBMATTI_Render_DefaultConfig(void);




LIBMATTI_RenderBackendKind LIBMATTI_Render_BackendFromName(const char *name);
const char *LIBMATTI_Render_BackendName(LIBMATTI_RenderBackendKind backend);






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


const char *LIBMATTI_Render_AvailableBackends(void);





LIBMATTI_Renderer *LIBMATTI_Render_InitOpenGL(const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitVulkan(const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitSDL3(const LIBMATTI_RenderConfig *config);

LIBMATTI_Renderer *LIBMATTI_Render_Init(LIBMATTI_RenderBackendKind backend,
                                        const LIBMATTI_RenderConfig *config);
LIBMATTI_Renderer *LIBMATTI_Render_InitByName(const char *name,
                                              const LIBMATTI_RenderConfig *config);
void LIBMATTI_Render_Destroy(LIBMATTI_Renderer *renderer);



const char *LIBMATTI_Render_GetBackendName(const LIBMATTI_Renderer *renderer);
const char *LIBMATTI_Render_GetBackendDescription(const LIBMATTI_Renderer *renderer);
const char *LIBMATTI_Render_GetLastError(void);




int LIBMATTI_Render_GetWidth(const LIBMATTI_Renderer *renderer);
int LIBMATTI_Render_GetHeight(const LIBMATTI_Renderer *renderer);
void LIBMATTI_Render_GetSize(const LIBMATTI_Renderer *renderer, int *width, int *height);



void LIBMATTI_Render_GetWindowSize(const LIBMATTI_Renderer *renderer, int *width, int *height);





int LIBMATTI_Render_Resize(LIBMATTI_Renderer *renderer, int width, int height);




void LIBMATTI_Render_Clear(LIBMATTI_Renderer *renderer, uint32_t argb);
void LIBMATTI_Render_DrawPixel(LIBMATTI_Renderer *renderer, int x, int y, uint32_t argb);
uint32_t LIBMATTI_Render_GetPixel(const LIBMATTI_Renderer *renderer, int x, int y);
void LIBMATTI_Render_FillRect(LIBMATTI_Renderer *renderer, int x, int y, int width, int height,
                              uint32_t argb);
void LIBMATTI_Render_DrawLine(LIBMATTI_Renderer *renderer, int x0, int y0, int x1, int y1,
                              uint32_t argb);



void LIBMATTI_Render_Present(LIBMATTI_Renderer *renderer);



void LIBMATTI_Render_PollEvents(LIBMATTI_Renderer *renderer);
int LIBMATTI_Render_ShouldClose(const LIBMATTI_Renderer *renderer);

double LIBMATTI_Render_GetTime(const LIBMATTI_Renderer *renderer);
void LIBMATTI_Render_SetTitle(LIBMATTI_Renderer *renderer, const char *title);







void LIBMATTI_Render_GetMousePosition(const LIBMATTI_Renderer *renderer, double *x, double *y);
int LIBMATTI_Render_IsMouseDown(const LIBMATTI_Renderer *renderer, int button);
int LIBMATTI_Render_IsKeyDown(const LIBMATTI_Renderer *renderer, int key);




void LIBMATTI_Render_RequestClose(LIBMATTI_Renderer *renderer);




void *LIBMATTI_Render_GetNativeWindow(const LIBMATTI_Renderer *renderer);



const uint32_t *LIBMATTI_Render_GetPixels(const LIBMATTI_Renderer *renderer);




int LIBMATTI_Render_SavePPM(const LIBMATTI_Renderer *renderer, const char *path);

#ifdef __cplusplus
}
#endif

#endif
