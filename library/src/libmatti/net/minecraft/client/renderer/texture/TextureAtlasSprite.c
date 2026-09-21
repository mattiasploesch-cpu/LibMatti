// Implementation of SpriteContents, TextureAtlasSprite and the
// MipmapGenerator (the mean/cutout/dark_cutout strategies over ARGB pixels).

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"

#include "libmatti/com/mojang/blaze3d/platform/TextureUtil.h"
#include "libmatti/net/minecraft/util/ARGB.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final float ALPHA_CUTOFF = 0.5F, STRICT_ALPHA_CUTOFF = 0.3F
#define ALPHA_CUTOFF 0.5f
#define STRICT_ALPHA_CUTOFF 0.3f

// ---------------------------------------------------------------------------
// SpriteContents
// ---------------------------------------------------------------------------

LIBMATTI_MC_SpriteContents *LIBMATTI_MC_SpriteContents_New(LIBMATTI_MC_Identifier *name, LIBMATTI_MC_FrameSize frameSize,
                                                           LIBMATTI_B3D_NativeImage *image,
                                                           LIBMATTI_MC_AnimationMetadataSection *animation)
{
    LIBMATTI_MC_SpriteContents *contents = calloc(1, sizeof(LIBMATTI_MC_SpriteContents));
    contents->name = name;
    contents->width = frameSize.width;
    contents->height = frameSize.height;
    contents->originalImage = image;
    contents->byMipLevel = malloc(sizeof(LIBMATTI_B3D_NativeImage *));
    contents->byMipLevel[0] = image;
    contents->mipLevelCount = 1;
    // Java: MipmapStrategy.AUTO default, alphaCutoffBias 0.0F
    contents->mipmapStrategy = 0;
    contents->alphaCutoffBias = 0.0f;

    // Java: the AnimatedTexture - the port keeps the frame layout:
    // the animation divides the image into the frame grid.
    if (animation != NULL)
    {
        int columns = image->width / frameSize.width;
        int rows = image->height / frameSize.height;
        contents->frameCount = animation->hasFrames ? (int) animation->frameCount : columns * rows;
        contents->isAnimated = contents->frameCount > 1;
        contents->defaultFrameTime = animation->defaultFrameTime;
        contents->interpolatedFrames = animation->interpolatedFrames;
    }
    else
    {
        contents->frameCount = 1;
        contents->isAnimated = 0;
    }
    return contents;
}

// Java: MipmapGenerator.hasTransparentPixel
static int has_transparent_pixel(const LIBMATTI_B3D_NativeImage *image)
{
    for (int x = 0; x < image->width; x++)
    {
        for (int y = 0; y < image->height; y++)
        {
            if (LIBMATTI_MC_ARGB_ALPHA(LIBMATTI_B3D_NativeImage_GetPixel(image, x, y)) == 0)
                return 1;
        }
    }
    return 0;
}

// Java: MipmapGenerator.alphaTestCoverage(image, cutoff, alphaScale)
static float alpha_test_coverage(const LIBMATTI_B3D_NativeImage *image, float cutoff, float alphaScale)
{
    int width = image->width;
    int height = image->height;
    float total = 0.0f;
    for (int y = 0; y < height - 1; y++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            float f1 = LIBMATTI_MC_Mth_Clamp(LIBMATTI_MC_ARGB_AlphaFloat(LIBMATTI_B3D_NativeImage_GetPixel(image, x, y)) * alphaScale, 0.0f, 1.0f);
            float f2 = LIBMATTI_MC_Mth_Clamp(LIBMATTI_MC_ARGB_AlphaFloat(LIBMATTI_B3D_NativeImage_GetPixel(image, x + 1, y)) * alphaScale, 0.0f, 1.0f);
            float f3 = LIBMATTI_MC_Mth_Clamp(LIBMATTI_MC_ARGB_AlphaFloat(LIBMATTI_B3D_NativeImage_GetPixel(image, x, y + 1)) * alphaScale, 0.0f, 1.0f);
            float f4 = LIBMATTI_MC_Mth_Clamp(LIBMATTI_MC_ARGB_AlphaFloat(LIBMATTI_B3D_NativeImage_GetPixel(image, x + 1, y + 1)) * alphaScale, 0.0f, 1.0f);
            float covered = 0.0f;
            for (int j1 = 0; j1 < 4; j1++)
            {
                float fy = (j1 + 0.5f) / 4.0f;
                for (int k1 = 0; k1 < 4; k1++)
                {
                    float fx = (k1 + 0.5f) / 4.0f;
                    float f8 = f1 * (1.0f - fx) * (1.0f - fy) + f2 * fx * (1.0f - fy) + f3 * (1.0f - fx) * fy + f4 * fx * fy;
                    if (f8 > cutoff)
                        covered++;
                }
            }
            total += covered / 16.0f;
        }
    }
    return total / (float) ((width - 1) * (height - 1));
}

// Java: MipmapGenerator.scaleAlphaToCoverage(image, targetCoverage, cutoff, bias)
static void scale_alpha_to_coverage(LIBMATTI_B3D_NativeImage *image, float targetCoverage, float cutoff, float bias)
{
    float low = 0.0f;
    float high = 4.0f;
    float scale = 1.0f;
    float bestScale = 1.0f;
    float bestDelta = 3.4028235e38f;

    for (int i = 0; i < 5; i++)
    {
        float coverage = alpha_test_coverage(image, cutoff, scale);
        float delta = coverage - targetCoverage;
        if (delta < 0.0f)
            delta = -delta;
        if (delta < bestDelta)
        {
            bestDelta = delta;
            bestScale = scale;
        }

        if (coverage < targetCoverage)
        {
            low = scale;
        }
        else
        {
            if (!(coverage > targetCoverage))
                break;
            high = scale;
        }
        scale = (low + high) * 0.5f;
    }

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int pixel = LIBMATTI_B3D_NativeImage_GetPixel(image, x, y);
            float alpha = LIBMATTI_MC_ARGB_AlphaFloat(pixel);
            alpha = alpha * scale + bias + 0.025f;
            alpha = LIBMATTI_MC_Mth_Clamp(alpha, 0.0f, 1.0f);
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, LIBMATTI_MC_ARGB_ColorAlpha((int) (alpha * 255.0f), pixel & 0xFFFFFF));
        }
    }
}

// Java: MipmapGenerator.darkenedAlphaBlend
static int darkened_alpha_blend(int a, int b, int c, int d)
{
    float f = 0.0f, f1 = 0.0f, f2 = 0.0f, f3 = 0.0f;
    int pixels[4] = {a, b, c, d};
    for (int i = 0; i < 4; i++)
    {
        int pixel = pixels[i];
        if (LIBMATTI_MC_ARGB_ALPHA(pixel) != 0)
        {
            f += LIBMATTI_MC_ARGB_SrgbToLinearChannel(LIBMATTI_MC_ARGB_ALPHA(pixel));
            f1 += LIBMATTI_MC_ARGB_SrgbToLinearChannel(LIBMATTI_MC_ARGB_RED(pixel));
            f2 += LIBMATTI_MC_ARGB_SrgbToLinearChannel(LIBMATTI_MC_ARGB_GREEN(pixel));
            f3 += LIBMATTI_MC_ARGB_SrgbToLinearChannel(LIBMATTI_MC_ARGB_BLUE(pixel));
        }
    }
    float count = 4.0f;
    int alpha = LIBMATTI_MC_ARGB_LinearToSrgbChannel(f / count);
    int red = LIBMATTI_MC_ARGB_LinearToSrgbChannel(f1 / count);
    int green = LIBMATTI_MC_ARGB_LinearToSrgbChannel(f2 / count);
    int blue = LIBMATTI_MC_ARGB_LinearToSrgbChannel(f3 / count);
    return LIBMATTI_MC_ARGB_Color(alpha, red, green, blue);
}

// Java: public void increaseMipLevel(int) / MipmapGenerator.generateMipLevels
void LIBMATTI_MC_SpriteContents_IncreaseMipLevel(LIBMATTI_MC_SpriteContents *contents, int mipLevel)
{
    // Java: resolve AUTO from the transparent pixel
    int strategy = contents->mipmapStrategy;
    if (strategy == 0)
        strategy = has_transparent_pixel(contents->byMipLevel[0]) ? 2 : 1; // CUTOUT : MEAN

    if (contents->mipLevelCount == 1 && !contents->originalImage->width) { /* unreachable guard */ }

    if ((size_t) (mipLevel + 1) <= contents->mipLevelCount)
        return;

    LIBMATTI_B3D_NativeImage **mips = calloc((size_t) mipLevel + 1, sizeof(LIBMATTI_B3D_NativeImage *));
    mips[0] = contents->byMipLevel[0];
    int cutoutFamily = strategy == 2 || strategy == 3 || strategy == 4;
    float cutoff = strategy == 3 ? STRICT_ALPHA_CUTOFF : ALPHA_CUTOFF;
    float targetCoverage = cutoutFamily ? alpha_test_coverage(mips[0], cutoff, 1.0f) : 0.0f;

    for (int i = 1; i <= mipLevel; i++)
    {
        if ((size_t) i < contents->mipLevelCount)
        {
            mips[i] = contents->byMipLevel[i];
        }
        else
        {
            LIBMATTI_B3D_NativeImage *src = mips[i - 1];
            LIBMATTI_B3D_NativeImage *dst =
                LIBMATTI_B3D_NativeImage_New(src->width >> 1, src->height >> 1, 0);
            for (int y = 0; y < dst->height; y++)
            {
                for (int x = 0; x < dst->width; x++)
                {
                    int j1 = LIBMATTI_B3D_NativeImage_GetPixel(src, x * 2 + 0, y * 2 + 0);
                    int k1 = LIBMATTI_B3D_NativeImage_GetPixel(src, x * 2 + 1, y * 2 + 0);
                    int l1 = LIBMATTI_B3D_NativeImage_GetPixel(src, x * 2 + 0, y * 2 + 1);
                    int i2 = LIBMATTI_B3D_NativeImage_GetPixel(src, x * 2 + 1, y * 2 + 1);
                    int mean = strategy == 4 ? darkened_alpha_blend(j1, k1, l1, i2)
                                             : LIBMATTI_MC_ARGB_MeanLinear(j1, k1, l1, i2);
                    LIBMATTI_B3D_NativeImage_SetPixel(dst, x, y, mean);
                }
            }
            mips[i] = dst;
        }

        // Java: if (flag) scaleAlphaToCoverage(anativeimage[i], f1, f, bias)
        if (cutoutFamily)
            scale_alpha_to_coverage(mips[i], targetCoverage, cutoff, contents->alphaCutoffBias);
    }

    // Java: the old array is replaced; the images are owned by the contents
    free(contents->byMipLevel);
    contents->byMipLevel = mips;
    contents->mipLevelCount = (size_t) mipLevel + 1;
}

void LIBMATTI_MC_SpriteContents_Free(LIBMATTI_MC_SpriteContents *contents)
{
    if (contents == NULL)
        return;
    // Java: close() closes every byMipLevel image; the port frees them all
    for (size_t i = 0; i < contents->mipLevelCount; i++)
        LIBMATTI_B3D_NativeImage_Free(contents->byMipLevel[i]);
    free(contents->byMipLevel);
    LIBMATTI_MC_Identifier_Free(contents->name);
    free(contents);
}

// ---------------------------------------------------------------------------
// TextureAtlasSprite
// ---------------------------------------------------------------------------

LIBMATTI_MC_TextureAtlasSprite *LIBMATTI_MC_TextureAtlasSprite_New(LIBMATTI_MC_Identifier *atlas,
                                                                   LIBMATTI_MC_SpriteContents *contents,
                                                                   int atlasWidth, int atlasHeight,
                                                                   int x, int y, int padding)
{
    LIBMATTI_MC_TextureAtlasSprite *sprite = calloc(1, sizeof(LIBMATTI_MC_TextureAtlasSprite));
    sprite->atlas = LIBMATTI_MC_Identifier_FromNamespaceAndPath(atlas->namespace, atlas->path);
    sprite->contents = contents;
    sprite->x = x + padding;
    sprite->y = y + padding;
    // Java: this.width = contents.width(); this.height = contents.height()
    // clipped to the atlas bounds (the padding the Stitcher adds is around)
    sprite->width = contents->width;
    sprite->height = contents->height;
    (void) atlasWidth;
    (void) atlasHeight;
    return sprite;
}

int LIBMATTI_MC_TextureAtlasSprite_GetX(const LIBMATTI_MC_TextureAtlasSprite *sprite)
{
    return sprite->x;
}

int LIBMATTI_MC_TextureAtlasSprite_GetY(const LIBMATTI_MC_TextureAtlasSprite *sprite)
{
    return sprite->y;
}

void LIBMATTI_MC_TextureAtlasSprite_Free(LIBMATTI_MC_TextureAtlasSprite *sprite)
{
    if (sprite == NULL)
        return;
    LIBMATTI_MC_SpriteContents_Free(sprite->contents);
    LIBMATTI_MC_Identifier_Free(sprite->atlas);
    free(sprite);
}
