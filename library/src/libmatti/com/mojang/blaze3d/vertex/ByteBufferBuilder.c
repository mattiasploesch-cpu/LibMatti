#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final long DEFAULT_MAX_CAPACITY = 4294967295L;
#define DEFAULT_MAX_CAPACITY 4294967295LL
// Java: private static final int MAX_GROWTH_SIZE = 2097152;
#define MAX_GROWTH_SIZE 2097152

struct LIBMATTI_B3D_ByteBufferBuilder
{
    unsigned char *pointer;
    long long capacity;
    long long maxCapacity;
    long long writeOffset;
    long long nextResultOffset;
    int resultCount;
    int generation;
};

struct LIBMATTI_B3D_ByteBufferBuilder_Result
{
    LIBMATTI_B3D_ByteBufferBuilder *builder;
    long long offset;
    int capacity;
    int generation;
    int closed;
};

static LIBMATTI_ML_Logger *logger(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_NewMax(int initialCapacity, long long maxCapacity)
{
    LIBMATTI_B3D_ByteBufferBuilder *builder = calloc(1, sizeof(LIBMATTI_B3D_ByteBufferBuilder));
    builder->capacity = initialCapacity;
    builder->maxCapacity = maxCapacity;
    // Java: this.pointer = ALLOCATOR.malloc(p_408080_); OOM on 0.
    builder->pointer = malloc((size_t) initialCapacity);
    if (builder->pointer == NULL)
    {
        free(builder);
        return NULL;
    }
    return builder;
}

LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_New(int initialCapacity)
{
    return LIBMATTI_B3D_ByteBufferBuilder_NewMax(initialCapacity, DEFAULT_MAX_CAPACITY);
}

LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_B3D_ByteBufferBuilder_ExactlySized(int size)
{
    return LIBMATTI_B3D_ByteBufferBuilder_NewMax(size, size);
}

void LIBMATTI_B3D_ByteBufferBuilder_Free(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    if (builder == NULL)
        return;
    // Java: public void close() - frees the pointer, generation -1 invalidates
    // every outstanding Result.
    if (builder->pointer != NULL)
    {
        free(builder->pointer);
        builder->pointer = NULL;
        builder->generation = -1;
    }
    free(builder);
}

// Java: private void resize(long size)
static int resize(LIBMATTI_B3D_ByteBufferBuilder *builder, long long size)
{
    unsigned char *grown = realloc(builder->pointer, (size_t) size);
    if (grown == NULL)
        return 0;
    LIBMATTI_ML_Logger_Debug(logger(), NULL, "Needed to grow BufferBuilder buffer", NULL);
    builder->pointer = grown;
    builder->capacity = size;
    return 1;
}

// Java: private void ensureCapacity(long required)
static int ensure_capacity(LIBMATTI_B3D_ByteBufferBuilder *builder, long long required)
{
    if (required <= builder->capacity)
        return 1;
    if (required > builder->maxCapacity)
        return 0;

    // Java: long i = Math.min(this.capacity, 2097152L);
    //       long j = Mth.clamp(this.capacity + i, required, this.maxCapacity);
    long long growth = builder->capacity < MAX_GROWTH_SIZE ? builder->capacity : MAX_GROWTH_SIZE;
    long long newCapacity = builder->capacity + growth;
    if (newCapacity < required)
        newCapacity = required;
    if (newCapacity > builder->maxCapacity)
        newCapacity = builder->maxCapacity;
    return resize(builder, newCapacity);
}

unsigned char *LIBMATTI_B3D_ByteBufferBuilder_Reserve(LIBMATTI_B3D_ByteBufferBuilder *builder, int bytes)
{
    long long offset = builder->writeOffset;
    long long end = offset + bytes;
    if (!ensure_capacity(builder, end))
        return NULL;
    builder->writeOffset = end;
    // Java: return Math.addExact(this.pointer, i);
    return builder->pointer + offset;
}

LIBMATTI_B3D_ByteBufferBuilder_Result *LIBMATTI_B3D_ByteBufferBuilder_Build(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    if (builder->pointer == NULL)
        return NULL;
    long long offset = builder->nextResultOffset;
    long long size = builder->writeOffset - offset;
    if (size == 0)
        return NULL;
    // Java: throw on results larger than INT_MAX; the port returns NULL.
    if (size > 2147483647LL)
        return NULL;

    builder->nextResultOffset = builder->writeOffset;
    builder->resultCount++;
    LIBMATTI_B3D_ByteBufferBuilder_Result *result = malloc(sizeof(LIBMATTI_B3D_ByteBufferBuilder_Result));
    result->builder = builder;
    result->offset = offset;
    result->capacity = (int) size;
    result->generation = builder->generation;
    result->closed = 0;
    return result;
}

// Java: private void discardResults()
static void discard_results(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    long long remaining = builder->writeOffset - builder->nextResultOffset;
    if (remaining > 0)
        memmove(builder->pointer, builder->pointer + builder->nextResultOffset, (size_t) remaining);
    builder->writeOffset = remaining;
    builder->nextResultOffset = 0;
    builder->generation++;
}

void LIBMATTI_B3D_ByteBufferBuilder_Clear(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    // Java: if (this.resultCount > 0) LOGGER.warn("Clearing BufferBuilder with unused batches");
    if (builder->resultCount > 0)
        LIBMATTI_ML_Logger_Warn(logger(), NULL, "Clearing BufferBuilder with unused batches");
    LIBMATTI_B3D_ByteBufferBuilder_Discard(builder);
}

void LIBMATTI_B3D_ByteBufferBuilder_Discard(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    if (builder->resultCount > 0)
    {
        discard_results(builder);
        builder->resultCount = 0;
    }
}

// Java: boolean isValid(int generation)
static int is_valid(const LIBMATTI_B3D_ByteBufferBuilder *builder, int generation)
{
    return generation == builder->generation;
}

// Java: void freeResult() - the builder's internal counter.
static void free_result(LIBMATTI_B3D_ByteBufferBuilder *builder)
{
    if (--builder->resultCount <= 0)
        discard_results(builder);
}

unsigned char *LIBMATTI_B3D_ByteBufferBuilder_Result_ByteBuffer(
    const LIBMATTI_B3D_ByteBufferBuilder_Result *result, size_t *outSize)
{
    if (result == NULL || !is_valid(result->builder, result->generation))
    {
        if (outSize != NULL)
            *outSize = 0;
        return NULL;
    }
    if (outSize != NULL)
        *outSize = (size_t) result->capacity;
    return result->builder->pointer + result->offset;
}

void LIBMATTI_B3D_ByteBufferBuilder_Result_Free(LIBMATTI_B3D_ByteBufferBuilder_Result *result)
{
    if (result == NULL)
        return;
    if (!result->closed)
    {
        result->closed = 1;
        if (is_valid(result->builder, result->generation))
            free_result(result->builder);
    }
    free(result);
}
