// Port of net.minecraft.world.level.Level (the in-memory part).

#include "libmatti/net/minecraft/world/entity/Entity.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/LevelReader.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include <stdlib.h>
#include <string.h>

static int64_t chunk_key(int chunkX, int chunkZ)
{
    return LIBMATTI_MC_ChunkPos_AsLong(chunkX, chunkZ);
}

LIBMATTI_MC_Level *LIBMATTI_MC_Level_New(int minY, int height, const char *dimension, bool isClientSide)
{
    LIBMATTI_MC_Level *level = calloc(1, sizeof(LIBMATTI_MC_Level));
    level->heightAccessor = LIBMATTI_MC_LevelHeightAccessor_Create(minY, height);
    // Java: ResourceKey.create(Registries.ROOT_REGISTRY_NAME path, Identifier)
    level->dimension = LIBMATTI_MC_ResourceKey_CreateRegistryKeyParsed(dimension);
    level->isClientSide = isClientSide;
    return level;
}

LIBMATTI_MC_LevelHeightAccessor LIBMATTI_MC_Level_GetHeightAccessor(struct LIBMATTI_MC_Level *level)
{
    return level->heightAccessor;
}

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_Level_GetChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ)
{
    int64_t key = chunk_key(chunkX, chunkZ);
    for (int i = 0; i < level->chunkCount; i++)
    {
        if (level->chunks[i].packedPos == key)
            return level->chunks[i].chunk;
    }
    return NULL;
}

void LIBMATTI_MC_Level_SetChunk(struct LIBMATTI_MC_Level *level, LIBMATTI_MC_LevelChunk *chunk)
{
    // Java: the chunk cache replaces an existing entry for the same position
    int64_t key = LIBMATTI_MC_ChunkPos_AsLong(chunk->base.chunkPos.x, chunk->base.chunkPos.z);
    for (int i = 0; i < level->chunkCount; i++)
    {
        if (level->chunks[i].packedPos == key)
        {
            level->chunks[i].chunk = chunk;
            return;
        }
    }
    if (level->chunkCount == level->chunkCapacity)
    {
        level->chunkCapacity = level->chunkCapacity > 0 ? level->chunkCapacity * 2 : 16;
        level->chunks = realloc(level->chunks, sizeof(*level->chunks) * (size_t) level->chunkCapacity);
    }
    level->chunks[level->chunkCount].packedPos = key;
    level->chunks[level->chunkCount].chunk = chunk;
    level->chunkCount++;
}

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_Level_GetChunkAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: getChunkAt = getChunk(SectionPos.blockToSectionCoord(x), blockToSectionCoord(z))
    // - and Java's getChunk(x, z) is getChunk(FULL, load=true), so the block access
    // paths create the chunk like the chunk source's "should always be able to
    // create a chunk" fallback
    return LIBMATTI_MC_LevelReader_GetChunk(level,
                                            LIBMATTI_MC_SectionPos_BlockToSectionCoord(LIBMATTI_MC_Vec3i_GetX(&pos->base)),
                                            LIBMATTI_MC_SectionPos_BlockToSectionCoord(LIBMATTI_MC_Vec3i_GetZ(&pos->base)),
                                            true);
}

bool LIBMATTI_MC_Level_HasChunk(struct LIBMATTI_MC_Level *level, int chunkX, int chunkZ)
{
    return LIBMATTI_MC_Level_GetChunk(level, chunkX, chunkZ) != NULL;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_Level_GetBlockState(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: VOID_AIR outside the valid bounds
    if (!LIBMATTI_MC_Level_IsInValidBounds(level, pos))
        return LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("VOID_AIR"));

    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunkAt(level, pos);
    if (chunk == NULL)
        return LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR());
    return LIBMATTI_MC_LevelChunk_GetBlockState(chunk, pos);
}

bool LIBMATTI_MC_Level_SetBlockAndFlags(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags, int recursionLeft)
{
    (void) recursionLeft;
    // Java: setBlock(pos, state, flags, recursion) - the bounds check, the chunk
    // write, then the update side effects (neighbour updates + sends stay with the
    // game port)
    if (!LIBMATTI_MC_Level_IsInValidBounds(level, pos))
        return false;
    if (!level->isClientSide && level->isDebug)
        return false;

    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunkAt(level, pos);
    if (chunk == NULL)
        return false;

    // Java: Level.setBlock - the old block entity is removed when the block changes
    LIBMATTI_MC_BlockState *oldState = LIBMATTI_MC_LevelChunk_SetBlockState(chunk, pos, state, flags);
    if (oldState == NULL)
        return false;

    if (LIBMATTI_MC_BlockState_GetBlock(oldState) != LIBMATTI_MC_BlockState_GetBlock(state))
    {
        LIBMATTI_MC_BlockEntity *oldEntity = LIBMATTI_MC_LevelChunk_GetBlockEntity(chunk, pos);
        if (oldEntity != NULL)
            LIBMATTI_MC_LevelChunk_RemoveBlockEntity(chunk, pos);
    }

    // Java: (flags & UPDATE_NEIGHBORS) != 0 -> updateNeighborsAt(pos, block)
    if ((flags & LIBMATTI_MC_Level_UPDATE_NEIGHBORS) != 0)
        LIBMATTI_MC_Level_UpdateNeighborsAt(level, pos, LIBMATTI_MC_BlockState_GetBlock(state));
    return true;
}

bool LIBMATTI_MC_Level_SetBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state, int flags)
{
    // Java: setBlock(pos, state, flags) = setBlock(pos, state, flags, 512)
    return LIBMATTI_MC_Level_SetBlockAndFlags(level, pos, state, flags, 512);
}

bool LIBMATTI_MC_Level_RemoveBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, int flags)
{
    // Java: removeBlock = setBlock(pos, fluidState.createLegacyBlock(), flags) - the
    // port has no fluids, so removal is air
    return LIBMATTI_MC_Level_SetBlock(level, pos, LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR()), flags);
}

bool LIBMATTI_MC_Level_DestroyBlock(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, bool drop)
{
    // Java: destroyBlock(pos, drop) - the drop side is game-port content
    (void) drop;
    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_Level_GetBlockState(level, pos);
    if (LIBMATTI_MC_BlockState_GetBlock(state) == (void *) LIBMATTI_MC_VanillaBlocks_AIR())
        return false;
    return LIBMATTI_MC_Level_RemoveBlock(level, pos, LIBMATTI_MC_Level_UPDATE_SUPPRESS_DROPS);
}

bool LIBMATTI_MC_Level_IsOutsideBuildHeight(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_LevelHeightAccessor_IsOutsideBuildHeight(&level->heightAccessor, LIBMATTI_MC_Vec3i_GetY(&pos->base));
}

static bool is_in_world_bounds_horizontal(const LIBMATTI_MC_BlockPos *pos)
{
    // Java: Math.abs(x) < getMaxHorizontal() && Math.abs(z) < getMaxHorizontal()
    int x = LIBMATTI_MC_Vec3i_GetX(&pos->base);
    int z = LIBMATTI_MC_Vec3i_GetZ(&pos->base);
    return x > -LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE && x < LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE
           && z > -LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE && z < LIBMATTI_MC_BlockPos_MAX_HORIZONTAL_COORDINATE;
}

bool LIBMATTI_MC_Level_IsInWorldBounds(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return !LIBMATTI_MC_Level_IsOutsideBuildHeight(level, pos) && is_in_world_bounds_horizontal(pos);
}

bool LIBMATTI_MC_Level_IsInValidBounds(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_Level_IsInWorldBounds(level, pos);
}

long LIBMATTI_MC_Level_GetGameTime(struct LIBMATTI_MC_Level *level)
{
    return level->gameTime;
}

void LIBMATTI_MC_Level_SetGameTime(struct LIBMATTI_MC_Level *level, long gameTime)
{
    level->gameTime = gameTime;
}

long LIBMATTI_MC_Level_GetDayTime(struct LIBMATTI_MC_Level *level)
{
    return level->dayTime;
}

void LIBMATTI_MC_Level_SetDayTime(struct LIBMATTI_MC_Level *level, long dayTime)
{
    level->dayTime = dayTime;
}

bool LIBMATTI_MC_Level_IsRaining(struct LIBMATTI_MC_Level *level)
{
    return level->raining;
}

void LIBMATTI_MC_Level_SetRaining(struct LIBMATTI_MC_Level *level, bool raining)
{
    level->raining = raining;
}

bool LIBMATTI_MC_Level_IsClientSide(struct LIBMATTI_MC_Level *level)
{
    return level->isClientSide;
}

bool LIBMATTI_MC_Level_IsDebug(struct LIBMATTI_MC_Level *level)
{
    return level->isDebug;
}

LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Level_GetDimension(struct LIBMATTI_MC_Level *level)
{
    return level->dimension;
}

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_Level_GetBlockEntity(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: getBlockEntity through the chunk
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunkAt(level, pos);
    return chunk != NULL ? LIBMATTI_MC_LevelChunk_GetBlockEntity(chunk, pos) : NULL;
}

int LIBMATTI_MC_Level_GetLightEmission(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    return LIBMATTI_MC_BlockGetter_GetLightEmission(level, pos);
}

void LIBMATTI_MC_Level_UpdateNeighborsAt(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Block *block)
{
    // Java: updateNeighborsAt - the neighbour updater is game-port content
    (void) level;
    (void) pos;
    (void) block;
}

int LIBMATTI_MC_Level_GetChunkCount(struct LIBMATTI_MC_Level *level)
{
    return level->chunkCount;
}

// ---------------------------------------------------------------------------
// Java: the entity surface (the EntityLookup/TransientEntitySectionManager pair
// flattened into the level until the P5 tick loop needs the per-section maps)
// ---------------------------------------------------------------------------

// the entity-count helper (the count lives in the struct as an int)
static int entityCount_of(struct LIBMATTI_MC_Level *level)
{
    return level->entityCount;
}

// Java: public boolean addFreshEntity(Entity) - the accepted-entity tail
bool LIBMATTI_MC_Level_AddEntity(struct LIBMATTI_MC_Level *level, struct LIBMATTI_MC_Entity *entity)
{
    if (level == NULL || entity == NULL)
        return false;
    if (entityCount_of(level) >= level->entityCapacity)
    {
        int next = level->entityCapacity > 0 ? level->entityCapacity * 2 : 16;
        struct LIBMATTI_MC_Entity **grown = realloc(level->entities, (size_t) next * sizeof(struct LIBMATTI_MC_Entity *));
        if (grown == NULL)
            return false;
        level->entities = grown;
        level->entityCapacity = next;
    }
    level->entities[level->entityCount++] = entity;
    entity->level = level; // Java: entity.setLevel + entity.setAddedToLevel
    return true;
}

// Java: the removal dispatch - drops the entity from the level list (the
// RemovalReason was set through Entity.setRemoved by the caller)
bool LIBMATTI_MC_Level_RemoveEntity(struct LIBMATTI_MC_Level *level, struct LIBMATTI_MC_Entity *entity)
{
    if (level == NULL || entity == NULL)
        return false;
    for (int i = 0; i < level->entityCount; i++)
    {
        if (level->entities[i] == entity)
        {
            memmove(&level->entities[i], &level->entities[i + 1],
                    (size_t) (level->entityCount - i - 1) * sizeof(struct LIBMATTI_MC_Entity *));
            level->entityCount--;
            return true;
        }
    }
    return false;
}

int LIBMATTI_MC_Level_GetEntityCount(struct LIBMATTI_MC_Level *level)
{
    return level != NULL ? level->entityCount : 0;
}

struct LIBMATTI_MC_Entity *LIBMATTI_MC_Level_GetEntity(struct LIBMATTI_MC_Level *level, int index)
{
    if (level == NULL || index < 0 || index >= level->entityCount)
        return NULL;
    return level->entities[index];
}

// Java: getEntities(Entity, AABB, Predicate) - the box-overlap scan
int LIBMATTI_MC_Level_GetEntitiesInBox(struct LIBMATTI_MC_Level *level, double minX, double minY, double minZ,
                                       double maxX, double maxY, double maxZ,
                                       struct LIBMATTI_MC_Entity **out, int outCapacity)
{
    if (level == NULL || out == NULL)
        return 0;
    int found = 0;
    for (int i = 0; i < level->entityCount && found < outCapacity; i++)
    {
        const LIBMATTI_MC_AABB *bb = LIBMATTI_MC_Entity_GetBoundingBox(level->entities[i]);
        if (bb == NULL)
            continue;
        // Java: AABB.intersects - the interval overlap on all three axes
        if (bb->maxX < minX || bb->minX > maxX)
            continue;
        if (bb->maxY < minY || bb->minY > maxY)
            continue;
        if (bb->maxZ < minZ || bb->minZ > maxZ)
            continue;
        out[found++] = level->entities[i];
    }
    return found;
}

void LIBMATTI_MC_Level_Free(struct LIBMATTI_MC_Level *level)
{
    for (int i = 0; i < level->chunkCount; i++)
        LIBMATTI_MC_LevelChunk_Free(level->chunks[i].chunk);
    free(level->chunks);
    free(level->entities);
    free(level);
}
