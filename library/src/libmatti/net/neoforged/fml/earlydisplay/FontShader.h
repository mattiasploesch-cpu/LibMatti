







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






unsigned int LIBMATTI_FML_FontShader_Compile(int *screenSizeLocation);

#endif
