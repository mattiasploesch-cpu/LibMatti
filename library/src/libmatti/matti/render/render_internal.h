




#ifndef MATTICRAFT_MATTI_RENDER_RENDER_INTERNAL_H
#define MATTICRAFT_MATTI_RENDER_RENDER_INTERNAL_H

#include "libmatti/matti/render/render.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct LIBMATTI_RenderBackendOps
{


    const char *name;


    const char *description;




    int (*is_available)(void);





    int (*init)(LIBMATTI_Renderer *renderer);



    void (*shutdown)(LIBMATTI_Renderer *renderer);

    void (*poll_events)(LIBMATTI_Renderer *renderer);
    int (*should_close)(const LIBMATTI_Renderer *renderer);
    void (*get_window_size)(const LIBMATTI_Renderer *renderer, int *width, int *height);




    void (*present)(LIBMATTI_Renderer *renderer);

    double (*get_time)(const LIBMATTI_Renderer *renderer);
    void (*set_title)(LIBMATTI_Renderer *renderer, const char *title);
    void *(*get_native_window)(const LIBMATTI_Renderer *renderer);






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



const LIBMATTI_RenderBackendOps *LIBMATTI_Render_OpenGLOps(void);
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_VulkanOps(void);
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_SDL3Ops(void);



void LIBMATTI_Render_SetLastError(const char *format, ...);



LIBMATTI_Renderer *LIBMATTI_Render_Alloc(const LIBMATTI_RenderBackendOps *ops,
                                         const LIBMATTI_RenderConfig *config);
void LIBMATTI_Render_Free(LIBMATTI_Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif
