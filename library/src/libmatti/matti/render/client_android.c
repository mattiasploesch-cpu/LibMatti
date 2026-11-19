/* Android renderer: EGL on the ANativeWindow owned by the Compose GameActivity.
 *
 * The rest of the client draws into the same CPU ARGB framebuffer used by the
 * desktop backends. This file only creates the EGL surface, uploads that
 * framebuffer as a texture and presents it with a fullscreen triangle. */

#include "libmatti/matti/render/render_internal.h"
#include "libmatti/matti/render/android_surface.h"

#include <pthread.h>
#include <time.h>

#if defined(__ANDROID__)

#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LIBMATTI_AndroidBackend
{
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    ANativeWindow *window;
    GLuint program;
    GLuint texture;
    GLuint vao;
    int texture_width;
    int texture_height;
    int initialized;
} LIBMATTI_AndroidBackend;

static pthread_mutex_t surface_lock = PTHREAD_MUTEX_INITIALIZER;
static ANativeWindow *shared_window = NULL;
static int shared_width = 1280;
static int shared_height = 720;
static int surface_lost = 0;
static int close_requested = 0;

void LIBMATTI_Android_SetSurface(void *window)
{
    ANativeWindow *next = (ANativeWindow *) window;
    ANativeWindow *previous = NULL;

    pthread_mutex_lock(&surface_lock);
    previous = shared_window;
    if (previous == next)
    {
        /* fromSurface() returned a second reference to the same window. */
        if (next != NULL)
            ANativeWindow_release(next);
    }
    else
    {
        shared_window = next;
        surface_lost = next == NULL;
        close_requested = next == NULL;
    }
    pthread_mutex_unlock(&surface_lock);

    if (previous != NULL && previous != next)
        ANativeWindow_release(previous);
}

void LIBMATTI_Android_SetSize(int width, int height)
{
    pthread_mutex_lock(&surface_lock);
    if (width > 0) shared_width = width;
    if (height > 0) shared_height = height;
    pthread_mutex_unlock(&surface_lock);
}

int LIBMATTI_Android_HasSurface(void)
{
    int available = 0;
    pthread_mutex_lock(&surface_lock);
    available = shared_window != NULL && !surface_lost;
    pthread_mutex_unlock(&surface_lock);
    return available;
}

static ANativeWindow *current_window(void)
{
    ANativeWindow *window = NULL;
    pthread_mutex_lock(&surface_lock);
    window = shared_window;
    pthread_mutex_unlock(&surface_lock);
    return window;
}

static GLuint compile_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    if (shader == 0)
        return 0;
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        GLsizei length = 0;
        glGetShaderInfoLog(shader, (GLsizei) sizeof(log), &length, log);
        LIBMATTI_Render_SetLastError("Android GLES shader compile failed: %.*s", (int) length, log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint create_program(void)
{
    static const char *vertex =
        "#version 300 es\n"
        "out vec2 vUV;\n"
        "void main() {\n"
        "    vec2 p = vec2((gl_VertexID == 1) ? 3.0 : -1.0,\n"
        "                  (gl_VertexID == 2) ? 3.0 : -1.0);\n"
        "    vUV = (p + 1.0) * 0.5;\n"
        "    gl_Position = vec4(p, 0.0, 1.0);\n"
        "}\n";
    static const char *fragment =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D uTexture;\n"
        "in vec2 vUV;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = texture(uTexture, vec2(vUV.x, 1.0 - vUV.y)).bgra;\n"
        "}\n";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex);
    if (vs == 0)
        return 0;
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment);
    if (fs == 0)
    {
        glDeleteShader(vs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        GLsizei length = 0;
        glGetProgramInfoLog(program, (GLsizei) sizeof(log), &length, log);
        LIBMATTI_Render_SetLastError("Android GLES program link failed: %.*s", (int) length, log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

static int android_is_available(void)
{
    return LIBMATTI_Android_HasSurface();
}

static void android_backend_free(LIBMATTI_AndroidBackend *backend)
{
    if (backend == NULL)
        return;
    if (backend->display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(backend->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (backend->texture != 0)
            glDeleteTextures(1, &backend->texture);
        if (backend->vao != 0)
            glDeleteVertexArrays(1, &backend->vao);
        if (backend->program != 0)
            glDeleteProgram(backend->program);
        if (backend->surface != EGL_NO_SURFACE)
            eglDestroySurface(backend->display, backend->surface);
        if (backend->context != EGL_NO_CONTEXT)
            eglDestroyContext(backend->display, backend->context);
        eglTerminate(backend->display);
    }
    free(backend);
}

static int android_init(LIBMATTI_Renderer *renderer)
{
    ANativeWindow *window = current_window();
    if (window == NULL)
    {
        LIBMATTI_Render_SetLastError("Android SurfaceView is not ready");
        return 0;
    }

    LIBMATTI_AndroidBackend *backend = calloc(1, sizeof(LIBMATTI_AndroidBackend));
    if (backend == NULL)
    {
        LIBMATTI_Render_SetLastError("out of memory while creating the Android backend");
        return 0;
    }
    backend->display = EGL_NO_DISPLAY;
    backend->surface = EGL_NO_SURFACE;
    backend->context = EGL_NO_CONTEXT;
    backend->window = window;
    renderer->backend = backend;

    backend->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (backend->display == EGL_NO_DISPLAY || !eglInitialize(backend->display, NULL, NULL))
    {
        LIBMATTI_Render_SetLastError("Android EGL init failed (0x%x)", eglGetError());
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    const EGLint config_attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE,
    };
    EGLint config_count = 0;
    if (!eglChooseConfig(backend->display, config_attributes, &backend->config, 1, &config_count)
        || config_count < 1)
    {
        LIBMATTI_Render_SetLastError("Android EGL found no ES3 window config");
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    const EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    backend->context = eglCreateContext(backend->display, backend->config, EGL_NO_CONTEXT,
                                        context_attributes);
    if (backend->context == EGL_NO_CONTEXT)
    {
        LIBMATTI_Render_SetLastError("Android EGL context creation failed (0x%x)", eglGetError());
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    backend->surface = eglCreateWindowSurface(backend->display, backend->config, window, NULL);
    if (backend->surface == EGL_NO_SURFACE)
    {
        LIBMATTI_Render_SetLastError("Android EGL window surface creation failed (0x%x)",
                                     eglGetError());
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    if (!eglMakeCurrent(backend->display, backend->surface, backend->surface, backend->context))
    {
        LIBMATTI_Render_SetLastError("Android eglMakeCurrent failed (0x%x)", eglGetError());
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    int width = ANativeWindow_getWidth(window);
    int height = ANativeWindow_getHeight(window);
    if (width > 0 && height > 0)
        LIBMATTI_Render_Resize(renderer, width, height);

    backend->program = create_program();
    if (backend->program == 0)
    {
        android_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    glGenTextures(1, &backend->texture);
    glBindTexture(GL_TEXTURE_2D, backend->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderer->width, renderer->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    backend->texture_width = renderer->width;
    backend->texture_height = renderer->height;

    glGenVertexArrays(1, &backend->vao);
    glUseProgram(backend->program);
    glUniform1i(glGetUniformLocation(backend->program, "uTexture"), 0);
    glUseProgram(0);

    backend->initialized = 1;
    return 1;
}

static void android_shutdown(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    android_backend_free((LIBMATTI_AndroidBackend *) renderer->backend);
    renderer->backend = NULL;
}

static void android_poll_events(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static int android_should_close(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return close_requested || surface_lost;
}

static void android_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    LIBMATTI_AndroidBackend *backend = renderer != NULL ? renderer->backend : NULL;
    int w = shared_width;
    int h = shared_height;
    if (backend != NULL && backend->window != NULL)
    {
        int window_width = ANativeWindow_getWidth(backend->window);
        int window_height = ANativeWindow_getHeight(backend->window);
        if (window_width > 0) w = window_width;
        if (window_height > 0) h = window_height;
    }
    if (width != NULL) *width = w;
    if (height != NULL) *height = h;
}

static int android_ensure_texture(LIBMATTI_Renderer *renderer, LIBMATTI_AndroidBackend *backend)
{
    if (backend->texture_width == renderer->width
        && backend->texture_height == renderer->height)
        return 1;

    glBindTexture(GL_TEXTURE_2D, backend->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderer->width, renderer->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    backend->texture_width = renderer->width;
    backend->texture_height = renderer->height;
    return 1;
}

static void android_present(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_AndroidBackend *backend = renderer != NULL ? renderer->backend : NULL;
    if (backend == NULL || !backend->initialized || surface_lost)
        return;

    int width = ANativeWindow_getWidth(backend->window);
    int height = ANativeWindow_getHeight(backend->window);
    if (width <= 0 || height <= 0)
        return;

    if (width != renderer->width || height != renderer->height)
        LIBMATTI_Render_Resize(renderer, width, height);

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(backend->program);
    glActiveTexture(GL_TEXTURE0);
    android_ensure_texture(renderer, backend);
    glBindTexture(GL_TEXTURE_2D, backend->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderer->width, renderer->height, GL_RGBA,
                    GL_UNSIGNED_BYTE, renderer->pixels);
    glBindVertexArray(backend->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    if (!eglSwapBuffers(backend->display, backend->surface))
    {
        EGLint error = eglGetError();
        if (error == EGL_BAD_SURFACE || error == EGL_CONTEXT_LOST)
            surface_lost = 1;
    }
}

static double android_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) now.tv_sec + (double) now.tv_nsec / 1000000000.0;
}

static void android_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    (void) renderer;
    (void) title;
}

static void *android_get_native_window(const LIBMATTI_Renderer *renderer)
{
    LIBMATTI_AndroidBackend *backend = renderer != NULL ? renderer->backend : NULL;
    return backend != NULL ? (void *) backend->window : NULL;
}

static void android_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    (void) renderer;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
}

static int android_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    (void) renderer;
    (void) button;
    return 0;
}

static int android_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    (void) renderer;
    (void) key;
    return 0;
}

static void android_request_close(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    close_requested = 1;
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_AndroidOps(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "android",
        "Android EGL + GLES3 (SurfaceView)",
        android_is_available,
        android_init,
        android_shutdown,
        android_poll_events,
        android_should_close,
        android_get_window_size,
        android_present,
        android_get_time,
        android_set_title,
        android_get_native_window,
        android_get_mouse_position,
        android_is_mouse_down,
        android_is_key_down,
        android_request_close,
    };
    return &ops;
}

#else

void LIBMATTI_Android_SetSurface(void *window) { (void) window; }
void LIBMATTI_Android_SetSize(int width, int height) { (void) width; (void) height; }
int LIBMATTI_Android_HasSurface(void) { return 0; }
const LIBMATTI_RenderBackendOps *LIBMATTI_Render_AndroidOps(void) { return NULL; }

#endif
