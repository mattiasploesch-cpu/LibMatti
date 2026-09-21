#ifndef MATTICRAFT_JAVA_AWT_IMAGE_H
#define MATTICRAFT_JAVA_AWT_IMAGE_H

#include <stddef.h>

// Java: public abstract class Image - the port also carries the decoded BufferedImage
// raster (8-bit RGBA, width * height * 4 bytes) once an ImageIO reader decoded it.
typedef struct
{
    // Java: the encoded image source bytes (kept for the producers that never decode)
    unsigned char *data;
    size_t length;
    int width;
    int height;
    // Java: BufferedImage raster (TYPE_INT_ARGB byte layout: R, G, B, A per pixel);
    // NULL while the image only holds its encoded source
    unsigned char *pixels;
} LIBMATTI_JAWT_Image;

// Java: public abstract int getWidth(ImageObserver observer)
int LIBMATTI_JAWT_Image_GetWidth(const LIBMATTI_JAWT_Image *image);
// Java: public abstract int getHeight(ImageObserver observer)
int LIBMATTI_JAWT_Image_GetHeight(const LIBMATTI_JAWT_Image *image);
// Java: BufferedImage.getRGB(x, y) - 0 when the image was never decoded
unsigned int LIBMATTI_JAWT_Image_GetRGB(const LIBMATTI_JAWT_Image *image, int x, int y);
void LIBMATTI_JAWT_Image_Free(LIBMATTI_JAWT_Image *image);

#endif //MATTICRAFT_JAVA_AWT_IMAGE_H
