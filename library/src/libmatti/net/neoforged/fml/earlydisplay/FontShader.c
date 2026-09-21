// Port of the earlydisplay font shader compilation (Java: ElementShader's
// compileProgram over the theme resources - the port embeds the same sources,
// see FontShader.h).

#include "libmatti/net/neoforged/fml/earlydisplay/FontShader.h"

#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdio.h>

// Java: ElementShader.compileProgram checks the compile/link status and logs
// the info log; the port reads it through the binding's log getters.
static int compile_ok(unsigned int shader, const char *label)
{
    if (!LIBMATTI_GL_glGetShaderiv(shader, LIBMATTI_GL_GL_COMPILE_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetShaderInfoLog(shader);
        fprintf(stderr, "[FONTSHADER] ERROR: %s: %s\n", label,
                log != NULL && log[0] != '\0' ? log : "compile failed");
        return 0;
    }
    return 1;
}

unsigned int LIBMATTI_FML_FontShader_Compile(int *screenSizeLocation)
{
    if (screenSizeLocation != NULL)
        *screenSizeLocation = -1;

    unsigned int vertex = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_VERTEX_SHADER);
    unsigned int fragment = LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_FRAGMENT_SHADER);
    if (vertex == 0 || fragment == 0)
        return 0;

    LIBMATTI_GL_glShaderSource(vertex, MATTI_FML_FONT_VERT);
    LIBMATTI_GL_glShaderSource(fragment, MATTI_FML_FONT_FRAG);
    LIBMATTI_GL_glCompileShader(vertex);
    LIBMATTI_GL_glCompileShader(fragment);

    if (!compile_ok(vertex, "font vertex shader") || !compile_ok(fragment, "font fragment shader"))
    {
        LIBMATTI_GL_glDeleteShader(vertex);
        LIBMATTI_GL_glDeleteShader(fragment);
        return 0;
    }

    unsigned int program = LIBMATTI_GL_glCreateProgram();
    LIBMATTI_GL_glAttachShader(program, vertex);
    LIBMATTI_GL_glAttachShader(program, fragment);

    // Java: gui.vert's attribute names (the port binds them to the batcher's
    // fixed slots 0/1/2 - POS/TEX/COLOR, see SimpleFont's VAO setup).
    LIBMATTI_GL_glBindAttribLocation(program, 0, "position");
    LIBMATTI_GL_glBindAttribLocation(program, 1, "uv");
    LIBMATTI_GL_glBindAttribLocation(program, 2, "color");

    LIBMATTI_GL_glLinkProgram(program);
    LIBMATTI_GL_glDeleteShader(vertex);
    LIBMATTI_GL_glDeleteShader(fragment);
    if (!LIBMATTI_GL_glGetProgramiv(program, LIBMATTI_GL_GL_LINK_STATUS))
    {
        const char *log = LIBMATTI_GL_glGetProgramInfoLog(program);
        fprintf(stderr, "[FONTSHADER] ERROR: font program link failed: %s\n",
                log != NULL && log[0] != '\0' ? log : "unknown reason");
        LIBMATTI_GL_glDeleteProgram(program);
        return 0;
    }

    if (screenSizeLocation != NULL)
        *screenSizeLocation = LIBMATTI_GL_glGetUniformLocation(program, "screenSize");
    return program;
}
