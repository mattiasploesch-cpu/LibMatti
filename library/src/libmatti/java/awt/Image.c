#include "libmatti/java/awt/Image.h"

#include <stdlib.h>

int LIBMATTI_JAWT_Image_GetWidth(const LIBMATTI_JAWT_Image *image)
{
    return image->width;
}

int LIBMATTI_JAWT_Image_GetHeight(const LIBMATTI_JAWT_Image *image)
{
    return image->height;
}

// Java: BufferedImage.getRGB(x, y) - the packed ARGB pixel
unsigned int LIBMATTI_JAWT_Image_GetRGB(const LIBMATTI_JAWT_Image *image, int x, int y)
{
    if (image->pixels == NULL || x < 0 || y < 0 || x >= image->width || y >= image->height)
        return 0;

    const unsigned char *pixel = image->pixels + ((size_t) y * (size_t) image->width + (size_t) x) * 4;
    return ((unsigned int) pixel[3] << 24) | ((unsigned int) pixel[0] << 16) |
           ((unsigned int) pixel[1] << 8) | (unsigned int) pixel[2];
}

void LIBMATTI_JAWT_Image_Free(LIBMATTI_JAWT_Image *image)
{
    if (image == NULL) return;
    free(image->data);
    free(image->pixels);
    free(image);
}
