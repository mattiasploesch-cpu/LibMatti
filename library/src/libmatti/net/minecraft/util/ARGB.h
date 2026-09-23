// Port of net.minecraft.util.ARGB (the color helpers MipmapGenerator and
// TextureUtil use).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_ARGB_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_ARGB_H

#include <math.h>

// Java: private ARGB() {}

// Java: public static int alpha(int color)
static inline int LIBMATTI_MC_ARGB_ALPHA(int color)
{
    return color >> 24;
}

// Java: public static int red(int color)
static inline int LIBMATTI_MC_ARGB_RED(int color)
{
    return (color >> 0) & 0xFF;
}

// Java: public static int green(int color)
static inline int LIBMATTI_MC_ARGB_GREEN(int color)
{
    return (color >> 8) & 0xFF;
}

// Java: public static int blue(int color)
static inline int LIBMATTI_MC_ARGB_BLUE(int color)
{
    return (color >> 16) & 0xFF;
}

// Java: public static int color(int alpha, int red, int green, int blue)
static inline int LIBMATTI_MC_ARGB_Color(int alpha, int red, int green, int blue)
{
    return (alpha & 0xFF) << 24 | (blue & 0xFF) << 16 | (green & 0xFF) << 8 | (red & 0xFF);
}

// Converts normalized components to packed bytes by multiplying by 255 and
// truncating. Inputs are not clamped before the low eight bits are packed.
static inline int LIBMATTI_MC_ARGB_ColorFromFloat(float alpha, float red, float green, float blue)
{
    return LIBMATTI_MC_ARGB_Color((int) (alpha * 255.0f), (int) (red * 255.0f), (int) (green * 255.0f), (int) (blue * 255.0f));
}

// Returns the encoded red byte normalized to the range [0, 1].
static inline float LIBMATTI_MC_ARGB_RedFloat(int color)
{
    return (float) LIBMATTI_MC_ARGB_RED(color) / 255.0f;
}

// Returns the encoded green byte normalized to the range [0, 1].
static inline float LIBMATTI_MC_ARGB_GreenFloat(int color)
{
    return (float) LIBMATTI_MC_ARGB_GREEN(color) / 255.0f;
}

// Returns the encoded blue byte normalized to the range [0, 1].
static inline float LIBMATTI_MC_ARGB_BlueFloat(int color)
{
    return (float) LIBMATTI_MC_ARGB_BLUE(color) / 255.0f;
}

// Java: public static int color(int alpha, int rgb)
static inline int LIBMATTI_MC_ARGB_ColorAlpha(int alpha, int rgb)
{
    return alpha << 24 | (rgb & 0xFFFFFF);
}

// Java: public static int opaque(int color)
static inline int LIBMATTI_MC_ARGB_Opaque(int color)
{
    return color | 0xFF000000;
}

// Java: public static int transparent(int color)
static inline int LIBMATTI_MC_ARGB_Transparent(int color)
{
    return color & 0xFFFFFF;
}

// Java: public static float alphaFloat(int color)
static inline float LIBMATTI_MC_ARGB_AlphaFloat(int color)
{
    return (float) LIBMATTI_MC_ARGB_ALPHA(color) / 255.0f;
}

// Java: public static float srgbToLinearChannel(int value)
static inline float LIBMATTI_MC_ARGB_SrgbToLinearChannel(int value)
{
    float f = (float) value / 255.0f;
    return f < 0.04045f ? f / 12.92f : (float) pow((double) ((f + 0.055f) / 1.055f), 2.4);
}

// Java: public static int linearToSrgbChannel(float value)
static inline int LIBMATTI_MC_ARGB_LinearToSrgbChannel(float value)
{
    value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    return value < 0.0031308f ? (int) (12.92f * value * 255.0f + 0.5f)
                              : (int) ((1.055f * (float) pow((double) value, 0.4166666666666667) - 0.055f) * 255.0f + 0.5f);
}

// Java: public static int meanLinear(int a, int b, int c, int d)
static inline int LIBMATTI_MC_ARGB_MeanLinear(int a, int b, int c, int d)
{
    int alpha = (LIBMATTI_MC_ARGB_ALPHA(a) + LIBMATTI_MC_ARGB_ALPHA(b) + LIBMATTI_MC_ARGB_ALPHA(c) + LIBMATTI_MC_ARGB_ALPHA(d)) / 4 + 1;
    int red = (LIBMATTI_MC_ARGB_RED(a) + LIBMATTI_MC_ARGB_RED(b) + LIBMATTI_MC_ARGB_RED(c) + LIBMATTI_MC_ARGB_RED(d)) / 4;
    int green = (LIBMATTI_MC_ARGB_GREEN(a) + LIBMATTI_MC_ARGB_GREEN(b) + LIBMATTI_MC_ARGB_GREEN(c) + LIBMATTI_MC_ARGB_GREEN(d)) / 4;
    int blue = (LIBMATTI_MC_ARGB_BLUE(a) + LIBMATTI_MC_ARGB_BLUE(b) + LIBMATTI_MC_ARGB_BLUE(c) + LIBMATTI_MC_ARGB_BLUE(d)) / 4;
    return LIBMATTI_MC_ARGB_Color(alpha, red, green, blue);
}

#endif //MATTICRAFT_NET_MINECRAFT_UTIL_ARGB_H
