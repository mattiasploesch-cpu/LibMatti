// Port of java.nio.ByteBuffer (the heap-buffer surface the loader and game
// need: absolute/relative reads and writes, flip/rewind/clear, slice).

#ifndef MATTICRAFT_JAVA_NIO_BYTEBUFFER_H
#define MATTICRAFT_JAVA_NIO_BYTEBUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct LIBMATTI_JN_ByteBuffer
{
    unsigned char *array;   // Java: the backing array (hb)
    size_t capacity;
    size_t position;
    size_t limit;
    int ownsArray;          // whether Free() releases array
} LIBMATTI_JN_ByteBuffer;

// Java: public static ByteBuffer allocate(int capacity)
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Allocate(size_t capacity);
// Java: public static ByteBuffer wrap(byte[] array) - the buffer does NOT own the array
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Wrap(unsigned char *array, size_t length);
// Java: public static ByteBuffer wrap(byte[] array, int offset, int length)
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_WrapRange(unsigned char *array, size_t offset, size_t length);
void LIBMATTI_JN_ByteBuffer_Free(LIBMATTI_JN_ByteBuffer *buffer);

// Java: public final int capacity() / position() / limit() / remaining()
size_t LIBMATTI_JN_ByteBuffer_Capacity(const LIBMATTI_JN_ByteBuffer *buffer);
size_t LIBMATTI_JN_ByteBuffer_Position(const LIBMATTI_JN_ByteBuffer *buffer);
size_t LIBMATTI_JN_ByteBuffer_Limit(const LIBMATTI_JN_ByteBuffer *buffer);
size_t LIBMATTI_JN_ByteBuffer_Remaining(const LIBMATTI_JN_ByteBuffer *buffer);

// Java: public Buffer position(int newPosition) - 0 on failure
int LIBMATTI_JN_ByteBuffer_SetPosition(LIBMATTI_JN_ByteBuffer *buffer, size_t newPosition);
// Java: public Buffer limit(int newLimit)
int LIBMATTI_JN_ByteBuffer_SetLimit(LIBMATTI_JN_ByteBuffer *buffer, size_t newLimit);

// Java: public Buffer clear() - position=0, limit=capacity
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Clear(LIBMATTI_JN_ByteBuffer *buffer);
// Java: public Buffer flip() - limit=position, position=0
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Flip(LIBMATTI_JN_ByteBuffer *buffer);
// Java: public Buffer rewind() - position=0
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Rewind(LIBMATTI_JN_ByteBuffer *buffer);

// Java: public ByteBuffer slice() - shares the array, owns its own position
LIBMATTI_JN_ByteBuffer *LIBMATTI_JN_ByteBuffer_Slice(const LIBMATTI_JN_ByteBuffer *buffer);

// Java: public ByteBuffer put(byte b) / get() - relative, 0/-1 at limit
int LIBMATTI_JN_ByteBuffer_Put(LIBMATTI_JN_ByteBuffer *buffer, unsigned char value);
int LIBMATTI_JN_ByteBuffer_Get(LIBMATTI_JN_ByteBuffer *buffer);
// Java: public ByteBuffer put(int index, byte b) / get(int index) - absolute
int LIBMATTI_JN_ByteBuffer_PutAt(LIBMATTI_JN_ByteBuffer *buffer, size_t index, unsigned char value);
int LIBMATTI_JN_ByteBuffer_GetAt(const LIBMATTI_JN_ByteBuffer *buffer, size_t index);
// Java: public ByteBuffer put(byte[] src) - bulk relative put
size_t LIBMATTI_JN_ByteBuffer_PutBytes(LIBMATTI_JN_ByteBuffer *buffer, const unsigned char *src, size_t length);
// Java: public ByteBuffer get(byte[] dst) - bulk relative get, returns bytes read
size_t LIBMATTI_JN_ByteBuffer_GetBytes(LIBMATTI_JN_ByteBuffer *buffer, unsigned char *dst, size_t length);

// Java: ByteBuffer.putShort/getShort/putInt/getInt/putLong/getLong (big endian,
// like Java's default ByteOrder.BIG_ENDIAN) - relative versions
int LIBMATTI_JN_ByteBuffer_PutShort(LIBMATTI_JN_ByteBuffer *buffer, uint16_t value);
int LIBMATTI_JN_ByteBuffer_GetShort(LIBMATTI_JN_ByteBuffer *buffer);
int LIBMATTI_JN_ByteBuffer_PutInt(LIBMATTI_JN_ByteBuffer *buffer, uint32_t value);
uint32_t LIBMATTI_JN_ByteBuffer_GetInt(LIBMATTI_JN_ByteBuffer *buffer);
int LIBMATTI_JN_ByteBuffer_PutLong(LIBMATTI_JN_ByteBuffer *buffer, uint64_t value);
uint64_t LIBMATTI_JN_ByteBuffer_GetLong(LIBMATTI_JN_ByteBuffer *buffer);

// Java: public final boolean hasRemaining()
int LIBMATTI_JN_ByteBuffer_HasRemaining(const LIBMATTI_JN_ByteBuffer *buffer);

// Java: public byte[] array() - the backing array without copying
unsigned char *LIBMATTI_JN_ByteBuffer_Array(const LIBMATTI_JN_ByteBuffer *buffer);

#endif //MATTICRAFT_JAVA_NIO_BYTEBUFFER_H
