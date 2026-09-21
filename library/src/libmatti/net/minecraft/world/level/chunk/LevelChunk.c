// Port of net.minecraft.world.level.chunk.LevelChunk.

#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_LevelChunk_New(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_ChunkPos *pos)
{
    LIBMATTI_MC_LevelChunk *chunk = calloc(1, sizeof(LIBMATTI_MC_LevelChunk));
    // Java: the ChunkAccess super constructor receives the level's height accessor
    LIBMATTI_MC_LevelHeightAccessor heightAccessor = LIBMATTI_MC_Level_GetHeightAccessor(level);
    LIBMATTI_MC_ChunkAccess_Init(&chunk->base, pos, &heightAccessor, 0);
    chunk->level = level;
    return chunk;
}

LIBMATTI_MC_LevelChunkSection **LIBMATTI_MC_LevelChunk_GetSections(LIBMATTI_MC_LevelChunk *chunk)
{
    return chunk->base.sections;
}

const LIBMATTI_MC_ChunkPos *LIBMATTI_MC_LevelChunk_GetPos(const LIBMATTI_MC_LevelChunk *chunk)
{
    return &chunk->base.chunkPos;
}

int LIBMATTI_MC_LevelChunk_GetHeight(const LIBMATTI_MC_LevelChunk *chunk)
{
    return chunk->base.levelHeightAccessor.height;
}

int LIBMATTI_MC_LevelChunk_GetMinY(const LIBMATTI_MC_LevelChunk *chunk)
{
    return chunk->base.levelHeightAccessor.minY;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunk_GetBlockState(const LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: the isDebug branch (barrier/debug layer) is game-port content
    int x = LIBMATTI_MC_Vec3i_GetX(&pos->base);
    int y = LIBMATTI_MC_Vec3i_GetY(&pos->base);
    int z = LIBMATTI_MC_Vec3i_GetZ(&pos->base);
    return LIBMATTI_MC_ChunkAccess_GetBlockStateXYZ(&chunk->base, x, y, z);
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunk_SetBlockState(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags)
{
    (void) flags;
    // Java: LevelChunk.setBlockState - the section lookup, the section write, then
    // the heightmap updates over the four live heightmaps (the WG pair belongs to
    // worldgen only)
    int x = LIBMATTI_MC_Vec3i_GetX(&pos->base);
    int y = LIBMATTI_MC_Vec3i_GetY(&pos->base);
    int z = LIBMATTI_MC_Vec3i_GetZ(&pos->base);

    int index = LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk->base.levelHeightAccessor, y);
    if (index < 0 || index >= chunk->base.sectionCount)
        return NULL;

    LIBMATTI_MC_LevelChunkSection *section = chunk->base.sections[index];
    // Java: flag = section.hasOnlyAir(); if (flag && state.isAir()) return null;
    bool wasOnlyAir = LIBMATTI_MC_LevelChunkSection_HasOnlyAir(section);
    if (wasOnlyAir && state == LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR()))
        return NULL;

    LIBMATTI_MC_BlockState *oldState = LIBMATTI_MC_LevelChunkSection_SetBlockState(section, x & 15, y & 15, z & 15, state);

    // Java: the four heightmap updates (MOTION_BLOCKING, MOTION_BLOCKING_NO_LEAVES,
    // OCEAN_FLOOR, WORLD_SURFACE) - each runs even when not yet primed, exactly like
    // Java's heightmaps.get(...) after getOrCreateHeightmapUnprimed priming on demand
    static const LIBMATTI_MC_HeightmapTypes LIVE_TYPES[] = {
        LIBMATTI_MC_Heightmap_MOTION_BLOCKING,
        LIBMATTI_MC_Heightmap_MOTION_BLOCKING_NO_LEAVES,
        LIBMATTI_MC_Heightmap_OCEAN_FLOOR,
        LIBMATTI_MC_Heightmap_WORLD_SURFACE,
    };
    for (size_t i = 0; i < sizeof(LIVE_TYPES) / sizeof(LIVE_TYPES[0]); i++)
    {
        LIBMATTI_MC_Heightmap *heightmap = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(&chunk->base, (int) LIVE_TYPES[i]);
        LIBMATTI_MC_Heightmap_Update(heightmap, x & 15, y, z & 15, state);
    }
    LIBMATTI_MC_ChunkAccess_MarkUnsaved(&chunk->base);
    return oldState;
}

// Java: BlockState.hasBlockEntity() = getBlock() instanceof EntityBlock - the port
// keeps an EntityBlock registry-free approximation: the state's block must be in
// some BlockEntityType's validBlocks set
static bool state_has_block_entity(LIBMATTI_MC_BlockState *state)
{
    for (int i = 0; i < 49; i++)
    {
        // the 49 vanilla types are static-lifetime entries
        extern const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_AccessIndex(int);
        const LIBMATTI_MC_BlockEntityType *type = LIBMATTI_MC_BlockEntityType_AccessIndex(i);
        if (type != NULL && LIBMATTI_MC_BlockEntityType_IsValid(type, state))
            return true;
    }
    return false;
}

static LIBMATTI_MC_BlockEntity *create_block_entity(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: private createBlockEntity - the state's EntityBlock.newBlockEntity
    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_LevelChunk_GetBlockState(chunk, pos);
    if (!state_has_block_entity(state))
        return NULL;
    // the vanilla path: find the type whose validBlocks contain this block
    for (int i = 0; i < 49; i++)
    {
        extern const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_AccessIndex(int);
        const LIBMATTI_MC_BlockEntityType *type = LIBMATTI_MC_BlockEntityType_AccessIndex(i);
        if (type != NULL && LIBMATTI_MC_BlockEntityType_IsValid(type, state))
            return LIBMATTI_MC_BlockEntityType_Create(type, pos, state);
    }
    return NULL;
}

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_GetBlockEntity(const LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_LevelChunk_GetBlockEntityWithCreation((LIBMATTI_MC_LevelChunk *) chunk, pos,
                                                            LIBMATTI_MC_LevelChunkEntityCreationType_CHECK);
}

static LIBMATTI_MC_ChunkBlockEntityEntry *find_block_entity(LIBMATTI_MC_ChunkAccess *base, const LIBMATTI_MC_BlockPos *pos)
{
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
    for (int i = 0; i < base->blockEntityCount; i++)
    {
        if (base->blockEntities[i].packedPos == packed)
            return &base->blockEntities[i];
    }
    return NULL;
}

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_GetBlockEntityWithCreation(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos,
                                                                           LIBMATTI_MC_LevelChunkEntityCreationType creationType)
{
    // Java: 1:1 - the live map, then the pending promote, then the immediate create
    LIBMATTI_MC_ChunkAccess *base = &chunk->base;
    LIBMATTI_MC_ChunkBlockEntityEntry *entry = find_block_entity(base, pos);
    LIBMATTI_MC_BlockEntity *blockEntity = entry != NULL ? entry->blockEntity : NULL;

    if (blockEntity == NULL)
    {
        int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
        for (int i = 0; i < base->pendingBlockEntityCount; i++)
        {
            if (base->pendingBlockEntities[i].packedPos == packed)
            {
                void *tag = base->pendingBlockEntities[i].tag;
                LIBMATTI_MC_BlockEntity *promoted = LIBMATTI_MC_LevelChunk_PromotePendingBlockEntity(chunk, pos, tag);
                if (promoted != NULL)
                    return promoted;
                break;
            }
        }
    }

    if (blockEntity == NULL)
    {
        if (creationType == LIBMATTI_MC_LevelChunkEntityCreationType_IMMEDIATE)
        {
            blockEntity = create_block_entity(chunk, pos);
            if (blockEntity != NULL)
                LIBMATTI_MC_LevelChunk_SetBlockEntity(chunk, blockEntity);
        }
    }
    else if (LIBMATTI_MC_BlockEntity_IsRemoved(blockEntity))
    {
        // Java: remove + null
        int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
        for (int i = 0; i < base->blockEntityCount; i++)
        {
            if (base->blockEntities[i].packedPos == packed)
            {
                base->blockEntities[i] = base->blockEntities[base->blockEntityCount - 1];
                base->blockEntityCount--;
                break;
            }
        }
        return NULL;
    }

    return blockEntity;
}

void LIBMATTI_MC_LevelChunk_SetBlockEntity(LIBMATTI_MC_LevelChunk *chunk, LIBMATTI_MC_BlockEntity *blockEntity)
{
    // Java: setBlockEntity - the hasBlockEntity check, the level wiring, the map put
    const LIBMATTI_MC_BlockPos *blockPos = LIBMATTI_MC_BlockEntity_GetBlockPos(blockEntity);
    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_LevelChunk_GetBlockState(chunk, blockPos);
    if (!state_has_block_entity(state))
        return;

    if (LIBMATTI_MC_BlockEntity_GetBlockState(blockEntity) != state)
    {
        if (!LIBMATTI_MC_BlockEntityType_IsValid(LIBMATTI_MC_BlockEntity_GetType(blockEntity), state))
            return;
        LIBMATTI_MC_BlockEntity_SetBlockState(blockEntity, state);
    }

    LIBMATTI_MC_BlockEntity_SetLevel(blockEntity, chunk->level);
    LIBMATTI_MC_BlockEntity_ClearRemoved(blockEntity);

    LIBMATTI_MC_ChunkAccess *base = &chunk->base;
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(blockPos);
    for (int i = 0; i < base->blockEntityCount; i++)
    {
        if (base->blockEntities[i].packedPos == packed)
        {
            // Java: the previous entry setRemoved when replaced
            if (base->blockEntities[i].blockEntity != blockEntity)
                LIBMATTI_MC_BlockEntity_SetRemoved(base->blockEntities[i].blockEntity);
            base->blockEntities[i].blockEntity = blockEntity;
            return;
        }
    }
    if (base->blockEntityCount == base->blockEntityCapacity)
    {
        base->blockEntityCapacity = base->blockEntityCapacity > 0 ? base->blockEntityCapacity * 2 : 8;
        base->blockEntities = realloc(base->blockEntities,
                                      sizeof(*base->blockEntities) * (size_t) base->blockEntityCapacity);
    }
    base->blockEntities[base->blockEntityCount].packedPos = packed;
    base->blockEntities[base->blockEntityCount].blockEntity = blockEntity;
    base->blockEntityCount++;
}

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_LevelChunk_PromotePendingBlockEntity(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos, void *tag)
{
    // Java: promotePendingBlockEntity decodes the tag (type by id, loadWithComponents)
    // and registers the entity. The tag decode is game-port content; the port moves
    // the pending slot and creates the base entity for the position's state.
    LIBMATTI_MC_ChunkAccess *base = &chunk->base;
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);

    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_LevelChunk_GetBlockState(chunk, pos);
    if (!state_has_block_entity(state))
        return NULL;

    LIBMATTI_MC_BlockEntity *blockEntity = create_block_entity(chunk, pos);
    if (blockEntity == NULL)
        return NULL;

    LIBMATTI_MC_LevelChunk_SetBlockEntity(chunk, blockEntity);
    // Java: pendingBlockEntities.remove(pos)
    for (int i = 0; i < base->pendingBlockEntityCount; i++)
    {
        if (base->pendingBlockEntities[i].packedPos == packed)
        {
            base->pendingBlockEntities[i] = base->pendingBlockEntities[base->pendingBlockEntityCount - 1];
            base->pendingBlockEntityCount--;
            break;
        }
    }
    (void) tag;
    return blockEntity;
}

void LIBMATTI_MC_LevelChunk_RemoveBlockEntity(LIBMATTI_MC_LevelChunk *chunk, const LIBMATTI_MC_BlockPos *pos)
{
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
    LIBMATTI_MC_ChunkAccess *base = &chunk->base;
    for (int i = 0; i < base->blockEntityCount; i++)
    {
        if (base->blockEntities[i].packedPos == packed)
        {
            base->blockEntities[i] = base->blockEntities[base->blockEntityCount - 1];
            base->blockEntityCount--;
            return;
        }
    }
    // Java: removeBlockEntity also clears a pending entry
    for (int i = 0; i < base->pendingBlockEntityCount; i++)
    {
        if (base->pendingBlockEntities[i].packedPos == packed)
        {
            base->pendingBlockEntities[i] = base->pendingBlockEntities[base->pendingBlockEntityCount - 1];
            base->pendingBlockEntityCount--;
            return;
        }
    }
}

bool LIBMATTI_MC_LevelChunk_IsInLevel(const LIBMATTI_MC_LevelChunk *chunk)
{
    // Java: this.loaded || this.level.isClientSide()
    return chunk->loaded || LIBMATTI_MC_Level_IsClientSide(chunk->level);
}

bool LIBMATTI_MC_LevelChunk_IsLoaded(const LIBMATTI_MC_LevelChunk *chunk)
{
    return chunk->loaded;
}

void LIBMATTI_MC_LevelChunk_SetLoaded(LIBMATTI_MC_LevelChunk *chunk, bool loaded)
{
    chunk->loaded = loaded;
}

void LIBMATTI_MC_LevelChunk_ClearAllBlockEntities(LIBMATTI_MC_LevelChunk *chunk)
{
    chunk->base.blockEntityCount = 0;
    chunk->base.pendingBlockEntityCount = 0;
}

void LIBMATTI_MC_LevelChunk_Free(LIBMATTI_MC_LevelChunk *chunk)
{
    LIBMATTI_MC_ChunkAccess_Free(&chunk->base);
    free(chunk);
}
