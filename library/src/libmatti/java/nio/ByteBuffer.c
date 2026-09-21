#include "libmatti/java/nio/ByteBuffer.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Allocate(size_t capacity)
{
    LIBMATTI_JN_ByteBuffer *buffer = calloc(1, sizeof(LIBMATTI_JN_ByteBuffer));
    buffer->array = capacity > 0 ? malloc(capacity) : NULL;
    buffer->capacity = capacity;
    buffer->limit = capacity;
    buffer->ownsArray = 1;
    return buffer;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Wrap(unsigned char *array, size_t length)
{
    LIBMATTI_JN_ByteBuffer *buffer = calloc(1, sizeof(LIBMATTI_JN_ByteBuffer));
    buffer->array = array;
    buffer->capacity = length;
    buffer->limit = length;
    buffer->ownsArray = 0;
    return buffer;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_WrapRange(unsigned char *array, size_t offset, size_t length)
{
    LIBMATTI_JN_ByteBuffer *buffer = LIBMATTI_JN_ByteBuffer_Wrap(array, offset + length);
    buffer->position = offset;
    return buffer;
}

void LIBMATTI_JN_ByteBuffer_Free(LIBMATTI_JN_ByteBuffer *buffer)
{
    if (buffer == NULL)
        return;
    if (buffer->ownsArray)
        free(buffer->array);
    free(buffer);
}

size_t LIBMATTI_JN_ByteBuffer_Capacity(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->capacity;
}

size_t LIBMATTI_JN_ByteBuffer_Position(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->position;
}

size_t LIBMATTI_JN_ByteBuffer_Limit(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->limit;
}

size_t LIBMATTI_JN_ByteBuffer_Remaining(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->limit - buffer->position;
}

int LIBMATTI_JN_ByteBuffer_SetPosition(LIBMATTI_JN_ByteBuffer *buffer, size_t newPosition)
{
    if (newPosition > buffer->limit)
        return 0;
    buffer->position = newPosition;
    return 1;
}

int LIBMATTI_JN_ByteBuffer_SetLimit(LIBMATTI_JN_ByteBuffer *buffer, size_t newLimit)
{
    if (newLimit > buffer->capacity)
        return 0;
    buffer->limit = newLimit;
    if (buffer->position > newLimit)
        buffer->position = newLimit;
    return 1;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Clear(LIBMATTI_JN_ByteBuffer *buffer)
{
    buffer->position = 0;
    buffer->limit = buffer->capacity;
    return buffer;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Flip(LIBMATTI_JN_ByteBuffer *buffer)
{
    buffer->limit = buffer->position;
    buffer->position = 0;
    return buffer;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Rewind(LIBMATTI_JN_ByteBuffer *buffer)
{
    buffer->position = 0;
    return buffer;
}

LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Slice(const LIBMATTI_JN_ByteBuffer *buffer)
{
    size_t remaining = LIBMATTI_JN_ByteBuffer_Remaining(buffer);
    LIBMATTI_JN_ByteBuffer *slice = calloc(1, sizeof(LIBMATTI_JN_ByteBuffer));
    slice->array = buffer->array + buffer->position;
    slice->capacity = remaining;
    slice->limit = remaining;
    slice->ownsArray = 0;
    return slice;
}

int LIBMATTI_JN_ByteBuffer_Put(LIBMATTI_JN_ByteBuffer *buffer, unsigned char value)
{
    if (buffer->position >= buffer->limit)
        return 0;
    buffer->array[buffer->position++] = value;
    return 1;
}

int LIBMATTI_JN_ByteBuffer_Get(LIBMATTI_JN_ByteBuffer *buffer)
{
    if (buffer->position >= buffer->limit)
        return -1;
    return buffer->array[buffer->position++];
}

int LIBMATTI_JN_ByteBuffer_PutAt(LIBMATTI_JN_ByteBuffer *buffer, size_t index, unsigned char value)
{
    if (index >= buffer->limit)
        return 0;
    buffer->array[index] = value;
    return 1;
}

int LIBMATTI_JN_ByteBuffer_GetAt(const LIBMATTI_JN_ByteBuffer *buffer, size_t index)
{
    if (index >= buffer->limit)
        return -1;
    return buffer->array[index];
}

size_t LIBMATTI_JN_ByteBuffer_PutBytes(LIBMATTI_JN_ByteBuffer *buffer, const unsigned char *src, size_t length)
{
    size_t remaining = LIBMATTI_JN_ByteBuffer_Remaining(buffer);
    if (length > remaining)
        length = remaining;
    memcpy(buffer->array + buffer->position, src, length);
    buffer->position += length;
    return length;
}

size_t LIBMATTI_JN_ByteBuffer_GetBytes(LIBMATTI_JN_ByteBuffer *buffer, unsigned char *dst, size_t length)
{
    size_t remaining = LIBMATTI_JN_ByteBuffer_Remaining(buffer);
    if (length > remaining)
        length = remaining;
    memcpy(dst, buffer->array + buffer->position, length);
    buffer->position += length;
    return length;
}

// Java's default ByteOrder is BIG_ENDIAN for the typed accessors
int LIBMATTI_JN_ByteBuffer_PutShort(LIBMATTI_JN_ByteBuffer *buffer, uint16_t value)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 2)
        return 0;
    buffer->array[buffer->position++] = (unsigned char) (value >> 8);
    buffer->array[buffer->position++] = (unsigned char) value;
    return 1;
}

int LIBMATTI_JN_ByteBuffer_GetShort(LIBMATTI_JN_ByteBuffer *buffer)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 2)
        return -1;
    unsigned short value = (unsigned short) (buffer->array[buffer->position] << 8 |
                                             buffer->array[buffer->position + 1]);
    buffer->position += 2;
    return value;
}

int LIBMATTI_JN_ByteBuffer_PutInt(LIBMATTI_JN_ByteBuffer *buffer, uint32_t value)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 4)
        return 0;
    buffer->array[buffer->position++] = (unsigned char) (value >> 24);
    buffer->array[buffer->position++] = (unsigned char) (value >> 16);
    buffer->array[buffer->position++] = (unsigned char) (value >> 8);
    buffer->array[buffer->position++] = (unsigned char) value;
    return 1;
}

uint32_t LIBMATTI_JN_ByteBuffer_GetInt(LIBMATTI_JN_ByteBuffer *buffer)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 4)
        return 0;
    uint32_t value = ((uint32_t) buffer->array[buffer->position] << 24) |
                     ((uint32_t) buffer->array[buffer->position + 1] << 16) |
                     ((uint32_t) buffer->array[buffer->position + 2] << 8) |
                     (uint32_t) buffer->array[buffer->position + 3];
    buffer->position += 4;
    return value;
}

int LIBMATTI_JN_ByteBuffer_PutLong(LIBMATTI_JN_ByteBuffer *buffer, uint64_t value)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 8)
        return 0;
    for (int shift = 56; shift >= 0; shift -= 8)
        buffer->array[buffer->position++] = (unsigned char) (value >> shift);
    return 1;
}

uint64_t LIBMATTI_JN_ByteBuffer_GetLong(LIBMATTI_JN_ByteBuffer *buffer)
{
    if (LIBMATTI_JN_ByteBuffer_Remaining(buffer) < 8)
        return 0;
    uint64_t value = 0;
    for (int i = 0; i < 8; i++)
        value = (value << 8) | buffer->array[buffer->position + i];
    buffer->position += 8;
    return value;
}

int LIBMATTI_JN_ByteBuffer_HasRemaining(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->position < buffer->limit;
}

unsigned char *LIBMATTI_JN_ByteBuffer_Array(const LIBMATTI_JN_ByteBuffer *buffer)
{
    return buffer->array;
}
