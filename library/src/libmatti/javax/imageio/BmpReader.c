// Port of the javax.imageio BMP reader path (com.sun.imageio.plugins.bmp.BMPImageReader).
// The plugin reads the BITMAPINFOHEADER and expands the uncompressed bottom-up rows of the
// 24/32-bit variants; the RLE-compressed variants are rejected the same way.

#include "libmatti/javax/imageio/ImageIO.h"

#include <stdlib.h>
#include <string.h>

static unsigned int read_u32(const unsigned char *data, size_t offset)
{
    return (unsigned int) data[offset] | ((unsigned int) data[offset + 1] << 8) |
           ((unsigned int) data[offset + 2] << 16) | ((unsigned int) data[offset + 3] << 24);
}

static unsigned short read_u16(const unsigned char *data, size_t offset)
{
    return (unsigned short) ((unsigned short) data[offset] | ((unsigned short) data[offset + 1] << 8));
}

unsigned char *LIBMATTI_JXI_ImageIO_DecodeBmp(const unsigned char *data, size_t length, int *width, int *height)
{
    // Java: the 'BM' signature was matched by the registry already
    if (length < 54 || data[0] != 'B' || data[1] != 'M') return NULL;

    unsigned int pixelOffset = read_u32(data, 10);
    unsigned int headerSize = read_u32(data, 14);
    if (headerSize < 40) return NULL; // Java: only BITMAPINFOHEADER and newer
    if (length < (size_t) pixelOffset) return NULL;

    int widthValue = (int) read_u32(data, 18);
    int heightValue = (int) read_u32(data, 22);
    unsigned short planes = read_u16(data, 26);
    unsigned short bitCount = read_u16(data, 28);
    unsigned int compression = read_u32(data, 30);

    if (planes != 1) return NULL;
    // Java: BI_RGB only - the RLE variants are separate destination types
    if (compression != 0) return NULL;
    if (bitCount != 24 && bitCount != 32) return NULL;
    if (widthValue <= 0 || heightValue == 0) return NULL;

    int bottomUp = heightValue > 0;
    int rows = bottomUp ? heightValue : -heightValue;
    size_t bytesPerPixel = bitCount / 8;
    // Java: every row is padded to a multiple of 4 bytes
    size_t rowBytes = ((size_t) widthValue * bytesPerPixel + 3) & ~(size_t) 3;

    if (length < (size_t) pixelOffset + rowBytes * (size_t) rows) return NULL;

    unsigned char *argb = malloc((size_t) widthValue * (size_t) rows * 4);
    for (int y = 0; y < rows; y++)
    {
        // Java: the image is stored bottom-up for positive heights
        const unsigned char *line = data + pixelOffset + (size_t) y * rowBytes;
        int imageY = bottomUp ? rows - 1 - y : y;

        for (int x = 0; x < widthValue; x++)
        {
            // BMP stores BGR(A)
            const unsigned char *pixel = line + (size_t) x * bytesPerPixel;
            unsigned char *out = argb + ((size_t) imageY * (size_t) widthValue + (size_t) x) * 4;
            out[0] = pixel[2];
            out[1] = pixel[1];
            out[2] = pixel[0];
            out[3] = bitCount == 32 ? pixel[3] : 255;
        }
    }

    *width = widthValue;
    *height = rows;
    return argb;
}
