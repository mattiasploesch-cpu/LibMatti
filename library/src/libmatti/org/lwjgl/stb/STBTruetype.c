// Port of org.lwjgl.stb.STBTruetype (implementation). Every function forwards to
// the vendored stb_truetype.h; the mapping notes quote the LWJGL javadoc behaviour.

#include "libmatti/org/lwjgl/stb/STBTruetype.h"

int LIBMATTI_STBTT_InitFont(LIBMATTI_STBTT_Fontinfo *info, const unsigned char *data)
{
    // Java: stbtt_InitFont(info, buffer) resolves the first font in the file.
    return stbtt_InitFont(info, data, stbtt_GetFontOffsetForIndex(data, 0));
}

void LIBMATTI_STBTT_GetScaledFontVMetrics(const unsigned char *data, int fontIndex, float fontSize,
                                          float *ascent, float *descent, float *lineGap)
{
    stbtt_GetScaledFontVMetrics(data, fontIndex, fontSize, ascent, descent, lineGap);
}

int LIBMATTI_STBTT_PackBegin(LIBMATTI_STBTT_PackContext *context, unsigned char *pixels, int width, int height,
                             int strideInBytes, int padding, void *allocContext)
{
    return stbtt_PackBegin(context, pixels, width, height, strideInBytes, padding, allocContext);
}

void LIBMATTI_STBTT_PackEnd(LIBMATTI_STBTT_PackContext *context)
{
    stbtt_PackEnd(context);
}

void LIBMATTI_STBTT_PackSetOversampling(LIBMATTI_STBTT_PackContext *context, int h, int v)
{
    stbtt_PackSetOversampling(context, h, v);
}

void LIBMATTI_STBTT_PackSetSkipMissingCodepoints(LIBMATTI_STBTT_PackContext *context, int skip)
{
    stbtt_PackSetSkipMissingCodepoints(context, skip);
}

int LIBMATTI_STBTT_PackFontRanges(LIBMATTI_STBTT_PackContext *context, const unsigned char *data, int fontIndex,
                                  const LIBMATTI_STBTT_PackRange *ranges, int rangeCount)
{
    // The C pack range is layout-compatible with stbtt_pack_range (the port keeps
    // the LWJGL field order); the cast mirrors LWJGL's native call. stb does not
    // const-qualify the ranges (it writes the packed result back into them).
    return stbtt_PackFontRanges(context, data, fontIndex, (stbtt_pack_range *) ranges, rangeCount);
}

void LIBMATTI_STBTT_GetPackedQuad(const LIBMATTI_STBTT_Packedchar *packedChars, int texWidth, int texHeight,
                                  int charIndex, float *x, float *y, LIBMATTI_STBTT_AlignedQuad *quad,
                                  int alignToInteger)
{
    // stbtt_GetPackedQuad fills an stbtt_aligned_quad; copy into the port's struct
    // (LWJGL returns the same six floats through the STBTTAlignedQuad buffer).
    stbtt_aligned_quad q;
    stbtt_GetPackedQuad(packedChars, texWidth, texHeight, charIndex, x, y, &q, alignToInteger);
    quad->x0 = q.x0;
    quad->y0 = q.y0;
    quad->s0 = q.s0;
    quad->t0 = q.t0;
    quad->x1 = q.x1;
    quad->y1 = q.y1;
    quad->s1 = q.s1;
    quad->t1 = q.t1;
}
