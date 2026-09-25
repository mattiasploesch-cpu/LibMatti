// Port of net.minecraft.world.level.Level (the in-memory part: chunk storage,
// block access, level data fields). The tick loop, entities, neighbour updates
// and the chunk source stay with the game port.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVEL_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVEL_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/level/BlockGetter.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"

struct LIBMATTI_MC_Entity;

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: net.minecraft.world.level.Level - the dimension ResourceKeys
#define LIBMATTI_MC_Level_OVERWORLD "minecraft:overworld"
#define LIBMATTI_MC_Level_NETHER "minecraft:the_nether"
#define LIBMATTI_MC_Level_END "minecraft:the_end"

// Java: the Block.UPDATE_* flag constants
#define LIBMATTI_MC_Level_UPDATE_NEIGHBORS 1
#define LIBMATTI_MC_Level_UPDATE_CLIENTS 2
#define LIBMATTI_MC_Level_UPDATE_INVISIBLE 4
#define LIBMATTI_MC_Level_UPDATE_IMMEDIATE 8
#define LIBMATTI_MC_Level_UPDATE_KNOWN_SHAPE 16
#define LIBMATTI_MC_Level_UPDATE_SUPPRESS_DROPS 32

// Java: public class Level implements LevelAccessor
typedef struct LIBMATTI_MC_Level
{
    // Java: the LevelHeightAccessor surface (dimension height range)
    LIBMATTI_MC_LevelHeightAccessor heightAccessor;
    // Java: private final ResourceKey<Level> dimension
    LIBMATTI_MC_ResourceKey *dimension;
    // Java: private final boolean isClientSide
    bool isClientSide;
    // Java: private final boolean isDebug
    bool isDebug;
    // Java: protected final WritableLevelData levelData (fields flattened until the
    // LevelData model lands)
    long gameTime;
    long dayTime;
    bool raining;
    bool thundering;
    // Java: the in-memory chunk map (LongOpenHashMap in Java's ClientChunkCache/
    // ServerChunkCache; the port keys by ChunkPos.toLong())
    struct ChunkEntry
    {
        int64_t packedPos;
        LIBMATTI_MC_LevelChunk *chunk;
    } *chunks;
    int chunkCount;
    int chunkCapacity;
    // Java: private final EntityLookup entityLookup (the entitiesById map) - the
    // port keeps a growable pointer array until the P5 tick loop needs the map
    struct LIBMATTI_MC_Entity **entities;
    int entityCount;
    int entityCapacity;
} LIBMATTI_MC_Level;

// Java: Level(...) - the port takes the height range + the dimension key; the
// client flag defaults to true (this is the client's in-memory world)
LIBMATTI_MC_Level *LIBMATTI_MC_Level_New(int minY, int height, const char *dimension, bool isClientSide);
// Java: the LevelHeightAccessor view of this level (ChunkAccess receives it)
LIBMATTI_MC_LevelHeightAccessor LIBMATTI_MC_Level_GetHeightAccessor(struct LIBMATTI_MC_Level *level);

// Java: public LevelChunk getChunk(int chunkX, int chunkZ) - NULL when not loaded
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_Level_GetChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ);
// Java: the ClientChunkCache/ServerChunkCache storage side - stores the chunk
void LIBMATTI_MC_Level_SetChunk(struct LIBMATTI_MC_Level *level, LIBMATTI_MC_LevelChunk *chunk);
// Java: public LevelChunk getChunkAt(BlockPos) = getChunk(blockToSectionCoord(x), ...)
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_Level_GetChunkAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: boolean hasChunk(int chunkX, int chunkZ)
bool LIBMATTI_MC_Level_HasChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ);

// Java: public BlockState getBlockState(BlockPos) - VOID_AIR outside the bounds
LIBMATTI_MC_BlockState *LIBMATTI_MC_Level_GetBlockState(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: public boolean setBlock(BlockPos, BlockState, int flags) - flags without the
// neighbour-update side effects (the updater is game-port content); the block entity
// teardown/create around the state change is included
bool LIBMATTI_MC_Level_SetBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags);
bool LIBMATTI_MC_Level_SetBlockAndFlags(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags, int recursionLeft);
// Java: public boolean removeBlock(BlockPos, int flags)
bool LIBMATTI_MC_Level_RemoveBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, int flags);
// Java: public boolean destroyBlock(BlockPos, boolean drop, ...)
bool LIBMATTI_MC_Level_DestroyBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, bool drop);

// Java: public boolean isInWorldBounds(BlockPos) / isInValidBounds(BlockPos)
bool LIBMATTI_MC_Level_IsInWorldBounds(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
bool LIBMATTI_MC_Level_IsInValidBounds(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
// Java: isOutsideBuildHeight(BlockPos) through the height accessor
bool LIBMATTI_MC_Level_IsOutsideBuildHeight(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);

// Java: the level data accessors
long LIBMATTI_MC_Level_GetGameTime(struct LIBMATTI_MC_Level *level);
void LIBMATTI_MC_Level_SetGameTime(struct LIBMATTI_MC_Level *level, long gameTime);
long LIBMATTI_MC_Level_GetDayTime(struct LIBMATTI_MC_Level *level);
void LIBMATTI_MC_Level_SetDayTime(struct LIBMATTI_MC_Level *level, long dayTime);
bool LIBMATTI_MC_Level_IsRaining(struct LIBMATTI_MC_Level *level);
void LIBMATTI_MC_Level_SetRaining(struct LIBMATTI_MC_Level *level, bool raining);
bool LIBMATTI_MC_Level_IsClientSide(struct LIBMATTI_MC_Level *level);
bool LIBMATTI_MC_Level_IsDebug(struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Level_GetDimension(struct LIBMATTI_MC_Level *level);

// BlockGetter surface on the Level - getBlockEntity returns the typed BlockEntity
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_Level_GetBlockEntity(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);
int LIBMATTI_MC_Level_GetLightEmission(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos);

// Java: setBlock -> updateNeighborsAt side (the flag-1 tail) is a no-op stub the
// game port overrides; kept for the call sites
void LIBMATTI_MC_Level_UpdateNeighborsAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Block *block);

// the number of loaded chunks (the test surface)
int LIBMATTI_MC_Level_GetChunkCount(struct LIBMATTI_MC_Level *level);
void LIBMATTI_MC_Level_Free(struct LIBMATTI_MC_Level *level);

// Java: the entity surface - addEntity (addFreshEntity's tail), getEntities and
// the removal dispatch; the entities are borrowed (the spawner owns them)
bool LIBMATTI_MC_Level_AddEntity(struct LIBMATTI_MC_Level *level, struct LIBMATTI_MC_Entity *entity);
bool LIBMATTI_MC_Level_RemoveEntity(struct LIBMATTI_MC_Level *level, struct LIBMATTI_MC_Entity *entity);
int LIBMATTI_MC_Level_GetEntityCount(struct LIBMATTI_MC_Level *level);
struct LIBMATTI_MC_Entity *LIBMATTI_MC_Level_GetEntity(struct LIBMATTI_MC_Level *level, int index);
// Java: getEntities(Class, AABB, Predicate) - the box-overlap filter (NULL box/predicate = all)
int LIBMATTI_MC_Level_GetEntitiesInBox(struct LIBMATTI_MC_Level *level, double minX, double minY, double minZ,
                                       double maxX, double maxY, double maxZ,
                                       struct LIBMATTI_MC_Entity **out, int outCapacity);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVEL_H
