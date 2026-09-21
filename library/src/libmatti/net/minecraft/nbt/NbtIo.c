// Port of net.minecraft.nbt.NbtIo. The DataInput/DataOutput layer is a memory
// cursor with Java's big-endian encoding and writeUTF's modified UTF-8 (the port
// maps it to plain UTF-8, the ASCII game strings are identical); the gzip layer
// is zlib like Java's GZIP* streams.

#include "libmatti/net/minecraft/nbt/NbtIo.h"

#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// the small file helper used by the *File variants (Java: Files.newInputStream)
static unsigned char *nbt_file_read_all(const char *path, size_t *length)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 0)
    {
        fclose(file);
        return NULL;
    }
    unsigned char *data = malloc((size_t) size + 1);
    if (fread(data, 1, (size_t) size, file) != (size_t) size)
    {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *length = (size_t) size;
    return data;
}

// ---------------------------------------------------------------------------
// the DataInput/DataOutput cursor
// ---------------------------------------------------------------------------

typedef struct
{
    const uint8_t *in;
    size_t length;
    size_t position;
    int overflow;
    // Java: the NbtAccounter every load runs through
    LIBMATTI_MC_NbtAccounter *accounter;
    // output side (write mode)
    uint8_t *out;
    size_t outCapacity;
    size_t outLength;
} Stream;

static void stream_init_read(Stream *stream, const uint8_t *data, size_t length, LIBMATTI_MC_NbtAccounter *accounter)
{
    stream->in = data;
    stream->length = length;
    stream->position = 0;
    stream->overflow = 0;
    stream->accounter = accounter;
    stream->out = NULL;
    stream->outCapacity = 0;
    stream->outLength = 0;
}

static void stream_init_write(Stream *stream)
{
    stream->in = NULL;
    stream->length = 0;
    stream->position = 0;
    stream->overflow = 0;
    stream->accounter = NULL;
    stream->outCapacity = 256;
    stream->out = malloc(stream->outCapacity);
    stream->outLength = 0;
}

static void stream_put(Stream *stream, const void *data, size_t length)
{
    if (stream->outLength + length > stream->outCapacity)
    {
        while (stream->outLength + length > stream->outCapacity)
            stream->outCapacity *= 2;
        stream->out = realloc(stream->out, stream->outCapacity);
    }
    memcpy(stream->out + stream->outLength, data, length);
    stream->outLength += length;
}

static int stream_get(Stream *stream, void *data, size_t length)
{
    if (stream->position + length > stream->length)
    {
        stream->overflow = 1;
        return 0;
    }
    memcpy(data, stream->in + stream->position, length);
    stream->position += length;
    return 1;
}

// Java: DataInput.readByte()
static int8_t read_byte(Stream *stream)
{
    uint8_t value = 0;
    stream_get(stream, &value, 1);
    return (int8_t) value;
}

// Java: DataOutput.writeByte(int)
static void write_byte(Stream *stream, int8_t value)
{
    stream_put(stream, &value, 1);
}

static int16_t read_short(Stream *stream)
{
    uint8_t bytes[2];
    stream_get(stream, bytes, 2);
    return (int16_t) ((uint16_t) bytes[0] << 8 | (uint16_t) bytes[1]);
}

static void write_short(Stream *stream, int16_t value)
{
    uint16_t raw = (uint16_t) value;
    uint8_t bytes[2] = {(uint8_t) (raw >> 8), (uint8_t) raw};
    stream_put(stream, bytes, 2);
}

static int32_t read_int(Stream *stream)
{
    uint8_t bytes[4];
    stream_get(stream, bytes, 4);
    return (int32_t) ((uint32_t) bytes[0] << 24 | (uint32_t) bytes[1] << 16 | (uint32_t) bytes[2] << 8 |
                      (uint32_t) bytes[3]);
}

static void write_int(Stream *stream, int32_t value)
{
    uint32_t raw = (uint32_t) value;
    uint8_t bytes[4] = {(uint8_t) (raw >> 24), (uint8_t) (raw >> 16), (uint8_t) (raw >> 8), (uint8_t) raw};
    stream_put(stream, bytes, 4);
}

static int64_t read_long(Stream *stream)
{
    uint8_t bytes[8];
    stream_get(stream, bytes, 8);
    uint64_t raw = 0;
    for (int i = 0; i < 8; i++)
        raw = raw << 8 | bytes[i];
    return (int64_t) raw;
}

static void write_long(Stream *stream, int64_t value)
{
    uint64_t raw = (uint64_t) value;
    uint8_t bytes[8];
    for (int i = 7; i >= 0; i--)
    {
        bytes[i] = (uint8_t) raw;
        raw >>= 8;
    }
    stream_put(stream, bytes, 8);
}

static float read_float(Stream *stream)
{
    uint32_t raw = (uint32_t) read_int(stream);
    float value;
    memcpy(&value, &raw, 4);
    return value;
}

static void write_float(Stream *stream, float value)
{
    uint32_t raw;
    memcpy(&raw, &value, 4);
    write_int(stream, (int32_t) raw);
}

static double read_double(Stream *stream)
{
    uint64_t raw = (uint64_t) read_long(stream);
    double value;
    memcpy(&value, &raw, 8);
    return value;
}

static void write_double(Stream *stream, double value)
{
    uint64_t raw;
    memcpy(&raw, &value, 8);
    write_long(stream, (int64_t) raw);
}

// Java: DataOutput.writeUTF / DataInput.readUTF - Java's modified UTF-8 encodes
// NUL as C0 80; the port reads/writes plain UTF-8 with the 2-byte length prefix
// (identical bytes for the ASCII strings the game data uses)
static char *read_string(Stream *stream)
{
    int16_t length = read_short(stream);
    if (length < 0)
    {
        stream->overflow = 1;
        return NULL;
    }
    char *text = malloc((size_t) length + 1);
    if (!stream_get(stream, text, (size_t) length))
    {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    if (stream->accounter != NULL)
        LIBMATTI_MC_NbtAccounter_AccountBytesFactor(stream->accounter, 2, length);
    return text;
}

static void write_string(Stream *stream, const char *text)
{
    size_t length = strlen(text);
    // Java: writeUTF stores the UTF-8 encoded length as an unsigned short
    write_short(stream, (int16_t) (uint16_t) length);
    stream_put(stream, text, length);
}

// Java: StringTag.skipString(DataInput)
static void skip_string(Stream *stream)
{
    if (stream->position + 2 > stream->length)
    {
        stream->overflow = 1;
        return;
    }
    uint16_t length = (uint16_t) ((uint16_t) stream->in[stream->position] << 8 |
                                  (uint16_t) stream->in[stream->position + 1]);
    stream->position += 2 + length;
}

// ---------------------------------------------------------------------------
// the tag load/store (TagTypes.getType(id).load(input, accounter))
// ---------------------------------------------------------------------------

static LIBMATTI_MC_Nbt_Tag *load_tag(Stream *stream, uint8_t id);

static LIBMATTI_MC_Nbt_Tag *load_compound_contents(Stream *stream)
{
    LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 48);
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_CompoundTag_New();
    uint8_t typeId;
    while ((typeId = (uint8_t) read_byte(stream)) != 0)
    {
        char *key = read_string(stream);
        if (key == NULL)
        {
            LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
            free(compound);
            return NULL;
        }
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 28);
        LIBMATTI_MC_Nbt_Tag *tag = load_tag(stream, typeId);
        if (tag == NULL)
        {
            free(key);
            LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(compound);
            free(compound);
            return NULL;
        }
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 36);
        LIBMATTI_MC_Nbt_Tag *replaced = LIBMATTI_MC_Nbt_CompoundTag_Put(compound, key, tag);
        LIBMATTI_MC_Nbt_Tag_Free(replaced);
        free(key);
    }
    return (LIBMATTI_MC_Nbt_Tag *) compound;
}

static LIBMATTI_MC_Nbt_Tag *load_list_contents(Stream *stream)
{
    LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 36);
    uint8_t typeId = (uint8_t) read_byte(stream);
    int32_t count = read_int(stream);
    if (count < 0)
        return NULL;
    if (typeId == 0 && count > 0)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Missing type on ListTag");
        return NULL;
    }
    LIBMATTI_MC_NbtAccounter_AccountBytesFactor(stream->accounter, 4, count);

    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    for (int32_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_Tag *tag = load_tag(stream, typeId);
        if (tag == NULL)
        {
            LIBMATTI_MC_Nbt_ListTag_FreeEntries(list);
            free(list);
            return NULL;
        }
        // Java: addAndUnwrap - the {"" : tag} wrapper compounds unwrap on load
        LIBMATTI_MC_Nbt_ListTag_AddAndUnwrap(list, tag);
    }
    return (LIBMATTI_MC_Nbt_Tag *) list;
}

static LIBMATTI_MC_Nbt_Tag *load_tag(Stream *stream, uint8_t id)
{
    switch (id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 9);
        return LIBMATTI_MC_Nbt_ByteTag_Of(read_byte(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_SHORT:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 10);
        return LIBMATTI_MC_Nbt_ShortTag_Of(read_short(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_INT:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 12);
        return LIBMATTI_MC_Nbt_IntTag_Of(read_int(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_LONG:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 16);
        return LIBMATTI_MC_Nbt_LongTag_Of(read_long(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 12);
        return LIBMATTI_MC_Nbt_FloatTag_Of(read_float(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 16);
        return LIBMATTI_MC_Nbt_DoubleTag_Of(read_double(stream));
    }
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 24);
        int32_t length = read_int(stream);
        if (length < 0)
        {
            stream->overflow = 1;
            return NULL;
        }
        LIBMATTI_MC_NbtAccounter_AccountBytesFactor(stream->accounter, 1, length);
        int8_t *data = malloc(length > 0 ? (size_t) length : 1);
        stream_get(stream, data, (size_t) length);
        LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_ByteArrayTag_Of(data, (size_t) length);
        free(data);
        return tag;
    }
    case LIBMATTI_MC_Nbt_TAG_STRING:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 36);
        char *text = read_string(stream);
        if (text == NULL)
            return NULL;
        LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_StringTag_Of(text);
        free(text);
        return tag;
    }
    case LIBMATTI_MC_Nbt_TAG_LIST:
    {
        if (LIBMATTI_MC_NbtAccounter_PushDepth(stream->accounter))
        {
            LIBMATTI_MC_Nbt_Tag *tag = load_list_contents(stream);
            LIBMATTI_MC_NbtAccounter_PopDepth(stream->accounter);
            return tag;
        }
        return NULL;
    }
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
    {
        if (LIBMATTI_MC_NbtAccounter_PushDepth(stream->accounter))
        {
            LIBMATTI_MC_Nbt_Tag *tag = load_compound_contents(stream);
            LIBMATTI_MC_NbtAccounter_PopDepth(stream->accounter);
            return tag;
        }
        return NULL;
    }
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 24);
        int32_t length = read_int(stream);
        if (length < 0)
        {
            stream->overflow = 1;
            return NULL;
        }
        LIBMATTI_MC_NbtAccounter_AccountBytesFactor(stream->accounter, 4, length);
        int32_t *data = malloc(length > 0 ? (size_t) length * sizeof(int32_t) : sizeof(int32_t));
        for (int32_t i = 0; i < length; i++)
            data[i] = read_int(stream);
        LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_IntArrayTag_Of(data, (size_t) length);
        free(data);
        return tag;
    }
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
    {
        LIBMATTI_MC_NbtAccounter_AccountBytes(stream->accounter, 24);
        int32_t length = read_int(stream);
        if (length < 0)
        {
            stream->overflow = 1;
            return NULL;
        }
        LIBMATTI_MC_NbtAccounter_AccountBytesFactor(stream->accounter, 8, length);
        int64_t *data = malloc(length > 0 ? (size_t) length * sizeof(int64_t) : sizeof(int64_t));
        for (int32_t i = 0; i < length; i++)
            data[i] = read_long(stream);
        LIBMATTI_MC_Nbt_Tag *tag = LIBMATTI_MC_Nbt_LongArrayTag_Of(data, (size_t) length);
        free(data);
        return tag;
    }
    default:
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Failed to read NBT data: unknown tag type");
        stream->overflow = 1;
        return NULL;
    }
    }
}

static void store_tag(Stream *stream, const LIBMATTI_MC_Nbt_Tag *tag);

// Java: private static void writeNamedTag(String, Tag, DataOutput)
static void write_named_tag(Stream *stream, const char *key, const LIBMATTI_MC_Nbt_Tag *tag)
{
    write_byte(stream, (int8_t) tag->id);
    if (tag->id != 0)
    {
        write_string(stream, key);
        store_tag(stream, tag);
    }
}

static void store_compound_contents(Stream *stream, const LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    for (size_t i = 0; i < compound->count; i++)
        write_named_tag(stream, compound->keys[i], compound->values[i]);
    write_byte(stream, 0);
}

static void store_tag(Stream *stream, const LIBMATTI_MC_Nbt_Tag *tag)
{
    switch (tag->id)
    {
    case LIBMATTI_MC_Nbt_TAG_BYTE:
        write_byte(stream, tag->as.byteValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_SHORT:
        write_short(stream, tag->as.shortValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_INT:
        write_int(stream, tag->as.intValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_LONG:
        write_long(stream, tag->as.longValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_FLOAT:
        write_float(stream, tag->as.floatValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_DOUBLE:
        write_double(stream, tag->as.doubleValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_BYTE_ARRAY:
    {
        write_int(stream, (int32_t) tag->as.byteArray.length);
        stream_put(stream, tag->as.byteArray.data, tag->as.byteArray.length);
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_STRING:
        write_string(stream, tag->as.stringValue);
        break;
    case LIBMATTI_MC_Nbt_TAG_LIST:
    {
        const LIBMATTI_MC_Nbt_ListTag *list = (const LIBMATTI_MC_Nbt_ListTag *) tag;
        uint8_t elementType = LIBMATTI_MC_Nbt_ListTag_IdentifyRawElementType(list);
        write_byte(stream, (int8_t) elementType);
        write_int(stream, (int32_t) list->count);
        for (size_t i = 0; i < list->count; i++)
        {
            // Java: wrapIfNeeded - non-compound lists wrap every element
            if (elementType != LIBMATTI_MC_Nbt_TAG_COMPOUND &&
                list->elements[i]->id != elementType)
            {
                LIBMATTI_MC_Nbt_CompoundTag *wrapper = LIBMATTI_MC_Nbt_CompoundTag_New();
                LIBMATTI_MC_Nbt_CompoundTag_Put(wrapper, "", LIBMATTI_MC_Nbt_Tag_Copy(list->elements[i]));
                store_tag(stream, (const LIBMATTI_MC_Nbt_Tag *) wrapper);
                LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(wrapper);
                free(wrapper);
            }
            else
            {
                store_tag(stream, list->elements[i]);
            }
        }
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_COMPOUND:
        store_compound_contents(stream, (const LIBMATTI_MC_Nbt_CompoundTag *) tag);
        break;
    case LIBMATTI_MC_Nbt_TAG_INT_ARRAY:
    {
        write_int(stream, (int32_t) tag->as.intArray.length);
        for (size_t i = 0; i < tag->as.intArray.length; i++)
            write_int(stream, tag->as.intArray.data[i]);
        break;
    }
    case LIBMATTI_MC_Nbt_TAG_LONG_ARRAY:
    {
        write_int(stream, (int32_t) tag->as.longArray.length);
        for (size_t i = 0; i < tag->as.longArray.length; i++)
            write_long(stream, tag->as.longArray.data[i]);
        break;
    }
    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// the public surface
// ---------------------------------------------------------------------------

// Java: public static CompoundTag read(DataInput) - readUnnamedTag + the compound cast
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_Read(const uint8_t *data, size_t length,
                                                        LIBMATTI_MC_Nbt_AccounterView *accounter)
{
    LIBMATTI_MC_NbtAccounter local;
    if (accounter == NULL)
    {
        local = LIBMATTI_MC_NbtAccounter_UnlimitedHeap();
        accounter = &local;
    }
    Stream stream;
    stream_init_read(&stream, data, length, accounter);

    uint8_t typeId = (uint8_t) read_byte(&stream);
    if (typeId != LIBMATTI_MC_Nbt_TAG_COMPOUND)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Root tag must be a named compound tag");
        return NULL;
    }
    // Java: skipString then the type's load
    skip_string(&stream);
    LIBMATTI_MC_Nbt_Tag *tag = load_tag(&stream, typeId);
    if (tag == NULL || stream.overflow)
    {
        LIBMATTI_MC_Nbt_Tag_Free(tag);
        return NULL;
    }
    return (LIBMATTI_MC_Nbt_CompoundTag *) tag;
}

// Java: public static void write(CompoundTag, DataOutput) - writeUnnamedTag with the
// empty root name
int LIBMATTI_MC_Nbt_NbtIo_Write(LIBMATTI_MC_Nbt_CompoundTag *compound, uint8_t **outData, size_t *outLength)
{
    Stream stream;
    stream_init_write(&stream);
    write_byte(&stream, (int8_t) LIBMATTI_MC_Nbt_TAG_COMPOUND);
    write_string(&stream, "");
    store_tag(&stream, (const LIBMATTI_MC_Nbt_Tag *) compound);
    *outData = stream.out;
    *outLength = stream.outLength;
    return 1;
}

// Java: readCompressed - GZIPInputStream over the bytes
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadCompressed(const uint8_t *data, size_t length,
                                                                  LIBMATTI_MC_Nbt_AccounterView *accounter)
{
    LIBMATTI_MC_NbtAccounter local;
    if (accounter == NULL)
    {
        local = LIBMATTI_MC_NbtAccounter_UnlimitedHeap();
        accounter = &local;
    }

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    // +32 lets zlib accept the gzip wrapper (Java: GZIPInputStream)
    if (inflateInit2(&zs, 15 + 32) != Z_OK)
        return NULL;

    // the compressed size is unknown; inflate until the stream ends
    size_t capacity = length * 4 + 1024;
    uint8_t *out = malloc(capacity);
    size_t outLength = 0;

    zs.next_in = (Bytef *) data;
    zs.avail_in = (uInt) length;
    int status;
    do
    {
        if (outLength == capacity)
        {
            capacity *= 2;
            out = realloc(out, capacity);
        }
        zs.next_out = out + outLength;
        zs.avail_out = (uInt) (capacity - outLength);
        status = inflate(&zs, Z_NO_FLUSH);
        if (status != Z_OK && status != Z_STREAM_END && status != Z_BUF_ERROR)
        {
            inflateEnd(&zs);
            free(out);
            return NULL;
        }
        outLength = capacity - zs.avail_out;
    } while (status != Z_STREAM_END);
    inflateEnd(&zs);

    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_NbtIo_Read(out, outLength, accounter);
    free(out);
    return compound;
}

// Java: writeCompressed - GZIPOutputStream over the bytes
int LIBMATTI_MC_Nbt_NbtIo_WriteCompressed(LIBMATTI_MC_Nbt_CompoundTag *compound, uint8_t **outData,
                                          size_t *outLength)
{
    uint8_t *raw;
    size_t rawLength;
    if (!LIBMATTI_MC_Nbt_NbtIo_Write(compound, &raw, &rawLength))
        return 0;

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    // Java: GZIPOutputStream
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        free(raw);
        return 0;
    }

    size_t capacity = deflateBound(&zs, (uLong) rawLength) + 64;
    uint8_t *out = malloc(capacity);
    zs.next_in = raw;
    zs.avail_in = (uInt) rawLength;
    zs.next_out = out;
    zs.avail_out = (uInt) capacity;
    int status = deflate(&zs, Z_FINISH);
    deflateEnd(&zs);
    free(raw);
    if (status != Z_STREAM_END)
    {
        free(out);
        return 0;
    }
    *outData = out;
    *outLength = capacity - zs.avail_out;
    return 1;
}

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadCompressedFile(const char *path,
                                                                      LIBMATTI_MC_Nbt_AccounterView *accounter)
{
    size_t length = 0;
    unsigned char *data = nbt_file_read_all(path, &length);
    if (data == NULL)
        return NULL;
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_NbtIo_ReadCompressed(data, length, accounter);
    free(data);
    return compound;
}

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadFile(const char *path,
                                                            LIBMATTI_MC_Nbt_AccounterView *accounter)
{
    size_t length = 0;
    unsigned char *data = nbt_file_read_all(path, &length);
    if (data == NULL)
        return NULL;
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_NbtIo_Read(data, length, accounter);
    free(data);
    return compound;
}

int LIBMATTI_MC_Nbt_NbtIo_WriteCompressedFile(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *path)
{
    uint8_t *data;
    size_t length;
    if (!LIBMATTI_MC_Nbt_NbtIo_WriteCompressed(compound, &data, &length))
        return 0;
    FILE *file = fopen(path, "wb");
    if (file == NULL)
    {
        free(data);
        return 0;
    }
    fwrite(data, 1, length, file);
    fclose(file);
    free(data);
    return 1;
}

int LIBMATTI_MC_Nbt_NbtIo_WriteFile(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *path)
{
    uint8_t *data;
    size_t length;
    if (!LIBMATTI_MC_Nbt_NbtIo_Write(compound, &data, &length))
        return 0;
    FILE *file = fopen(path, "wb");
    if (file == NULL)
    {
        free(data);
        return 0;
    }
    fwrite(data, 1, length, file);
    fclose(file);
    free(data);
    return 1;
}
