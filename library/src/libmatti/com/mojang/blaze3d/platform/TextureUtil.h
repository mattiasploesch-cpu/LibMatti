// Port of com.mojang.blaze3d.platform.TextureUtil - the image preprocessing
// helpers MipmapGenerator routes to (the GL texture setup the Java file also
// carries lives in the GlStateManager texture path).

#ifndef MATTICRAFT_BLAZE3D_PLATFORM_TEXTUREUTIL_H
#define MATTICRAFT_BLAZE3D_PLATFORM_TEXTUREUTIL_H

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static void solidify(NativeImage) - the BFS that fills fully
// transparent pixels with the nearest opaque colour (alpha 0 preserved)
void LIBMATTI_B3D_TextureUtil_Solidify(LIBMATTI_B3D_NativeImage *image);

// Java: public static void fillEmptyAreasWithDarkColor(NativeImage)
void LIBMATTI_B3D_TextureUtil_FillEmptyAreasWithDarkColor(LIBMATTI_B3D_NativeImage *image);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_BLAZE3D_PLATFORM_TEXTUREUTIL_H
