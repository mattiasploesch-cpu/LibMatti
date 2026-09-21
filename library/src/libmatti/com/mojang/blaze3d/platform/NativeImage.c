// Port of com.mojang.blaze3d.platform.NativeImage.
// Java: STB decode; the port: the javax.imageio PngReader port decodes the PNG
// container through zlib and yields the same RGBA byte raster the STB path
// produces (RGBA byte order), which the ABGR int accessors then interpret.

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"

#include "libmatti/javax/imageio/PngReader.h"

#include <stdlib.h>
#include <string.h>

// Java: public NativeImage(int width, int height, boolean clear)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_New(int width, int height, int clear)
{
    return LIBMATTI_B3D_NativeImage_NewWithFormat(LIBMATTI_B3D_NativeImageFormat_RGBA, width, height, clear);
}

// Java: public NativeImage(Format format, int width, int height, boolean clear)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_NewWithFormat(LIBMATTI_B3D_NativeImage_Format format,
                                                                 int width, int height, int clear)
{
    // Java: if (width > 0 && height > 0) {...} else throw IllegalArgumentException
    if (width <= 0 || height <= 0)
        return NULL;

    LIBMATTI_B3D_NativeImage *image = calloc(1, sizeof(LIBMATTI_B3D_NativeImage));
    image->format = format;
    image->width = width;
    image->height = height;
    // Java: nmemCalloc when clear, nmemAlloc otherwise
    image->pixels = malloc((size_t) width * (size_t) height * 4);
    if (image->pixels == NULL)
    {
        free(image);
        return NULL;
    }
    if (clear)
        memset(image->pixels, 0, (size_t) width * (size_t) height * 4);
    return image;
}

// Java: private NativeImage(Format, int, int, boolean, long pixels)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_Wrap(LIBMATTI_B3D_NativeImage_Format format, int width,
                                                        int height, uint32_t *pixels)
{
    if (width <= 0 || height <= 0 || pixels == NULL)
        return NULL;

    LIBMATTI_B3D_NativeImage *image = calloc(1, sizeof(LIBMATTI_B3D_NativeImage));
    image->format = format;
    image->width = width;
    image->height = height;
    image->pixels = pixels;
    return image;
}

// Java: public static NativeImage read(InputStream) - TextureUtil.readResource
// reads the stream, PngInfo.validateHeader checks the signature, STB decodes.
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_Read(const unsigned char *data, size_t length)
{
    // Java: PngInfo.validateHeader - the 8-byte signature
    if (data == NULL || length < 8
        || data[0] != (unsigned char) 0x89 || data[1] != 'P' || data[2] != 'N' || data[3] != 'G'
        || data[4] != '\r' || data[5] != '\n' || data[6] != (unsigned char) 0x1A || data[7] != '\n')
    {
        return NULL;
    }

    // The PngReader port decodes into an 8-bit RGBA byte raster.
    int width = 0;
    int height = 0;
    unsigned char *decoded = LIBMATTI_JXI_PngReader_Decode(data, length, &width, &height);
    if (decoded == NULL)
        return NULL;

    return LIBMATTI_B3D_NativeImage_Wrap(LIBMATTI_B3D_NativeImageFormat_RGBA, width, height,
                                         (uint32_t *) decoded);
}

int LIBMATTI_B3D_NativeImage_GetWidth(const LIBMATTI_B3D_NativeImage *image)
{
    return image->width;
}

int LIBMATTI_B3D_NativeImage_GetHeight(const LIBMATTI_B3D_NativeImage *image)
{
    return image->height;
}

// Java: private boolean isOutsideBounds(int x, int y)
static int is_outside_bounds(const LIBMATTI_B3D_NativeImage *image, int x, int y)
{
    return x < 0 || x >= image->width || y < 0 || y >= image->height;
}

// Java: public int getPixelABGR(int x, int y)
int LIBMATTI_B3D_NativeImage_GetPixelABGR(const LIBMATTI_B3D_NativeImage *image, int x, int y)
{
    if (is_outside_bounds(image, x, y))
        return 0;
    return (int) image->pixels[x + (size_t) y * (size_t) image->width];
}

// Java: public void setPixelABGR(int x, int y, int value)
void LIBMATTI_B3D_NativeImage_SetPixelABGR(LIBMATTI_B3D_NativeImage *image, int x, int y, int abgr)
{
    if (is_outside_bounds(image, x, y))
        return;
    image->pixels[x + (size_t) y * (size_t) image->width] = (uint32_t) abgr;
}

// Java: public int getPixel(int x, int y) { return ARGB.fromABGR(getPixelABGR(x, y)); }
int LIBMATTI_B3D_NativeImage_GetPixel(const LIBMATTI_B3D_NativeImage *image, int x, int y)
{
    int abgr = LIBMATTI_B3D_NativeImage_GetPixelABGR(image, x, y);
    // Java: ARGB.fromABGR - ABGR int to ARGB int
    int a = (abgr >> 24) & 0xFF;
    int r = (abgr >> 16) & 0xFF;
    int g = (abgr >> 8) & 0xFF;
    int b = abgr & 0xFF;
    return a << 24 | r << 16 | g << 8 | b;
}

// Java: public void setPixel(int x, int y, int argb) { setPixelABGR(x, y, ARGB.toABGR(argb)); }
void LIBMATTI_B3D_NativeImage_SetPixel(LIBMATTI_B3D_NativeImage *image, int x, int y, int argb)
{
    int a = (argb >> 24) & 0xFF;
    int r = (argb >> 16) & 0xFF;
    int g = (argb >> 8) & 0xFF;
    int b = argb & 0xFF;
    LIBMATTI_B3D_NativeImage_SetPixelABGR(image, x, y, a << 24 | b << 16 | g << 8 | r);
}

// Java: public int[] getPixelsABGR()
int *LIBMATTI_B3D_NativeImage_GetPixelsABGR(const LIBMATTI_B3D_NativeImage *image)
{
    int *pixels = malloc((size_t) image->width * (size_t) image->height * sizeof(int));
    if (pixels == NULL)
        return NULL;
    for (int i = 0; i < image->width * image->height; i++)
        pixels[i] = (int) image->pixels[i];
    return pixels;
}

// Java: public int[] getPixels() - ABGR values through ARGB.fromABGR
int *LIBMATTI_B3D_NativeImage_GetPixels(const LIBMATTI_B3D_NativeImage *image)
{
    int *pixels = LIBMATTI_B3D_NativeImage_GetPixelsABGR(image);
    if (pixels == NULL)
        return NULL;
    for (int i = 0; i < image->width * image->height; i++)
    {
        int abgr = pixels[i];
        int a = (abgr >> 24) & 0xFF;
        int r = (abgr >> 16) & 0xFF;
        int g = (abgr >> 8) & 0xFF;
        int b = abgr & 0xFF;
        pixels[i] = a << 24 | r << 16 | g << 8 | b;
    }
    return pixels;
}

// Java: public NativeImage mappedCopy(IntUnaryOperator)
LIBMATTI_B3D_NativeImage *LIBMATTI_B3D_NativeImage_MappedCopy(const LIBMATTI_B3D_NativeImage *image,
                                                              int (*operator_)(int argb))
{
    LIBMATTI_B3D_NativeImage *copy = LIBMATTI_B3D_NativeImage_New(image->width, image->height, 0);
    if (copy == NULL)
        return NULL;
    for (int i = 0; i < image->width * image->height; i++)
    {
        int abgr = (int) image->pixels[i];
        int a = (abgr >> 24) & 0xFF;
        int r = (abgr >> 16) & 0xFF;
        int g = (abgr >> 8) & 0xFF;
        int b = abgr & 0xFF;
        int argb = a << 24 | r << 16 | g << 8 | b;
        int mapped = operator_(argb);
        int ma = (mapped >> 24) & 0xFF;
        int mr = (mapped >> 16) & 0xFF;
        int mg = (mapped >> 8) & 0xFF;
        int mb = mapped & 0xFF;
        copy->pixels[i] = (uint32_t) (ma << 24 | mb << 16 | mg << 8 | mr);
    }
    return copy;
}

// Java: public void close()
void LIBMATTI_B3D_NativeImage_Free(LIBMATTI_B3D_NativeImage *image)
{
    if (image == NULL)
        return;
    free(image->pixels);
    free(image);
}
