// Port of org.lwjgl.stb.STBTruetype (the slice the earlydisplay font path uses).
// The calls map 1:1 onto the vendored stb_truetype.h (vendor/stb); the wrapper
// only mirrors LWJGL's Java-facing shape so ports of LWJGL consumers stay 1:1.
//
// Java: stbtt_InitFont(info, buffer) / stbtt_PackBegin / stbtt_PackFontRanges /
//       stbtt_GetPackedQuad / stbtt_GetScaledFontVMetrics / stbtt_PackSetOversampling /
//       stbtt_PackSetSkipMissingCodepoints

#ifndef MATTICRAFT_ORG_LWJGL_STB_STBTRUETYPE_H
#define MATTICRAFT_ORG_LWJGL_STB_STBTRUETYPE_H

#include "libmatti/defines.h"

// The vendored upstream header lives next to the binding (vendor/stb on the
// include path; same layout when installed).
#include <stb_truetype.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: STBTTFontinfo / STBTTPackedchar.Buffer / STBTTPackContext / STBTTAlignedQuad
typedef stbtt_fontinfo LIBMATTI_STBTT_Fontinfo;
typedef stbtt_packedchar LIBMATTI_STBTT_Packedchar;
typedef stbtt_pack_context LIBMATTI_STBTT_PackContext;

// Java: STBTTAlignedQuad - x0/y0/x1/y1 quad position, s0/t0/s1/t1 texture coords
typedef struct LIBMATTI_STBTT_AlignedQuad
{
    float x0, y0, s0, t0;
    float x1, y1, s1, t1;
} LIBMATTI_STBTT_AlignedQuad;

// Java: stbtt_InitFont(STBTTFontinfo, ByteBuffer) with fontOffset resolved via
// stbtt_GetFontOffsetForIndex(data, 0)
int LIBMATTI_STBTT_InitFont(LIBMATTI_STBTT_Fontinfo *info, const unsigned char *data);

// Java: stbtt_GetScaledFontVMetrics(data, fontIndex, fontSize, ascent, descent, lineGap)
void LIBMATTI_STBTT_GetScaledFontVMetrics(const unsigned char *data, int fontIndex, float fontSize,
                                          float *ascent, float *descent, float *lineGap);

// Java: stbtt_PackBegin(pc, pixels, width, height, stride, padding, allocContext)
int LIBMATTI_STBTT_PackBegin(LIBMATTI_STBTT_PackContext *context, unsigned char *pixels, int width, int height,
                             int strideInBytes, int padding, void *allocContext);

// Java: stbtt_PackEnd(pc)
void LIBMATTI_STBTT_PackEnd(LIBMATTI_STBTT_PackContext *context);

// Java: stbtt_PackSetOversampling(pc, h, v)
void LIBMATTI_STBTT_PackSetOversampling(LIBMATTI_STBTT_PackContext *context, int h, int v);

// Java: stbtt_PackSetSkipMissingCodepoints(pc, skip)
void LIBMATTI_STBTT_PackSetSkipMissingCodepoints(LIBMATTI_STBTT_PackContext *context, int skip);

// Java: STBTTPackRange.set(fontSize, firstUnicodeChar, lastUnicodeChar, numChars,
//       packedChars, padding, oversampling) - one contiguous codepoint range.
// Layout-compatible with stbtt_pack_range (array_of_unicode_codepoints stays
// NULL, the port only packs contiguous ranges like the SimpleFont constructor).
typedef struct LIBMATTI_STBTT_PackRange
{
    float fontSize;
    int firstUnicodeCodepoint;
    int *arrayOfUnicodeCodepoints; // always NULL (Java: firstUnicodeChar form)
    int numChars;
    LIBMATTI_STBTT_Packedchar *packedChars;
    unsigned char oversampleX; // internal use, like stb (Java: set() takes oversampling)
    unsigned char oversampleY;
} LIBMATTI_STBTT_PackRange;

// Java: stbtt_PackFontRanges(pc, data, fontIndex, STBTTPackRange.Buffer)
int LIBMATTI_STBTT_PackFontRanges(LIBMATTI_STBTT_PackContext *context, const unsigned char *data, int fontIndex,
                                  const LIBMATTI_STBTT_PackRange *ranges, int rangeCount);

// Java: stbtt_GetPackedQuad(packedchars, texWidth, texHeight, charIndex, &x, &y, quad, alignToInteger)
void LIBMATTI_STBTT_GetPackedQuad(const LIBMATTI_STBTT_Packedchar *packedChars, int texWidth, int texHeight,
                                  int charIndex, float *x, float *y, LIBMATTI_STBTT_AlignedQuad *quad,
                                  int alignToInteger);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_ORG_LWJGL_STB_STBTRUETYPE_H
