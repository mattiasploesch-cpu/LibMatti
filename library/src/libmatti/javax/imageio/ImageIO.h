#ifndef MATTICRAFT_JAVAX_IMAGEIO_IMAGEIO_H
#define MATTICRAFT_JAVAX_IMAGEIO_IMAGEIO_H

#include "libmatti/java/awt/Image.h"
#include "libmatti/java/net/URL.h"

// Java: public final class ImageIO
// Java: public static BufferedImage read(URL input) throws IOException
LIBMATTI_JAWT_Image *LIBMATTI_JXI_ImageIO_Read(LIBMATTI_JN_URL *input);

// Java: com.sun.imageio.plugins.bmp.BMPImageReader - dispatched by the reader registry
unsigned char *LIBMATTI_JXI_ImageIO_DecodeBmp(const unsigned char *data, size_t length, int *width, int *height);

#endif //MATTICRAFT_JAVAX_IMAGEIO_IMAGEIO_H
