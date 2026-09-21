// Port of net.neoforged.fml.earlydisplay.render.SimpleFont (implementation).
// The pack path mirrors the Java constructor line by line: v-metrics at size 24,
// one pack range over codepoints 32..(32+ASCII_GLYPH_COUNT), oversampling 1,
// skip missing codepoints, upload to a GL_R8/GL_RED 256x128 texture, then
// stbtt_GetPackedQuad per glyph to derive pos/uv.

#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"

#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/stb/STBTruetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final int ASCII_GLYPH_COUNT = 127 - 32;
#define ASCII_GLYPH_COUNT (127 - 32)
// Java: int fontSize = 24; int texwidth = 256; int texheight = 128;
#define FONT_SIZE 24
#define TEX_WIDTH 256
#define TEX_HEIGHT 128

// Java: SimpleBufferBuilder.Format POS_TEX_COLOR (POS 2f, TEX 2f, COLOR 4ub)
// stride = 8 + 8 + 4 = 20 bytes, quads drawn as 6 indices each.
#define FONT_STRIDE 20
#define MAX_VERTICES 8192 // Java: new SimpleBufferBuilder("shared", 8192)

struct LIBMATTI_FML_SimpleFont
{
    unsigned int textureId;
    int lineSpacing;
    int descent;
    LIBMATTI_FML_SimpleFont_Glyph glyphs[ASCII_GLYPH_COUNT];

    // The draw-time batcher (Java: RenderContext's shared SimpleBufferBuilder
    // in POS_TEX_COLOR/QUADS): cached VAO/VBO/IBO per process like Java caches
    // VERTEX_ARRAYS/VERTEX_BUFFERS per format.
    unsigned int vao, vbo, ibo;
    unsigned char *vertices;
    unsigned int vertexCount;
};

// ---------------------------------------------------------------------------
// GL helpers (Java: Texture.createEmpty / Texture.writeToTexture)
// ---------------------------------------------------------------------------

static unsigned int texture_create_empty(int width, int height)
{
    unsigned int texId = 0;
    LIBMATTI_GL_glGenTextures(1, &texId);
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, texId);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T, LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    // Java: linearFilter false for the font (Texture.createEmpty(..., false))
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
    // Java: GL_R8 internal, GL_RED external, no initial data
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_R8, width, height, 0,
                             LIBMATTI_GL_GL_RED, LIBMATTI_GL_GL_UNSIGNED_BYTE, NULL);
    return texId;
}

static void texture_write(unsigned int textureId, int width, int height, const unsigned char *pixels)
{
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, textureId);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ROW_LENGTH, width);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_SKIP_PIXELS, 0);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_SKIP_ROWS, 0);
    // Java: components 1 (GL_RED)
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 1);
    LIBMATTI_GL_glTexSubImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, 0, 0, width, height,
                                LIBMATTI_GL_GL_RED, LIBMATTI_GL_GL_UNSIGNED_BYTE, pixels);
}

// ---------------------------------------------------------------------------
// Batcher (Java: SimpleBufferBuilder begin/pos/tex/colour/draw, POS_TEX_COLOR)
// ---------------------------------------------------------------------------

static void batch_reset(LIBMATTI_FML_SimpleFont *font)
{
    if (font->vao != 0)
        return;

    // Java: the VAO/VBO pair is created lazily on the first draw.
    LIBMATTI_GL_glGenVertexArrays(1, &font->vao);
    LIBMATTI_GL_glGenBuffers(1, &font->vbo);
    LIBMATTI_GL_glBindVertexArray(font->vao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, font->vbo);

    // Java: Format.bind() - attribute 0 = POS (2f), 1 = TEX (2f), 2 = COLOR (4ub normalized)
    LIBMATTI_GL_glEnableVertexAttribArray(0);
    LIBMATTI_GL_glVertexAttribPointer(0, 2, LIBMATTI_GL_GL_FLOAT, 0, FONT_STRIDE, (const void *) 0);
    LIBMATTI_GL_glEnableVertexAttribArray(1);
    LIBMATTI_GL_glVertexAttribPointer(1, 2, LIBMATTI_GL_GL_FLOAT, 0, FONT_STRIDE, (const void *) 8);
    LIBMATTI_GL_glEnableVertexAttribArray(2);
    LIBMATTI_GL_glVertexAttribPointer(2, 4, LIBMATTI_GL_GL_UNSIGNED_BYTE, 1, FONT_STRIDE, (const void *) 16);

    // Java: Mode.QUADS keeps an element buffer (2 triangles per quad).
    LIBMATTI_GL_glGenBuffers(1, &font->ibo);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, font->ibo);
    LIBMATTI_GL_glBindVertexArray(0);
}

static size_t batch_pack_quad(LIBMATTI_FML_SimpleFont *font, float x0, float y0, float x1, float y1,
                              float s0, float t0, float s1, float t1, unsigned int argb)
{
    // Java: colour(packedColor) expands ARGB into the 4ub attribute (r, g, b, a).
    const unsigned char a = (unsigned char) ((argb >> 24) & 0xFF);
    const unsigned char r = (unsigned char) ((argb >> 16) & 0xFF);
    const unsigned char g = (unsigned char) ((argb >> 8) & 0xFF);
    const unsigned char b = (unsigned char) (argb & 0xFF);

    // The two floats of POS and TEX + the 4 colour bytes, per vertex.
    float quad[4][5] = {
        {x0, y0, s0, t0, 0},
        {x1, y0, s1, t0, 0},
        {x0, y1, s0, t1, 0},
        {x1, y1, s1, t1, 0},
    };
    unsigned char rgba[4] = {r, g, b, a};

    for (int v = 0; v < 4; v++)
    {
        unsigned char *vertex = font->vertices + font->vertexCount * FONT_STRIDE;
        memcpy(vertex, quad[v], 4 * sizeof(float));
        memcpy(vertex + 16, rgba, 4);
        font->vertexCount++;
    }
    return 4;
}

// ---------------------------------------------------------------------------
// Font construction (Java: the SimpleFont(ThemeResource, Path) constructor)
// ---------------------------------------------------------------------------

LIBMATTI_FML_SimpleFont *LIBMATTI_FML_SimpleFont_New(const unsigned char *ttfData, size_t dataLength)
{
    if (ttfData == NULL || dataLength == 0)
        return NULL;

    LIBMATTI_FML_SimpleFont *font = calloc(1, sizeof(LIBMATTI_FML_SimpleFont));

    // Java: var info = STBTTFontinfo.create(); if (!stbtt_InitFont(info, buf)) throw
    LIBMATTI_STBTT_Fontinfo info;
    if (!LIBMATTI_STBTT_InitFont(&info, ttfData))
    {
        // Java: throw new RuntimeException("Font file invalid")
        fprintf(stderr, "ERROR: SimpleFont: font file invalid\n");
        free(font);
        return NULL;
    }

    // Java: stbtt_GetScaledFontVMetrics(buf, 0, fontSize, ascent, descent, lineGap)
    float ascent = 0, descentF = 0, lineGap = 0;
    LIBMATTI_STBTT_GetScaledFontVMetrics(ttfData, 0, FONT_SIZE, &ascent, &descentF, &lineGap);
    font->lineSpacing = (int) (ascent - descentF + lineGap);
    font->descent = (int) descentF;

    unsigned char *bitmap = malloc(TEX_WIDTH * TEX_HEIGHT);

    // Java: this.textureId = Texture.createEmpty("font texture ...", texwidth, texheight, GL_R8, GL_RED, false)
    font->textureId = texture_create_empty(TEX_WIDTH, TEX_HEIGHT);

    // Java: packRange.set(fontSize, 32, null, ASCII_GLYPH_COUNT, packedchars, (byte)1, (byte)1)
    LIBMATTI_STBTT_Packedchar *packedChars = malloc(ASCII_GLYPH_COUNT * sizeof(LIBMATTI_STBTT_Packedchar));
    LIBMATTI_STBTT_PackRange range = {FONT_SIZE, 32, NULL, ASCII_GLYPH_COUNT, packedChars, 1, 1};

    LIBMATTI_STBTT_PackContext packContext;
    // Java: stbtt_PackBegin(pc, bitmap, texwidth, texheight, 0, 1, NULL)
    if (!LIBMATTI_STBTT_PackBegin(&packContext, bitmap, TEX_WIDTH, TEX_HEIGHT, 0, 1, NULL))
    {
        fprintf(stderr, "ERROR: SimpleFont: stbtt_PackBegin failed\n");
        free(bitmap);
        free(packedChars);
        free(font);
        return NULL;
    }
    LIBMATTI_STBTT_PackSetOversampling(&packContext, 1, 1);
    LIBMATTI_STBTT_PackSetSkipMissingCodepoints(&packContext, 1);
    LIBMATTI_STBTT_PackFontRanges(&packContext, ttfData, 0, &range, 1);
    LIBMATTI_STBTT_PackEnd(&packContext);
    texture_write(font->textureId, TEX_WIDTH, TEX_HEIGHT, bitmap);
    free(bitmap);

    // Java: stbtt_GetPackedQuad per glyph (x resets to 0, y starts at fontSize)
    for (int i = 0; i < ASCII_GLYPH_COUNT; i++)
    {
        float x = 0.0f, y = (float) FONT_SIZE;
        LIBMATTI_STBTT_AlignedQuad q;
        LIBMATTI_STBTT_GetPackedQuad(packedChars, TEX_WIDTH, TEX_HEIGHT, i, &x, &y, &q, 1);
        LIBMATTI_FML_SimpleFont_Glyph *glyph = &font->glyphs[i];
        glyph->codepoint = 32 + i;
        glyph->charwidth = (int) x; // Java: (int) (x[0] - 0f)
        glyph->pos[0] = (int) q.x0;
        glyph->pos[1] = (int) q.y0;
        glyph->pos[2] = (int) q.x1;
        glyph->pos[3] = (int) q.y1;
        glyph->uv[0] = q.s0;
        glyph->uv[1] = q.t0;
        glyph->uv[2] = q.s1;
        glyph->uv[3] = q.t1;
    }
    free(packedChars);

    font->vertices = malloc(MAX_VERTICES * FONT_STRIDE);
    return font;
}

void LIBMATTI_FML_SimpleFont_Free(LIBMATTI_FML_SimpleFont *font)
{
    if (font == NULL)
        return;
    if (font->textureId != 0)
    {
        LIBMATTI_GL_glDeleteTextures(1, &font->textureId);
        font->textureId = 0;
    }
    if (font->vao != 0)
    {
        LIBMATTI_GL_glDeleteVertexArrays(1, &font->vao);
        LIBMATTI_GL_glDeleteBuffers(1, &font->vbo);
        LIBMATTI_GL_glDeleteBuffers(1, &font->ibo);
    }
    free(font->vertices);
    free(font);
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

static const LIBMATTI_FML_SimpleFont_Glyph *get_glyph(const LIBMATTI_FML_SimpleFont *font, int codepoint)
{
    // Java: codepoint < ' ' || codepoint - ' ' > ASCII_GLYPH_COUNT -> null
    if (codepoint < 32 || codepoint - 32 >= ASCII_GLYPH_COUNT)
        return NULL;
    return &font->glyphs[codepoint - 32];
}

static const LIBMATTI_FML_SimpleFont_Glyph *get_space_glyph(const LIBMATTI_FML_SimpleFont *font)
{
    return get_glyph(font, ' ');
}

int LIBMATTI_FML_SimpleFont_StringWidth(const LIBMATTI_FML_SimpleFont *font, const char *text)
{
    // Java: stringWidth - \n and \t count 0, the glyph advance otherwise
    int length = 0;
    for (const unsigned char *p = (const unsigned char *) text; *p != '\0'; p++)
    {
        int c = *p;
        if (c == '\n' || c == '\t')
            continue;
        const LIBMATTI_FML_SimpleFont_Glyph *glyph = get_glyph(font, c);
        if (glyph != NULL)
            length += glyph->charwidth;
    }
    return length;
}

void LIBMATTI_FML_SimpleFont_MeasureText(const LIBMATTI_FML_SimpleFont *font, const char *text,
                                         float *outWidth, float *outHeight)
{
    // Java: measureText - width accumulates advances, \n wraps, \t is 4 spaces,
    // height = y + descent (+ lineSpacing when the last line is not empty)
    float width = 0.0f, height = 0.0f, x = 0.0f, y = 0.0f;
    for (const unsigned char *p = (const unsigned char *) text; *p != '\0'; p++)
    {
        int c = *p;
        if (c == '\n')
        {
            if (x > width) width = x;
            x = 0.0f;
            y += (float) font->lineSpacing;
        }
        else if (c == '\t')
        {
            x += (float) (get_space_glyph(font)->charwidth * 4);
        }
        else
        {
            const LIBMATTI_FML_SimpleFont_Glyph *glyph = get_glyph(font, c);
            if (glyph != NULL)
                x += (float) glyph->charwidth;
        }
    }
    if (x > width) width = x;
    height = y + (float) font->descent;
    if (x > 0.0f)
        height += (float) font->lineSpacing;
    if (outWidth != NULL) *outWidth = width;
    if (outHeight != NULL) *outHeight = height;
}

int LIBMATTI_FML_SimpleFont_LineSpacing(const LIBMATTI_FML_SimpleFont *font)
{
    return font->lineSpacing;
}

int LIBMATTI_FML_SimpleFont_Descent(const LIBMATTI_FML_SimpleFont *font)
{
    return font->descent;
}

unsigned int LIBMATTI_FML_SimpleFont_TextureId(const LIBMATTI_FML_SimpleFont *font)
{
    return font->textureId;
}

// ---------------------------------------------------------------------------
// Drawing (Java: Glyph.loadQuad + generateVerticesForTexts + SimpleBufferBuilder.draw)
// ---------------------------------------------------------------------------

size_t LIBMATTI_FML_SimpleFont_DrawTexts(LIBMATTI_FML_SimpleFont *font, float x, float y,
                                         const LIBMATTI_FML_SimpleFont_DisplayText *texts, size_t textCount)
{
    // The batcher's VAO/VBO pair needs a GL context (Java: SimpleBufferBuilder
    // throws on draw without one); the glyph quad packing itself is CPU work and
    // runs either way, so the return value reports the packed bytes regardless.
    if (font->vao == 0)
        batch_reset(font);

    font->vertexCount = 0;
    float penX = x, penY = y;
    float minX = x;

    for (size_t t = 0; t < textCount; t++)
    {
        // Java: DisplayText.generateStringArray - the per-codepoint switch
        for (const unsigned char *p = (const unsigned char *) texts[t].string; *p != '\0'; p++)
        {
            int c = *p;
            if (c == '\n')
            {
                penX = minX;
                penY += (float) font->lineSpacing;
            }
            else if (c == '\t')
            {
                penX += (float) (get_space_glyph(font)->charwidth * 4);
            }
            else if (c == ' ')
            {
                penX += (float) get_space_glyph(font)->charwidth;
            }
            else
            {
                const LIBMATTI_FML_SimpleFont_Glyph *glyph = get_glyph(font, c);
                if (glyph == NULL)
                    continue;
                // Java: loadQuad - quad positions offset by the pen, then the
                // advance; the port packs the two triangles immediately.
                float x0 = penX + (float) glyph->pos[0];
                float y0 = penY + (float) glyph->pos[1];
                float x1 = penX + (float) glyph->pos[2];
                float y1 = penY + (float) glyph->pos[3];
                batch_pack_quad(font, x0, y0, x1, y1, glyph->uv[0], glyph->uv[1], glyph->uv[2], glyph->uv[3],
                                texts[t].colour);
                penX += (float) glyph->charwidth;
            }
        }
    }
    if (font->vertexCount == 0)
        return 0;
    size_t vertexBytes = font->vertexCount * FONT_STRIDE;

    if (font->vao == 0)
        return vertexBytes; // no GL context: the packed quads are the result

    // Java: draw() - upload the vertex data and draw the quads as triangles.
    LIBMATTI_GL_glBindVertexArray(font->vao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, font->vbo);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, (long) vertexBytes, font->vertices,
                             LIBMATTI_GL_GL_DYNAMIC_DRAW);

    // Java: Mode.QUADS -> vertices + vertices/2 indices (6 per quad).
    unsigned int quadCount = font->vertexCount / 4;
    unsigned int indexCount = quadCount * 6;
    unsigned int *indices = malloc(indexCount * sizeof(unsigned int));
    for (unsigned int q = 0; q < quadCount; q++)
    {
        unsigned int base = q * 4;
        unsigned int *target = indices + q * 6;
        target[0] = base;
        target[1] = base + 1;
        target[2] = base + 2;
        target[3] = base + 2;
        target[4] = base + 1;
        target[5] = base + 3;
    }
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, font->ibo);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, (long) (indexCount * sizeof(unsigned int)),
                             indices, LIBMATTI_GL_GL_DYNAMIC_DRAW);
    free(indices);

    LIBMATTI_GL_glDrawElements(LIBMATTI_GL_GL_TRIANGLES, (int) indexCount, LIBMATTI_GL_GL_UNSIGNED_INT, NULL);
    LIBMATTI_GL_glBindVertexArray(0);

    return vertexBytes;
}
