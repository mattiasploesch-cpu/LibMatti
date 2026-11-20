#ifndef MATTICRAFT_MATTI_RENDER_ANDROID_SURFACE_H
#define MATTICRAFT_MATTI_RENDER_ANDROID_SURFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* The Java SurfaceView hands its ANativeWindow to the renderer. The pointer is
 * an owned ANativeWindow reference; the renderer releases it when it is
 * replaced or cleared. */
void LIBMATTI_Android_SetSurface(void *window);
void LIBMATTI_Android_SetSize(int width, int height);
int LIBMATTI_Android_HasSurface(void);

#ifdef __cplusplus
}
#endif

#endif
