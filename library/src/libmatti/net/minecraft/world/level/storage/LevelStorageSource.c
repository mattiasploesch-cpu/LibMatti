// Port of net.minecraft.world.level.storage.LevelStorageSource (implementation).
// The level.dat file rides the gzip NbtIo pair (Java: NbtIo.readCompressed over
// LEVEL_DATA_FILE); the dimension folders follow getDimensionPath's layout.

#include "libmatti/net/minecraft/world/level/storage/LevelStorageSource.h"

#include "libmatti/net/minecraft/SharedConstants.h"
#include "libmatti/net/minecraft/nbt/NbtIo.h"
#include "libmatti/net/minecraft/nbt/NbtAccounter.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/SerializableChunkData.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static char *join_path(const char *a, const char *b)
{
    size_t length = strlen(a) + strlen(b) + 2;
    char *out = malloc(length);
    if (out == NULL)
        return NULL;
    snprintf(out, length, "%s/%s", a, b);
    return out;
}

static char *join_path3(const char *a, const char *b, const char *c)
{
    size_t length = strlen(a) + strlen(b) + strlen(c) + 3;
    char *out = malloc(length);
    if (out == NULL)
        return NULL;
    snprintf(out, length, "%s/%s/%s", a, b, c);
    return out;
}

static int make_dirs(const char *path)
{
    char copy[1024];
    snprintf(copy, sizeof(copy), "%s", path);
    size_t length = strlen(copy);
    while (length > 1 && copy[length - 1] == '/')
        copy[--length] = '\0';
    for (size_t i = 1; i <= length; i++)
    {
        if (copy[i] == '/' || copy[i] == '\0')
        {
            char saved = copy[i];
            copy[i] = '\0';
            if (mkdir(copy, 0777) != 0)
            {
                struct stat st;
                if (stat(copy, &st) != 0 || !S_ISDIR(st.st_mode))
                    return -1;
            }
            copy[i] = saved;
        }
    }
    return 0;
}

LIBMATTI_MC_LevelStorageSource *LIBMATTI_MC_LevelStorageSource_CreateDefault(const char *baseDir)
{
    if (baseDir == NULL)
        return NULL;
    LIBMATTI_MC_LevelStorageSource *source = calloc(1, sizeof(LIBMATTI_MC_LevelStorageSource));
    if (source == NULL)
        return NULL;
    source->baseDir = strdup(baseDir);
    // Java: the backup dir rides FMLPaths.BACKUPDIR; the port mirrors baseDir
    source->backupDir = join_path(baseDir, "backups");
    if (source->baseDir == NULL || source->backupDir == NULL)
    {
        free(source->baseDir);
        free(source->backupDir);
        free(source);
        return NULL;
    }
    return source;
}

void LIBMATTI_MC_LevelStorageSource_Free(LIBMATTI_MC_LevelStorageSource *source)
{
    if (source == NULL)
        return;
    free(source->baseDir);
    free(source->backupDir);
    free(source);
}

char *LIBMATTI_MC_LevelStorageAccess_GetDimensionPath(const LIBMATTI_MC_LevelStorageAccess *access,
                                                      const char *dimensionId)
{
    // Java: getDimensionPath - the overworld lives at the level root, the other
    // dimensions nest under dimensions/<namespace>/<path>
    char *levelPath = join_path(access->source->baseDir, access->levelId);
    if (levelPath == NULL)
        return NULL;
    if (strcmp(dimensionId, LIBMATTI_MC_Level_OVERWORLD) == 0)
        return levelPath;

    const char *separator = strchr(dimensionId, ':');
    if (separator == NULL)
    {
        free(levelPath);
        return NULL;
    }
    char namespaceBuffer[128];
    size_t namespaceLength = (size_t) (separator - dimensionId);
    if (namespaceLength >= sizeof(namespaceBuffer))
        namespaceLength = sizeof(namespaceBuffer) - 1;
    memcpy(namespaceBuffer, dimensionId, namespaceLength);
    namespaceBuffer[namespaceLength] = '\0';

    // Java: dimensions/<namespace>/<path>
    char *out = malloc(strlen(levelPath) + strlen(namespaceBuffer) + strlen(separator + 1) + 24);
    if (out != NULL)
        sprintf(out, "%s/dimensions/%s/%s", levelPath, namespaceBuffer, separator + 1);
    free(levelPath);
    return out;
}

char *LIBMATTI_MC_LevelStorageAccess_GetLevelPath(const LIBMATTI_MC_LevelStorageAccess *access,
                                                  const char *resourceName)
{
    char *levelPath = join_path(access->source->baseDir, access->levelId);
    if (levelPath == NULL || resourceName == NULL)
        return levelPath;
    char *out = join_path(levelPath, resourceName);
    free(levelPath);
    return out;
}

LIBMATTI_MC_LevelStorageAccess *LIBMATTI_MC_LevelStorageSource_CreateAccess(LIBMATTI_MC_LevelStorageSource *source,
                                                                            const char *levelId,
                                                                            const char *dimensionId)
{
    if (source == NULL || levelId == NULL || dimensionId == NULL)
        return NULL;
    LIBMATTI_MC_LevelStorageAccess *access = calloc(1, sizeof(LIBMATTI_MC_LevelStorageAccess));
    if (access == NULL)
        return NULL;
    access->source = source;
    access->levelId = strdup(levelId);
    access->dimensionId = strdup(dimensionId);
    if (access->levelId == NULL || access->dimensionId == NULL)
    {
        LIBMATTI_MC_LevelStorageAccess_Free(access);
        return NULL;
    }

    // Java: createAccess only opens the folder tree; the region storage wires
    // in the loaded dimension's region folder right away
    char *dimensionPath = LIBMATTI_MC_LevelStorageAccess_GetDimensionPath(access, dimensionId);
    if (dimensionPath == NULL)
    {
        LIBMATTI_MC_LevelStorageAccess_Free(access);
        return NULL;
    }
    char *regionFolder = join_path(dimensionPath, LIBMATTI_MC_LevelStorage_REGION_DIR);
    free(dimensionPath);
    if (regionFolder == NULL)
    {
        LIBMATTI_MC_LevelStorageAccess_Free(access);
        return NULL;
    }
    make_dirs(regionFolder);
    access->regionStorage = LIBMATTI_MC_RegionFileStorage_New(regionFolder, false);
    free(regionFolder);
    if (access->regionStorage == NULL)
    {
        LIBMATTI_MC_LevelStorageAccess_Free(access);
        return NULL;
    }
    return access;
}

int LIBMATTI_MC_LevelStorageAccess_ReadLevelData(LIBMATTI_MC_LevelStorageAccess *access,
                                                 LIBMATTI_MC_Nbt_CompoundTag **outData)
{
    if (outData != NULL)
        *outData = NULL;
    char *dataPath = LIBMATTI_MC_LevelStorageAccess_GetLevelPath(access, LIBMATTI_MC_LevelStorage_LEVEL_DATA_FILE);
    if (dataPath == NULL)
        return 0;
    // Java: NbtIo.readCompressed(path, accounter) - the gzip level.dat
    LIBMATTI_MC_Nbt_CompoundTag *root = LIBMATTI_MC_Nbt_NbtIo_ReadCompressedFile(dataPath, NULL);
    free(dataPath);
    if (root == NULL)
        return 0;
    LIBMATTI_MC_Nbt_CompoundTag *data = LIBMATTI_MC_Nbt_CompoundTag_GetCompoundOrEmpty(root,
                                                                                        LIBMATTI_MC_LevelStorageSource_TAG_DATA);
    LIBMATTI_MC_Nbt_CompoundTag *copy = (LIBMATTI_MC_Nbt_CompoundTag *) LIBMATTI_MC_Nbt_Tag_Copy(
        (const LIBMATTI_MC_Nbt_Tag *) data);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) root);
    if (copy == NULL)
        return 0;
    if (outData != NULL)
        *outData = copy;
    return 1;
}

int LIBMATTI_MC_LevelStorageAccess_WriteLevelData(LIBMATTI_MC_LevelStorageAccess *access,
                                                  LIBMATTI_MC_Nbt_CompoundTag *data)
{
    if (data == NULL)
        return -1;
    char *levelPath = LIBMATTI_MC_LevelStorageAccess_GetLevelPath(access, LIBMATTI_MC_LevelStorage_LEVEL_DATA_FILE);
    if (levelPath == NULL)
        return -1;
    LIBMATTI_MC_Nbt_CompoundTag *root = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_Put(root, LIBMATTI_MC_LevelStorageSource_TAG_DATA,
                                    LIBMATTI_MC_Nbt_Tag_Copy((const LIBMATTI_MC_Nbt_Tag *) data));

    // Java: the write goes to level.dat_new -> rename(level.dat) with the old
    // file rotated into level.dat_old; the port keeps the two-step tail.
    // (NbtIo's write pair reports success as a truthy return.)
    char *newPath = LIBMATTI_MC_LevelStorageAccess_GetLevelPath(access, "level.dat_new");
    char *oldPath = LIBMATTI_MC_LevelStorageAccess_GetLevelPath(access, LIBMATTI_MC_LevelStorage_OLD_LEVEL_DATA_FILE);
    int wrote = LIBMATTI_MC_Nbt_NbtIo_WriteCompressedFile(root, newPath);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) root);
    int result = -1;
    if (wrote)
    {
        remove(oldPath);
        rename(levelPath, oldPath);
        result = rename(newPath, levelPath) == 0 ? 0 : -1;
    }
    free(levelPath);
    free(newPath);
    free(oldPath);
    return result;
}

void LIBMATTI_MC_LevelStorageAccess_Free(LIBMATTI_MC_LevelStorageAccess *access)
{
    if (access == NULL)
        return;
    LIBMATTI_MC_RegionFileStorage_Close(access->regionStorage);
    LIBMATTI_MC_RegionFileStorage_Free(access->regionStorage);
    free(access->levelId);
    free(access->dimensionId);
    free(access);
}

// ---------------------------------------------------------------------------
// the LevelData <-> "Data" compound (Java: PrimaryLevelData.save/tag)
// ---------------------------------------------------------------------------

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_LevelStorage_BuildLevelData(LIBMATTI_MC_Level *level,
                                                                     const char *levelName)
{
    LIBMATTI_MC_Nbt_CompoundTag *data = LIBMATTI_MC_Nbt_CompoundTag_New();
    // Java: the Data compound's format metadata
    LIBMATTI_MC_Nbt_CompoundTag_PutString(data, "LevelName", levelName != NULL ? levelName : "New World");
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "version", 19133); // Java: the anvil save version
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "DataVersion", LIBMATTI_MC_SharedConstants_GetDataVersion());
    LIBMATTI_MC_Nbt_CompoundTag_PutLong(data, "LastPlayed", (int64_t) time(NULL) * 1000);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "GameType", 1); // Java: creative default
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(data, "hardcore", 0);

    // Java: the WritableLevelData fields the port flattens onto Level
    if (level != NULL)
    {
        LIBMATTI_MC_Nbt_CompoundTag_PutLong(data, "Time", LIBMATTI_MC_Level_GetGameTime(level));
        LIBMATTI_MC_Nbt_CompoundTag_PutLong(data, "DayTime", LIBMATTI_MC_Level_GetDayTime(level));
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(data, "raining", LIBMATTI_MC_Level_IsRaining(level) ? 1 : 0);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "SpawnX", 0);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "SpawnY", 64);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(data, "SpawnZ", 0);
    }
    return data;
}

int LIBMATTI_MC_LevelStorage_ApplyLevelData(LIBMATTI_MC_Level *level, const LIBMATTI_MC_Nbt_CompoundTag *data)
{
    if (level == NULL || data == NULL)
        return 0;
    int64_t gameTime = 0;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetLong(data, "Time", &gameTime))
        LIBMATTI_MC_Level_SetGameTime(level, gameTime);
    int64_t dayTime = 0;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetLong(data, "DayTime", &dayTime))
        LIBMATTI_MC_Level_SetDayTime(level, dayTime);
    int raining = 0;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetBoolean(data, "raining", &raining))
        LIBMATTI_MC_Level_SetRaining(level, raining != 0);
    return 1;
}

// ---------------------------------------------------------------------------
// the chunk save/load pass (Java: ChunkMap.save / read over the region storage)
// ---------------------------------------------------------------------------

int LIBMATTI_MC_LevelStorage_SaveChunks(LIBMATTI_MC_LevelStorageAccess *access, LIBMATTI_MC_Level *level)
{
    if (access == NULL || level == NULL)
        return -1;
    int saved = 0;
    for (int i = 0; i < level->chunkCount; i++)
    {
        LIBMATTI_MC_LevelChunk *chunk = level->chunks[i].chunk;
        if (chunk == NULL)
            continue;
        // Java: ChunkMap.save - every loaded chunk's SerializableChunkData
        LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_SerializableChunkData_Write(chunk);
        if (tag == NULL)
            continue;
        int result = LIBMATTI_MC_RegionFileStorage_Write(access->regionStorage, LIBMATTI_MC_LevelChunk_GetPos(chunk),
                                                         tag);
        LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
        if (result == 0)
        {
            LIBMATTI_MC_ChunkAccess_TryMarkSaved(&chunk->base);
            saved++;
        }
    }
    return saved;
}

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelStorage_LoadChunk(LIBMATTI_MC_LevelStorageAccess *access,
                                                           LIBMATTI_MC_Level *level, int chunkX, int chunkZ)
{
    if (access == NULL || level == NULL)
        return NULL;
    LIBMATTI_MC_ChunkPos pos = {chunkX, chunkZ};
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_RegionFileStorage_Read(access->regionStorage, &pos);
    if (tag == NULL)
        return NULL;
    // Java: the status gate - only full chunks spin up (the ProtoChunk pipeline
    // is worldgen content)
    if (!LIBMATTI_MC_SerializableChunkData_IsFullChunk(tag))
    {
        LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
        return NULL;
    }
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_SerializableChunkData_Read(level, tag);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
    if (chunk != NULL)
        LIBMATTI_MC_Level_SetChunk(level, chunk);
    return chunk;
}
