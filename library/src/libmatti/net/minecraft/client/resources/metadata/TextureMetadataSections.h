// Port of net.minecraft.client.resources.metadata.texture.TextureMetadataSection,
// net.minecraft.client.resources.metadata.animation.AnimationMetadataSection,
// net.minecraft.client.resources.metadata.animation.AnimationFrame and
// net.minecraft.client.resources.metadata.animation.FrameSize.
// Java parses them through DataResult codecs; the port reads the parsed JSON
// object directly (the same fields the codec declarations name).

#ifndef MATTICRAFT_MC_CLIENT_METADATA_TEXTURE_SECTIONS_H
#define MATTICRAFT_MC_CLIENT_METADATA_TEXTURE_SECTIONS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ---------------------------------------------------------------------------
// TextureMetadataSection("texture")
// ---------------------------------------------------------------------------

// Java: record TextureMetadataSection(boolean blur, boolean clamp,
//       MipmapStrategy mipmapStrategy, float alphaCutoffBias)
typedef struct LIBMATTI_MC_TextureMetadataSection
{
    // Java: DEFAULT_BLUR = false, DEFAULT_CLAMP = false,
    //       DEFAULT_ALPHA_CUTOFF_BIAS = 0.0F
    int blur;
    int clamp;
    // Java: MipmapStrategy - AUTO is the default; the mip strategies the port
    // keeps: AUTO, MEAN, CUTOUT, STRICT_CUTOUT, DARK_CUTOUT
    int mipmapStrategy; // 0=AUTO 1=MEAN 2=CUTOUT 3=STRICT_CUTOUT 4=DARK_CUTOUT
    float alphaCutoffBias;
} LIBMATTI_MC_TextureMetadataSection;

// Java: MipmapStrategy.CODEC - parses "auto"/"mean"/"cutout"/"strict_cutout"/"dark_cutout"
int LIBMATTI_MC_MipmapStrategy_FromName(const char *name, int *outStrategy);
const char *LIBMATTI_MC_MipmapStrategy_ToName(int strategy);

// Java: the TextureMetadataSection.TYPE "texture" section decoder - reads the
// parsed JSON object; returns NULL and sets outError on a malformed section.
LIBMATTI_MC_TextureMetadataSection *LIBMATTI_MC_TextureMetadataSection_Parse(const void *jsonObject, char **outError);
void LIBMATTI_MC_TextureMetadataSection_Free(LIBMATTI_MC_TextureMetadataSection *section);

// ---------------------------------------------------------------------------
// AnimationMetadataSection("animation")
// ---------------------------------------------------------------------------

// Java: record AnimationFrame(int index, int time)
typedef struct LIBMATTI_MC_AnimationFrame
{
    int index;
    // Java: timeOr(defaultTime) resolves to this at parse time in the port
    int time;
} LIBMATTI_MC_AnimationFrame;

// Java: record FrameSize(int width, int height)
typedef struct LIBMATTI_MC_FrameSize
{
    int width;
    int height;
} LIBMATTI_MC_FrameSize;

// Java: record AnimationMetadataSection(Optional<List<AnimationFrame>> frames,
//       Optional<Integer> frameWidth, Optional<Integer> frameHeight,
//       int defaultFrameTime, boolean interpolatedFrames)
typedef struct LIBMATTI_MC_AnimationMetadataSection
{
    // Java: Optional<List<AnimationFrame>> - hasFrames=0 means "use all frames"
    int hasFrames;
    LIBMATTI_MC_AnimationFrame *frames;
    size_t frameCount;

    int hasFrameWidth;
    int frameWidth;
    int hasFrameHeight;
    int frameHeight;

    // Java: frametime defaults to 1
    int defaultFrameTime;
    int interpolatedFrames;
} LIBMATTI_MC_AnimationMetadataSection;

// Java: AnimationMetadataSection.calculateFrameSize(imageWidth, imageHeight)
LIBMATTI_MC_FrameSize LIBMATTI_MC_AnimationMetadataSection_CalculateFrameSize(
    const LIBMATTI_MC_AnimationMetadataSection *section, int imageWidth, int imageHeight);

// Java: the AnimationMetadataSection.TYPE "animation" section decoder
LIBMATTI_MC_AnimationMetadataSection *LIBMATTI_MC_AnimationMetadataSection_Parse(const void *jsonObject, char **outError);
void LIBMATTI_MC_AnimationMetadataSection_Free(LIBMATTI_MC_AnimationMetadataSection *section);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_METADATA_TEXTURE_SECTIONS_H
