// Port of net.minecraft.world.level.Level (the in-memory part).

#include <math.h>

#include "libmatti/net/minecraft/world/entity/Entity.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/ClipContext.h"
#include "libmatti/net/minecraft/world/level/LevelReader.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include <float.h>
#include <math.h>
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

// ---------------------------------------------------------------------------
// Java: CollisionGetter - the block-collision scan. The BlockCollisions iterator
// walks every position the box overlaps and yields the collision shapes; the
// port yields full-block AABBs (the VoxelShape port collapses to cubes) for
// every state with properties.hasCollision set (air rides the same gate).
// ---------------------------------------------------------------------------
int LIBMATTI_MC_Level_GetBlockCollisions(struct LIBMATTI_MC_Level *level, double minX, double minY, double minZ,
                                         double maxX, double maxY, double maxZ,
                                         LIBMATTI_MC_AABB **out, int outCapacity)
{
    if (level == NULL || out == NULL)
        return 0;
    // Java: the walk bounds floor to the int grid (the BlockCollisions ctor
    // clamps into the world); the port keeps the plain floor sweep
    int x0 = (int) floor(minX - 1.0e-7);
    int y0 = (int) floor(minY - 1.0e-7);
    int z0 = (int) floor(minZ - 1.0e-7);
    int x1 = (int) floor(maxX + 1.0e-7);
    int y1 = (int) floor(maxY + 1.0e-7);
    int z1 = (int) floor(maxZ + 1.0e-7);
    int found = 0;
    LIBMATTI_MC_BlockPos pos = {{0, 0, 0}};
    for (int y = y0; y <= y1 && found < outCapacity; y++)
    {
        for (int z = z0; z <= z1 && found < outCapacity; z++)
        {
            for (int x = x0; x <= x1 && found < outCapacity; x++)
            {
                pos.base.x = x;
                pos.base.y = y;
                pos.base.z = z;
                LIBMATTI_MC_BlockState *state = LIBMATTI_MC_Level_GetBlockState(level, &pos);
                if (state == NULL)
                    continue;
                const LIBMATTI_MC_Block *block = (const LIBMATTI_MC_Block *) LIBMATTI_MC_BlockState_GetBlock(state);
                if (block == NULL || block->properties == NULL || !block->properties->hasCollision)
                    continue;
                // Java: the shape yields its bounding boxes; the full block is the
                // unit cube of the position
                LIBMATTI_MC_AABB *shape = LIBMATTI_MC_AABB_New(x, y, z, x + 1.0, y + 1.0, z + 1.0);
                if (shape != NULL)
                    out[found++] = shape;
            }
        }
    }
    return found;
}

// Java: CollisionGetter.noBlockCollision - no block collision shape intersects
bool LIBMATTI_MC_Level_NoBlockCollision(struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_AABB *box)
{
    if (level == NULL || box == NULL)
        return true;
    LIBMATTI_MC_AABB *hits[1];
    return LIBMATTI_MC_Level_GetBlockCollisions(level, box->minX, box->minY, box->minZ,
                                                box->maxX, box->maxY, box->maxZ, hits, 1) == 0;
}

// ---------------------------------------------------------------------------
// Java: BlockGetter.clip / traverseBlocks - the DDA ray cast. The per-cell
// lambda clips the ray against the context's block shape (the fluid shape is
// empty through the NONE mode); the traversal walks the cells the segment
// passes through with the ±1.0E-7-lerped endpoints exactly like Java (the
// first non-empty clip wins, the walk exhausts into the miss with the ray end
// as the location and the approximate nearest of the reverse ray as the face).
// ---------------------------------------------------------------------------
static int level_clip_step(LIBMATTI_MC_Level *level, const LIBMATTI_MC_ClipContext *context,
                           const LIBMATTI_MC_BlockPos *pos, const LIBMATTI_MC_Vec3 *from,
                           const LIBMATTI_MC_Vec3 *to, LIBMATTI_MC_BlockHitResult *out)
{
    LIBMATTI_MC_AABB *shape = LIBMATTI_MC_ClipContext_GetBlockShape(context, level, pos);
    if (shape == NULL)
        return 0;

    // Java: VoxelShape.clip - a start point inside the shape (after the 0.001
    // step along the ray) hits immediately with inside=true and the face riding
    // the approximate nearest of the reverse ray.
    double rayX = to->x - from->x, rayY = to->y - from->y, rayZ = to->z - from->z;
    if (rayX * rayX + rayY * rayY + rayZ * rayZ >= 1.0e-7)
    {
        double insideX = from->x + rayX * 0.001;
        double insideY = from->y + rayY * 0.001;
        double insideZ = from->z + rayZ * 0.001;
        if (insideX >= shape->minX && insideX <= shape->maxX && insideY >= shape->minY && insideY <= shape->maxY
            && insideZ >= shape->minZ && insideZ <= shape->maxZ)
        {
            free(shape);
            out->location = *from;
            out->type = LIBMATTI_MC_HitResult_BLOCK;
            out->direction = LIBMATTI_MC_Direction_GetApproximateNearest((float) -rayX, (float) -rayY, (float) -rayZ);
            out->blockPos = *pos;
            out->inside = true;
            out->worldBorder = false;
            return 1;
        }
    }

    int found = 0;
    LIBMATTI_MC_Vec3 *location = LIBMATTI_MC_AABB_Clip(shape, from, to, &found);
    int direction = found ? LIBMATTI_MC_AABB_ClipDirection(shape, from, to) : -1;
    if (location != NULL)
    {
        out->location = *location;
        free(location);
    }
    free(shape);
    if (!found || direction < 0)
        return 0;
    out->type = LIBMATTI_MC_HitResult_BLOCK;
    out->direction = (LIBMATTI_MC_Direction) direction;
    out->blockPos = *pos;
    out->inside = false;
    out->worldBorder = false;
    return 1;
}

// Java: BlockGetter.clipWithState / clipWithInteractionOverride collapsed - the
// explicit per-cell clip the traversal lambda runs.
LIBMATTI_MC_BlockHitResult LIBMATTI_MC_Level_ClipWithState(LIBMATTI_MC_Level *level, const LIBMATTI_MC_Vec3 *from,
                                                           const LIBMATTI_MC_Vec3 *to, const LIBMATTI_MC_BlockPos *pos,
                                                           const LIBMATTI_MC_BlockState *state)
{
    (void) level;
    (void) state; // the shape model rides the position's full cube
    LIBMATTI_MC_BlockHitResult out;
    memset(&out, 0, sizeof(out));
    LIBMATTI_MC_AABB *shape = LIBMATTI_MC_AABB_FromBlockPos(pos);
    int found = 0;
    LIBMATTI_MC_Vec3 *location = LIBMATTI_MC_AABB_Clip(shape, from, to, &found);
    int direction = found ? LIBMATTI_MC_AABB_ClipDirection(shape, from, to) : -1;
    if (location != NULL)
    {
        out.location = *location;
        free(location);
    }
    free(shape);
    if (found && direction >= 0)
    {
        out.type = LIBMATTI_MC_HitResult_BLOCK;
        out.direction = (LIBMATTI_MC_Direction) direction;
        out.blockPos = *pos;
        out.inside = false;
        out.worldBorder = false;
    }
    return out;
}

LIBMATTI_MC_BlockHitResult LIBMATTI_MC_Level_Clip(LIBMATTI_MC_Level *level, LIBMATTI_MC_ClipContext *context)
{
    const LIBMATTI_MC_Vec3 *from = &context->from;
    const LIBMATTI_MC_Vec3 *to = &context->to;

    // Java: the miss lambda - the location rides the ray end, the direction the
    // approximate nearest of the reverse ray, the block the containing one.
    LIBMATTI_MC_BlockPos toPos = {{LIBMATTI_MC_Mth_FloorD(to->x), LIBMATTI_MC_Mth_FloorD(to->y),
                                   LIBMATTI_MC_Mth_FloorD(to->z)}};
    LIBMATTI_MC_Vec3 reverse = {from->x - to->x, from->y - to->y, from->z - to->z};
    LIBMATTI_MC_Direction missDirection = LIBMATTI_MC_Direction_GetApproximateNearest(
        (float) reverse.x, (float) reverse.y, (float) reverse.z);

    // Java: Vec3.equals - the degenerate zero-length ray misses immediately.
    if (from->x == to->x && from->y == to->y && from->z == to->z)
        return LIBMATTI_MC_BlockHitResult_Miss(to, missDirection, &toPos);

    // Java: both endpoints lerp by -1.0E-7 toward the other so the grid floors
    // never land exactly on a boundary from the wrong side.
    double endX = LIBMATTI_MC_Mth_LerpD(-1.0E-7, to->x, from->x);
    double endY = LIBMATTI_MC_Mth_LerpD(-1.0E-7, to->y, from->y);
    double endZ = LIBMATTI_MC_Mth_LerpD(-1.0E-7, to->z, from->z);
    double startX = LIBMATTI_MC_Mth_LerpD(-1.0E-7, from->x, to->x);
    double startY = LIBMATTI_MC_Mth_LerpD(-1.0E-7, from->y, to->y);
    double startZ = LIBMATTI_MC_Mth_LerpD(-1.0E-7, from->z, to->z);

    int x = LIBMATTI_MC_Mth_FloorD(startX);
    int y = LIBMATTI_MC_Mth_FloorD(startY);
    int z = LIBMATTI_MC_Mth_FloorD(startZ);

    LIBMATTI_MC_BlockPos pos = {{x, y, z}};
    LIBMATTI_MC_BlockHitResult hit;
    memset(&hit, 0, sizeof(hit));
    if (level_clip_step(level, context, &pos, from, to, &hit))
        return hit;

    double dirX = endX - startX;
    double dirY = endY - startY;
    double dirZ = endZ - startZ;
    int stepX = LIBMATTI_MC_Mth_Sign(dirX);
    int stepY = LIBMATTI_MC_Mth_Sign(dirY);
    int stepZ = LIBMATTI_MC_Mth_Sign(dirZ);
    // Java: l == 0 ? Double.MAX_VALUE : l / d - the t per whole cell on the axis
    double tDeltaX = stepX == 0 ? DBL_MAX : (double) stepX / dirX;
    double tDeltaY = stepY == 0 ? DBL_MAX : (double) stepY / dirY;
    double tDeltaZ = stepZ == 0 ? DBL_MAX : (double) stepZ / dirZ;
    // Java: the t to the first boundary per axis from the fractional start
    double tMaxX = tDeltaX * (stepX > 0 ? 1.0 - LIBMATTI_MC_Mth_FracD(startX) : LIBMATTI_MC_Mth_FracD(startX));
    double tMaxY = tDeltaY * (stepY > 0 ? 1.0 - LIBMATTI_MC_Mth_FracD(startY) : LIBMATTI_MC_Mth_FracD(startY));
    double tMaxZ = tDeltaZ * (stepZ > 0 ? 1.0 - LIBMATTI_MC_Mth_FracD(startZ) : LIBMATTI_MC_Mth_FracD(startZ));

    while (tMaxX <= 1.0 || tMaxY <= 1.0 || tMaxZ <= 1.0)
    {
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                x += stepX;
                tMaxX += tDeltaX;
            }
            else
            {
                z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
        else if (tMaxY < tMaxZ)
        {
            y += stepY;
            tMaxY += tDeltaY;
        }
        else
        {
            z += stepZ;
            tMaxZ += tDeltaZ;
        }

        pos.base.x = x;
        pos.base.y = y;
        pos.base.z = z;
        if (level_clip_step(level, context, &pos, from, to, &hit))
            return hit;
    }

    return LIBMATTI_MC_BlockHitResult_Miss(to, missDirection, &toPos);
}

void LIBMATTI_MC_Level_Free(struct LIBMATTI_MC_Level *level)
{
    for (int i = 0; i < level->chunkCount; i++)
        LIBMATTI_MC_LevelChunk_Free(level->chunks[i].chunk);
    free(level->chunks);
    free(level->entities);
    free(level);
}
