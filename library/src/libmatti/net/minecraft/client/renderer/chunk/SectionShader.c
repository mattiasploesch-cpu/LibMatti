// The terrain draw program (see SectionShader.h). Java's core shaders live in
// assets/minecraft/shaders/core/position_color; the port embeds the same
// attribute set the BLOCK format carries (position, color, uv0, uv2, normal).

#include "libmatti/net/minecraft/client/renderer/chunk/SectionShader.h"

#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdio.h>

// Java: position_color.vsh - the MVP transform, the block offset as the model
// origin, the lightmap coordinate through.
static const char *TERRAIN_VERT =
    "#version 150\n"
    "uniform mat4 mvp;\n"
    "uniform vec3 origin;\n"
    "in vec3 position;\n"
    "in vec4 color;\n"
    "in vec2 uv0;\n"
    "in vec2 uv2;\n"
    "in vec3 normal;\n"
    "out vec4 vtxColor;\n"
    "out vec2 vtxUv0;\n"
    "out vec2 vtxUv2;\n"
    "void main() {\n"
    "    gl_Position = mvp * vec4(position + origin, 1.0);\n"
    "    vtxColor = color;\n"
    "    vtxUv0 = uv0;\n"
    "    vtxUv2 = uv2;\n"
    "}\n";

// Java: block.fsh - the atlas sample multiplied by the vertex colour (the
// AO/light tint rides in the vertex color, the lightmap is folded there too).
static const char *TERRAIN_FRAG =
    "#version 150\n"
    "in vec4 vtxColor;\n"
    "in vec2 vtxUv0;\n"
    "in vec2 vtxUv2;\n"
    "uniform sampler2D Sampler0;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    fragColor = texture(Sampler0, vtxUv0) * vtxColor;\n"
    "}\n";

static int compile_ok(unsigned int shader, const char *label)
{
    if (!LIBMATTI_GL_glGetShaderiv(shader, LIBMATTI_GL_GL_COMPILE_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetShaderInfoLog(shader);
        fprintf(stderr, "[SECTIONSHADER] ERROR: %s: %s\n", label,
                log != NULL && log[0] != '\0' ? log : "compile failed");
        return 0;
    }
    return 1;
}

unsigned int LIBMATTI_MC_SectionShader_Compile(int *mvpLocation, int *originLocation, int *useTextureLocation)
{
    if (mvpLocation != NULL)
        *mvpLocation = -1;
    if (originLocation != NULL)
        *originLocation = -1;
    if (useTextureLocation != NULL)
        *useTextureLocation = -1;

    unsigned int vertex = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_VERTEX_SHADER);
    unsigned int fragment = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_FRAGMENT_SHADER);
    if (vertex == 0 || fragment == 0)
        return 0;

    LIBMATTI_GL_glShaderSource(vertex, TERRAIN_VERT);
    LIBMATTI_GL_glShaderSource(fragment, TERRAIN_FRAG);
    LIBMATTI_GL_glCompileShader(vertex);
    LIBMATTI_GL_glCompileShader(fragment);

    if (!compile_ok(vertex, "terrain vertex shader") || !compile_ok(fragment, "terrain fragment shader"))
    {
        LIBMATTI_GL_glDeleteShader(vertex);
        LIBMATTI_GL_glDeleteShader(fragment);
        return 0;
    }

    unsigned int program = LIBMATTI_GL_glCreateProgram();
    LIBMATTI_GL_glAttachShader(program, vertex);
    LIBMATTI_GL_glAttachShader(program, fragment);

    // Java: the BLOCK format's attribute order - the port binds the VAO slots
    // 0..4 (position, color, uv0, uv2, normal).
    LIBMATTI_GL_glBindAttribLocation(program, 0, "position");
    LIBMATTI_GL_glBindAttribLocation(program, 1, "color");
    LIBMATTI_GL_glBindAttribLocation(program, 2, "uv0");
    LIBMATTI_GL_glBindAttribLocation(program, 3, "uv2");
    LIBMATTI_GL_glBindAttribLocation(program, 4, "normal");

    LIBMATTI_GL_glLinkProgram(program);
    LIBMATTI_GL_glDeleteShader(vertex);
    LIBMATTI_GL_glDeleteShader(fragment);
    if (!LIBMATTI_GL_glGetProgramiv(program, LIBMATTI_GL_GL_LINK_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetProgramInfoLog(program);
        fprintf(stderr, "[SECTIONSHADER] ERROR: terrain program link failed: %s\n",
                log != NULL && log[0] != '\0' ? log : "unknown reason");
        LIBMATTI_GL_glDeleteProgram(program);
        return 0;
    }

    if (mvpLocation != NULL)
        *mvpLocation = LIBMATTI_GL_glGetUniformLocation(program, "mvp");
    if (originLocation != NULL)
        *originLocation = LIBMATTI_GL_glGetUniformLocation(program, "origin");
    if (useTextureLocation != NULL)
        *useTextureLocation = LIBMATTI_GL_glGetUniformLocation(program, "useTexture");
    return program;
}
