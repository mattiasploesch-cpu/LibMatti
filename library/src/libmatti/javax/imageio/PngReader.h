



#ifndef MATTICRAFT_JAVAX_IMAGEIO_PNGREADER_H
#define MATTICRAFT_JAVAX_IMAGEIO_PNGREADER_H

#include <stddef.h>

// Java: the decoded BufferedImage.TYPE_INT_ARGB pixel data; NULL on a decoding error
unsigned char *LIBMATTI_JXI_PngReader_Decode(const unsigned char *data, size_t length, int *width, int *height);

#endif
