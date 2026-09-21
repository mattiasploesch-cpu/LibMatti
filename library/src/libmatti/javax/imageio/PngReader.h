// Port of the javax.imageio PNG reader plugin (com.sun.imageio.plugins.png.PNGImageReader).
// The port decodes the PNG container itself with zlib, exactly what the Java plugin does.

#ifndef MATTICRAFT_JAVAX_IMAGEIO_PNGREADER_H
#define MATTICRAFT_JAVAX_IMAGEIO_PNGREADER_H

#include <stddef.h>

// Java: the decoded BufferedImage.TYPE_INT_ARGB pixel data; NULL on a decoding error
unsigned char *LIBMATTI_JXI_PngReader_Decode(const unsigned char *data, size_t length, int *width, int *height);

#endif //MATTICRAFT_JAVAX_IMAGEIO_PNGREADER_H
