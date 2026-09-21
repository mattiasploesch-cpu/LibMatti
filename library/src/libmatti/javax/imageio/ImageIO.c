// Port of javax.imageio.ImageIO.
// Java selects a registered ImageReaderSpi for the stream; the port dispatches on the
// container signature exactly like the plugin registry does (PNG, then BMP).

#include "libmatti/javax/imageio/ImageIO.h"

#include "libmatti/javax/imageio/PngReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public static BufferedImage read(URL input) throws IOException
LIBMATTI_JAWT_Image *LIBMATTI_JXI_ImageIO_Read(LIBMATTI_JN_URL *input)
{
    // Java: ImageIO.read(null) throws IllegalArgumentException("input == null!")
    if (input == NULL) return NULL;

    FILE *file = fopen(LIBMATTI_JN_URL_ToString(input), "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length < 0)
    {
        fclose(file);
        return NULL;
    }

    unsigned char *data = malloc((size_t) length);
    size_t readLength = fread(data, 1, (size_t) length, file);
    fclose(file);

    LIBMATTI_JAWT_Image *image = calloc(1, sizeof(LIBMATTI_JAWT_Image));

    // Java: the registry matches the reader by the stream's magic bytes
    int width = 0;
    int height = 0;
    unsigned char *pixels = NULL;
    if (readLength >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G')
    {
        pixels = LIBMATTI_JXI_PngReader_Decode(data, readLength, &width, &height);
    }
    else if (readLength >= 2 && data[0] == 'B' && data[1] == 'M')
    {
        pixels = LIBMATTI_JXI_ImageIO_DecodeBmp(data, readLength, &width, &height);
    }

    if (pixels != NULL)
    {
        // Java: the decoded BufferedImage's raster
        image->pixels = pixels;
        image->width = width;
        image->height = height;
        image->data = NULL;
        image->length = 0;
    }
    else
    {
        // Java: read returns null when no reader can decode the stream
        free(image);
        free(data);
        return NULL;
    }

    free(data);
    return image;
}
