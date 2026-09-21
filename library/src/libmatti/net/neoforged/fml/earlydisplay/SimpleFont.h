// Port of net.neoforged.fml.earlydisplay.render.SimpleFont (the Monocraft.ttf
// bitmap font the earlydisplay renders). Java packs the ASCII range 32..126
// into a 256x128 GL_R8/GL_RED texture through LWJGL's STBTruetype; the port
// drives the same pack path over the vendored stb_truetype and keeps the
// Glyph/DisplayText model.
//
// The port renders with a tiny POS_TEX_COLOR quad batcher ported from
// SimpleBufferBuilder (the slice the font path needs): positions in screen
// pixels, GL handles the projection through the font shader's screenSize
// uniform, exactly like the Java vertex shader (position / screenSize * 2 - 1).

#ifndef MATTICRAFT_FML_EARLYDISPLAY_SIMPLEFONT_H
#define MATTICRAFT_FML_EARLYDISPLAY_SIMPLEFONT_H

#include "libmatti/defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: SimpleFont.Glyph - the packed quad for one codepoint
typedef struct LIBMATTI_FML_SimpleFont_Glyph
{
    int codepoint;   // Java: char c
    int charwidth;   // the x advance (Java: (int)(x[0] - 0f))
    int pos[4];      // Java: int[] pos {x0, y0, x1, y1} (rounded)
    float uv[4];     // Java: float[] uv {s0, t0, s1, t1}
} LIBMATTI_FML_SimpleFont_Glyph;

// Java: SimpleFont.DisplayText - a string with an RGBA packed colour
typedef struct LIBMATTI_FML_SimpleFont_DisplayText
{
    const char *string; // not owned (the caller keeps the buffer alive per call)
    unsigned int colour; // ARGB packed, like ThemeColor.toArgb()
} LIBMATTI_FML_SimpleFont_DisplayText;

typedef struct LIBMATTI_FML_SimpleFont LIBMATTI_FML_SimpleFont;

// Java: new SimpleFont(themeResource, externalThemeDirectory) - the resource
// buffer is the raw TTF bytes; fontSize is 24, the page is 256x128 (the Java
// constants). Returns NULL on a bad font or a failed pack.
LIBMATTI_FML_SimpleFont *LIBMATTI_FML_SimpleFont_New(const unsigned char *ttfData, size_t dataLength);

// Java: public void close() - deletes the texture
void LIBMATTI_FML_SimpleFont_Free(LIBMATTI_FML_SimpleFont *font);

// Java: public int stringWidth(String) - \n and \t count 0
int LIBMATTI_FML_SimpleFont_StringWidth(const LIBMATTI_FML_SimpleFont *font, const char *text);

// Java: public Size measureText(CharSequence) - width/height of the wrapped text
void LIBMATTI_FML_SimpleFont_MeasureText(const LIBMATTI_FML_SimpleFont *font, const char *text,
                                         float *outWidth, float *outHeight);

// Java: public int lineSpacing() / descent() / int textureId()
int LIBMATTI_FML_SimpleFont_LineSpacing(const LIBMATTI_FML_SimpleFont *font);
int LIBMATTI_FML_SimpleFont_Descent(const LIBMATTI_FML_SimpleFont *font);
unsigned int LIBMATTI_FML_SimpleFont_TextureId(const LIBMATTI_FML_SimpleFont *font);

// Java: SimpleFont.Glyph.loadQuad + DisplayText.generateStringArray +
// generateVerticesForTexts collapsed into one draw call (Java splits the same
// three steps across RenderContext.renderText): batch the glyph quads for the
// texts at (x, y), upload and draw. Returns the bytes written (0 when nothing
// was drawn or no GL context exists).
size_t LIBMATTI_FML_SimpleFont_DrawTexts(LIBMATTI_FML_SimpleFont *font, float x, float y,
                                         const LIBMATTI_FML_SimpleFont_DisplayText *texts, size_t textCount);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_FML_EARLYDISPLAY_SIMPLEFONT_H
