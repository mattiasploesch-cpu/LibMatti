// Port of the earlydisplay font pipeline's shader piece (Theme.SHADER_FONT).
// Java ships gui.vert + gui_font.frag as theme resources; the port embeds the
// same GLSL sources (copied 1:1 from
// vendor/FancyModLoader/earlydisplay/src/main/resources/.../theme/) and
// compiles the pair with the GL binding (no file I/O, like Java's
// ElementShader does with its static strings).

#ifndef MATTICRAFT_FML_EARLYDISPLAY_FONTSHADER_H
#define MATTICRAFT_FML_EARLYDISPLAY_FONTSHADER_H

// Java: gui.vert - screen-space positions scaled by the screenSize uniform
#define MATTI_FML_FONT_VERT \
    "#version 150 core\n" \
    "\n" \
    "uniform vec2 screenSize;\n" \
    "in vec2 position;\n" \
    "in vec2 uv;\n" \
    "in vec4 color;\n" \
    "out vec2 fTex;\n" \
    "out vec4 fColour;\n" \
    "\n" \
    "void main() {\n" \
    "    fTex = uv;\n" \
    "    fColour = color;\n" \
    "    gl_Position = vec4((position / screenSize) * 2 - 1, 0.0, 1.0);\n" \
    "}\n"

// Java: gui_font.frag - the glyph texture's red channel tints the vertex colour
#define MATTI_FML_FONT_FRAG \
    "#version 150 core\n" \
    "uniform sampler2D tex;\n" \
    "in vec2 fTex;\n" \
    "in vec4 fColour;\n" \
    "out vec4 fragColor;\n" \
    "\n" \
    "void main() {\n" \
    "    fragColor = texture(tex, fTex).r * fColour;\n" \
    "}\n"

// Compile (and link) the embedded font shader pair; returns the program id
// (0 when no GL context exists or compilation failed, like the binding's
// fallbacks). screenSizeLocation receives the uniform location (-1 when the
// compiler optimized it away or the program is invalid).
unsigned int LIBMATTI_FML_FontShader_Compile(int *screenSizeLocation);

#endif //MATTICRAFT_FML_EARLYDISPLAY_FONTSHADER_H
