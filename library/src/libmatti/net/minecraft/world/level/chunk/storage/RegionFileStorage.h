// Port of net.minecraft.world.level.chunk.storage.RegionFileStorage (the
// per-folder region container with the 256-entry LRU cache). Java's IOWorker
// sits in front of this class and saves asynchronously; the port writes
// synchronously on the caller's thread (the game port's flush points decide
// when).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILESTORAGE_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILESTORAGE_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFile.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final String ANVIL_EXTENSION = ".mca"
#define LIBMATTI_MC_RegionFileStorage_ANVIL_EXTENSION ".mca"
// Java: private static final int MAX_CACHE_SIZE = 256
#define LIBMATTI_MC_RegionFileStorage_MAX_CACHE_SIZE 256

// Java: public final class RegionFileStorage implements AutoCloseable
typedef struct LIBMATTI_MC_RegionFileStorage
{
    char *folder; // Java: private final Path folder (owned)
    bool sync;    // Java: private final boolean sync
    // Java: private final Long2ObjectLinkedOpenHashMap<RegionFile> regionCache -
    // the entry at index 0 is the most recently used
    LIBMATTI_MC_RegionFile *cache[LIBMATTI_MC_RegionFileStorage_MAX_CACHE_SIZE];
    int64_t cacheKeys[LIBMATTI_MC_RegionFileStorage_MAX_CACHE_SIZE];
    int cacheCount;
} LIBMATTI_MC_RegionFileStorage;

// Java: RegionFileStorage(RegionStorageInfo, Path, boolean)
LIBMATTI_MC_RegionFileStorage *LIBMATTI_MC_RegionFileStorage_New(const char *folder, bool sync);
// Java: public CompoundTag read(ChunkPos) - NULL when the chunk is absent; the
// caller owns the compound
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_RegionFileStorage_Read(LIBMATTI_MC_RegionFileStorage *storage,
                                                                 const LIBMATTI_MC_ChunkPos *pos);
// Java: protected void write(ChunkPos, CompoundTag) - NULL clears the entry
int LIBMATTI_MC_RegionFileStorage_Write(LIBMATTI_MC_RegionFileStorage *storage, const LIBMATTI_MC_ChunkPos *pos,
                                        LIBMATTI_MC_Nbt_CompoundTag *tag);
// Java: public void close() / flush()
void LIBMATTI_MC_RegionFileStorage_Flush(LIBMATTI_MC_RegionFileStorage *storage);
int LIBMATTI_MC_RegionFileStorage_Close(LIBMATTI_MC_RegionFileStorage *storage);
void LIBMATTI_MC_RegionFileStorage_Free(LIBMATTI_MC_RegionFileStorage *storage);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILESTORAGE_H
