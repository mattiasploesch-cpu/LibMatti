// OpenGL renderer backend: GLFW window + OpenGL 3.2 core context, a
// fullscreen triangle and a streaming RGBA texture. The CPU framebuffer from
// render.c is uploaded once per Present. This is the backend that matches the
// current game renderer's GLFW/OpenGL window setup.

#include "libmatti/matti/render/render_internal.h"

#include "libmatti/org/lwjgl/glfw/GLFW.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LIBMATTI_OpenGLBackend
{
    long window;
    unsigned int texture;
    int texture_width;
    int texture_height;
    unsigned int program;
    unsigned int vao;
    int window_width;
    int window_height;
    int initialized;
} LIBMATTI_OpenGLBackend;

static const char *VERTEX_SHADER = "#version 150\n"
                                   "out vec2 vUV;\n"
                                   "void main() {\n"
                                   "    vec2 p = vec2((gl_VertexID == 1) ? 3.0 : -1.0,\n"
                                   "                  (gl_VertexID == 2) ? 3.0 : -1.0);\n"
                                   "    vUV = (p + 1.0) * 0.5;\n"
                                   "    gl_Position = vec4(p, 0.0, 1.0);\n"
                                   "}\n";

static const char *FRAGMENT_SHADER = "#version 150\n"
                                     "uniform sampler2D uTexture;\n"
                                     "in vec2 vUV;\n"
                                     "out vec4 fragColor;\n"
                                     "void main() {\n"
                                     "    fragColor = texture(uTexture, vec2(vUV.x, 1.0 - vUV.y));\n"
                                     "}\n";

static unsigned int compile_shader(unsigned int type, const char *source)
{
    unsigned int shader = LIBMATTI_GL_glCreateShader(type);
    if (shader == 0)
        return 0;
    LIBMATTI_GL_glShaderSource(shader, source);
    LIBMATTI_GL_glCompileShader(shader);
    if (!LIBMATTI_GL_glGetShaderi(shader, LIBMATTI_GL_GL_COMPILE_STATUS))
    {
        LIBMATTI_Render_SetLastError("OpenGL shader compile failed: %s",
                                     LIBMATTI_GL_glGetShaderInfoLog(shader));
        LIBMATTI_GL_glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int create_program(void)
{
    unsigned int vertex = compile_shader(LIBMATTI_GL_GL_VERTEX_SHADER, VERTEX_SHADER);
    if (vertex == 0)
        return 0;
    unsigned int fragment = compile_shader(LIBMATTI_GL_GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    if (fragment == 0)
    {
        LIBMATTI_GL_glDeleteShader(vertex);
        return 0;
    }

    unsigned int program = LIBMATTI_GL_glCreateProgram();
    LIBMATTI_GL_glAttachShader(program, vertex);
    LIBMATTI_GL_glAttachShader(program, fragment);
    LIBMATTI_GL_glLinkProgram(program);
    LIBMATTI_GL_glDeleteShader(vertex);
    LIBMATTI_GL_glDeleteShader(fragment);

    if (!LIBMATTI_GL_glGetProgrami(program, LIBMATTI_GL_GL_LINK_STATUS))
    {
        LIBMATTI_Render_SetLastError("OpenGL shader link failed: %s",
                                     LIBMATTI_GL_glGetProgramInfoLog(program));
        LIBMATTI_GL_glDeleteProgram(program);
        return 0;
    }
    return program;
}

// The CPU framebuffer can grow after init (the UI bridge resizes it to the
// fullscreen window size), so the streaming texture is reallocated on demand.
static int opengl_ensure_texture(LIBMATTI_Renderer *renderer, LIBMATTI_OpenGLBackend *backend)
{
    if (backend->texture == 0)
        LIBMATTI_GL_glGenTextures(1, &backend->texture);
    if (backend->texture == 0)
        return 0;
    if (backend->texture_width == renderer->width && backend->texture_height == renderer->height)
        return 1;

    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, backend->texture);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER,
                                LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER,
                                LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S,
                                LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T,
                                LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 4);
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA8, renderer->width,
                             renderer->height, 0, LIBMATTI_GL_GL_BGRA,
                             LIBMATTI_GL_GL_UNSIGNED_BYTE, NULL);
    backend->texture_width = renderer->width;
    backend->texture_height = renderer->height;
    return 1;
}

static void opengl_backend_free(LIBMATTI_OpenGLBackend *backend)
{
    if (backend == NULL)
        return;
    if (backend->initialized)
    {
        if (backend->texture != 0)
            LIBMATTI_GL_glDeleteTextures(1, &backend->texture);
        if (backend->vao != 0)
            LIBMATTI_GL_glDeleteVertexArrays(1, &backend->vao);
        if (backend->program != 0)
            LIBMATTI_GL_glDeleteProgram(backend->program);
        backend->initialized = 0;
    }
    if (backend->window != 0)
    {
        LIBMATTI_GLFW_glfwDestroyWindow(backend->window);
        backend->window = 0;
    }
    LIBMATTI_GLFW_glfwTerminate();
    free(backend);
}

static int opengl_is_available(void)
{
    // The GLFW/GL bindings are dlopen-based: the libraries only have to be
    // present at runtime. Init reports a useful error if they are missing.
    return 1;
}

static int opengl_init(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_OpenGLBackend *backend = calloc(1, sizeof(LIBMATTI_OpenGLBackend));
    if (backend == NULL)
    {
        LIBMATTI_Render_SetLastError("out of memory while creating the OpenGL backend");
        return 0;
    }
    renderer->backend = backend;

    if (!LIBMATTI_GLFW_glfwInit())
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        LIBMATTI_Render_SetLastError("GLFW init failed: %s",
                                     description != NULL ? description : "unknown error");
        opengl_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    LIBMATTI_GLFW_glfwDefaultWindowHints();
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CLIENT_API, LIBMATTI_GLFW_OPENGL_API);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MAJOR, 3);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CONTEXT_VERSION_MINOR, 2);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_PROFILE, LIBMATTI_GLFW_OPENGL_CORE_PROFILE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_OPENGL_FORWARD_COMPAT, LIBMATTI_GLFW_TRUE);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_RESIZABLE,
                                renderer->config.fullscreen ? LIBMATTI_GLFW_FALSE
                                                            : LIBMATTI_GLFW_TRUE);

    // Fullscreen is a borderless window at the current desktop mode, not a
    // monitor mode change. glfwCreateWindow(..., monitor, ...) switches the
    // monitor (and remote-streaming hosts such as Sunshine capture that tiny
    // mode). A decorated-free window at the desktop size never changes it.
    int window_width = renderer->width;
    int window_height = renderer->height;
    int monitor_x = 0, monitor_y = 0;
    long monitor = 0;
    if (renderer->config.fullscreen)
    {
        LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_DECORATED, LIBMATTI_GLFW_FALSE);
        LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_VISIBLE, LIBMATTI_GLFW_FALSE);
        monitor = LIBMATTI_GLFW_glfwGetPrimaryMonitor();
        if (monitor != 0)
        {
            int mode_width = 0, mode_height = 0;
            LIBMATTI_GLFW_glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
            if (LIBMATTI_GLFW_glfwGetVideoMode(monitor, &mode_width, &mode_height, NULL, NULL,
                                               NULL, NULL)
                && mode_width > 0 && mode_height > 0)
            {
                window_width = mode_width;
                window_height = mode_height;
            }
        }
    }

    backend->window = LIBMATTI_GLFW_glfwCreateWindow(window_width, window_height,
                                                     renderer->title, 0, 0);
    if (backend->window == 0)
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        LIBMATTI_Render_SetLastError("OpenGL window creation failed: %s",
                                     description != NULL ? description : "unknown error");
        opengl_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    if (renderer->config.fullscreen)
    {
        LIBMATTI_GLFW_glfwSetWindowPos(backend->window, monitor_x, monitor_y);
        // The UI is drawn in native window pixels. Keeping it at 854x480 and
        // scaling it in Present would enlarge the already GUI-scaled menus.
        if (!LIBMATTI_Render_Resize(renderer, window_width, window_height))
        {
            LIBMATTI_Render_SetLastError("could not allocate the fullscreen framebuffer");
            opengl_backend_free(backend);
            renderer->backend = NULL;
            return 0;
        }
    }

    LIBMATTI_GLFW_glfwMakeContextCurrent(backend->window);
    if (!LIBMATTI_GL_Load() || !LIBMATTI_GL_IsAvailable())
    {
        LIBMATTI_Render_SetLastError("could not load the OpenGL entry points");
        opengl_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    LIBMATTI_GLFW_glfwSwapInterval(renderer->config.vsync ? 1 : 0);
    LIBMATTI_GLFW_glfwFocusWindow(backend->window);
    LIBMATTI_GLFW_glfwShowWindow(backend->window);

    backend->program = create_program();
    if (backend->program == 0)
    {
        opengl_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }

    LIBMATTI_GL_glGenVertexArrays(1, &backend->vao);
    LIBMATTI_GL_glBindVertexArray(backend->vao);

    if (!opengl_ensure_texture(renderer, backend))
    {
        LIBMATTI_Render_SetLastError("OpenGL streaming texture creation failed");
        opengl_backend_free(backend);
        renderer->backend = NULL;
        return 0;
    }
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, 0);

    LIBMATTI_GL_glUseProgram(backend->program);
    LIBMATTI_GL_glUniform1i(LIBMATTI_GL_glGetUniformLocation(backend->program, "uTexture"), 0);
    LIBMATTI_GL_glUseProgram(0);

    backend->initialized = 1;
    backend->window_width = renderer->width;
    backend->window_height = renderer->height;
    return 1;
}

static void opengl_shutdown(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    opengl_backend_free((LIBMATTI_OpenGLBackend *) renderer->backend);
    renderer->backend = NULL;
}

static void opengl_poll_events(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    LIBMATTI_GLFW_glfwPollEvents();
}

static int opengl_should_close(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    return backend == NULL || backend->window == 0
               || LIBMATTI_GLFW_glfwWindowShouldClose(backend->window);
}

static void opengl_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    int w = renderer->width;
    int h = renderer->height;
    if (backend != NULL && backend->window != 0)
    {
        LIBMATTI_GLFW_glfwGetFramebufferSize(backend->window, &w, &h);
        if (w <= 0) w = renderer->width;
        if (h <= 0) h = renderer->height;
    }
    if (width != NULL) *width = w;
    if (height != NULL) *height = h;
}

static void opengl_present(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (backend == NULL || backend->window == 0 || !backend->initialized)
        return;

    int width = 0, height = 0;
    LIBMATTI_GLFW_glfwGetFramebufferSize(backend->window, &width, &height);
    if (width <= 0 || height <= 0)
        return;

    backend->window_width = width;
    backend->window_height = height;

    LIBMATTI_GLFW_glfwMakeContextCurrent(backend->window);
    LIBMATTI_GL_glViewport(0, 0, width, height);
    LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_DEPTH_TEST);
    LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_BLEND);
    LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_CULL_FACE);
    LIBMATTI_GL_glDisable(LIBMATTI_GL_GL_SCISSOR_TEST);
    LIBMATTI_GL_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    LIBMATTI_GL_glClear(LIBMATTI_GL_GL_COLOR_BUFFER_BIT);

    LIBMATTI_GL_glUseProgram(backend->program);
    LIBMATTI_GL_glActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
    if (!opengl_ensure_texture(renderer, backend))
        return;
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, backend->texture);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 4);
    LIBMATTI_GL_glTexSubImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, 0, 0, renderer->width,
                                renderer->height, LIBMATTI_GL_GL_BGRA,
                                LIBMATTI_GL_GL_UNSIGNED_BYTE, renderer->pixels);
    LIBMATTI_GL_glBindVertexArray(backend->vao);
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, 3);
    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, 0);
    LIBMATTI_GL_glUseProgram(0);

    LIBMATTI_GLFW_glfwSwapBuffers(backend->window);
}

static double opengl_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return LIBMATTI_GLFW_glfwGetTime();
}

static void opengl_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (backend != NULL && backend->window != 0)
        LIBMATTI_GLFW_glfwSetWindowTitle(backend->window, title);
}

static void *opengl_get_native_window(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    return backend != NULL ? (void *) (size_t) backend->window : NULL;
}

static void opengl_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
    if (backend == NULL || backend->window == 0)
        return;

    double cursor_x = 0.0, cursor_y = 0.0;
    int window_width = 0, window_height = 0;
    LIBMATTI_GLFW_glfwGetCursorPos(backend->window, &cursor_x, &cursor_y);
    LIBMATTI_GLFW_glfwGetWindowSize(backend->window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        return;
    if (x != NULL)
        *x = cursor_x * (double) renderer->width / (double) window_width;
    if (y != NULL)
        *y = cursor_y * (double) renderer->height / (double) window_height;
}

static int opengl_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (backend == NULL || backend->window == 0)
        return 0;
    return LIBMATTI_GLFW_glfwGetMouseButton(backend->window, button) == LIBMATTI_GLFW_PRESS;
}

static int opengl_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (backend == NULL || backend->window == 0)
        return 0;
    return LIBMATTI_GLFW_glfwGetKey(backend->window, key) == LIBMATTI_GLFW_PRESS;
}

static void opengl_request_close(LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_OpenGLBackend *backend = renderer->backend;
    if (backend != NULL && backend->window != 0)
        LIBMATTI_GLFW_glfwSetWindowShouldClose(backend->window, 1);
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_OpenGLOps(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "opengl",
        "OpenGL 3.2 core (GLFW)",
        opengl_is_available,
        opengl_init,
        opengl_shutdown,
        opengl_poll_events,
        opengl_should_close,
        opengl_get_window_size,
        opengl_present,
        opengl_get_time,
        opengl_set_title,
        opengl_get_native_window,
        opengl_get_mouse_position,
        opengl_is_mouse_down,
        opengl_is_key_down,
        opengl_request_close,
    };
    return &ops;
}
