// Port of net.minecraft.world.level.chunk.LevelChunk.
//
// Java: public class LevelChunk extends ChunkAccess - the port embeds the
// ChunkAccess data layout and adds the level back pointer plus the block entity
// lifecycle (get/set/remove, EntityCreationType, promotePendingBlockEntity).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNK_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNK_H

#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"
#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static enum EntityCreationType
typedef enum LIBMATTI_MC_LevelChunkEntityCreationType
{
    LIBMATTI_MC_LevelChunkEntityCreationType_IMMEDIATE = 0,
    LIBMATTI_MC_LevelChunkEntityCreationType_QUEUED = 1,
    LIBMATTI_MC_LevelChunkEntityCreationType_CHECK = 2,
} LIBMATTI_MC_LevelChunkEntityCreationType;

// Java: public class LevelChunk extends ChunkAccess
typedef struct LIBMATTI_MC_LevelChunk
{
    // Java: the ChunkAccess base (sections, heightmaps, block entity containers,
    // post-processing, unsaved flag)
    LIBMATTI_MC_ChunkAccess base;
    // Java: final Level level
    struct LIBMATTI_MC_Level *level;
    // Java: private boolean loaded
    bool loaded;
} LIBMATTI_MC_LevelChunk;

// Java: public LevelChunk(Level level, ChunkPos pos)
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelChunk_New(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_ChunkPos *pos);
// Java: getSections() / getPos() / the height accessor view
LIBMATTI_MC_LevelChunkSection **LIBMATTI_MC_LevelChunk_GetSections(LIBMATTI_MC_LevelChunk *chunk);
const LIBMATTI_MC_ChunkPos *LIBMATTI_MC_LevelChunk_GetPos(const LIBMATTI_MC_LevelChunk *chunk);
int LIBMATTI_MC_LevelChunk_GetHeight(const LIBMATTI_MC_LevelChunk *chunk);
int LIBMATTI_MC_LevelChunk_GetMinY(const LIBMATTI_MC_LevelChunk *chunk);

// Java: public BlockState getBlockState(BlockPos) - section lookup, air outside
LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunk_GetBlockState(const LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos);
// Java: ChunkAccess.setBlockState(BlockPos, BlockState, int flags) - returns the
// previous state or NULL; the port also drives the heightmap update
LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunk_SetBlockState(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags);

// Java: public BlockEntity getBlockEntity(BlockPos) = getBlockEntity(CHECK)
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_GetBlockEntity(const LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos);
// Java: public BlockEntity getBlockEntity(BlockPos, EntityCreationType)
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_GetBlockEntityWithCreation(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_LevelChunkEntityCreationType creationType);
// Java: public void setBlockEntity(BlockEntity) - validates hasBlockEntity + wires level
void LIBMATTI_MC_LevelChunk_SetBlockEntity(LIBMATTI_MC_LevelChunk *chunk, LIBMATTI_MC_BlockEntity *blockEntity);
// Java: public void removeBlockEntity(BlockPos)
void LIBMATTI_MC_LevelChunk_RemoveBlockEntity(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos);
// Java: private BlockEntity promotePendingBlockEntity(BlockPos, CompoundTag) - the
// pending NBT becomes a live block entity (the tag decode is game-port content; the
// port moves the slot and creates the base entity)
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_PromotePendingBlockEntity(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos, void *tag);

// Java: private boolean isInLevel() = loaded || level.isClientSide()
bool LIBMATTI_MC_LevelChunk_IsInLevel(const LIBMATTI_MC_LevelChunk *chunk);
// Java: private boolean loaded + setLoaded
bool LIBMATTI_MC_LevelChunk_IsLoaded(const LIBMATTI_MC_LevelChunk *chunk);
void LIBMATTI_MC_LevelChunk_SetLoaded(LIBMATTI_MC_LevelChunk *chunk, bool loaded);
// Java: public void clearAllBlockEntities()
void LIBMATTI_MC_LevelChunk_ClearAllBlockEntities(LIBMATTI_MC_LevelChunk *chunk);
void LIBMATTI_MC_LevelChunk_Free(LIBMATTI_MC_LevelChunk *chunk);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNK_H
