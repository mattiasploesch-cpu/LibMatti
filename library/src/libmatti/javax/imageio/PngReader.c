// Port of the javax.imageio PNG reader plugin (com.sun.imageio.plugins.png.PNGImageReader).
// The container is validated like Java's plugin: signature, IHDR as the first chunk, palette
// metadata before the IDAT, consecutive IDAT chunks, IEND. The zlib stream inflates into the
// filtered scanlines; the five PNG filters are undone line by line and every supported bit
// depth (1/2/4/8/16) and colour type (0/2/3/4/6) is expanded into 8-bit RGBA, like the
// BufferedImage types the Java plugin emits.

#include "libmatti/javax/imageio/PngReader.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// Java: PNGImageReader.PNG magic: 0x89 'P' 'N' 'G' '\r' '\n' 0x1a '\n'
static const unsigned char PNG_SIGNATURE[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};

typedef struct
{
    const unsigned char *data;
    size_t length;
    size_t position;
} Reader;

static int read_u8(Reader *reader, unsigned int *value)
{
    if (reader->position + 1 > reader->length) return 0;
    *value = reader->data[reader->position++];
    return 1;
}

static int read_u32(Reader *reader, unsigned int *value)
{
    if (reader->position + 4 > reader->length) return 0;
    *value = ((unsigned int) reader->data[reader->position] << 24) |
             ((unsigned int) reader->data[reader->position + 1] << 16) |
             ((unsigned int) reader->data[reader->position + 2] << 8) |
             (unsigned int) reader->data[reader->position + 3];
    reader->position += 4;
    return 1;
}

static int read_bytes(Reader *reader, unsigned char *buffer, size_t length)
{
    if (reader->position + length > reader->length) return 0;
    memcpy(buffer, reader->data + reader->position, length);
    reader->position += length;
    return 1;
}

// Java: chunkSkip reads past the CRC without verifying it
static int skip_crc(Reader *reader)
{
    if (reader->position + 4 > reader->length) return 0;
    reader->position += 4;
    return 1;
}

// Java: int channels(int colorType)
static int channels_of(unsigned int colorType)
{
    switch (colorType)
    {
    case 0: return 1; // gray
    case 2: return 3; // truecolor
    case 3: return 1; // palette
    case 4: return 2; // gray + alpha
    case 6: return 4; // truecolor + alpha
    default: return -1;
    }
}

// Java: the palette image channel count depends on the tRNS length
static int palette_channels(int bitDepth, int trnsCount)
{
    int samples = 1;
    // Java: getImageTypes - the tRNS adds a second (alpha) band
    if (trnsCount > 0) samples = 2;
    (void) bitDepth;
    return samples;
}

// Java: the bytes per sample of a bit depth (16-bit samples occupy two bytes)
static int sample_bytes(int bitDepth)
{
    return bitDepth == 16 ? 2 : 1;
}

// Java: the byte width of one unfiltered scanline (spec: floor((width * channels * bitDepth + 7) / 8))
static size_t scanline_bytes(int width, int channels, int bitDepth)
{
    return ((size_t) width * (size_t) channels * (size_t) bitDepth + 7) / 8;
}

// Java: the Adam7 pass geometry
static const int ADAM7_X_OFFSET[7] = {0, 4, 0, 2, 0, 1, 0};
static const int ADAM7_Y_OFFSET[7] = {0, 0, 4, 0, 2, 0, 1};
static const int ADAM7_X_STEP[7] = {8, 8, 4, 4, 2, 2, 1};
static const int ADAM7_Y_STEP[7] = {8, 8, 8, 4, 4, 2, 2};

// Java: the sample at (x, channel) of a packed line, most significant bit first
static int read_sample(const unsigned char *line, int x, int channel, int channels, int bitDepth)
{
    if (bitDepth >= 8)
    {
        int bytes = sample_bytes(bitDepth);
        size_t index = ((size_t) x * (size_t) channels + (size_t) channel) * (size_t) bytes;
        if (bytes == 2) return line[index]; // the high byte of the unsigned short
        return line[index];
    }

    int bit = (x * channels + channel) * bitDepth;
    unsigned char byte = line[bit / 8];
    int shift = 8 - bitDepth - (bit % 8);
    unsigned char mask = (unsigned char) (bitDepth == 1 ? 0x01 : (bitDepth == 2 ? 0x03 : 0x0f));
    return (byte >> shift) & mask;
}

// Java: PNGImageReader private method that expands a decoded sample row into RGBA rows.
// Every colour type/bitness ends as 8-bit RGBA (BufferedImage.TYPE_INT_ARGB semantics).
static void emit_row(const unsigned char *line, int rowWidth, int channels, int bitDepth,
                     unsigned int colorType, const unsigned char *palette, int paletteCount,
                     const unsigned char *trns, int trnsCount,
                     unsigned char *image, int imageWidth, int imageY, int imageX, int xStep)
{
    unsigned char *out = image + ((size_t) imageY * (size_t) imageWidth + (size_t) imageX) * 4;

    for (int x = 0; x < rowWidth; x++)
    {
        unsigned char *pixel = out + (size_t) x * (size_t) xStep * 4;
        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;
        unsigned char a = 255;

        switch (colorType)
        {
        case 0: // gray
        case 4: // gray + alpha
        {
            int gray = read_sample(line, x, 0, channels, bitDepth);
            if (bitDepth < 8)
            {
                // Java: the smaller depths are scaled to the full 8-bit range
                int max = (1 << bitDepth) - 1;
                gray = gray * 255 / max;
            }
            r = g = b = (unsigned char) gray;
            if (channels == 2) a = (unsigned char) read_sample(line, x, 1, channels, bitDepth);
            break;
        }
        case 2: // truecolor
        case 6: // truecolor + alpha
        {
            r = (unsigned char) read_sample(line, x, 0, channels, bitDepth);
            g = (unsigned char) read_sample(line, x, 1, channels, bitDepth);
            b = (unsigned char) read_sample(line, x, 2, channels, bitDepth);
            if (channels == 4) a = (unsigned char) read_sample(line, x, 3, channels, bitDepth);
            break;
        }
        case 3: // palette
        {
            int index = read_sample(line, x, 0, channels, bitDepth);
            if (index < paletteCount)
            {
                r = palette[index * 3];
                g = palette[index * 3 + 1];
                b = palette[index * 3 + 2];
            }
            if (trns != NULL && index < trnsCount) a = trns[index];
            break;
        }
        default:
            break;
        }

        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;
        pixel[3] = a;
    }
}

// Java: private void readImage(int pass) - the filter loop over the scanlines of a pass
static int unfilter_pass(const unsigned char *raw, size_t rawLength, size_t *rawPosition,
                         int rowWidth, int channels, int bitDepth, size_t lineBytes,
                         unsigned int colorType, const unsigned char *palette, int paletteCount,
                         const unsigned char *trns, int trnsCount,
                         unsigned char *image, int imageWidth, int imageHeight,
                         int yOffset, int yStep, int xOffset, int xStep)
{
    unsigned char *prior = calloc(lineBytes, 1);
    unsigned char *current = malloc(lineBytes);
    int rows = 0;

    for (int y = yOffset; y < imageHeight; y += yStep, rows++)
    {
        if (*rawPosition + 1 + lineBytes > rawLength)
        {
            free(prior);
            free(current);
            return 0;
        }

        unsigned int filterType = raw[(*rawPosition)++];
        memcpy(current, raw + *rawPosition, lineBytes);
        *rawPosition += lineBytes;

        for (size_t x = 0; x < lineBytes; x++)
        {
            unsigned char raw0 = current[x];
            unsigned char left = x >= (size_t) channels * (size_t) sample_bytes(bitDepth)
                                     ? current[x - (size_t) channels * (size_t) sample_bytes(bitDepth)]
                                     : 0;
            unsigned char up = prior[x];
            unsigned char upperLeft = x >= (size_t) channels * (size_t) sample_bytes(bitDepth)
                                          ? prior[x - (size_t) channels * (size_t) sample_bytes(bitDepth)]
                                          : 0;

            int prediction;
            switch (filterType)
            {
            case 0: prediction = 0; break;        // None
            case 1: prediction = left; break;     // Sub
            case 2: prediction = up; break;       // Up
            case 3: prediction = (left + up) / 2; break; // Average
            case 4: // Paeth
            {
                int p = left + up - upperLeft;
                int pa = p - left;
                if (pa < 0) pa = -pa;
                int pb = p - up;
                if (pb < 0) pb = -pb;
                int pc = p - upperLeft;
                if (pc < 0) pc = -pc;
                prediction = pa <= pb && pa <= pc ? left : (pb <= pc ? up : upperLeft);
                break;
            }
            default:
                free(prior);
                free(current);
                return 0;
            }
            current[x] = (unsigned char) (raw0 + prediction);
        }

        int rowWidthValue = xOffset == -1 ? rowWidth : (imageWidth - xOffset + xStep - 1) / xStep;
        emit_row(current, rowWidthValue, channels, bitDepth, colorType, palette, paletteCount, trns,
                 trnsCount, image, imageWidth, y, xOffset, xStep);

        unsigned char *swap = prior;
        prior = current;
        current = swap;
    }

    free(prior);
    free(current);
    return 1;
}

unsigned char *LIBMATTI_JXI_PngReader_Decode(const unsigned char *data, size_t length, int *width, int *height)
{
    Reader reader = {data, length, 0};

    unsigned char signature[8];
    if (!read_bytes(&reader, signature, 8) || memcmp(signature, PNG_SIGNATURE, 8) != 0)
        return NULL;

    unsigned int chunkLength;
    unsigned int chunkType;
    if (!read_u32(&reader, &chunkLength) || !read_u32(&reader, &chunkType))
        return NULL;

    // Java: the IHDR must be the first chunk, 13 bytes long
    if (chunkType != 0x49484452 || chunkLength != 13) return NULL; // 'IHDR'

    unsigned int chunkWidth;
    unsigned int chunkHeight;
    unsigned int bitDepthValue;
    unsigned int colorTypeValue;
    unsigned int compression;
    unsigned int filterMethod;
    unsigned int interlaceMethod;
    if (!read_u32(&reader, &chunkWidth) || !read_u32(&reader, &chunkHeight) ||
        !read_u8(&reader, &bitDepthValue) || !read_u8(&reader, &colorTypeValue) ||
        !read_u8(&reader, &compression) || !read_u8(&reader, &filterMethod) ||
        !read_u8(&reader, &interlaceMethod))
        return NULL;
    skip_crc(&reader);

    // Java: the plugin rejects the invalid combinations before decoding
    if (chunkWidth == 0 || chunkHeight == 0 || chunkWidth > 0x7fffffffu || chunkHeight > 0x7fffffffu)
        return NULL;
    if (compression != 0 || filterMethod != 0) return NULL;
    if (interlaceMethod > 1) return NULL;
    int channels = channels_of(colorTypeValue);
    if (channels < 0) return NULL;

    int bitDepth = (int) bitDepthValue;
    switch (colorTypeValue)
    {
    case 0: if (bitDepth != 1 && bitDepth != 2 && bitDepth != 4 && bitDepth != 8 && bitDepth != 16) return NULL; break;
    case 2:
    case 4:
    case 6: if (bitDepth != 8 && bitDepth != 16) return NULL; break;
    case 3: if (bitDepth != 1 && bitDepth != 2 && bitDepth != 4 && bitDepth != 8) return NULL; break;
    default: return NULL;
    }

    int widthValue = (int) chunkWidth;
    int heightValue = (int) chunkHeight;

    unsigned char *argb = calloc((size_t) widthValue * (size_t) heightValue, 4);
    unsigned char *palette = NULL;
    int paletteCount = 0;
    unsigned char *trns = NULL;
    int trnsCount = 0;

    // Java: the zlib stream is fed the concatenated IDAT chunks
    unsigned char *compressed = NULL;
    size_t compressedLength = 0;
    int sawHeader = 0;
    int sawIDAT = 0;
    int idatEnded = 0;
    int sawEnd = 0;

    while (!sawEnd)
    {
        if (!read_u32(&reader, &chunkLength) || !read_u32(&reader, &chunkType))
            goto error;

        if (chunkType == 0x504c5445) // 'PLTE' - must appear before the IDAT
        {
            if (sawIDAT || (chunkLength % 3) != 0 || chunkLength / 3 > 256) goto error;
            paletteCount = (int) (chunkLength / 3);
            free(palette);
            palette = malloc(chunkLength);
            if (!read_bytes(&reader, palette, chunkLength)) goto error;
            if (!skip_crc(&reader)) goto error;
            sawHeader = 1;
            continue;
        }
        if (chunkType == 0x74524e53) // 'tRNS' - before the IDAT
        {
            if (sawIDAT) goto error;
            free(trns);
            trnsCount = (int) chunkLength;
            trns = malloc((size_t) chunkLength + 1);
            if (!read_bytes(&reader, trns, chunkLength)) goto error;
            trns[chunkLength] = 0;
            if (!skip_crc(&reader)) goto error;
            continue;
        }
        if (chunkType == 0x49444154) // 'IDAT' - consecutive
        {
            if (!sawHeader && colorTypeValue != 3) sawHeader = 1; // a truecolor image needs no PLTE
            if (!sawHeader) goto error;
            if (idatEnded) goto error;
            sawIDAT = 1;

            unsigned char *grown = realloc(compressed, compressedLength + chunkLength);
            if (grown == NULL) goto error;
            compressed = grown;
            if (!read_bytes(&reader, compressed + compressedLength, chunkLength)) goto error;
            compressedLength += chunkLength;
            if (!skip_crc(&reader)) goto error;
            continue;
        }
        if (chunkType == 0x49454e44) // 'IEND'
        {
            sawEnd = 1;
            skip_crc(&reader);
            break;
        }

        // Java: every other chunk is skipped (metadata the port does not need)
        if (reader.position + chunkLength + 4 > reader.length) goto error;
        reader.position += chunkLength + 4;
    }

    if (!sawIDAT || compressedLength == 0) goto error;

    // Java: inflate the whole stream, then walk the scanlines
    unsigned char *raw = NULL;
    size_t rawCapacity = 65536;
    size_t rawLength = 0;
    raw = malloc(rawCapacity);

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (inflateInit(&stream) != Z_OK) goto error;

    stream.next_in = compressed;
    stream.avail_in = (uInt) compressedLength;

    for (;;)
    {
        if (rawLength == rawCapacity)
        {
            rawCapacity *= 2;
            unsigned char *grown = realloc(raw, rawCapacity);
            if (grown == NULL)
            {
                inflateEnd(&stream);
                goto error;
            }
            raw = grown;
        }
        stream.next_out = raw + rawLength;
        stream.avail_out = (uInt) (rawCapacity - rawLength);
        uInt before = stream.avail_out;
        int result = inflate(&stream, Z_NO_FLUSH);
        rawLength = rawCapacity - stream.avail_out;
        if (result == Z_STREAM_END) break;
        if (result != Z_OK || stream.avail_out == before)
        {
            inflateEnd(&stream);
            goto error;
        }
    }
    inflateEnd(&stream);

    // Java: the pass loop - a non-interlaced image is one pass with a single geometry
    int imageChannels = colorTypeValue == 3 ? palette_channels(bitDepth, trnsCount) : channels;
    size_t lineBytes = scanline_bytes(widthValue, imageChannels, bitDepth);
    size_t rawPosition = 0;

    if (interlaceMethod == 0)
    {
        if (!unfilter_pass(raw, rawLength, &rawPosition, widthValue, imageChannels, bitDepth, lineBytes,
                           colorTypeValue, palette, paletteCount, trns, trnsCount, argb, widthValue,
                           heightValue, 0, 1, 0, 1))
            goto error;
    }
    else
    {
        // Java: seven Adam7 passes, each with its own scanline width
        for (int pass = 0; pass < 7; pass++)
        {
            int passWidth = (widthValue - ADAM7_X_OFFSET[pass] + ADAM7_X_STEP[pass] - 1) / ADAM7_X_STEP[pass];
            int passHeight = (heightValue - ADAM7_Y_OFFSET[pass] + ADAM7_Y_STEP[pass] - 1) / ADAM7_Y_STEP[pass];
            if (passWidth <= 0 || passHeight <= 0) continue;

            size_t passLineBytes = scanline_bytes(passWidth, imageChannels, bitDepth);
            if (!unfilter_pass(raw, rawLength, &rawPosition, passWidth, imageChannels, bitDepth,
                               passLineBytes, colorTypeValue, palette, paletteCount, trns, trnsCount,
                               argb, widthValue, heightValue, ADAM7_Y_OFFSET[pass], ADAM7_Y_STEP[pass],
                               ADAM7_X_OFFSET[pass], ADAM7_X_STEP[pass]))
                goto error;
        }
    }

    *width = widthValue;
    *height = heightValue;
    free(raw);
    free(compressed);
    free(palette);
    free(trns);
    return argb;

error:
    free(argb);
    free(raw);
    free(compressed);
    free(palette);
    free(trns);
    return NULL;
}
