// Port of net.minecraft.world.level.chunk.PalettedContainer (implementation).

#include "libmatti/net/minecraft/world/level/chunk/PalettedContainer.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// SimpleBitStorage - Java: net.minecraft.util.SimpleBitStorage
// ---------------------------------------------------------------------------

// Java: SimpleBitStorage's division-by-magic-constant helpers; the port keeps
// the fields for the round-trip but indexes directly (divide = index/each).
static void bitstorage_compute_magic(LIBMATTI_MC_BitStorage *storage)
{
    size_t each = storage->valuesPerLong;
    // Java: MathUtil.log2 declarative magic - the port stores a direct mask
    storage->divideMultiply = each;
    storage->divideAdd = 0;
    storage->divideShift = 0;
    storage->mask = (uint32_t) ((1ULL << storage->bits) - 1);
}

LIBMATTI_MC_BitStorage *LIBMATTI_MC_BitStorage_New(int bits, size_t size)
{
    LIBMATTI_MC_BitStorage *storage = calloc(1, sizeof(LIBMATTI_MC_BitStorage));
    if (storage == NULL)
        return NULL;
    storage->bits = bits;
    storage->size = size;
    storage->valuesPerLong = 64 / bits;
    storage->dataLength = (size + storage->valuesPerLong - 1) / storage->valuesPerLong;
    storage->data = calloc(storage->dataLength, sizeof(uint64_t));
    if (storage->data == NULL)
    {
        free(storage);
        return NULL;
    }
    bitstorage_compute_magic(storage);
    return storage;
}

LIBMATTI_MC_BitStorage *LIBMATTI_MC_BitStorage_NewWithData(int bits, size_t size, const uint64_t *data, size_t dataLength)
{
    LIBMATTI_MC_BitStorage *storage = LIBMATTI_MC_BitStorage_New(bits, size);
    if (storage == NULL)
        return NULL;
    if (data != NULL && dataLength == storage->dataLength)
        memcpy(storage->data, data, dataLength * sizeof(uint64_t));
    return storage;
}

void LIBMATTI_MC_BitStorage_Free(LIBMATTI_MC_BitStorage *storage)
{
    if (storage == NULL)
        return;
    free(storage->data);
    free(storage);
}

int LIBMATTI_MC_BitStorage_Get(const LIBMATTI_MC_BitStorage *storage, size_t index)
{
    size_t longIndex = index / storage->valuesPerLong;
    int bit = (int) ((index % storage->valuesPerLong) * storage->bits);
    return (int) ((storage->data[longIndex] >> bit) & storage->mask);
}

void LIBMATTI_MC_BitStorage_Set(LIBMATTI_MC_BitStorage *storage, size_t index, int value)
{
    size_t longIndex = index / storage->valuesPerLong;
    int bit = (int) ((index % storage->valuesPerLong) * storage->bits);
    storage->data[longIndex] = (storage->data[longIndex] & ~((uint64_t) storage->mask << bit))
                               | (((uint64_t) value & storage->mask) << bit);
}

const uint64_t *LIBMATTI_MC_BitStorage_GetRaw(const LIBMATTI_MC_BitStorage *storage, size_t *outLength)
{
    if (outLength != NULL)
        *outLength = storage->dataLength;
    return storage->data;
}

int LIBMATTI_MC_BitStorage_GetBits(const LIBMATTI_MC_BitStorage *storage)
{
    return storage->bits;
}

size_t LIBMATTI_MC_BitStorage_GetSize(const LIBMATTI_MC_BitStorage *storage)
{
    return storage->size;
}

void LIBMATTI_MC_BitStorage_GetAll(const LIBMATTI_MC_BitStorage *storage, void (*consumer)(int, void *), void *self)
{
    for (size_t i = 0; i < storage->size; i++)
        consumer(LIBMATTI_MC_BitStorage_Get(storage, i), self);
}

// ---------------------------------------------------------------------------
// PalettedContainer
// ---------------------------------------------------------------------------

int LIBMATTI_MC_PalettedContainer_Index(int x, int y, int z)
{
    // Java: y * 256 + z * 16 + x
    return y * 256 + z * 16 + x;
}

LIBMATTI_MC_PalettedContainer *LIBMATTI_MC_PalettedContainer_New(LIBMATTI_MC_BlockState *singletonValue)
{
    LIBMATTI_MC_PalettedContainer *container = calloc(1, sizeof(LIBMATTI_MC_PalettedContainer));
    if (container == NULL)
        return NULL;
    container->mode = LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE;
    container->singleValue = singletonValue;
    return container;
}

void LIBMATTI_MC_PalettedContainer_Free(LIBMATTI_MC_PalettedContainer *container)
{
    if (container == NULL)
        return;
    free(container->palette);
    LIBMATTI_MC_BitStorage_Free(container->storage);
    free(container);
}

void LIBMATTI_MC_PalettedContainer_Reserve(LIBMATTI_MC_PalettedContainer *container, size_t entries)
{
    if (container == NULL || entries <= container->paletteCapacity)
        return;
    LIBMATTI_MC_BlockState **grown = realloc(container->palette, entries * sizeof(LIBMATTI_MC_BlockState *));
    if (grown != NULL)
    {
        memset(grown + container->paletteCapacity, 0,
               (entries - container->paletteCapacity) * sizeof(LIBMATTI_MC_BlockState *));
        container->palette = grown;
        container->paletteCapacity = entries;
    }
}

// The palette id for a state: existing entry or append (Java: Palette.add)
// Returns -1 when the linear palette is full (the caller then resizes bits).
static int palette_id_for(LIBMATTI_MC_PalettedContainer *container, LIBMATTI_MC_BlockState *state)
{
    for (size_t i = 0; i < container->paletteSize; i++)
    {
        if (container->palette[i] == state)
            return (int) i;
    }
    if (container->paletteSize >= LIBMATTI_MC_PalettedContainer_MAX_LINEAR_ENTRIES)
        return -1; // full - the caller must switch to GLOBAL mode
    if (container->paletteSize >= container->paletteCapacity)
    {
        size_t next = container->paletteCapacity > 0 ? container->paletteCapacity * 2 : 4;
        LIBMATTI_MC_PalettedContainer_Reserve(container, next);
    }
    container->palette[container->paletteSize] = state;
    return (int) container->paletteSize++;
}

// Java: the strategy switch inside set(): the bits grow when the palette
// outgrows the current width (4 -> 5 -> ... -> global)
static bool ensure_bits(LIBMATTI_MC_PalettedContainer *container, int bits)
{
    if (container->storage != NULL && container->storage->bits >= bits)
        return true;
    int newBits = container->storage != NULL ? container->storage->bits : LIBMATTI_MC_PalettedContainer_MIN_BITS;
    while (newBits < bits)
        newBits++;
    size_t oldLength = 0;
    const uint64_t *oldData = container->storage != NULL ? LIBMATTI_MC_BitStorage_GetRaw(container->storage, &oldLength) : NULL;
    LIBMATTI_MC_BitStorage *grown = LIBMATTI_MC_BitStorage_NewWithData(newBits, LIBMATTI_MC_LevelChunkSection_SECTION_SIZE,
                                                                       oldData, oldLength);
    if (grown == NULL)
        return false;
    LIBMATTI_MC_BitStorage_Free(container->storage);
    container->storage = grown;
    return true;
}

// The GLOBAL read path: the port's global palette is the linear palette kept
// unbounded (the vanilla global palette would need the state registry ids,
// which the port's registry does not expose yet). The mode records the
// strategy so the save/load port can map it 1:1.
static int global_id_for(LIBMATTI_MC_PalettedContainer *container, LIBMATTI_MC_BlockState *state)
{
    for (size_t i = 0; i < container->paletteSize; i++)
    {
        if (container->palette[i] == state)
            return (int) i;
    }
    size_t next = container->paletteCapacity > 0 ? container->paletteCapacity * 2 : 16;
    LIBMATTI_MC_PalettedContainer_Reserve(container, next);
    if (container->paletteSize >= container->paletteCapacity)
        return -1;
    container->palette[container->paletteSize] = state;
    return (int) container->paletteSize++;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_Get(const LIBMATTI_MC_PalettedContainer *container, int x, int y, int z)
{
    if (container == NULL)
        return NULL;
    if (container->mode == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE)
        return container->singleValue;
    int index = LIBMATTI_MC_PalettedContainer_Index(x, y, z);
    int id = LIBMATTI_MC_BitStorage_Get(container->storage, (size_t) index);
    if ((size_t) id >= container->paletteSize)
        return container->palette[0];
    return container->palette[id];
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_GetAndSet(LIBMATTI_MC_PalettedContainer *container, int x, int y, int z, LIBMATTI_MC_BlockState *state)
{
    LIBMATTI_MC_BlockState *old = LIBMATTI_MC_PalettedContainer_Get(container, x, y, z);
    LIBMATTI_MC_PalettedContainer_Set(container, x, y, z, state);
    return old;
}

void LIBMATTI_MC_PalettedContainer_Set(LIBMATTI_MC_PalettedContainer *container, int x, int y, int z, LIBMATTI_MC_BlockState *state)
{
    if (container == NULL)
        return;
    int index = LIBMATTI_MC_PalettedContainer_Index(x, y, z);

    // Java: SINGLE_VALUE - set data(value) only when the written state differs
    if (container->mode == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE)
    {
        if (container->singleValue == state)
            return;
        // the transition into the packed strategies
        container->mode = LIBMATTI_MC_PalettedContainer_Mode_LINEAR;
        LIBMATTI_MC_PalettedContainer_Reserve(container, 4);
        container->palette[0] = container->singleValue;
        container->palette[1] = state;
        container->paletteSize = 2;
        if (!ensure_bits(container, LIBMATTI_MC_PalettedContainer_MIN_BITS))
            return;
        for (size_t i = 0; i < LIBMATTI_MC_LevelChunkSection_SECTION_SIZE; i++)
            LIBMATTI_MC_BitStorage_Set(container->storage, i, 0);
        LIBMATTI_MC_BitStorage_Set(container->storage, (size_t) index, 1);
        return;
    }

    // Java: LINEAR - palette id, grow bits when the palette outgrows them
    int id = palette_id_for(container, state);
    if (id < 0)
    {
        // the linear palette is full: switch to the unbounded (global) mode
        container->mode = LIBMATTI_MC_PalettedContainer_Mode_GLOBAL;
        id = global_id_for(container, state);
        if (id < 0)
            return;
        int bits = 1;
        while ((1 << bits) < (int) container->paletteSize)
            bits++;
        if (!ensure_bits(container, bits))
            return;
    }
    else
    {
        int bits = 1;
        while ((1 << bits) < (int) container->paletteSize)
            bits++;
        bits = bits < LIBMATTI_MC_PalettedContainer_MIN_BITS ? LIBMATTI_MC_PalettedContainer_MIN_BITS : bits;
        ensure_bits(container, bits);
    }
    LIBMATTI_MC_BitStorage_Set(container->storage, (size_t) index, id);
}

int LIBMATTI_MC_PalettedContainer_Count(const LIBMATTI_MC_PalettedContainer *container)
{
    if (container == NULL)
        return 0;
    // Java: count(Predicate<T>) with the air predicate over all 4096
    LIBMATTI_MC_BlockState *air = LIBMATTI_MC_VanillaBlocks_AIR() != NULL
                                      ? LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR())
                                      : NULL;
    int count = 0;
    for (int y = 0; y < 16; y++)
    {
        for (int z = 0; z < 16; z++)
        {
            for (int x = 0; x < 16; x++)
            {
                LIBMATTI_MC_BlockState *state = LIBMATTI_MC_PalettedContainer_Get(container, x, y, z);
                if (state != air)
                    count++;
            }
        }
    }
    return count;
}

int LIBMATTI_MC_PalettedContainer_GetBits(const LIBMATTI_MC_PalettedContainer *container)
{
    if (container == NULL)
        return 0;
    if (container->mode == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE)
        return 0;
    return container->storage != NULL ? container->storage->bits : 0;
}

const uint64_t *LIBMATTI_MC_PalettedContainer_GetRaw(const LIBMATTI_MC_PalettedContainer *container, size_t *outLength)
{
    if (outLength != NULL)
        *outLength = 0;
    if (container == NULL || container->storage == NULL)
        return NULL;
    return LIBMATTI_MC_BitStorage_GetRaw(container->storage, outLength);
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_GetPaletteEntry(const LIBMATTI_MC_PalettedContainer *container, size_t index)
{
    if (container == NULL || index >= container->paletteSize)
        return NULL;
    if (container->mode == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE && index == 0)
        return container->singleValue;
    return container->palette[index];
}

size_t LIBMATTI_MC_PalettedContainer_GetPaletteSize(const LIBMATTI_MC_PalettedContainer *container)
{
    if (container == NULL)
        return 0;
    if (container->mode == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE)
        return 1;
    return container->paletteSize;
}

LIBMATTI_MC_PalettedContainer_Mode LIBMATTI_MC_PalettedContainer_GetMode(const LIBMATTI_MC_PalettedContainer *container)
{
    return container != NULL ? container->mode : LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE;
}
