// Port of net.minecraft.world.level.chunk.PalettedContainer plus the
// net.minecraft.util.SimpleBitStorage it rides on. Java stores the 4096
// section states through a palette (the distinct states) and a BitStorage
// (the packed indices); single-value and global-palette configurations
// bypass the packed storage like Java's trivial strategy.
//
// The C port keeps the three Java strategies: SINGLE_VALUE (one state, no
// storage), LINEAR (the id -> state palette array) and GLOBAL (the direct
// registry ids, used when the linear palette outgrows its bits).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_PALETTEDCONTAINER_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_PALETTEDCONTAINER_H

#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: PalettedContainer.Strategy - the port fixes the BLOCK strategy's
// dimension (4 bits at minimum, up to 15 palette entries per linear page)
#define LIBMATTI_MC_PalettedContainer_MIN_BITS 4
#define LIBMATTI_MC_PalettedContainer_MAX_LINEAR_ENTRIES 16

// Java: the storage record (SimpleBitStorage): values/bits with the magic
// values precomputed (valuesPerLong, divideMultiply/add/shift)
typedef struct LIBMATTI_MC_BitStorage
{
    int bits;              // bits per entry
    size_t size;           // entry count (4096 for sections)
    size_t valuesPerLong;  // 64 / bits
    uint64_t *data;        // the packed words
    size_t dataLength;     // Java: data.length - divide(size, valuesPerLong)
    // Java: the magic constants (divideMultiply/divideAdd/divideShift)
    uint64_t divideMultiply;
    uint64_t divideAdd;
    int divideShift;
    uint32_t mask;
} LIBMATTI_MC_BitStorage;

// Java: SimpleBitStorage(int bits, int size, long[] data)
LIBMATTI_MC_BitStorage *LIBMATTI_MC_BitStorage_New(int bits, size_t size);
LIBMATTI_MC_BitStorage *LIBMATTI_MC_BitStorage_NewWithData(int bits, size_t size, const uint64_t *data, size_t dataLength);
void LIBMATTI_MC_BitStorage_Free(LIBMATTI_MC_BitStorage *storage);

// Java: public int get(int index) / public void set(int index, int value)
int LIBMATTI_MC_BitStorage_Get(const LIBMATTI_MC_BitStorage *storage, size_t index);
void LIBMATTI_MC_BitStorage_Set(LIBMATTI_MC_BitStorage *storage, size_t index, int value);
// Java: public long[] getRaw()
const uint64_t *LIBMATTI_MC_BitStorage_GetRaw(const LIBMATTI_MC_BitStorage *storage, size_t *outLength);
// Java: public int getBits() / public int getSize()
int LIBMATTI_MC_BitStorage_GetBits(const LIBMATTI_MC_BitStorage *storage);
size_t LIBMATTI_MC_BitStorage_GetSize(const LIBMATTI_MC_BitStorage *storage);
// Java: public void getAll(IntConsumer)
void LIBMATTI_MC_BitStorage_GetAll(const LIBMATTI_MC_BitStorage *storage, void (*consumer)(int value, void *self), void *self);

// Java: PalettedContainer.DiscardData omitted - the port keeps the strategies:
typedef enum LIBMATTI_MC_PalettedContainer_Mode
{
    LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE,
    LIBMATTI_MC_PalettedContainer_Mode_LINEAR,
    LIBMATTI_MC_PalettedContainer_Mode_GLOBAL,
} LIBMATTI_MC_PalettedContainer_Mode;

typedef struct LIBMATTI_MC_PalettedContainer
{
    LIBMATTI_MC_PalettedContainer_Mode mode;
    // Java: private T value - the SINGLE_VALUE strategy's one state
    LIBMATTI_MC_BlockState *singleValue;
    // Java: private Palette<T> palette - the id -> state array (LINEAR)
    LIBMATTI_MC_BlockState **palette;
    size_t paletteSize;      // the used palette entries
    size_t paletteCapacity;  // the allocated entries
    // Java: private Palette<T> palette - the GLOBAL strategy reads registry ids
    // directly; the port keeps no array and indexes the vanilla registry.
    LIBMATTI_MC_BitStorage *storage;
    // Java: private final IdMapper<T> registry - the global palette lookup
    const void *registry; // reserved: the block-state registry for GLOBAL mode
} LIBMATTI_MC_PalettedContainer;

// Java: PalettedContainer(T singletonValue, Strategy, PaletteHolder) - all air
LIBMATTI_MC_PalettedContainer *LIBMATTI_MC_PalettedContainer_New(LIBMATTI_MC_BlockState *singletonValue);
void LIBMATTI_MC_PalettedContainer_Free(LIBMATTI_MC_PalettedContainer *container);
// The resize path Java runs inside set(): the palette outgrew its bits
void LIBMATTI_MC_PalettedContainer_Reserve(LIBMATTI_MC_PalettedContainer *container, size_t entries);

// Java: public T get(int x, int y, int z) / getAndSet(int, int, int, T)
LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_Get(const LIBMATTI_MC_PalettedContainer *container, int x, int y, int z);
LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_GetAndSet(LIBMATTI_MC_PalettedContainer *container, int x, int y, int z, LIBMATTI_MC_BlockState *state);
// Java: public void set(int x, int y, int z, T)
void LIBMATTI_MC_PalettedContainer_Set(LIBMATTI_MC_PalettedContainer *container, int x, int y, int z, LIBMATTI_MC_BlockState *state);
// Java: public int count() - non-air entries (through the storage walk)
int LIBMATTI_MC_PalettedContainer_Count(const LIBMATTI_MC_PalettedContainer *container);

// Java: private int getIndex(int x, int y, int z) - y * 256 + z * 16 + x
int LIBMATTI_MC_PalettedContainer_Index(int x, int y, int z);

// The save/load surface (Java: write/read through the codec):
// packets carry bits + palette states + raw words
int LIBMATTI_MC_PalettedContainer_GetBits(const LIBMATTI_MC_PalettedContainer *container);
const uint64_t *LIBMATTI_MC_PalettedContainer_GetRaw(const LIBMATTI_MC_PalettedContainer *container, size_t *outLength);
LIBMATTI_MC_BlockState *LIBMATTI_MC_PalettedContainer_GetPaletteEntry(const LIBMATTI_MC_PalettedContainer *container, size_t index);
size_t LIBMATTI_MC_PalettedContainer_GetPaletteSize(const LIBMATTI_MC_PalettedContainer *container);
LIBMATTI_MC_PalettedContainer_Mode LIBMATTI_MC_PalettedContainer_GetMode(const LIBMATTI_MC_PalettedContainer *container);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_PALETTEDCONTAINER_H
