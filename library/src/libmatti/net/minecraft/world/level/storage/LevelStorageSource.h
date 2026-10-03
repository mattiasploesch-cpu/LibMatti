// Port of net.minecraft.world.level.storage.LevelStorageSource (the save-format
// root) plus the net.minecraft.world.level.storage.LevelResource names and the
// LevelData read/write the WritableLevelData flattening needs.
//
// The wire layout (1:1 with Java):
//   <baseDir>/<levelId>/level.dat            gzip NBT {"Data": {...}}
//   <baseDir>/<levelId>/level.dat_old        the previous save
//   <baseDir>/<levelId>/region/*.mca         the overworld chunks
//   <baseDir>/<levelId>/dimensions/<ns>/<path>/region/*.mca   the other dimensions
//
// The port keeps the Data compound at the vanilla keys the client's flattened
// LevelData needs (Time, DayTime, SpawnX/Y/Z, raining, thundering) plus the
// format metadata (LevelName, version, GameType, hardcore, DataVersion).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_STORAGE_LEVELSTORAGESOURCE_H
#define MATTICRAFT_MC_WORLD_LEVEL_STORAGE_LEVELSTORAGESOURCE_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileStorage.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: LevelStorageSource.TAG_DATA + the LevelResource ids
#define LIBMATTI_MC_LevelStorageSource_TAG_DATA "Data"
#define LIBMATTI_MC_LevelStorage_LEVEL_DATA_FILE "level.dat"
#define LIBMATTI_MC_LevelStorage_OLD_LEVEL_DATA_FILE "level.dat_old"
#define LIBMATTI_MC_LevelStorage_PLAYER_DATA_DIR "playerdata"
#define LIBMATTI_MC_LevelStorage_REGION_DIR "region"

// Java: public class LevelStorageSource
typedef struct LIBMATTI_MC_LevelStorageSource
{
    char *baseDir;    // Java: private final Path baseDir (owned)
    char *backupDir;  // Java: private final Path backupDir (owned)
} LIBMATTI_MC_LevelStorageSource;

// Java: public class LevelStorageAccess implements AutoCloseable - the port adds
// the region storage the loaded dimension uses
typedef struct LIBMATTI_MC_LevelStorageAccess
{
    LIBMATTI_MC_LevelStorageSource *source; // Java: LevelStorageSource parent
    char *levelId;                          // Java: the level directory name (owned)
    LIBMATTI_MC_RegionFileStorage *regionStorage; // the dimension's region/*.mca (owned)
    char *dimensionId;                      // the region storage's dimension (owned)
} LIBMATTI_MC_LevelStorageAccess;

// Java: createDefault(Path) - the source over "saves"
LIBMATTI_MC_LevelStorageSource *LIBMATTI_MC_LevelStorageSource_CreateDefault(const char *baseDir);
void LIBMATTI_MC_LevelStorageSource_Free(LIBMATTI_MC_LevelStorageSource *source);

// Java: validateAndCreateAccess/createAccess(String) - opens the level directory
// (creating it on demand); dimensionId wires the region storage folder
// (overworld = <level>/region, others = <level>/dimensions/<ns>/<path>/region)
LIBMATTI_MC_LevelStorageAccess *LIBMATTI_MC_LevelStorageSource_CreateAccess(LIBMATTI_MC_LevelStorageSource *source,
                                                                            const char *levelId,
                                                                            const char *dimensionId);
// Java: getDimensionPath(ResourceKey) - the overworld stays at the level root
char *LIBMATTI_MC_LevelStorageAccess_GetDimensionPath(const LIBMATTI_MC_LevelStorageAccess *access,
                                                      const char *dimensionId);
// Java: getLevelPath(LevelResource) - the caller frees
char *LIBMATTI_MC_LevelStorageAccess_GetLevelPath(const LIBMATTI_MC_LevelStorageAccess *access,
                                                  const char *resourceName);
// Java: getDataTag() / saveDataTag(...) - the level.dat {"Data": {...}} file
// (gzip like Java's NbtIo.readCompressed/writeCompressed); Read returns 0/1
// found, Write returns 0 on success
int LIBMATTI_MC_LevelStorageAccess_ReadLevelData(LIBMATTI_MC_LevelStorageAccess *access,
                                                 LIBMATTI_MC_Nbt_CompoundTag **outData);
int LIBMATTI_MC_LevelStorageAccess_WriteLevelData(LIBMATTI_MC_LevelStorageAccess *access,
                                                  LIBMATTI_MC_Nbt_CompoundTag *data);
// Java: close() - the region storage flushes and closes
void LIBMATTI_MC_LevelStorageAccess_Free(LIBMATTI_MC_LevelStorageAccess *access);

// Java: the ServerLevel save path (ServerLevel.saveDataTag / PrimaryLevelData
// via TagValueOutput) - the port's flattened LevelData keys
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_LevelStorage_BuildLevelData(LIBMATTI_MC_Level *level,
                                                                     const char *levelName);
// Java: PrimaryLevelData.loadChildren/setDataTag - the fields restore onto the
// level (gameTime/dayTime/raining/thundering); returns 0/1 found
int LIBMATTI_MC_LevelStorage_ApplyLevelData(LIBMATTI_MC_Level *level, const LIBMATTI_MC_Nbt_CompoundTag *data);

// Java: the ClientChunkCache/ServerChunkCache chunk save pass - every chunk in
// the level writes through the region storage (unsaved chunks only, Java's
// ChunkMap.save the port keeps unconditional for the client)
int LIBMATTI_MC_LevelStorage_SaveChunks(LIBMATTI_MC_LevelStorageAccess *access, LIBMATTI_MC_Level *level);
// Java: ChunkMap.read - the chunk from the region storage into the level (NULL
// when absent); the caller frees / owns the chunk through the level
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelStorage_LoadChunk(LIBMATTI_MC_LevelStorageAccess *access,
                                                           LIBMATTI_MC_Level *level, int chunkX, int chunkZ);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_STORAGE_LEVELSTORAGESOURCE_H
