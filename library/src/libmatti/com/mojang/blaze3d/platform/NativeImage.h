// Port of com.mojang.blaze3d.platform.NativeImage (the raster behind every
// texture). Java decodes through STB; the port routes the PNG container through
// the javax.imageio PngReader port (zlib inflate), the only format the resource
// pipeline feeds in. Pixels are RGBA, stored in ABGR int order like Java's
// getPixel/setPixel int layout.

#ifndef MATTICRAFT_BLAZE3D_PLATFORM_NATIVEIMAGE_H
#define MATTICRAFT_BLAZE3D_PLATFORM_NATIVEIMAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static enum Format - RGBA is the only one the resource pipeline
// produces; the enum keeps the component counts Java's format table has.
typedef enum LIBMATTI_B3D_NativeImage_Format
{
    LIBMATTI_B3D_NativeImageFormat_RGBA = 0
} LIBMATTI_B3D_NativeImage_Format;

// Java: public final class NativeImage implements AutoCloseable
typedef struct LIBMATTI_B3D_NativeImage
{
    LIBMATTI_B3D_NativeImage_Format format;
    int width;
    int height;
    // Java: private long pixels - the port keeps the C pointer directly
    uint32_t *pixels;
} LIBMATTI_B3D_NativeImage;

// Java: public NativeImage(int width, int height, boolean clear)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_New(int width, int height, int clear);
// Java: public NativeImage(Format format, int width, int height, boolean clear)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_NewWithFormat(LIBMATTI_B3D_NativeImage_Format format,
                                                                 int width, int height, int clear);
// Java: private NativeImage(Format, int, int, boolean, long pixels) - takes ownership
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_Wrap(LIBMATTI_B3D_NativeImage_Format format, int width,
                                                        int height, uint32_t *pixels);

// Java: public static NativeImage read(InputStream) - reads the whole stream,
// validates the PNG header like PngInfo.validateHeader and decodes it
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_Read(const unsigned char *data, size_t length);

// Java: public int getWidth() / getHeight()
int LIBMATTI_B3D_NativeImage_GetWidth(const LIBMATTI_B3D_NativeImage *image);
int LIBMATTI_B3D_NativeImage_GetHeight(const LIBMATTI_B3D_NativeImage *image);

// Java: public int getPixel(int x, int y) - ARGB int (Java: getPixel via fromABGR)
int LIBMATTI_B3D_NativeImage_GetPixel(const LIBMATTI_B3D_NativeImage *image, int x, int y);
// Java: public void setPixel(int x, int y, int argb)
void LIBMATTI_B3D_NativeImage_SetPixel(LIBMATTI_B3D_NativeImage *image, int x, int y, int argb);

// Java: public int getPixelABGR(int x, int y)
int LIBMATTI_B3D_NativeImage_GetPixelABGR(const LIBMATTI_B3D_NativeImage *image, int x, int y);
// Java: public void setPixelABGR(int x, int y, int abgr)
void LIBMATTI_B3D_NativeImage_SetPixelABGR(LIBMATTI_B3D_NativeImage *image, int x, int y, int abgr);

// Java: public int[] getPixelsABGR() - malloc'd, caller frees; count = width*height
int *LIBMATTI_B3D_NativeImage_GetPixelsABGR(const LIBMATTI_B3D_NativeImage *image);
// Java: public int[] getPixels() - ARGB order
int *LIBMATTI_B3D_NativeImage_GetPixels(const LIBMATTI_B3D_NativeImage *image);

// Java: public NativeImage mappedCopy(IntUnaryOperator)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_MappedCopy(const LIBMATTI_B3D_NativeImage *image,
                                                              int (*operator_)(int argb));

// Java: public void close()
void LIBMATTI_B3D_NativeImage_Free(LIBMATTI_B3D_NativeImage *image);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_BLAZE3D_PLATFORM_NATIVEIMAGE_H
