// Port of net.minecraft.world.level.chunk.storage.RegionFileStorage (implementation).
// The region cache mirrors Java's Long2ObjectLinkedOpenHashMap move-to-front with
// the same 256-entry cap; the oldest entry closes on eviction.

#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileStorage.h"

#include "libmatti/net/minecraft/nbt/NbtIo.h"
#include "libmatti/net/minecraft/nbt/NbtAccounter.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Java: FileUtil.createDirectoriesSafe(folder)
static int create_directories(const char *folder)
{
    char copy[1024];
    snprintf(copy, sizeof(copy), "%s", folder);
    size_t length = strlen(copy);
    while (length > 1 && copy[length - 1] == '/')
        copy[--length] = '\0';
    for (size_t i = 1; i <= length; i++)
    {
        if (copy[i] == '/' || copy[i] == '\0')
        {
            char saved = copy[i];
            copy[i] = '\0';
            // mkdir fails with EEXIST when the folder is already there - fine
            if (mkdir(copy, 0777) != 0 && errno != EEXIST)
                return -1;
            copy[i] = saved;
        }
    }
    struct stat st;
    return stat(folder, &st) == 0 && S_ISDIR(st.st_mode) ? 0 : -1;
}

// Java: getRegionFile(ChunkPos) - the cache hit moves to front, the miss opens
// r.<rx>.<rz>.mca and evicts the oldest entry at the cap
static LIBMATTI_MC_RegionFile *get_region_file(LIBMATTI_MC_RegionFileStorage *storage, const LIBMATTI_MC_ChunkPos *pos)
{
    int64_t key = LIBMATTI_MC_ChunkPos_AsLong(LIBMATTI_MC_ChunkPos_GetRegionX(pos),
                                              LIBMATTI_MC_ChunkPos_GetRegionZ(pos));

    for (int i = 0; i < storage->cacheCount; i++)
    {
        if (storage->cacheKeys[i] == key)
        {
            // Java: getAndMoveToFirst
            LIBMATTI_MC_RegionFile *hit = storage->cache[i];
            memmove(storage->cache + 1, storage->cache, (size_t) i * sizeof(LIBMATTI_MC_RegionFile *));
            memmove(storage->cacheKeys + 1, storage->cacheKeys, (size_t) i * sizeof(int64_t));
            storage->cache[0] = hit;
            storage->cacheKeys[0] = key;
            return hit;
        }
    }

    if (storage->cacheCount >= LIBMATTI_MC_RegionFileStorage_MAX_CACHE_SIZE)
    {
        // Java: removeLast().close()
        LIBMATTI_MC_RegionFile_Free(storage->cache[storage->cacheCount - 1]);
        storage->cacheCount--;
    }

    create_directories(storage->folder);
    char path[1200];
    snprintf(path, sizeof(path), "%s/r.%d.%d%s", storage->folder, LIBMATTI_MC_ChunkPos_GetRegionX(pos),
             LIBMATTI_MC_ChunkPos_GetRegionZ(pos), LIBMATTI_MC_RegionFileStorage_ANVIL_EXTENSION);
    LIBMATTI_MC_RegionFile *regionFile = LIBMATTI_MC_RegionFile_New(path, storage->folder, -1);
    if (regionFile == NULL)
        return NULL;

    if (storage->cacheCount < LIBMATTI_MC_RegionFileStorage_MAX_CACHE_SIZE)
    {
        memmove(storage->cache + 1, storage->cache, (size_t) storage->cacheCount * sizeof(LIBMATTI_MC_RegionFile *));
        memmove(storage->cacheKeys + 1, storage->cacheKeys, (size_t) storage->cacheCount * sizeof(int64_t));
        storage->cacheCount++;
    }
    storage->cache[0] = regionFile;
    storage->cacheKeys[0] = key;
    return regionFile;
}

LIBMATTI_MC_RegionFileStorage *LIBMATTI_MC_RegionFileStorage_New(const char *folder, bool sync)
{
    if (folder == NULL)
        return NULL;
    LIBMATTI_MC_RegionFileStorage *storage = calloc(1, sizeof(LIBMATTI_MC_RegionFileStorage));
    if (storage == NULL)
        return NULL;
    storage->folder = strdup(folder);
    storage->sync = sync;
    if (storage->folder == NULL)
    {
        free(storage);
        return NULL;
    }
    return storage;
}

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_RegionFileStorage_Read(LIBMATTI_MC_RegionFileStorage *storage,
                                                                 const LIBMATTI_MC_ChunkPos *pos)
{
    LIBMATTI_MC_RegionFile *regionFile = get_region_file(storage, pos);
    if (regionFile == NULL)
        return NULL;
    size_t length = 0;
    uint8_t *data = LIBMATTI_MC_RegionFile_ReadChunk(regionFile, pos, &length);
    if (data == NULL)
        return NULL;
    // Java: NbtIo.read(datainputstream) - the named root compound
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_NbtIo_Read(data, length, NULL);
    free(data);
    return tag;
}

int LIBMATTI_MC_RegionFileStorage_Write(LIBMATTI_MC_RegionFileStorage *storage, const LIBMATTI_MC_ChunkPos *pos,
                                        LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    LIBMATTI_MC_RegionFile *regionFile = get_region_file(storage, pos);
    if (regionFile == NULL)
        return -1;
    if (tag == NULL)
        return LIBMATTI_MC_RegionFile_Clear(regionFile, pos);
    uint8_t *data = NULL;
    size_t length = 0;
    // NbtIo's write pair reports success as a truthy return (Java: void, the
    // stream throws instead)
    if (!LIBMATTI_MC_Nbt_NbtIo_Write(tag, &data, &length))
        return -1;
    int result = LIBMATTI_MC_RegionFile_WriteChunk(regionFile, pos, data, length);
    free(data);
    return result;
}

void LIBMATTI_MC_RegionFileStorage_Flush(LIBMATTI_MC_RegionFileStorage *storage)
{
    if (storage == NULL)
        return;
    for (int i = 0; i < storage->cacheCount; i++)
        LIBMATTI_MC_RegionFile_Flush(storage->cache[i]);
}

int LIBMATTI_MC_RegionFileStorage_Close(LIBMATTI_MC_RegionFileStorage *storage)
{
    if (storage == NULL)
        return 0;
    int result = 0;
    for (int i = 0; i < storage->cacheCount; i++)
    {
        if (LIBMATTI_MC_RegionFile_Close(storage->cache[i]) != 0)
            result = -1;
    }
    return result;
}

void LIBMATTI_MC_RegionFileStorage_Free(LIBMATTI_MC_RegionFileStorage *storage)
{
    if (storage == NULL)
        return;
    for (int i = 0; i < storage->cacheCount; i++)
        LIBMATTI_MC_RegionFile_Free(storage->cache[i]);
    free(storage->folder);
    free(storage);
}
