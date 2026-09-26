// The GUI renderer (see GuiRenderer.h) - the POSITION_COLOR_TEXTURE quad
// batcher the Java GuiGraphics blits ride (Java's GuiGraphics.drawSpecial over
// the RenderPipelines GUI_TEXTURED path). The shader folds the y-down screen
// pixels the layout unit produces into the clip space like the font shader
// (position / screenSize * 2 - 1 straight, no y negation - the FBO blit
// flips once for every pass, so the HUD shares the font's y-down space).

#include "libmatti/net/minecraft/client/gui/GuiRenderer.h"

#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: position_color_tex.vsh - the screen-space transform the font shader
// carries (position / screenSize * 2 - 1, no y negation - the layout's y-down
// pixels map straight onto the pass space; the negation pushed the hotbar to
// the TOP of the window).
static const char *GUI_VERT =
    "#version 150\n"
    "uniform vec2 screenSize;\n"
    "in vec3 position;\n"
    "in vec4 color;\n"
    "in vec2 uv0;\n"
    "out vec4 vtxColor;\n"
    "out vec2 vtxUv0;\n"
    "void main() {\n"
    "    vec2 ndc = position.xy / screenSize * 2.0 - 1.0;\n"
    "    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);\n"
    "    vtxColor = color;\n"
    "    vtxUv0 = uv0;\n"
    "}\n";

// Java: the GUI_TEXTURED fragment - the texture sample tinted by the vertex
// colour (the blit path's ARGB.white default rides the tint).
static const char *GUI_FRAG =
    "#version 150\n"
    "in vec4 vtxColor;\n"
    "in vec2 vtxUv0;\n"
    "uniform sampler2D Sampler0;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    fragColor = texture(Sampler0, vtxUv0) * vtxColor;\n"
    "}\n";

// The batch grows on demand (a full HUD frame stays well under the initial
// capacity; the growth keeps the batcher reusable for later screens).
typedef struct LIBMATTI_MC_GuiRenderer
{
    unsigned int program;
    int screenSizeLocation;
    unsigned int vao;
    unsigned int vbo;
    // The white 1x1 fallback the quads sample (see create_white_texture) and
    // the caller-set texture the next Flush binds instead (the atlas path).
    unsigned int whiteTexture;
    unsigned int boundTexture;
    LIBMATTI_MC_GuiVertex *vertices;
    size_t vertexCount;
    size_t vertexCapacity;
} LIBMATTI_MC_GuiRenderer;

static int compile_ok(unsigned int shader, const char *label)
{
    if (!LIBMATTI_GL_glGetShaderiv(shader, LIBMATTI_GL_GL_COMPILE_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetShaderInfoLog(shader);
        fprintf(stderr, "[GUIRENDERER] ERROR: %s: %s\n", label,
                log != NULL && log[0] != '\0' ? log : "compile failed");
        return 0;
    }
    return 1;
}

static unsigned int compile_program(void)
{
    unsigned int vertex = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_VERTEX_SHADER);
    unsigned int fragment = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_FRAGMENT_SHADER);
    if (vertex == 0 || fragment == 0)
        return 0;

    LIBMATTI_GL_glShaderSource(vertex, GUI_VERT);
    LIBMATTI_GL_glShaderSource(fragment, GUI_FRAG);
    LIBMATTI_GL_glCompileShader(vertex);
    LIBMATTI_GL_glCompileShader(fragment);

    if (!compile_ok(vertex, "gui vertex shader") || !compile_ok(fragment, "gui fragment shader"))
    {
        LIBMATTI_GL_glDeleteShader(vertex);
        LIBMATTI_GL_glDeleteShader(fragment);
        return 0;
    }

    unsigned int program = LIBMATTI_GL_glCreateProgram();
    LIBMATTI_GL_glAttachShader(program, vertex);
    LIBMATTI_GL_glAttachShader(program, fragment);

    // Java: the GUI format's attribute order - position, color, uv0.
    LIBMATTI_GL_glBindAttribLocation(program, 0, "position");
    LIBMATTI_GL_glBindAttribLocation(program, 1, "color");
    LIBMATTI_GL_glBindAttribLocation(program, 2, "uv0");

    LIBMATTI_GL_glLinkProgram(program);
    LIBMATTI_GL_glDeleteShader(vertex);
    LIBMATTI_GL_glDeleteShader(fragment);
    if (!LIBMATTI_GL_glGetProgramiv(program, LIBMATTI_GL_GL_LINK_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetProgramInfoLog(program);
        fprintf(stderr, "[GUIRENDERER] ERROR: gui program link failed: %s\n",
                log != NULL && log[0] != '\0' ? log : "unknown reason");
        LIBMATTI_GL_glDeleteProgram(program);
        return 0;
    }
    return program;
}

static void setup_vao(unsigned int vao, unsigned int vbo)
{
    LIBMATTI_GL_glBindVertexArray(vao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbo);
    // Java: the POSITION_COLOR_TEXTURE stride - 3 pos + 4 color + 2 uv floats.
    LIBMATTI_GL_glEnableVertexAttribArray(0);
    LIBMATTI_GL_glVertexAttribPointer(0, 3, LIBMATTI_GL_GL_FLOAT, 0, sizeof(LIBMATTI_MC_GuiVertex),
                                      (const void *) offsetof(LIBMATTI_MC_GuiVertex, x));
    LIBMATTI_GL_glEnableVertexAttribArray(1);
    LIBMATTI_GL_glVertexAttribPointer(1, 4, LIBMATTI_GL_GL_FLOAT, 0, sizeof(LIBMATTI_MC_GuiVertex),
                                      (const void *) offsetof(LIBMATTI_MC_GuiVertex, r));
    LIBMATTI_GL_glEnableVertexAttribArray(2);
    LIBMATTI_GL_glVertexAttribPointer(2, 2, LIBMATTI_GL_GL_FLOAT, 0, sizeof(LIBMATTI_MC_GuiVertex),
                                      (const void *) offsetof(LIBMATTI_MC_GuiVertex, u));
    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
}

// Java: the white 1x1 pixel the blit path falls back to while the HUD runs
// without sprite atlases (the GUI textures the vanilla path carries).
static unsigned int create_white_texture(void)
{
    unsigned int texture = 0;
    unsigned char pixel[4] = {255, 255, 255, 255};
    LIBMATTI_GL_glGenTextures(1, &texture);
    if (texture == 0)
        return 0;
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, texture);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA, 1, 1, 0,
                             LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, pixel);
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, 0);
    return texture;
}

LIBMATTI_MC_GuiRenderer *LIBMATTI_MC_GuiRenderer_New(void)
{
    unsigned int program = compile_program();
    if (program == 0)
        return NULL;

    unsigned int vao = 0, vbo = 0;
    LIBMATTI_GL_glGenVertexArrays(1, &vao);
    LIBMATTI_GL_glGenBuffers(1, &vbo);
    if (vao == 0 || vbo == 0)
    {
        if (vao != 0)
            LIBMATTI_GL_glDeleteVertexArrays(1, &vao);
        if (vbo != 0)
            LIBMATTI_GL_glDeleteBuffers(1, &vbo);
        LIBMATTI_GL_glDeleteProgram(program);
        return NULL;
    }

    LIBMATTI_MC_GuiRenderer *renderer = calloc(1, sizeof(*renderer));
    if (renderer == NULL)
    {
        LIBMATTI_GL_glDeleteVertexArrays(1, &vao);
        LIBMATTI_GL_glDeleteBuffers(1, &vbo);
        LIBMATTI_GL_glDeleteProgram(program);
        return NULL;
    }

    renderer->program = program;
    renderer->screenSizeLocation = LIBMATTI_GL_glGetUniformLocation(program, "screenSize");
    renderer->vao = vao;
    renderer->vbo = vbo;
    renderer->whiteTexture = create_white_texture();
    renderer->vertexCapacity = 1024;
    renderer->vertices = malloc(renderer->vertexCapacity * sizeof(*renderer->vertices));
    if (renderer->vertices == NULL)
    {
        renderer->vertexCapacity = 0;
    }
    setup_vao(renderer->vao, renderer->vbo);
    return renderer;
}

void LIBMATTI_MC_GuiRenderer_Free(LIBMATTI_MC_GuiRenderer *renderer)
{
    if (renderer == NULL)
        return;
    if (renderer->vao != 0)
        LIBMATTI_GL_glDeleteVertexArrays(1, &renderer->vao);
    if (renderer->vbo != 0)
        LIBMATTI_GL_glDeleteBuffers(1, &renderer->vbo);
    if (renderer->whiteTexture != 0)
        LIBMATTI_GL_glDeleteTextures(1, &renderer->whiteTexture);
    if (renderer->program != 0)
        LIBMATTI_GL_glDeleteProgram(renderer->program);
    free(renderer->vertices);
    free(renderer);
}

unsigned int LIBMATTI_MC_GuiRenderer_Program(const LIBMATTI_MC_GuiRenderer *renderer)
{
    return renderer != NULL ? renderer->program : 0;
}

void LIBMATTI_MC_GuiRenderer_SetTexture(LIBMATTI_MC_GuiRenderer *renderer, unsigned int texture)
{
    if (renderer == NULL)
        return;
    renderer->boundTexture = texture;
}

unsigned int LIBMATTI_MC_GuiRenderer_GetTexture(const LIBMATTI_MC_GuiRenderer *renderer)
{
    return renderer != NULL ? renderer->boundTexture : 0;
}

static int batch_reserve(LIBMATTI_MC_GuiRenderer *renderer, size_t extra)
{
    if (renderer->vertexCount + extra <= renderer->vertexCapacity)
        return 1;
    size_t capacity = renderer->vertexCapacity != 0 ? renderer->vertexCapacity : 256;
    while (capacity < renderer->vertexCount + extra)
        capacity *= 2;
    LIBMATTI_MC_GuiVertex *grown = realloc(renderer->vertices, capacity * sizeof(*grown));
    if (grown == NULL)
        return 0;
    renderer->vertices = grown;
    renderer->vertexCapacity = capacity;
    return 1;
}

static float channel(unsigned int rgba, int shift)
{
    return (float) ((rgba >> shift) & 0xFFu) / 255.0f;
}

// The two triangles over the rect corners (Java: GuiGraphics.blitSprite's
// innerData vertex packing - v0 v1 v2 + v0 v2 v3).
void LIBMATTI_MC_GuiRenderer_PackQuad(LIBMATTI_MC_GuiVertex *out, float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      unsigned int rgba)
{
    float r = channel(rgba, 16), g = channel(rgba, 8), b = channel(rgba, 0), a = channel(rgba, 24);
    out[0] = (LIBMATTI_MC_GuiVertex){x, y, r, g, b, a, u0, v0};
    out[1] = (LIBMATTI_MC_GuiVertex){x, y + height, r, g, b, a, u0, v1};
    out[2] = (LIBMATTI_MC_GuiVertex){x + width, y + height, r, g, b, a, u1, v1};
    out[3] = (LIBMATTI_MC_GuiVertex){x, y, r, g, b, a, u0, v0};
    out[4] = (LIBMATTI_MC_GuiVertex){x + width, y + height, r, g, b, a, u1, v1};
    out[5] = (LIBMATTI_MC_GuiVertex){x + width, y, r, g, b, a, u1, v0};
}

void LIBMATTI_MC_GuiRenderer_BlitQuad(LIBMATTI_MC_GuiRenderer *renderer,
                                      float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      unsigned int rgba)
{
    if (renderer == NULL)
        return;
    if (!batch_reserve(renderer, 6))
        return;

    LIBMATTI_MC_GuiRenderer_PackQuad(renderer->vertices + renderer->vertexCount,
                                     x, y, width, height, u0, v0, u1, v1, rgba);
    renderer->vertexCount += 6;
}

void LIBMATTI_MC_GuiRenderer_Flush(LIBMATTI_MC_GuiRenderer *renderer, float screenSizeX, float screenSizeY)
{
    if (renderer == NULL || renderer->vertexCount == 0)
        return;

    LIBMATTI_GL_glUseProgram(renderer->program);
    if (renderer->screenSizeLocation >= 0)
        LIBMATTI_GL_glUniform2f(renderer->screenSizeLocation, screenSizeX, screenSizeY);
    LIBMATTI_GL_glActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
    // Java: setShaderTexture - the atlas when set, the white 1x1 fallback
    // otherwise (the tint-only quads sample white).
    unsigned int texture = renderer->boundTexture != 0 ? renderer->boundTexture : renderer->whiteTexture;
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, texture);
    LIBMATTI_GL_glUniform1i(LIBMATTI_GL_glGetUniformLocation(renderer->program, "Sampler0"), 0);

    LIBMATTI_GL_glBindVertexArray(renderer->vao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, renderer->vbo);
    long bytes = (long) (renderer->vertexCount * sizeof(LIBMATTI_MC_GuiVertex));
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, bytes, renderer->vertices, LIBMATTI_GL_GL_DYNAMIC_DRAW);
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, (int) renderer->vertexCount);

    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    // Java: the GUI pass ends with its program unbound (RenderSystem's pass
    // teardown) - a live program here would leak into the next frame's world
    // draws (the same leak class the font path already avoids).
    LIBMATTI_GL_glUseProgram(0);
    renderer->vertexCount = 0;
}
