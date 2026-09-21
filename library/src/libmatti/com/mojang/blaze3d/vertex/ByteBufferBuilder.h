// Port of com.mojang.blaze3d.vertex.ByteBufferBuilder.
// Java allocates through MemoryUtil and hands out Result views by (offset,
// capacity, generation); the port keeps the same generation scheme so a Result
// from a discarded batch is detected as invalid exactly like Java.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_BYTEBUFFERBUILDER_H
#define MATTICRAFT_BLAZE3D_VERTEX_BYTEBUFFERBUILDER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class ByteBufferBuilder implements AutoCloseable
typedef struct LIBMATTI_B3D_ByteBufferBuilder LIBMATTI_B3D_ByteBufferBuilder;

// Java: public class ByteBufferBuilder.Result implements AutoCloseable
typedef struct LIBMATTI_B3D_ByteBufferBuilder_Result LIBMATTI_B3D_ByteBufferBuilder_Result;

// Java: public ByteBufferBuilder(int initialCapacity) - maxCapacity 0xFFFFFFFF.
LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_New(int initialCapacity);
// Java: public ByteBufferBuilder(int initialCapacity, long maxCapacity)
LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_NewMax(int initialCapacity, long long maxCapacity);
// Java: public static ByteBufferBuilder exactlySized(int size)
LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_ExactlySized(int size);
void LIBMATTI_B3D_ByteBufferBuilder_Free(LIBMATTI_B3D_ByteBufferBuilder *builder);

// Java: public long reserve(int bytes) - returns the absolute write pointer.
unsigned char *LIBMATTI_B3D_ByteBufferBuilder_Reserve(LIBMATTI_B3D_ByteBufferBuilder *builder, int bytes);

// Java: public @Nullable Result build() - NULL when nothing was written since
// the last result; the Result stays owned by the caller.
LIBMATTI_B3D_ByteBufferBuilder_Result *LIBMATTI_B3D_ByteBufferBuilder_Build(LIBMATTI_B3D_ByteBufferBuilder *builder);

// Java: public void clear() - warns about unused batches, then discards.
void LIBMATTI_B3D_ByteBufferBuilder_Clear(LIBMATTI_B3D_ByteBufferBuilder *builder);
// Java: public void discard()
void LIBMATTI_B3D_ByteBufferBuilder_Discard(LIBMATTI_B3D_ByteBufferBuilder *builder);

// ---- Result --------------------------------------------------------------
// Java: public ByteBuffer byteBuffer() - NULL once the generation is invalid.
unsigned char *LIBMATTI_B3D_ByteBufferBuilder_Result_ByteBuffer(
    const LIBMATTI_B3D_ByteBufferBuilder_Result *result, size_t *outSize);
// Java: public void close() - returns the result slot to the builder.
void LIBMATTI_B3D_ByteBufferBuilder_Result_Free(LIBMATTI_B3D_ByteBufferBuilder_Result *result);

#ifdef __cplusplus
}
#endif

#endif
