#include "libmatti/java/util/zip/Zip.h"

#include "libmatti/java/lang/Throwable.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// ---------------------------------------------------------------------------
// ZipException
// ---------------------------------------------------------------------------

// Java: public class ZipException extends IOException
static const char *ZIP_EXCEPTION_CLASS = "java.util.zip.ZipException";

struct LIBMATTI_JL_Throwable *LIBMATTI_JU_ZipException_New(const char *message)
{
    return LIBMATTI_JL_Throwable_NewNamed(ZIP_EXCEPTION_CLASS, message);
}

int LIBMATTI_JU_ZipException_IsInstance(const struct LIBMATTI_JL_Throwable *throwable)
{
    if (throwable == NULL || throwable->className == NULL)
        return 0;
    // Java: instanceof walks the super chain; IOException/Exception/Throwable all match
    return strcmp(throwable->className, ZIP_EXCEPTION_CLASS) == 0 ||
           strcmp(throwable->className, "java.io.IOException") == 0 ||
           strcmp(throwable->className, "java.lang.Exception") == 0;
}

// ---------------------------------------------------------------------------
// Deflater
// ---------------------------------------------------------------------------

struct LIBMATTI_JU_Deflater
{
    z_stream stream;
    int finished;
};

LIBMATTI_JU_Deflater *LIBMATTI_JU_Deflater_New(int level, int nowrap)
{
    LIBMATTI_JU_Deflater *deflater = calloc(1, sizeof(LIBMATTI_JU_Deflater));
    // Java: setLevel maps directly to zlib's levels
    if (deflateInit2(&deflater->stream, level, Z_DEFLATED, nowrap ? -15 : 15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        free(deflater);
        return NULL;
    }
    return deflater;
}

void LIBMATTI_JU_Deflater_Free(LIBMATTI_JU_Deflater *deflater)
{
    if (deflater == NULL)
        return;
    deflateEnd(&deflater->stream);
    free(deflater);
}

void LIBMATTI_JU_Deflater_SetInput(LIBMATTI_JU_Deflater *deflater, const unsigned char *input, size_t length)
{
    deflater->stream.next_in = (Bytef *) input;
    deflater->stream.avail_in = (uInt) length;
}

void LIBMATTI_JU_Deflater_Finish(LIBMATTI_JU_Deflater *deflater)
{
    // zlib signals the end through Z_FINISH on the next deflate call
    deflater->finished = 0;
}

int LIBMATTI_JU_Deflater_Finished(const LIBMATTI_JU_Deflater *deflater)
{
    return deflater->finished;
}

size_t LIBMATTI_JU_Deflater_Deflate(LIBMATTI_JU_Deflater *deflater, unsigned char *output, size_t outputLength)
{
    deflater->stream.next_out = output;
    deflater->stream.avail_out = (uInt) outputLength;
    int result = deflate(&deflater->stream, Z_NO_FLUSH);
    if (result == Z_STREAM_END)
        deflater->finished = 1;
    return outputLength - deflater->stream.avail_out;
}

unsigned char *LIBMATTI_JU_Deflater_Compress(const unsigned char *input, size_t length, size_t *outLength)
{
    // Java: a one-shot uses Deflater with a bound of input + overhead
    uLong bound = compressBound((uLong) length);
    unsigned char *output = malloc(bound);
    // Java's Deflater() default wraps zlib - no header when nowrap is requested;
    // the port matches Java's default (zlib header, nowrap=false)
    uLongf compressedLength = bound;
    if (compress2(output, &compressedLength, input, (uLong) length, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        free(output);
        return NULL;
    }
    if (outLength)
        *outLength = compressedLength;
    return output;
}

// ---------------------------------------------------------------------------
// Inflater
// ---------------------------------------------------------------------------

struct LIBMATTI_JU_Inflater
{
    z_stream stream;
    int finished;
};

LIBMATTI_JU_Inflater *LIBMATTI_JU_Inflater_New(int nowrap)
{
    LIBMATTI_JU_Inflater *inflater = calloc(1, sizeof(LIBMATTI_JU_Inflater));
    if (inflateInit2(&inflater->stream, nowrap ? -15 : 15) != Z_OK)
    {
        free(inflater);
        return NULL;
    }
    return inflater;
}

void LIBMATTI_JU_Inflater_Free(LIBMATTI_JU_Inflater *inflater)
{
    if (inflater == NULL)
        return;
    inflateEnd(&inflater->stream);
    free(inflater);
}

void LIBMATTI_JU_Inflater_SetInput(LIBMATTI_JU_Inflater *inflater, const unsigned char *input, size_t length)
{
    inflater->stream.next_in = (Bytef *) input;
    inflater->stream.avail_in = (uInt) length;
}

int LIBMATTI_JU_Inflater_Finished(const LIBMATTI_JU_Inflater *inflater)
{
    return inflater->finished;
}

int LIBMATTI_JU_Inflater_NeedsInput(const LIBMATTI_JU_Inflater *inflater)
{
    return inflater->stream.avail_in == 0;
}

size_t LIBMATTI_JU_Inflater_Inflate(LIBMATTI_JU_Inflater *inflater, unsigned char *output, size_t outputLength)
{
    inflater->stream.next_out = output;
    inflater->stream.avail_out = (uInt) outputLength;
    int result = inflate(&inflater->stream, Z_NO_FLUSH);
    if (result == Z_STREAM_END)
        inflater->finished = 1;
    if (result == Z_DATA_ERROR || result == Z_NEED_DICT)
        return 0; // Java: throws DataFormatException
    return outputLength - inflater->stream.avail_out;
}

unsigned char *LIBMATTI_JU_Inflater_Decompress(const unsigned char *input, size_t length,
                                               size_t outCapacityHint, size_t *outLength)
{
    size_t capacity = outCapacityHint > 0 ? outCapacityHint : length * 4;
    unsigned char *output = malloc(capacity);
    size_t written = 0;

    LIBMATTI_JU_Inflater *inflater = LIBMATTI_JU_Inflater_New(0);
    LIBMATTI_JU_Inflater_SetInput(inflater, input, length);

    while (!inflater->finished)
    {
        if (written == capacity)
        {
            capacity *= 2;
            output = realloc(output, capacity);
        }
        size_t chunk = LIBMATTI_JU_Inflater_Inflate(inflater, output + written, capacity - written);
        if (chunk == 0)
        {
            // Java: DataFormatException("corrupt stream") / needs more input
            if (!inflater->finished && !LIBMATTI_JU_Inflater_NeedsInput(inflater))
            {
                LIBMATTI_JU_Inflater_Free(inflater);
                free(output);
                return NULL;
            }
            if (LIBMATTI_JU_Inflater_NeedsInput(inflater) && !inflater->finished)
                break;
        }
        written += chunk;
    }
    LIBMATTI_JU_Inflater_Free(inflater);

    if (outLength)
        *outLength = written;
    return output;
}
