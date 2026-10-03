// Port of net.minecraft.world.level.chunk.storage.RegionFileVersion (implementation).
// The three wire codecs run over zlib: gzip (window 15+16), deflate/zlib (window 15)
// and the pass-through "none". Java's LZ4/CUSTOM ids stay unregistered - the port
// rejects them like Java's fromId(NULL) path.

#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileVersion.h"

#include <zlib.h>

#include <stdlib.h>
#include <string.h>

bool LIBMATTI_MC_RegionFileVersion_IsValidVersion(int id)
{
    // Java: VERSIONS.containsKey(id) - gzip, deflate and none (lz4 exists in
    // 1.21.11's map too; the port ships the zlib trio the format needs)
    return id == LIBMATTI_MC_RegionFileVersion_GZIP
           || id == LIBMATTI_MC_RegionFileVersion_DEFLATE
           || id == LIBMATTI_MC_RegionFileVersion_NONE;
}

// the deflateInit2 window bits per codec (the +16/32 flags pick the wrapper)
static int window_bits_for(int id)
{
    switch (id)
    {
        case LIBMATTI_MC_RegionFileVersion_GZIP:
            return 15 + 16; // gzip wrapper (java.util.zip.GZIPOutputStream)
        case LIBMATTI_MC_RegionFileVersion_DEFLATE:
            return 15; // zlib wrapper (java.util.zip.DeflaterOutputStream)
        default:
            return 0;
    }
}

int LIBMATTI_MC_RegionFileVersion_Compress(int id, const uint8_t *data, size_t length,
                                           uint8_t **outData, size_t *outLength)
{
    if (outData == NULL || outLength == NULL)
        return -1;
    *outData = NULL;
    *outLength = 0;

    if (id == LIBMATTI_MC_RegionFileVersion_NONE)
    {
        // Java: BufferedOutputStream - the pass-through
        uint8_t *copy = malloc(length > 0 ? length : 1);
        if (copy == NULL)
            return -1;
        if (length > 0)
            memcpy(copy, data, length);
        *outData = copy;
        *outLength = length;
        return 0;
    }

    int windowBits = window_bits_for(id);
    if (windowBits == 0)
        return -1;

    // Java: DeflaterOutputStream with the default level (-1 = Z_DEFAULT_COMPRESSION)
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return -1;

    // deflateBound gives the worst case for the chosen wrapper
    uLong bound = deflateBound(&stream, (uLong) length);
    uint8_t *out = malloc((size_t) bound + 1);
    if (out == NULL)
    {
        deflateEnd(&stream);
        return -1;
    }

    stream.next_in = (Bytef *) (length > 0 ? (void *) data : (const void *) "");
    stream.avail_in = (uInt) length;
    stream.next_out = out;
    stream.avail_out = (uInt) bound;

    int status = deflate(&stream, Z_FINISH);
    size_t produced = stream.total_out;
    int result = (status == Z_STREAM_END) ? 0 : -1;
    deflateEnd(&stream);
    if (result != 0)
    {
        free(out);
        return -1;
    }

    *outData = out;
    *outLength = produced;
    return 0;
}

int LIBMATTI_MC_RegionFileVersion_Decompress(int id, const uint8_t *data, size_t length,
                                             uint8_t **outData, size_t *outLength)
{
    if (outData == NULL || outLength == NULL)
        return -1;
    *outData = NULL;
    *outLength = 0;

    if (id == LIBMATTI_MC_RegionFileVersion_NONE)
    {
        uint8_t *copy = malloc(length > 0 ? length : 1);
        if (copy == NULL)
            return -1;
        if (length > 0)
            memcpy(copy, data, length);
        *outData = copy;
        *outLength = length;
        return 0;
    }

    int windowBits = window_bits_for(id);
    if (windowBits == 0)
        return -1;

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (inflateInit2(&stream, windowBits) != Z_OK)
        return -1;

    // the output grows until the stream ends (Java's InflaterInputStream read loop)
    size_t capacity = length > 0 ? length * 4 : 4096;
    uint8_t *out = malloc(capacity);
    if (out == NULL)
    {
        inflateEnd(&stream);
        return -1;
    }

    stream.next_in = (Bytef *) (length > 0 ? (void *) data : (const void *) "");
    stream.avail_in = (uInt) length;

    size_t produced = 0;
    int status;
    for (;;)
    {
        if (produced == capacity)
        {
            size_t next = capacity * 2;
            uint8_t *grown = realloc(out, next);
            if (grown == NULL)
            {
                free(out);
                inflateEnd(&stream);
                return -1;
            }
            out = grown;
            capacity = next;
        }
        stream.next_out = out + produced;
        stream.avail_out = (uInt) (capacity - produced);
        status = inflate(&stream, Z_NO_FLUSH);
        produced = stream.total_out;
        if (status == Z_STREAM_END)
            break;
        if (status != Z_OK && status != Z_BUF_ERROR)
        {
            free(out);
            inflateEnd(&stream);
            return -1;
        }
        if (status == Z_BUF_ERROR && stream.avail_in == 0)
        {
            // Java throws EOF on a truncated stream; the port reports failure
            free(out);
            inflateEnd(&stream);
            return -1;
        }
    }

    inflateEnd(&stream);
    *outData = out;
    *outLength = produced;
    return 0;
}
